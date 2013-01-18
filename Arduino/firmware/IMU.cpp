#include <Arduino.h>
#include <Wire.h>
#include "IMU.h"
#include "HMC5883L.h"

#define GYRO_CTRL_REG1 0x20
#define GYRO_CTRL_REG2 0x21
#define GYRO_CTRL_REG3 0x22
#define GYRO_CTRL_REG4 0x23
#define GYRO_CTRL_REG5 0x24

IMU::IMU()
{
  L3G4200D_Address = 105; //I2C address of the L3G4200D
}

void IMU::setup(){
  Wire.begin();
  
  // Gyro
  setupL3G4200D(2000); // Configure L3G4200  - 250, 500 or 2000 deg/sec
  delay(1500); //wait for the sensor to be ready 
  
  //HMC
  declinationAngle = 0.0457;
  HMC_error = 0;
  compass = HMC5883L(); // Construct a new HMC5883 compass.
  HMC_error = compass.SetScale(1.3); // Set the scale of the compass.
  if(HMC_error != 0) // If there is an error, print it out.
  {
    Serial.print("400: Compass error:");
    Serial.println(compass.GetErrorText(HMC_error));
  }
  HMC_error = compass.SetMeasurementMode(Measurement_Continuous); // Set the measurement  mode to Continuous
  if(HMC_error != 0) // If there is an error, print it out.
  {
    Serial.print("400: Compass error:");
    Serial.println(compass.GetErrorText(HMC_error));
  }
  
  global_heading = getHeading();
}

void IMU::setTimeNow()
{
  time = millis();
  last_heading = getHeading();
}

float IMU::getGyroX()
{
  byte MSB = readRegister(L3G4200D_Address, 0x29);
  byte LSB = readRegister(L3G4200D_Address, 0x28);
  int value = ((MSB << 8) | LSB);
  return value *.07;
}

float IMU::getGyroY()
{
  byte MSB = readRegister(L3G4200D_Address, 0x2B);
  byte LSB = readRegister(L3G4200D_Address, 0x2A);
  int value = ((MSB << 8) | LSB);
  return value *.07;
}


float IMU::getGyroZ()
{
  byte MSB = readRegister(L3G4200D_Address, 0x2D);
  byte LSB = readRegister(L3G4200D_Address, 0x2C);
  int value = ((MSB << 8) | LSB);
  return value *.07;
}

int IMU::getHeading()
{
  MagnetometerScaled scaled = compass.ReadScaledAxis();
  //float heading = atan2(scaled.YAxis, scaled.XAxis);
  float heading = atan2(scaled.ZAxis, scaled.YAxis);
  heading += declinationAngle;
  if(heading < 0) heading += 2*PI;
  if(heading > 2*PI) heading -= 2*PI;
  float headingDegrees = heading * 180/M_PI; 
  return headingDegrees;
}

int IMU::getGlobalHeading()
{
  long now = millis();
  
  // Get Data
  float z = getGyroZ();
  int heading = getHeading();
  
  // Calculate Interval and expected change
  long interval = now - time;
  time = now;
  int heading_diff = heading - last_heading;
  last_heading = heading;
  
  // Calculate Gyro change in heading
  double rotation = interval * z;
  rotation /= 1000;
  //int gyro_diff = (int)rotation;
  //Serial.println(gyro_diff);
  
  // Determin Movement
  if (global_heading >= 360) global_heading -= 360;
  if (global_heading < 0) global_heading += 360;
  if (!heading_diff) return global_heading;
  global_heading += rotation;
  return global_heading;
}

int IMU::setupL3G4200D(int scale){
  //From  Jim Lindblom of Sparkfun's code

  // Enable x, y, z and turn off power down:
  writeRegister(L3G4200D_Address, GYRO_CTRL_REG1, 0b00001111);

  // If you'd like to adjust/use the HPF, you can edit the line below to configure GYRO_CTRL_REG2:
  writeRegister(L3G4200D_Address, GYRO_CTRL_REG2, 0b00000000);

  // Configure GYRO_CTRL_REG3 to generate data ready interrupt on INT2
  // No interrupts used on INT1, if you'd like to configure INT1
  // or INT2 otherwise, consult the datasheet:
  writeRegister(L3G4200D_Address, GYRO_CTRL_REG3, 0b00001000);

  // GYRO_CTRL_REG4 controls the full-scale range, among other things:

  if(scale == 250){
    writeRegister(L3G4200D_Address, GYRO_CTRL_REG4, 0b00000000);
  }else if(scale == 500){
    writeRegister(L3G4200D_Address, GYRO_CTRL_REG4, 0b00010000);
  }else{
    writeRegister(L3G4200D_Address, GYRO_CTRL_REG4, 0b00110000);
  }

  // GYRO_CTRL_REG5 controls high-pass filtering of outputs, use it
  // if you'd like:
  writeRegister(L3G4200D_Address, GYRO_CTRL_REG5, 0b00000000);
}

void IMU::writeRegister(int deviceAddress, byte address, byte val) {
    Wire.beginTransmission(deviceAddress); // start transmission to device 
    Wire.write(address);       // send register address
    Wire.write(val);         // send value to write
    Wire.endTransmission();     // end transmission
}

int IMU::readRegister(int deviceAddress, byte address){
    int v;
    Wire.beginTransmission(deviceAddress);
    Wire.write(address); // register to read
    Wire.endTransmission();

    Wire.requestFrom(deviceAddress, 1); // read a byte

    while(!Wire.available()) {
        // waiting
    }

    v = Wire.read();
    return v;
}
