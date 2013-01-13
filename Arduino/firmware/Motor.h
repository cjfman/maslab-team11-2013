#ifndef MOTOR_h
#define MOTOR_h

#include <String.h>

class Motor
{
public:
Motor();
Motor(int speed_pin, int direction_pin);
int setSpeed(int speed);
void stop();

int speed_pin;
int direction_pin;
};

#endif
