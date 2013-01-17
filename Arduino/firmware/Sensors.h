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

class Gyro
{
public:
  Gyro();
  void setup();
  int getX();
  int getY();
  int getZ();
  
private:
  int L3G4200D_Address;
  int setupL3G4200D(int scale);
  void writeRegister(int deviceAddress, byte address, byte val);
  int readRegister(int deviceAddress, byte address);
};

#endif
