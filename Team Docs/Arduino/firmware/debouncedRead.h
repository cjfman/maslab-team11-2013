#ifndef DEBOUNCED_READ_h
#define DEBOUNCED_READ_h

class DebouncedRead
{
public:
DebouncedRead();
DebouncedRead(unsigned int pin);
DebouncedRead(unsigned int pin, unsigned int pull);
unsigned int read();

unsigned long time;
unsigned int value;
unsigned int pin;
};

#endif
