#ifndef SG90_H
#define SG90_H

void sg90Init(int pin);
void sg90Write(int angle);
void sg90Stop();
void sg90ProcessKey(char key);

#endif