#include "stepper.h"
#include <AccelStepper.h>
#include "driver/gpio.h"
#include "config.h" // 新增：引入引脚配置

const int DIR_PIN = ROBOT_STEPPER_DIR_PIN;
const int PUL_PIN = ROBOT_STEPPER_PUL_PIN;
const int ENA_PIN = ROBOT_STEPPER_ENA_PIN;

static AccelStepper stepper(AccelStepper::DRIVER, PUL_PIN, DIR_PIN);
static bool moving = false;
static bool continuousMode = false;
static int continuousSpeed = 0;

void stepperInit() {
  // 替换 pinMode 和 digitalWrite
  gpio_reset_pin((gpio_num_t)ENA_PIN);
  gpio_set_direction((gpio_num_t)ENA_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level((gpio_num_t)ENA_PIN, 1); // 默认 HIGH

  stepper.setMaxSpeed(5000);
  stepper.setAcceleration(5000);
}

void stepperStartMove(int steps, int speed) {
  continuousMode = false;
  continuousSpeed = 0;

  gpio_set_level((gpio_num_t)ENA_PIN, 0); // LOW
  vTaskDelay(pdMS_TO_TICKS(5)); // 替换 delay(5) 为 FreeRTOS 兼容延时

  stepper.setMaxSpeed(speed);
  stepper.move(steps);
  moving = true;
}

void stepperStartContinuous(int speed) {
  continuousMode = true;
  continuousSpeed = speed;

  gpio_set_level((gpio_num_t)ENA_PIN, 0); // LOW
  vTaskDelay(pdMS_TO_TICKS(5));

  stepper.setSpeed(speed);
  moving = true;
}

void stepperStop() {
  continuousMode = false;
  continuousSpeed = 0;
  moving = false;

  stepper.stop();
  gpio_set_level((gpio_num_t)ENA_PIN, 1); // HIGH
}

void stepperSetSpeed(int speed) {
  if (!continuousMode || !moving) return;
  continuousSpeed = speed;
  stepper.setSpeed(speed);
}

void stepperRun() {
  if (!moving) return;

  if (continuousMode) {
    stepper.runSpeed();
  } else {
    stepper.run();
    if (stepper.distanceToGo() == 0) {
      stepperStop();
    }
  }
}

bool stepperIsDone() {
  return !moving;
}