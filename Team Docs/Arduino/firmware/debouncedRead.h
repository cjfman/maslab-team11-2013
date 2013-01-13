#ifndef DEBOUNCED_READ_h
#define DEBOUNCED_READ_h

#include <Arduino.h>

class DebouncedRead
{
public:
  DebouncedRead();
  DebouncedRead(unsigned int pin);
  DebouncedRead(unsigned int pin, unsigned int pull);
  DebouncedRead(unsigned int pin, unsigned int pull, boolean inversion);
  unsigned int read();
  void setInvertion(boolean setting);

private:
  unsigned long time;
  unsigned int value;
  unsigned int pin;
  boolean inverted;
  
  void setup(unsigned int pin, unsigned int pull, boolean inversion);
};

#endif
