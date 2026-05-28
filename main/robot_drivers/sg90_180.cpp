#include "sg90_180.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char* TAG = "SG90_180";
static int sgPin180 = -1;
static ledc_channel_t sgChannel180 = LEDC_CHANNEL_1; // 专属通道1

static void setPulse(int duty) {
  if (sgPin180 < 0) return;
  ledc_set_duty(LEDC_LOW_SPEED_MODE, sgChannel180, duty);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, sgChannel180);
}

void sg90_180Init(int pin) {
  sgPin180 = pin;
  
  // 共享 Timer 0，只注册通道 1
  ledc_channel_config_t ledc_channel = {
      .gpio_num       = sgPin180,
      .speed_mode     = LEDC_LOW_SPEED_MODE,
      .channel        = sgChannel180,
      .intr_type      = LEDC_INTR_DISABLE,
      .timer_sel      = LEDC_TIMER_0,
      .duty           = 0,
      .hpoint         = 0
  };
  ledc_channel_config(&ledc_channel);
}

void sg90_180Write(int angle) {
  if (angle < 0) angle = 0;
  if (angle > 180) angle = 180;

  float pulseUs = 500 + (angle / 180.0) * 2000;
  int duty = (pulseUs / 20000.0) * 4095;

  setPulse(duty);
  ESP_LOGI(TAG, "Servo (GPIO%d): %d deg", sgPin180, angle);
}

void sg90_180ProcessKey(char key) {
  switch (key) {
    case 'q': case 'Q': sg90_180Write(0); break;
    case 'r': case 'R': sg90_180Write(90); break;
    case 'f': case 'F': sg90_180Write(180); break;
  }
}