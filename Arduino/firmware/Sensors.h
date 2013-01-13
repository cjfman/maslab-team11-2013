#ifndef SENSORS_h
#define SENSORS_h

#include <Arduino.h>

class IRSensor
{
public:
  IRSensor();
  IRSensor(int pin);
  IRSensor(int pin, int offset);
  int read();
  String readString();
  
private:
  int pin;
  int offset;
  
  void setup(int pin, int offset);
};

void setupGyro();
int gyroGetX();
int gyroGetY();
int gyroGetZ();


#endif
