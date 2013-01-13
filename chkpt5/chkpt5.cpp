#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp> 
#include <iostream>
#include <stdio.h> 
#include "cvblob/cvblob.h"
#include "Ballfinder.h"
using namespace cv;
/*
class camera : public VideoCapture
{
public:
   camera(int device);
;
*/
int main()
{
    VideoCapture cap(0); // open the default camera
    if(!cap.isOpened())  // check if we succeeded
        return -1;

    Mat frame, output;
    frame=imread("/home/yanni/maslab/epc/images/2013-01-07 14.47.29.jpg");
    namedWindow("edges",1);
    namedWindow("original",1);
    int var[6];
    createTrackbar("r1", "edges", &var[0], 255);
    createTrackbar("g1", "edges", &var[1], 255);
    createTrackbar("b1", "edges", &var[2], 255);
    createTrackbar("r2", "edges", &var[3], 255);
    createTrackbar("g2", "edges", &var[4], 255);
    createTrackbar("b2", "edges", &var[5], 255);

    for(;;)
    {
        //Mat frame;
        //cap >> frame; // get a new frame from camera
	//std::cin >> var[0] >> var[1] >> var[2] >> var[3] >> var[4] >> var[5] >> var[6];
	inRange(frame, Scalar(var[0], var[1], var[2]), Scalar(var[3], var[4], var[5]), output);
        //cvtColor(frame, edges, CV_BGR2GRAY);
        //GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
        //Canny(edges, edges, 0, 30, 3);
        imshow("edges", output);
	imshow("original", frame);
        if(waitKey(30) >= 0) break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
