#include "debouncedRead.h"

DebouncedRead::DebouncedRead()
{
  pinMode(0, INPUT);
  digitalWrite(0, HIGH);
  this->time = millis();
  this->pin = 0;
  this->value = digitalRead(0);
}
DebouncedRead::DebouncedRead(unsigned int pin)
{
  pinMode(pin, INPUT);
  digitalWrite(pin, HIGH);
  this->time = millis();
  this->pin = pin;
  this->value = digitalRead(this->pin);
}

DebouncedRead::DebouncedRead(unsigned int pin, unsigned int pull)
{
  pinMode(pin, INPUT);
  digitalWrite(pin, pull);  this->time = millis();
  this->pin = pin;
  this->value = digitalRead(this->pin);
}

unsigned int DebouncedRead::read()
{
  unsigned long now = millis();
  if (now - this->time > 100)
  {
    this->value = digitalRead(this->pin);
    this->time = now;
  }
  return this->value;
}
