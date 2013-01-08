#ifndef DEBOUNCED_READ
#define DEBOUNCED_READ

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
