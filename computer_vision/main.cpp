#include <iostream>
#include "Ballfinder.h"
using namespace cv;


int main()
{
  Ballfinder cpvision(0, "5567", true);
  cpvision.runserver();
  return 1;
}
