#include <iostream>
#include "Ballfinder.h"
using namespace cv;


int main()
{
  Ballfinder cpvision(1, "5555", true);
  cpvision.runserver();
  return 1;
}
