#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

#define AUDIO_INPUT_SAMPLE_RATE  16000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000

#define AUDIO_I2S_METHOD_SIMPLEX

#ifdef AUDIO_I2S_METHOD_SIMPLEX
#define AUDIO_I2S_MIC_GPIO_WS   GPIO_NUM_4
#define AUDIO_I2S_MIC_GPIO_SCK  GPIO_NUM_5
#define AUDIO_I2S_MIC_GPIO_DIN  GPIO_NUM_6
#define AUDIO_I2S_SPK_GPIO_DOUT GPIO_NUM_7
#define AUDIO_I2S_SPK_GPIO_BCLK GPIO_NUM_15
#define AUDIO_I2S_SPK_GPIO_LRCK GPIO_NUM_16
#else
#define AUDIO_I2S_GPIO_WS GPIO_NUM_4
#define AUDIO_I2S_GPIO_BCLK GPIO_NUM_5
#define AUDIO_I2S_GPIO_DIN  GPIO_NUM_6
#define AUDIO_I2S_GPIO_DOUT GPIO_NUM_7
#endif

#define BUILTIN_LED_GPIO        GPIO_NUM_48
#define BOOT_BUTTON_GPIO        GPIO_NUM_0
#define TOUCH_BUTTON_GPIO       GPIO_NUM_47
#define VOLUME_UP_BUTTON_GPIO   GPIO_NUM_40
#define VOLUME_DOWN_BUTTON_GPIO GPIO_NUM_39

#define DISPLAY_SDA_PIN GPIO_NUM_41
#define DISPLAY_SCL_PIN GPIO_NUM_42
#define DISPLAY_WIDTH   128

#if CONFIG_OLED_SSD1306_128X32
#define DISPLAY_HEIGHT  32
#elif CONFIG_OLED_SSD1306_128X64
#define DISPLAY_HEIGHT  64
#elif CONFIG_OLED_SH1106_128X64
#define DISPLAY_HEIGHT  64
#define SH1106
#else
#error "未选择 OLED 屏幕类型"
#endif

#define DISPLAY_MIRROR_X true
#define DISPLAY_MIRROR_Y true

// ======== 🤖 机器人外设全新安全引脚分配 ========
// 完美避开音频、屏幕、按键，请后续根据此表连接硬件
#define ROBOT_SG90_PIN          GPIO_NUM_18  // 原小智预留舵机引脚
#define ROBOT_SG90_180_PIN      GPIO_NUM_8   // 空闲可用
#define ROBOT_SERVO_PIN         GPIO_NUM_9   // 空闲可用
#define ROBOT_RELAY1_PIN        GPIO_NUM_13  // 吸盘继电器
#define ROBOT_RELAY2_PIN        GPIO_NUM_14  // 鼓风机继电器

// 步进电机专用宏定义（无需带 GPIO_NUM_ 前缀，供 AccelStepper 使用）
#define ROBOT_STEPPER_DIR_PIN   10
#define ROBOT_STEPPER_PUL_PIN   11
#define ROBOT_STEPPER_ENA_PIN   12
// ===============================================

#define BREAD_COMPACT_WIFI_WITH_SERVO_VERSION "1.0.0"

#endif // _BOARD_CONFIG_H_