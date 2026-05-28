#include "sg90.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char* TAG = "SG90";
static int sgPin = -1;
static ledc_channel_t sgChannel = LEDC_CHANNEL_0; // 专属通道0

static void setPulse(int duty) {
  if (sgPin < 0) return;
  ledc_set_duty(LEDC_LOW_SPEED_MODE, sgChannel, duty);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, sgChannel);
}

void sg90Init(int pin) {
  sgPin = pin;
  
  // 1. 初始化定时器 (50Hz, 12-bit)
  ledc_timer_config_t ledc_timer = {
      .speed_mode       = LEDC_LOW_SPEED_MODE,
      .duty_resolution  = LEDC_TIMER_12_BIT,
      .timer_num        = LEDC_TIMER_0,
      .freq_hz          = 50,
      .clk_cfg          = LEDC_AUTO_CLK
  };
  ledc_timer_config(&ledc_timer);

  // 2. 初始化通道
  ledc_channel_config_t ledc_channel = {
      .gpio_num       = sgPin,
      .speed_mode     = LEDC_LOW_SPEED_MODE,
      .channel        = sgChannel,
      .intr_type      = LEDC_INTR_DISABLE,
      .timer_sel      = LEDC_TIMER_0,
      .duty           = 0,
      .hpoint         = 0
  };
  ledc_channel_config(&ledc_channel);
}

void sg90Write(int angle) {
  if (angle < 0) angle = 0;
  if (angle > 180) angle = 180;

  // 严格保留队友的数学逻辑
  float pulseUs = 500 + (angle / 180.0) * 2000;
  int duty = (pulseUs / 20000.0) * 4095;

  setPulse(duty);
  ESP_LOGI(TAG, "Servo (GPIO%d): %d deg", sgPin, angle);
}

void sg90Stop() {
  sg90Write(90);
}

void sg90ProcessKey(char key) {
  switch (key) {
    case 't': case 'T': sg90Write(30); break;
    case 'y': case 'Y': sg90Write(90); break;
    case 'u': case 'U': sg90Write(150); break;
  }
}