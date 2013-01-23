#include <Arduino.h>
#include "Motor.h"

Motor::Motor()
{
  this->speed_pin = 0;
  this->direction_pin = 0;
}

Motor::Motor(int speed_pin, int direction_pin)
{
  this->speed_pin = speed_pin;
  this->direction_pin = direction_pin;
  pinMode(speed_pin, OUTPUT);
  pinMode(direction_pin, OUTPUT);
}

int Motor::setSpeed(int speed)
{
  //this->speed = speed;
  direction = (speed > 0) ? 1 : 0;
  int abs_speed = abs(speed);
  this->speed = abs_speed;
  abs_speed = (abs_speed <= 255) ? abs_speed : 255;
  analogWrite(speed_pin, abs_speed);
  digitalWrite(direction_pin, direction);
  return abs_speed;
}

void Motor::increment(int amount)
{
  speed += amount;
  analogWrite(speed_pin, speed);
}

void Motor::decrement(int amount)
{
  speed -= amount;
  analogWrite(speed_pin, speed);
}

void Motor::stop()
{
  this->setSpeed(0);
}
