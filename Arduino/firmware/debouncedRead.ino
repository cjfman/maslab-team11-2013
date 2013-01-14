#include "debouncedRead.h"

DebouncedRead::DebouncedRead()
{
  this->setup(0, HIGH, false);
}
DebouncedRead::DebouncedRead(unsigned int pin)
{
  this->setup(pin, HIGH, false);
}

DebouncedRead::DebouncedRead(unsigned int pin, unsigned int pull)
{
  this->setup(pin, pull, false);
}

DebouncedRead::DebouncedRead(unsigned int pin, unsigned int pull, boolean inversion)
{
  this->setup(pin, pull, inversion);
}

void DebouncedRead::setup(unsigned int pin, unsigned int pull, boolean inversion)
{
  pinMode(pin, INPUT);
  digitalWrite(pin, pull);  this->time = millis();
  this->pin = pin;
  this->value = digitalRead(this->pin);
  this->time = millis();
  this->inverted = inversion;
}

void DebouncedRead::setInvertion(boolean setting)
{
  this->inverted = setting;
}

unsigned int DebouncedRead::read()
{
  unsigned long now = millis();
  if (now - this->time > 100)
  {
    this->value = digitalRead(this->pin);
    this->time = now;
  }
  return (this->inverted) ? !this->value : this->value;
}
