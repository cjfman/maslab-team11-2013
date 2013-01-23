#ifndef IMU_h
#define IMU_h

#include <Arduino.h>
#include "HMC5883L.h"

class IMU
{
public:
  IMU();
  void setup();
  void setTimeNow();
  
  float getGyroX();
  float getGyroY();
  float getGyroZ();
  int getHeading();
  int getGlobalHeading();
  //void calcGlobalHeading();
  
private:
  int global_heading;
  int last_heading;
  long time;
  
  int L3G4200D_Address;
  HMC5883L compass;
  int HMC_error;
  float declinationAngle;
  
  int setupL3G4200D(int scale);
  void writeRegister(int deviceAddress, byte address, byte val);
  int readRegister(int deviceAddress, byte address);
};

#endif IMU_h
