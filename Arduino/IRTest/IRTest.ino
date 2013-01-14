void setup()
{
  pinMode(A15, INPUT);
  Serial.begin(115200);
}

void loop()
{
  int reading = analogRead(A15);
  float calculation = 0;
  if (reading >= 250)
  {
    calculation = 2230.0/reading - .87;
  } 
  else if (reading < 250 && reading >= 149)
  {
    calculation = 2920.0/reading - 3.83;
  } 
  else 
  {
    calculation = 5660.0/reading - 22.2;
  }
  Serial.print("Reading: " + String(reading) + " : ");
  Serial.println(calculation);
  delay(500); 
}
