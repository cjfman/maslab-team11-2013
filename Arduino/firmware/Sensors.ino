#include <Arduino.h>
#include "Sensors.h"

IRSensor::IRSensor()
{
  this->setup(A0, 0);
}

IRSensor::IRSensor(int pin)
{
  this->setup(pin, 0);
}

IRSensor::IRSensor(int pin, int offset)
{
  this->setup(pin, offset);
}

void IRSensor::setup(int pin, int offset)
{
  this->pin = pin;
  this->offset = offset;
  pinMode(pin, INPUT);
}

int IRSensor::read()
{
  int reading = analogRead(this->pin);
  float calculation = offset;
  if (reading >= 250)
  {
    calculation += 223000/reading - 87;
  } 
  else if (reading < 250 && reading >= 149)
  {
    calculation += 292000/reading - 383;
  } 
  else 
  {
    calculation += 566000/reading - 2220;
  }
  //Serial.print("Reading: " + String(reading) + " : ");
  //Serial.println(calculation);
  //delay(500);
  return calculation;
}

String IRSensor::readString()
{
  int reading = this->read();
  return String(reading/100) + "." + String(reading%100);
}
