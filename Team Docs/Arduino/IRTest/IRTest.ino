void setup()
{
  pinMode(A15, INPUT);
  Serial.begin(115200);
}

void loop()
{
  Serial.println(2076.0/(analogRead(A15) - 11)/2.54);
  delay(500); 
}
