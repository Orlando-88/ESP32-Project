#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// 指令类型定义
typedef enum {
    CMD_TYPE_SERVO,     // 控制舵机
    CMD_TYPE_STEPPER,   // 控制步进电机
    CMD_TYPE_RELAY      // 控制继电器
} actuator_cmd_type_t;

// 通用控制指令包 (扔进收件箱的“信件”)
typedef struct {
    actuator_cmd_type_t type; 
    int target_id;      // 目标ID (比如 1号舵机, 2号继电器)
    int value;          // 控制值 (比如 舵机角度0-180, 步进电机步数, 继电器0/1)
    int speed_delay;    // 附加参数：速度或延迟 (主要用于步进电机)
} actuator_cmd_t;

/**
 * @brief 初始化整个控制子系统（硬件初始化、创建Queue、启动后台任务）
 * @note 请在 Board::Initialize() 中调用
 */
void actuator_subsystem_init(void);

/**
 * @brief 发送控制指令到收件箱 (非阻塞)
 * @note 语音意图解析成功后调用此函数
 */
bool actuator_send_command(actuator_cmd_type_t type, int target_id, int value);

#ifdef __cplusplus
}
#endif