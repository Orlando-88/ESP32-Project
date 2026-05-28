#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_rom_sys.h" 

static const char *TAG = "STEPPER_SCREW";

// ================= 引脚定义 =================
#define MOTOR_PUL_PIN GPIO_NUM_4   
#define MOTOR_DIR_PIN GPIO_NUM_5   
#define MOTOR_ENA_PIN GPIO_NUM_6   

// ================= 电机与丝杆参数 =================
#define PULSES_PER_REV 1600        // 8细分，转一圈需要的脉冲数

// 【极限稳妥转速修改处】：修改为 200 微秒
// 这个参数确保了电机在12V、双电机并联、无加速算法情况下的最快安全瞬启速度。
#define PULSE_DELAY_US 200       

// ================= 初始化函数 =================
void motor_gpio_init(void)
{
    ESP_LOGI(TAG, "正在初始化丝杆控制引脚...");
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << MOTOR_PUL_PIN) | (1ULL << MOTOR_DIR_PIN) | (1ULL << MOTOR_ENA_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    gpio_set_level(MOTOR_PUL_PIN, 0); 
    gpio_set_level(MOTOR_ENA_PIN, 0); // 默认使能(锁死)
    ESP_LOGI(TAG, "丝杆初始化完成，已准备就绪。");
}

// ================= 步进电机转动函数 =================
void move_stepper(int dir, float revolutions)
{
    gpio_set_level(MOTOR_DIR_PIN, dir);
    
    // 为了更直观，增加上下移动的日志 (假设 dir=1 为上，0 为下，具体视你的丝杆螺纹方向而定)
    if(dir == 1) {
        ESP_LOGI(TAG, "丝杆正在 [上升] %.1f 圈...", revolutions);
    } else {
        ESP_LOGI(TAG, "丝杆正在 [下降] %.1f 圈...", revolutions);
    }

    int total_pulses = (int)(PULSES_PER_REV * revolutions);

    for (int i = 0; i < total_pulses; i++)
    {
        gpio_set_level(MOTOR_PUL_PIN, 1);
        esp_rom_delay_us(PULSE_DELAY_US);
        
        gpio_set_level(MOTOR_PUL_PIN, 0);
        esp_rom_delay_us(PULSE_DELAY_US);
    }
    ESP_LOGI(TAG, "--> 移动完成！");
}

// ================= 主函数 =================
void app_main(void)
{
    motor_gpio_init();
    vTaskDelay(pdMS_TO_TICKS(1000)); // 开机缓冲1秒

    while (1)
    {
        // 1. 向上转动 4 圈
        move_stepper(1, 4.0);
        
        // 停顿 1.5 秒
        vTaskDelay(pdMS_TO_TICKS(1500));

        // 2. 向下转动 4 圈
        move_stepper(0, 4.0);
        
        // 停顿 1.5 秒
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}