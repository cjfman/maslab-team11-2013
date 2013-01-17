#include <Wire.h>
#include "IMU.h"

IMU imu;

void setup()
{
  Serial.begin(115200);
  Serial.println("Setting up...");
  
  imu = IMU();
  imu.setup();
}

void loop()
{
  int heading = imu.getHeading();
  int global_heading = imu.getGlobalHeading();
  float x = imu.getGyroX();
  float y = imu.getGyroY();
  float z = imu.getGyroZ();
  
  Serial.print("X:");
  Serial.print(x);
  Serial.print(',');

  Serial.print(" Y:");
  Serial.print(y);

  Serial.print(" Z:");
  Serial.print(z);
  
  Serial.print(" Heading:");
  Serial.print(heading);
  
  Serial.print(" GH:");
  Serial.println(global_heading);
  
  //delay(100);
}
