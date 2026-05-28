#pragma once

// ==========================================
// 硬件引脚定义 (请根据你的实际连线修改)
// ==========================================
#define PIN_SERVO_1        4   // 舵机1
#define PIN_SERVO_2        5   // 舵机2
#define PIN_STEPPER_DIR    6   // 步进电机方向
#define PIN_STEPPER_STEP   7   // 步进电机脉冲
#define PIN_RELAY_1        15  // 继电器1
#define PIN_RELAY_2        16  // 继电器2

#define PIN_UART_PI_TX     17  // 接树莓派的 RX
#define PIN_UART_PI_RX     18  // 接树莓派的 TX

// ==========================================
// 运动极限与物理参数 (提取自旧代码)
// ==========================================
// 步进电机参数
#define STEPPER_STEPS_MAX  60000 // 最大行程步数
#define STEPPER_SPEED_US   150   // 步进脉冲间隔(微秒)，旧代码的SPEED=3000大概对应这个延迟

// 舵机参数 (SG90 50Hz, 占空比 2.5% ~ 12.5% 对应 0~180度)
#define SERVO_LEDC_TIMER       LEDC_TIMER_0
#define SERVO_LEDC_MODE        LEDC_LOW_SPEED_MODE
#define SERVO_LEDC_DUTY_RES    LEDC_TIMER_13_BIT // 13位分辨率 (8192)
#define SERVO_LEDC_FREQ        50                // 50Hz 

// 13位分辨率下，50Hz周期的占空比计算：
// 0度   (0.5ms) = 2.5% * 8192 = 204
// 180度 (2.5ms) = 12.5% * 8192 = 1024
#define SERVO_DUTY_MIN         204
#define SERVO_DUTY_MAX         1024

// ==========================================
// 队列与任务配置
// ==========================================
#define ACTUATOR_QUEUE_SIZE    10    // 收件箱容量
#define ACTUATOR_TASK_STACK    4096  // 执行任务栈大小
#define UART_TASK_STACK        4096  // 串口监听任务栈大小