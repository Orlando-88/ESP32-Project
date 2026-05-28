#include "protocol.h"
#include "servo.h"
#include "sg90.h"
#include "sg90_180.h"
#include "stepper.h"
#include "relay.h"
#include "relay2.h"

// 引入 ESP-IDF 及 FreeRTOS 原生头文件
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

static const char* TAG = "PROTOCOL";

const char MSG_START = '<';
const char MSG_END = '>';
const char CHECKSUM_SEP = '*';
const int BUFFER_SIZE = 128;

static char receiveBuffer[BUFFER_SIZE];
static int bufferIndex = 0;
static bool inMessage = false;

// 【核心魔改】为了兼顾高频步进电机驱动(AccelStepper)以及不阻塞小智系统的看门狗(WDT)
// 专为模式1和模式2里写死的 while(!stepperIsDone()) 设计
#define WAIT_STEPPER() do { \
    int yc = 0; \
    while (!stepperIsDone()) { \
        stepperRun(); \
        if (++yc % 200 == 0) vTaskDelay(pdMS_TO_TICKS(1)); /* 每200步让出1毫秒的CPU */ \
    } \
} while (0)


uint8_t calculateChecksum(const char* str) {
  uint8_t checksum = 0;
  while (*str) {
    checksum ^= *str++;
  }
  return checksum;
}

void processStepperKey(char c) {
  const int STEPPER_SPEED = 3000;
  switch (c) {
    case 'a': case 'A':
      stepperStartContinuous(STEPPER_SPEED);
      ESP_LOGI(TAG, "Stepper: Forward (CW)");
      break;
    case 'd': case 'D':
      stepperStartContinuous(-STEPPER_SPEED);
      ESP_LOGI(TAG, "Stepper: Reverse (CCW)");
      break;
    case 's': case 'S':
      stepperStop();
      ESP_LOGI(TAG, "Stepper: Stopped");
      break;
    case 'w': case 'W':
      stepperStartContinuous(STEPPER_SPEED * 2);
      ESP_LOGI(TAG, "Stepper: Fast Forward");
      break;
    case 'e': case 'E':
      stepperStartContinuous(-STEPPER_SPEED * 2);
      ESP_LOGI(TAG, "Stepper: Fast Reverse");
      break;
  }
}

void processAsKey(char c) {
  servoProcessKey(c);
  sg90ProcessKey(c);
  sg90_180ProcessKey(c);
  processStepperKey(c);
  relayProcessKey(c);
  relay2ProcessKey(c);
  protocolProcessKey(c);
}

void processCommand(char* msg) {
  char* checksumPtr = strchr(msg, CHECKSUM_SEP);
  if (checksumPtr == NULL) {
    ESP_LOGE(TAG, "ERROR: No checksum separator");
    return;
  }

  *checksumPtr = '\0';
  char command[BUFFER_SIZE];
  strcpy(command, msg);

  char receivedChecksumStr[3];
  strncpy(receivedChecksumStr, checksumPtr + 1, 2);
  receivedChecksumStr[2] = '\0';

  uint8_t expectedChecksum = calculateChecksum(command);
  uint8_t receivedChecksum = (uint8_t)strtol(receivedChecksumStr, NULL, 16);

  if (expectedChecksum != receivedChecksum) {
    ESP_LOGE(TAG, "ERROR: Checksum mismatch (expected: %02X, got: %s)",
                  expectedChecksum, receivedChecksumStr);
    return;
  }

  ESP_LOGI(TAG, "OK: Command received: %s", command);
  executeCommand(command);
}

// ---------------------------------------------------------
// 🚀 独立通讯守护进程：代替原有的 Arduino loop()
// 它会默默监听树莓派数据线发来的信息，并保持步进电机的运转脉冲
// ---------------------------------------------------------
void protocolTask(void *pvParameters) {
    ESP_LOGI(TAG, "Protocol Task Started, listening on Console / Data Cable...");

    // 将标准输入设置为非阻塞模式 (防止卡死任务)
    int fd = fileno(stdin);
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    int yield_counter = 0;

    while (1) {
        // 【关键】保证连续模式下步进电机一直有脉冲输出
        stepperRun();

        // 尝试从数据线读取上位机指令
        char incoming;
        int res = read(fd, &incoming, 1);

        if (res > 0) {
            // 解析协议
            if (incoming == MSG_START) {
              bufferIndex = 0;
              inMessage = true;
              continue;
            }

            if (inMessage) {
              if (incoming == '\n' || incoming == '\r' || incoming == MSG_END) {
                receiveBuffer[bufferIndex] = '\0';
                if (bufferIndex > 0) {
                  processCommand(receiveBuffer);
                }
                inMessage = false;
                continue;
              }

              if (bufferIndex < BUFFER_SIZE - 1) {
                receiveBuffer[bufferIndex++] = incoming;
              }
            } else {
              processAsKey(incoming);
            }
        }

        // 定期释放 CPU 防止系统崩溃，同时保证步进电机达到 6000Hz 刷新率
        if (++yield_counter >= 200) {
            yield_counter = 0;
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}

// 初始化时，不再只是清空变量，而是创建一个独立后台线程
void protocolInit() {
  bufferIndex = 0;
  inMessage = false;
  
  // 创建常驻通讯与电机刷新任务，优先级与常见组件对齐，栈设为4K
  xTaskCreate(protocolTask, "protocol_task", 4096, NULL, 5, NULL);
}

void protocolProcessKey(char key) {
  switch (key) {
    case '1': mode1Start(); break;
    case '2': mode2Start(); break;
    case '3': mode3Start(); break;
    case '4': mode4Start(); break;
    case '5': mode5Start(); break;
    case '0': modeStop(); break;
  }
}

void executeCommand(const char* command) {
  if (strcmp(command, "MODE1:START") == 0) {
    mode1Start();
  } else if (strcmp(command, "MODE2:START") == 0) {
    mode2Start();
  } else if (strcmp(command, "MODE3:START") == 0) {
    mode3Start();
  } else if (strcmp(command, "MODE4:START") == 0) {
    mode4Start();
  } else if (strcmp(command, "MODE5:START") == 0) {
    mode5Start();
  } else if (strcmp(command, "MODE:STOP") == 0) {
    modeStop();

  // Fine control commands
  } else if (strcmp(command, "ROD:ROTATE") == 0) {
    sg90Write(30);
    ESP_LOGI(TAG, "ROD:ROTATE: 旋转吸杆 (30 deg)");
  } else if (strcmp(command, "ROD:RETURN") == 0) {
    sg90Write(90);
    ESP_LOGI(TAG, "ROD:RETURN: 旋回吸杆 (90 deg)");
  } else if (strcmp(command, "ROD:DROP") == 0) {
    sg90Write(150);
    ESP_LOGI(TAG, "ROD:DROP: 吸杆下降 (150 deg)");
  } else if (strcmp(command, "MOVE:MERGE") == 0) {
    sg90_180Write(0);
    ESP_LOGI(TAG, "MOVE:MERGE: 移动合并吸杆 (0 deg)");
  } else if (strcmp(command, "MOVE:STOP") == 0) {
    sg90_180Write(90);
    ESP_LOGI(TAG, "MOVE:STOP: 停止移动吸杆 (90 deg)");
  } else if (strcmp(command, "MOVE:SEPARATE") == 0) {
    sg90_180Write(180);
    ESP_LOGI(TAG, "MOVE:SEPARATE: 移动拉开吸杆 (180 deg)");
  } else if (strcmp(command, "CLAMP:OPEN") == 0) {
    servoWrite(0);
    ESP_LOGI(TAG, "CLAMP:OPEN: 打开夹子 (0 deg)");
  } else if (strcmp(command, "CLAMP:CLOSE") == 0) {
    servoWrite(93);
    ESP_LOGI(TAG, "CLAMP:CLOSE: 关闭夹子 (93 deg)");
  } else if (strcmp(command, "PLATFORM:UP") == 0) {
    stepperStartContinuous(6000);
    ESP_LOGI(TAG, "PLATFORM:UP: 上移平台");
  } else if (strcmp(command, "PLATFORM:DOWN") == 0) {
    stepperStartContinuous(-6000);
    ESP_LOGI(TAG, "PLATFORM:DOWN: 下降平台");
  } else if (strcmp(command, "PLATFORM:STOP") == 0) {
    stepperStop();
    ESP_LOGI(TAG, "PLATFORM:STOP: 停止平台");
  } else if (strcmp(command, "SUCTION:ON") == 0) {
    relaySet(true);
    ESP_LOGI(TAG, "SUCTION:ON: 打开吸盘");
  } else if (strcmp(command, "SUCTION:OFF") == 0) {
    relaySet(false);
    ESP_LOGI(TAG, "SUCTION:OFF: 关闭吸盘");
  } else if (strcmp(command, "BLOWER:ON") == 0) {
    relay2Set(true);
    ESP_LOGI(TAG, "BLOWER:ON: 打开鼓风机");
  } else if (strcmp(command, "BLOWER:OFF") == 0) {
    relay2Set(false);
    ESP_LOGI(TAG, "BLOWER:OFF: 关闭鼓风机");
  } else {
    ESP_LOGW(TAG, "Unknown command: %s", command);
  }
}

// ======== 以下完美保留队友的时序与时间延迟逻辑 ========

void mode1Start() {
  const int STEPS = 60000;
  const int SPEED = 3000;

  ESP_LOGI(TAG, "Executing Mode 1 - Basic operation");

  servoWrite(0);
  vTaskDelay(pdMS_TO_TICKS(1000));

  stepperStartMove(STEPS, SPEED);
  WAIT_STEPPER();
  ESP_LOGI(TAG, "Mode1: Stepper forward done");

  servoWrite(90);
  vTaskDelay(pdMS_TO_TICKS(1000));

  stepperStartMove(-STEPS, SPEED);
  WAIT_STEPPER();
  ESP_LOGI(TAG, "Mode1: Stepper reverse done, sequence complete");
}

void mode2Start() {
  ESP_LOGI(TAG, "Executing Mode 2 - Advanced operation");

  sg90_180Write(90);
  vTaskDelay(pdMS_TO_TICKS(200));
  sg90Write(0);
  vTaskDelay(pdMS_TO_TICKS(1000));

  sg90Stop();
  relaySet(true);

  sg90Write(180);
  vTaskDelay(pdMS_TO_TICKS(1000));

  sg90_180Write(180);
  vTaskDelay(pdMS_TO_TICKS(200));
  sg90Stop();
  vTaskDelay(pdMS_TO_TICKS(100));
  sg90Write(0);
  vTaskDelay(pdMS_TO_TICKS(2100));

  sg90Stop();
  vTaskDelay(pdMS_TO_TICKS(1500));

  sg90Write(180);
  vTaskDelay(pdMS_TO_TICKS(2100));

  sg90Stop();

  relay2Set(true);
  vTaskDelay(pdMS_TO_TICKS(3000));
  relay2Set(false);

  stepperStartMove(48000, 3000);
  WAIT_STEPPER();

  relaySet(false);

  stepperStartMove(-48000, 3000);
  WAIT_STEPPER();

  sg90Stop();

  ESP_LOGI(TAG, "Mode2: sequence complete");
}

void mode3Start() {
  ESP_LOGI(TAG, "Executing Mode 3 - Precision control");
}

void mode4Start() {
  ESP_LOGI(TAG, "Executing Mode 4 - High speed mode");
}

void mode5Start() {
  ESP_LOGI(TAG, "Executing Mode 5 - Low power mode");
}

void modeStop() {
  stepperStop();
  sg90Stop();
  ESP_LOGI(TAG, "Mode: Stopped");
}