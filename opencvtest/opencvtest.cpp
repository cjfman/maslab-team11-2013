#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "cvblob/cvblob.h"
#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
//Scalar minred=Scalar(340, 151, 0);
//Scalar maxred=Scalar(360, 255, 0);

using namespace cv;

string convertInt(int number)
{
    if (number == 0)
        return "0";
    string temp="";
    string returnvalue="";
    while (number>0)
    {
        temp+=number%10+48;
        number/=10;
    }
    for (int i=0; i<temp.length(); i++)
        returnvalue+=temp[temp.length()-i-1];
    return returnvalue;
}

/*
class camera : public VideoCapture
{
public:
   camera(int device);
;
*/
int main()
{
   // VideoCapture cap(0); // open the default camera
    //if(!cap.isOpened())  // check if we succeeded
      //  return -1;

    Mat frame, HSV;
    //cap >> frame;
    frame=imread("/home/yanni/maslab-team11-2013/images/reference_images/2013-01-07 14.49.16.jpg");
    cvtColor(frame, HSV, CV_BGR2HSV, 0);
    namedWindow("edges",1);
    namedWindow("original",1);
    //namedWindow("blobs", 1);
    int var[6];
    createTrackbar("h1", "edges", &var[0], 360);
    createTrackbar("s1", "edges", &var[1], 255);
    createTrackbar("b1", "edges", &var[2], 255);
    createTrackbar("h2", "edges", &var[3], 360);
    createTrackbar("s2", "edges", &var[4], 255);
    createTrackbar("b2", "edges", &var[5], 255);

    std::map<cvb::CvLabel, cvb::CvBlob*>::iterator iter;
    //std::ofstream outfile;
    //outfile.open("coord", std::ios_base::trunc);
    for(;;)
    {
        Mat output;
        //cap >> frame; // get a new frame from camera
        //std::cin >> var[0] >> var[1] >> var[2] >> var[3] >> var[4] >> var[5] >> var[6];
        cvtColor(frame, HSV, CV_BGR2HSV, 0);
        inRange(HSV, Scalar(var[0]/2, var[1], var[2]), Scalar(var[3]/2, var[4], var[5]), output);
        //inRange(HSV, Scalar(80/2, 104, 0), Scalar(158/2, 255, 255), output);
        //cvtColor(frame, edges, CV_BGR2GRAY);
        //GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
        //Canny(edges, edges, 0, 30, 3);
        //cvb::CvBlobs blobs;
        //IplImage* source=new IplImage(output);
        //IplImage* dest=cvCreateImage(cvGetSize(source), IPL_DEPTH_LABEL, 1);
        //unsigned int result=cvb::cvLabel(source, dest, blobs);
        //cvb::cvFilterByArea(blobs, 500, 1000000);
        /*for (iter=blobs.begin(); iter!=blobs.end(); ++iter)
        {
            cvb::CvBlob* blob=(*iter).second;
            rectangle(frame, Point(blob->minx, blob->maxy), Point(blob->maxx, blob->miny), Scalar(255,0,0), 2);
            putText(frame, convertInt((blob->maxx+blob->minx)/2)+", "+convertInt((blob->maxy+blob->miny)/2), Point(blob->maxx, blob->maxy), FONT_HERSHEY_SIMPLEX, 1, Scalar(0,0,255));
        }
        //putText(frame, "found blobs", Point(10,30), FONT_HERSHEY_SIMPLEX, 3, Scalar(255,255,255));
        //cvb::cvRenderBlobs(dest, blobs, frame, frame, CV_BLOB_RENDER_BOUNDING_BOX);
        //imshow("blobs", Mat(source));
        */
        imshow("edges", output);
        imshow("original", frame);
        //cvb::cvReleaseBlobs(blobs);
        //delete source;
        if(waitKey(30) >= 0) break;
    }
    //outfile.close();
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
