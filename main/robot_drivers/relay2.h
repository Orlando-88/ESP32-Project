#ifndef RELAY2_H
#define RELAY2_H

void relay2Init(int pin);
void relay2Toggle();
void relay2Set(bool on);
bool relay2GetState();
void relay2ProcessKey(char key);

#endif