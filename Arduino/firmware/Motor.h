#ifndef MOTOR_h
#define MOTOR_h

#include <String.h>

class Motor
{
public:
Motor();
Motor(int speed_pin, int direction_pin);
int setSpeed(int speed);
void increment(int amount);
void decrement(int amount);
void stop();

int speed_pin;
int direction_pin;
int speed;
int direction;
};

#endif
