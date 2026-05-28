#ifndef STEPPER_H
#define STEPPER_H

void stepperInit();
void stepperStartMove(int steps, int speed);
void stepperRun();
bool stepperIsDone();
void stepperStartContinuous(int speed);
void stepperStop();
void stepperSetSpeed(int speed);

#endif