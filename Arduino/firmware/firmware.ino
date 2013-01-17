#include <Wire.h>
#include <stdlib.h>
//#include <avr/wdt.h>

#include "DebouncedRead.h"
#include "Motor.h"
#include "Sensors.h"
#include "IMU.h"

// Pins
#define left_speed 2
#define left_direction 3
#define right_speed 4
#define right_direction 5
#define debug_led 13
#define left_limit 52
#define right_limit 53
#define ir_1 A15

// Commands
#define cAllStop 911
#define cStatus 201
#define cRightLimit 202
#define cLeftLimit 203
#define cReadIR1 211
#define cGyroX 220
#define cGryoY 221
#define cGyroZ 222
#define cHeading 230
#define cForward 301
#define cRightSpeed 302
#define cLeftSpeed 303
#define cAutoStop 304

unsigned long stop_time;
void handshake();
String runCommand(String command);
String generateResponse(int code, int value);
String generateResponse(int code, unsigned int value);
String generateResponse(int code, float value);

// Sensors
String getStatus();
String runSensorCheck();
String checkLimitSwitches();
DebouncedRead leftLimit;
DebouncedRead rightLimit;
IRSensor IR1;
//Gyro gyro;
IMU imu;

// Motors
String allStop();
String setForwardSpeed(long speed);
Motor leftMotor;
Motor rightMotor;
bool auto_stop = false;

// Helper
long asciiToLong(String string);

void setup()
{
  pinMode(debug_led, OUTPUT);
  digitalWrite(debug_led, HIGH);
  
  Serial.begin(115200);
  Serial.println("000:Starting Up");
  
  // Set Motors
  Serial.println("001:Motors");
  leftMotor = Motor(left_speed, left_direction);
  rightMotor = Motor(right_speed, right_direction);

  // Set Limit Switches
  Serial.println("001:Limit Switches");
  rightLimit = DebouncedRead(right_limit, HIGH, true);
  leftLimit = DebouncedRead(left_limit, HIGH, true);
  
  // Set IR Sensors
  Serial.println("001:IR Sensors");
  IR1 = IRSensor(ir_1);
  
  // Setup IMU
  Serial.println("001:IMU");
  Wire.begin();
  //Serial.println("001:...Gyro");
  //gyro = Gyro();
  //gyro.setup();
  imu = IMU();
  imu.setup();
    
  // Send Ready and wait for init com
  Serial.println("100:Ready");
  handshake();
  
  // Set autostop time
  stop_time = millis();
  stop_time += 180000;
  digitalWrite(debug_led, LOW);
}

void loop()
{
  String command = "";
  while(true)
  {
    long time = millis();
    // Wait for line to be read
    while (!Serial.available())
    {
      // While waiting, check for 3 minute alert
      unsigned long now = millis();
      if (now > stop_time)
      {
        allStop();
        Serial.println("999: TIME OUT");
        while(1);
      }
      // Check for disconnection
      if (now - time > 2000)
      {
        allStop();
        //Serial.println("100: No communication. Autostop");
        //handshake();
        time = now;
        command = "";
      }
    }
    char c = Serial.read();
    if (c == '\n') break;
    command += c;
  }
  Serial.println(runCommand(command));
}

void handshake()
{
  String command = "";
  long time = millis();
  while(true)
  {
    while (!Serial.available())
    {
      // Check for communication timeout
      unsigned long now = millis();
      if (now - time > 5000)
      {
        Serial.println("100: No communication. Resend");
        time = now;
      }
    }
    
    // Read Character
    char c = Serial.read();
    if (c == '\n') 
    {
      if (command == "100")
        break;
      else
        command = "";
    }
    else
      command += c;
  }
  Serial.println("101:Begin");
}

String runCommand(String command)
{
  int code = asciiToLong(command.substring(0, 3));
  String parameters = command.substring(4);
  switch(code)
  {
  case cAllStop:
    return allStop();
  case cStatus:
    return getStatus();
  case cForward:
    return setForwardSpeed(asciiToLong(parameters));
  case cRightSpeed: 
    return generateResponse(code, rightMotor.setSpeed(asciiToLong(parameters)));
  case cLeftSpeed: 
    return generateResponse(code, leftMotor.setSpeed(asciiToLong(parameters)));
  case cAutoStop:
    return "304:" + String(auto_stop = (parameters == "true") ? true : false);
  case cRightLimit:
    return generateResponse(code, rightLimit.read());
  case cLeftLimit:
    return generateResponse(code, leftLimit.read());
  case cReadIR1:
    return "204:" + IR1.readString();
  case cGyroX:
    return generateResponse(code, imu.getGyroX());
  case cGryoY:
    return generateResponse(code, imu.getGyroY());
  case cGyroZ:
    return generateResponse(code, imu.getGyroZ());
  case cHeading:
    return generateResponse(code, imu.getHeading());
  default:
    return "404: command "+ String(code) + " not found";
  } 
}

String generateResponse(int code, int value)
{
  return String(code) + ":" + String(value);
}

String generateResponse(int code, unsigned int value)
{
  return String(code) + ":" + String(value);
}

String generateResponse(int code, float value)
{
  char buf[32];
  dtostrf(value, 4, 3, buf); 
  return String(code) + ":" + buf;
}

String getStatus()
{
  return "100: ready";
}

String allStop()
{
  setForwardSpeed(0);
  return "911: All Stopped";
}

String setForwardSpeed(long speed)
{
  rightMotor.setSpeed(speed);
  leftMotor.setSpeed(speed);
  return "301:" + String(speed);
}

String runSensorCheck()
{
  String response = "";
  response += checkLimitSwitches();
  return response;
}

String checkLimitSwitches()
{
  static boolean hit = false;
  int left = !leftLimit.read();
  int right = !rightLimit.read();
  hit &= left || right;
  if (hit)
    return "";
  if (left || right && !hit)
  {
    hit = true; 
    setForwardSpeed(0);
  }
  if (left && right)
    return "101:left and right";
  else if (right)
    return "102:right";
  else if (left)
    return "103:left";
  else
    return "";
}

long asciiToLong(String string)
{
  char c[10];
  string.toCharArray(c, 10);
  return atoi(c); 
}
