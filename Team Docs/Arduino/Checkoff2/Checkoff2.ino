#include <stdlib.h>
#include "debouncedRead.h"

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
#define cReadIR1 202
#define cForward 301
#define cRightSpeed 302
#define cLeftSpeed 303

/* Response Codes
  100: Ready to accept commands
  101: Right and left limit switches hit
  102: Right limit switch hit
  103: Left limit switch hit
  300:<Current Speed>
*/

String runCommand(String command);

// Sensors
String getStatus();
String runSensorCheck();
String checkLimitSwitches();
String getRightLimit();
String getLeftLimit();
DebouncedRead leftLimit;
DebouncedRead rightLimit;

// Motors
String setForwardSpeed(long speed);
String setRightSpeed(long speed);
String setLeftSpeed(long speed);

// Helper
long asciiToLong(String string);

void setup()
{
  Serial.begin(115200);
  pinMode(debug_light, OUTPUT);

  
  // Set Motors
  pinMode(left_speed, OUTPUT);
  pinMode(left_direction, OUTPUT);
  pinMode(right_speed, OUTPUT);
  pinMode(right_direction, OUTPUT);
  
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
      String message;
      if ((message = runSensorCheck()).length() > 0)
        Serial.println(message);
    }
    char c = Serial.read();
    if (c == '\n') break;
    command += c;
  }
  Serial.println(runCommand(command));
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
    return setRightSpeed(asciiToLong(parameters));
  case cLeftSpeed: 
    return setLeftSpeed(asciiToLong(parameters));
  default:
    return "404: command "+ String(code) + " not found";
  } 
}

String getStatus()
{
  return "401: unimplimented";
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

String setForwardSpeed(long speed)
{
  setRightSpeed(speed);
  setLeftSpeed(speed);
  analogWrite(debug_light, speed);
  return "301:" + String(speed);
}

String setRightSpeed(long speed)
{
  int direction = (speed > 0) ? 1 : 0;
  analogWrite(right_speed, abs(speed));
  digitalWrite(right_direction, direction);
  return "302:" + String(speed);
}

String setLeftSpeed(long speed)
{
  int direction = (speed > 0) ? 1 : 0;
  analogWrite(left_speed, abs(speed));
  digitalWrite(left_direction, direction);
  return "303:" + String(speed);
}

long asciiToLong(String string)
{
  /*int factor = 1;
  long result = 0;
  if (string[0] = '-') 
  {
    factor = -1;
    string = string.substring(1);
  }
  for (int i = 0; i < string.length(); i++)
  {
    if (string[i] < '0' or string[i] > '9')
    {
      return 0;
    }
    result = result*10 + (string[i] - '0');
  }
  return result * factor;*/
  char c[10];
  string.toCharArray(c, 10);
  return atoi(c); 
}
