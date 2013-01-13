#pragma once
using namespace cv;

struct Ball
{
};

class Ballfinder
{
private:
Mat frame;
Scalar minred, maxred, mingreen, maxgreen;
public:

Ballfinder(string image)
{
frame=imread(image);
minred=Scalar(0,0,62);
maxred=Scalar(255,50,255);
mingreen=Scalar(0,58,0);
maxgreen=Scalar(77,255,56);
}

void findballs(int* buf, int size)
{
Mat redout, greenout;
inRange(frame, minred, maxred, redout);
inRange(frame, mingreen, maxgreen, greenout);
}

};
