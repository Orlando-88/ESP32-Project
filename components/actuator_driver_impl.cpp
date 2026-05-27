#include "actuator_driver.h"
#include "actuator_config.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include <string.h>

static const char *TAG = "ACTUATOR";

// 唯一的全局收件箱 (Message Queue)
static QueueHandle_t actuator_queue = NULL;

// ==============================================================
// 第一部分：硬件底层驱动 (ESP-IDF Native)
// ==============================================================

static void hw_init_servo(int gpio_num, ledc_channel_t channel) {
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = SERVO_LEDC_MODE,
        .duty_resolution  = SERVO_LEDC_DUTY_RES,
        .timer_num        = SERVO_LEDC_TIMER,
        .freq_hz          = SERVO_LEDC_FREQ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .gpio_num       = gpio_num,
        .speed_mode     = SERVO_LEDC_MODE,
        .channel        = channel,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = SERVO_LEDC_TIMER,
        .duty           = 0, 
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);
}

static void hw_init_gpio(int gpio_num, gpio_mode_t mode) {
    gpio_reset_pin((gpio_num_t)gpio_num);
    gpio_set_direction((gpio_num_t)gpio_num, mode);
    if (mode == GPIO_MODE_OUTPUT) {
        gpio_set_level((gpio_num_t)gpio_num, 0); // 默认低电平
    }
}

// 角度转占空比
static uint32_t servo_angle_to_duty(int angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    return SERVO_DUTY_MIN + (SERVO_DUTY_MAX - SERVO_DUTY_MIN) * angle / 180;
}

// ==============================================================
// 第二部分：树莓派串口通信解析 (提取旧代码协议)
// ==============================================================

// 从 protocol.cpp 移植过来的异或校验核心算法
static char calculate_checksum(const char* str, int len) {
    char checksum = 0;
    for(int i = 0; i < len; i++) {
        checksum ^= str[i];
    }
    return checksum;
}

// 串口监听任务 (树莓派 -> ESP32)
static void uart_listener_task(void *arg) {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk= UART_SCLK_DEFAULT,
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, PIN_UART_PI_TX, PIN_UART_PI_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_1, 256 * 2, 0, 0, NULL, 0);

    uint8_t rx_buffer[128];
    while (1) {
        // 等待数据，不占用 CPU
        int len = uart_read_bytes(UART_NUM_1, rx_buffer, sizeof(rx_buffer) - 1, pdMS_TO_TICKS(100));
        if (len > 0) {
            rx_buffer[len] = '\0';
            // TODO: 这里将来接入具体的 <指令*校验和> 解析逻辑
            // 解析成功后调用 actuator_send_command() 把指令扔给执行任务
            ESP_LOGI(TAG, "UART RX: %s", rx_buffer);
        }
    }
}

// ==============================================================
// 第三部分：核心执行线程 (单一消费者，永不阻塞音频)
// ==============================================================

static void actuator_executor_task(void *arg) {
    actuator_cmd_t cmd;
    
    while (1) {
        // 在这里睡觉，直到收件箱里有信件 (portMAX_DELAY表示无限等待)
        if (xQueueReceive(actuator_queue, &cmd, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "Executing CMD: Type=%d, Target=%d, Val=%d", cmd.type, cmd.target_id, cmd.value);
            
            switch (cmd.type) {
                case CMD_TYPE_SERVO: {
                    // 动态选择 PWM 通道
                    ledc_channel_t ch = (cmd.target_id == 1) ? LEDC_CHANNEL_0 : LEDC_CHANNEL_1;
                    ledc_set_duty(SERVO_LEDC_MODE, ch, servo_angle_to_duty(cmd.value));
                    ledc_update_duty(SERVO_LEDC_MODE, ch);
                    break;
                }
                
                case CMD_TYPE_RELAY: {
                    int pin = (cmd.target_id == 1) ? PIN_RELAY_1 : PIN_RELAY_2;
                    gpio_set_level((gpio_num_t)pin, cmd.value > 0 ? 1 : 0);
                    break;
                }
                
                case CMD_TYPE_STEPPER: {
                    // 步进电机逻辑：设置方向
                    gpio_set_level((gpio_num_t)PIN_STEPPER_DIR, cmd.value >= 0 ? 1 : 0);
                    int steps = abs(cmd.value);
                    if (steps > STEPPER_STEPS_MAX) steps = STEPPER_STEPS_MAX;
                    
                    // 防阻塞执行循环
                    for (int i = 0; i < steps; i++) {
                        gpio_set_level((gpio_num_t)PIN_STEPPER_STEP, 1);
                        esp_rom_delay_us(STEPPER_SPEED_US); 
                        gpio_set_level((gpio_num_t)PIN_STEPPER_STEP, 0);
                        esp_rom_delay_us(STEPPER_SPEED_US);
                        
                        // 【核心避坑】每转100步，主动让出 CPU 1毫秒给语音识别任务！防止音频卡顿/看门狗复位
                        if (i % 100 == 0) {
                            vTaskDelay(pdMS_TO_TICKS(1));
                        }
                    }
                    break;
                }
            }
        }
    }
}

// ==============================================================
// 外部开放 API
// ==============================================================

bool actuator_send_command(actuator_cmd_type_t type, int target_id, int value) {
    if (!actuator_queue) return false;
    
    actuator_cmd_t cmd = {
        .type = type,
        .target_id = target_id,
        .value = value,
        .speed_delay = 0
    };
    
    // 把信件塞进队列，如果满了就不等了（0延时）
    return (xQueueSend(actuator_queue, &cmd, 0) == pdTRUE);
}

void actuator_subsystem_init(void) {
    ESP_LOGI(TAG, "Initializing Actuator Subsystem...");

    // 1. 初始化硬件引脚
    hw_init_servo(PIN_SERVO_1, LEDC_CHANNEL_0);
    hw_init_servo(PIN_SERVO_2, LEDC_CHANNEL_1);
    
    hw_init_gpio(PIN_RELAY_1, GPIO_MODE_OUTPUT);
    hw_init_gpio(PIN_RELAY_2, GPIO_MODE_OUTPUT);
    
    hw_init_gpio(PIN_STEPPER_DIR, GPIO_MODE_OUTPUT);
    hw_init_gpio(PIN_STEPPER_STEP, GPIO_MODE_OUTPUT);

    // 2. 创建收件箱 Queue
    actuator_queue = xQueueCreate(ACTUATOR_QUEUE_SIZE, sizeof(actuator_cmd_t));
    if (actuator_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create queue!");
        return;
    }

    // 3. 创建执行任务 (Core 1, 优先级给个中间值，不抢音频(Core 0/优先级高)的风头)
    xTaskCreatePinnedToCore(actuator_executor_task, "ActuatorTask", ACTUATOR_TASK_STACK, NULL, 5, NULL, 1);

    // 4. 创建串口监听任务
    xTaskCreatePinnedToCore(uart_listener_task, "UartTask", UART_TASK_STACK, NULL, 4, NULL, 1);
    
    ESP_LOGI(TAG, "Actuator Subsystem Initialized Successfully!");
}