#include <Wire.h>
#include "IMU.h"

IMU imu;

void calcGyro();

ISR(TIMER1_COMPA_vect)
{
  isaacFilter();
}

volatile double gyro_heading = 0;
volatile double mag_heading = 0;
volatile double heading = 0;
volatile boolean calculate = false;
volatile long start = 0;
volatile long finish = 0;

void setup()
{ 
  
  // initialize Timer1
  //cli();          // disable global interrupts
  TCCR1A = 0;     // set entire TCCR1A register to 0
  TCCR1B = 0;     // same for TCCR1B
 
  // set compare match register to desired timer count:
  OCR1A = 260;
  // turn on CTC mode:
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler:
  TCCR1B |= (1 << CS10);
  TCCR1B |= (1 << CS12);
  // enable timer compare interrupt:
  TIMSK1 |= (1 << OCIE1A);
  //sei();          // enable global interrupts
  
  Serial.begin(115200);
  Serial.println("Setting up...");
  
  pinMode(13, OUTPUT);
  
  imu = IMU();
  imu.setup();

  Serial.println("done");
}

void loop()
{
  /*int heading = 0; //imu.getHeading();
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
  
  Serial.print("Time:");
  Serial.print(finish - start);
  
  Serial.print(" Magnet Heading:");
  Serial.print(mag_heading);
  
  Serial.print(" Gyro:");
  Serial.print(gyro_heading);*/
  
  Serial.print(" Heading:");
  Serial.println(heading);
  
  //Serial.print(" GH:");
  //Serial.println(global_heading);
  
  //delay(1000);
}

void calcGyro()
{
  start = millis();
  
  const float alpha = 0.6;
  
  TIMSK1 &= ~(1 << OCIE1A); // Disable Timer Interrupt
  sei(); // Reenable Global Interrupt
  double x = imu.getGyroX();
  double xx = imu.getHeading();
  cli(); // Disable Global Interrupt
  TIMSK1 |= (1 << OCIE1A); // Reenable Timer Interrupt
  
  // Gyro high pass filter
  static double gyro_high = 0;
  double diff = x * 0.01664;
  static double old_diff = 0;
  gyro_high = (1 - alpha) * (gyro_high + diff - old_diff);
  gyro_heading += gyro_high;
  
  // Mag low pass filter
  mag_heading = ((1 - alpha) * xx) + (alpha * mag_heading);
  
  heading = mag_heading + gyro_high;
   
  finish = millis();
}

void isaacFilter()
{
  TIMSK1 &= ~(1 << OCIE1A); // Disable Timer Interrupt
  sei(); // Reenable Global Interrupt
  double x = imu.getGyroX();
  cli(); // Disable Global Interrupt
  TIMSK1 |= (1 << OCIE1A); // Reenable Timer Interrupt
  long now = millis();
  static double avg = 0;
  static long i = 0;
  if (now < 5000)
  {
    avg += x;
    i++;
  }
  else if (i > 0)
  {
    avg /= i;
    i = 0;
  }
  if (i == 0)
  {
    x -= avg;
    double diff = x * 0.01664;
    heading += diff;
  }
}

void kalmanFilter()
{
  TIMSK1 &= ~(1 << OCIE1A); // Disable Timer Interrupt
  sei(); // Reenable Global Interrupt
  double x = imu.getGyroX();
  double xx = imu.getHeading();
  cli(); // Disable Global Interrupt
  TIMSK1 |= (1 << OCIE1A); // Reenable Timer Interrupt
  heading = kalmanCalculate(xx, x, .01664);
}

// --- Kalman filter module  ----------------------------------------------------------------------------

float Q_angle  =  0.001; //0.001
float Q_gyro   =  0.003;  //0.003
float R_angle  =  0.03;  //0.03

float x_angle = 0;
float x_bias = 0;
float P_00 = 0, P_01 = 0, P_10 = 0, P_11 = 0;
float dt, y, S;
float K_0, K_1;

float kalmanCalculate(float newAngle, float newRate, float dt)
{
  //dt = float(looptime)/1000;
  x_angle += dt * (newRate - x_bias);
  P_00 +=  - dt * (P_10 + P_01) + Q_angle * dt;
  P_01 +=  - dt * P_11;
  P_10 +=  - dt * P_11;
  P_11 +=  + Q_gyro * dt;

  y = newAngle - x_angle;
  S = P_00 + R_angle;
  K_0 = P_00 / S;
  K_1 = P_10 / S;

  x_angle +=  K_0 * y;
  x_bias  +=  K_1 * y;
  P_00 -= K_0 * P_00;
  P_01 -= K_0 * P_01;
  P_10 -= K_1 * P_00;
  P_11 -= K_1 * P_01;

  return x_angle;
}
