// 放在 D:\AutoRubbishRobot\Orlando\main\main.cpp
// 这是你机器人的大脑总控室，程序的起点！

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "actuator_driver.h" // 引入你的动作控制模块
#include "esp_log.h"

static const char *TAG = "ORLANDO_MAIN";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "====================================");
    ESP_LOGI(TAG, "   Orlando Auto Rubbish Robot       ");
    ESP_LOGI(TAG, "   System Booting...                ");
    ESP_LOGI(TAG, "====================================");

    // 1. 初始化你的执行器子系统 (舵机、步进电机、继电器、串口监听)
    actuator_subsystem_init();

    ESP_LOGI(TAG, "Actuator subsystem initialized. Waiting for commands...");

    // 2. 主线程循环 (这里你可以做一些全局的心跳监控，或者啥也不干)
    while (1) {
        // 举个例子：开机5秒后，我们可以模拟一次开盖测试
        vTaskDelay(pdMS_TO_TICKS(5000));
        
        // 测试发信给收件箱：让1号舵机转到90度
        ESP_LOGI(TAG, "Testing: Sending command to open rubbish bin cover (Servo 1 to 90 deg)");
        actuator_send_command(CMD_TYPE_SERVO, 1, 90);

        vTaskDelay(pdMS_TO_TICKS(2000));

        // 测试发信给收件箱：让1号舵机转回0度
        ESP_LOGI(TAG, "Testing: Sending command to close rubbish bin cover (Servo 1 to 0 deg)");
        actuator_send_command(CMD_TYPE_SERVO, 1, 0);
        
        // 然后无限休眠，把 CPU 交给后台的 ActuatorTask 和 UartTask 去干活
        vTaskSuspend(NULL); 
    }
}