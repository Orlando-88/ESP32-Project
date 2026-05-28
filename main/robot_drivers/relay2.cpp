#include "relay2.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char* TAG = "Relay2";
static int relay2Pin = -1;
static bool relay2State = false;

void relay2Init(int pin) {
  relay2Pin = pin;
  if (relay2Pin >= 0) {
      gpio_reset_pin((gpio_num_t)relay2Pin);
      gpio_set_direction((gpio_num_t)relay2Pin, GPIO_MODE_OUTPUT);
      gpio_set_level((gpio_num_t)relay2Pin, 0);
  }
  relay2State = false;
}

void relay2Toggle() {
  if (relay2Pin < 0) return;
  relay2State = !relay2State;
  gpio_set_level((gpio_num_t)relay2Pin, relay2State ? 1 : 0);
  
  ESP_LOGI(TAG, "Relay2: %s", relay2State ? "ON" : "OFF");
}

void relay2Set(bool on) {
  if (relay2Pin < 0) return;
  relay2State = on;
  gpio_set_level((gpio_num_t)relay2Pin, relay2State ? 1 : 0);
  
  ESP_LOGI(TAG, "Relay2: %s", relay2State ? "ON" : "OFF");
}

bool relay2GetState() {
  return relay2State;
}

void relay2ProcessKey(char key) {
  switch (key) {
    case 'h':
    case 'H':
      relay2Toggle();
      break;
  }
}