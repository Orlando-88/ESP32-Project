#ifndef RELAY_H
#define RELAY_H

// 原本依赖 Arduino.h 提供 bool 类型，C++原生支持 bool，不需要额外包含
void relayInit(int pin);
void relayToggle();
void relaySet(bool on);
bool relayGetState();
void relayProcessKey(char key);

#endif