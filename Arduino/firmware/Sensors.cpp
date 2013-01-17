#include <Arduino.h>
#include <Wire.h>
#include "Sensors.h"

/******************************
* IR Sensor
******************************/

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

/*******************************
* Gyro
*******************************/

#define GYRO_CTRL_REG1 0x20
#define GYRO_CTRL_REG2 0x21
#define GYRO_CTRL_REG3 0x22
#define GYRO_CTRL_REG4 0x23
#define GYRO_CTRL_REG5 0x24

Gyro::Gyro()
{
  L3G4200D_Address = 105; //I2C address of the L3G4200D
}

void Gyro::setup(){
  setupL3G4200D(2000); // Configure L3G4200  - 250, 500 or 2000 deg/sec
  delay(1500); //wait for the sensor to be ready 
}

int Gyro::getX()
{
  byte MSB = readRegister(L3G4200D_Address, 0x29);
  byte LSB = readRegister(L3G4200D_Address, 0x28);
  return ((MSB << 8) | LSB);
}

int Gyro::getY()
{
  byte MSB = readRegister(L3G4200D_Address, 0x2B);
  byte LSB = readRegister(L3G4200D_Address, 0x2A);
  return ((MSB << 8) | LSB);
}


int Gyro::getZ()
{
  byte MSB = readRegister(L3G4200D_Address, 0x2D);
  byte LSB = readRegister(L3G4200D_Address, 0x2C);
  return ((MSB << 8) | LSB);
}

int Gyro::setupL3G4200D(int scale){
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

void Gyro::writeRegister(int deviceAddress, byte address, byte val) {
    Wire.beginTransmission(deviceAddress); // start transmission to device 
    Wire.write(address);       // send register address
    Wire.write(val);         // send value to write
    Wire.endTransmission();     // end transmission
}

int Gyro::readRegister(int deviceAddress, byte address){
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
