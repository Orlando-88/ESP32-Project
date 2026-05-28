#include "relay.h"
#include "driver/gpio.h"  // ESP-IDF GPIO 驱动
#include "esp_log.h"      // ESP-IDF 日志打印

static const char* TAG = "Relay1"; // 定义日志标签
static int relayPin = -1;
static bool relayState = false;

void relayInit(int pin) {
  relayPin = pin;
  if (relayPin >= 0) { // 加一层安全保护
      gpio_reset_pin((gpio_num_t)relayPin); // 恢复引脚默认状态
      gpio_set_direction((gpio_num_t)relayPin, GPIO_MODE_OUTPUT); // pinMode OUTPUT
      gpio_set_level((gpio_num_t)relayPin, 0); // 默认低电平
  }
  relayState = false;
}

void relayToggle() {
  if (relayPin < 0) return;
  relayState = !relayState;
  gpio_set_level((gpio_num_t)relayPin, relayState ? 1 : 0);
  
  // 替换 Serial.print 为 ESP_LOGI
  ESP_LOGI(TAG, "Relay: %s", relayState ? "ON" : "OFF");
}

void relaySet(bool on) {
  if (relayPin < 0) return;
  relayState = on;
  gpio_set_level((gpio_num_t)relayPin, relayState ? 1 : 0);
  
  ESP_LOGI(TAG, "Relay: %s", relayState ? "ON" : "OFF");
}

bool relayGetState() {
  return relayState;
}

void relayProcessKey(char key) {
  switch (key) {
    case 'g':
    case 'G':
      relayToggle();
      break;
  }
}