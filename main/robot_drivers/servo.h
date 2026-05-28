#ifndef SERVO_H
#define SERVO_H

void servoInit(int pin);
void servoWrite(int angle);
void servoProcessKey(char key);

#endif