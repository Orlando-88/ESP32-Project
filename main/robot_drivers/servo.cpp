#include "servo.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char* TAG = "SERVO";
static int servoPin = -1;
static ledc_channel_t servoChannel = LEDC_CHANNEL_2; // 专属通道2

static void setPulse(int duty) {
  if (servoPin < 0) return;
  ledc_set_duty(LEDC_LOW_SPEED_MODE, servoChannel, duty);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, servoChannel);
}

void servoInit(int pin) {
  servoPin = pin;
  
  // 共享 Timer 0，只注册通道 2
  ledc_channel_config_t ledc_channel = {
      .gpio_num       = servoPin,
      .speed_mode     = LEDC_LOW_SPEED_MODE,
      .channel        = servoChannel,
      .intr_type      = LEDC_INTR_DISABLE,
      .timer_sel      = LEDC_TIMER_0,
      .duty           = 0,
      .hpoint         = 0
  };
  ledc_channel_config(&ledc_channel);
}

void servoWrite(int angle) {
  if (angle < 0) angle = 0;
  if (angle > 180) angle = 180;

  float pulseUs = 500 + (angle / 180.0) * 2000;
  int duty = (pulseUs / 20000.0) * 4095;

  setPulse(duty);
  ESP_LOGI(TAG, "Servo (GPIO%d): %d deg", servoPin, angle);
}

void servoProcessKey(char key) {
  switch (key) {
    case 'z': case 'Z': servoWrite(0); break;
    case 'x': case 'X': servoWrite(93); break;
    case 'c': case 'C': servoWrite(180); break;
  }
}