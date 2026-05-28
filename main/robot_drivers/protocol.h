#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h> // 替换 Arduino 的 byte

void protocolInit();
void protocolProcessKey(char key);
uint8_t calculateChecksum(const char* str);
void executeCommand(const char* command);

void mode1Start();
void mode2Start();
void mode3Start();
void mode4Start();
void mode5Start();
void modeStop();

#endif