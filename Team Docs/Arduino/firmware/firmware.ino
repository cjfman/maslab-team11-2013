#include <stdlib.h>
#include "debouncedRead.h"
#include "Motor.h"

// Pins
#define left_speed 2
#define left_direction 3
#define right_speed 4
#define right_direction 5
#define debug_light 13
#define left_limit 52
#define right_limit 53

// Commands
#define cAllStop 911
#define cStatus 201
#define cRightLimit 202
#define cLeftLimit 203
#define cReadIR1 204
#define cForward 301
#define cRightSpeed 302
#define cLeftSpeed 303
#define cAutoStop 304

/* Response Codes
  100: Ready to accept commands
  101: Right and left limit switches hit
  102: Right limit switch hit
  103: Left limit switch hit
  202: Right limit status
  203: Left limit status
  300:<Current Speed>
*/

String runCommand(String command);

// Sensors
String getStatus();
String runSensorCheck();
String checkLimitSwitches();
DebouncedRead leftLimit;
DebouncedRead rightLimit;

// Motors
String setForwardSpeed(long speed);
Motor leftMotor;
Motor rightMotor;
bool auto_stop = false;

// Helper
long asciiToLong(String string);

void setup()
{
  Serial.begin(115200);
  pinMode(debug_light, OUTPUT);

  
  // Set Motors
  leftMotor = Motor(left_speed, left_direction);
  rightMotor = Motor(right_speed, right_direction);

  // Set Limit Switches
  rightLimit = DebouncedRead(right_limit);
  leftLimit = DebouncedRead(left_limit);
  
  Serial.println("\n100:Ready");
}

void loop()
{
  String command = "";
  while(true)
  {
    while (!Serial.available())
    {
      /*String message;
      if ((message = runSensorCheck()).length() > 0)
        Serial.println(message);*/
    }
    char c = Serial.read();
    if (c == '\n') break;
    command += c;
  }
  //Serial.println(runCommand(command));
}

String runCommand(String command)
{
  long code = asciiToLong(command.substring(0, 3));
  String parameters = command.substring(4);
  switch(code)
  {
  case cStatus:
    return getStatus();
  case cForward:
    return setForwardSpeed(asciiToLong(parameters));
  case cRightSpeed: 
    return "302:" + String(rightMotor.setSpeed(asciiToLong(parameters)));
  case cLeftSpeed: 
    return "303:" + String(leftMotor.setSpeed(asciiToLong(parameters)));
  case cAutoStop:
    return "304:" + String(auto_stop = (parameters == "true") ? true : false);
  case cRightLimit:
    return "202:" + String(rightLimit.read());
  case cLeftLimit:
    return "203:" + String(leftLimit.read());
  default:
    return "404: command "+ String(code) + " not found";
  } 
}

String getStatus()
{
  return "401: unimplimented";
}

String setForwardSpeed(long speed)
{
  rightMotor.setSpeed(speed);
  leftMotor.setSpeed(speed);
  analogWrite(13, speed);
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
