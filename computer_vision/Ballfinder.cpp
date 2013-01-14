#include "Ballfinder.h"
using namespace cv;


string Ballfinder::convertInt(int number)
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

Ballfinder::Ballfinder(int camera, string portnum, bool show_video)
{
    cap=VideoCapture(camera);
    show=show_video;
    //std::cout << "in constructor\n";
    if (show==true)
    {
        //std::cout << "making window\n";
        namedWindow("feed", 1);
        //std::cout << "made window\n";
    }
    initserver(portnum);
}

Ballfinder::~Ballfinder()
{
  delete comms;
}

string Ballfinder::findballs()
{
    Mat out;
    cvb::CvBlobs blobs;
    cap >> frame;
    cvtColor(frame, HSV, CV_BGR2HSV, 0);
    inRange(HSV, greenmin, greenmax, out);
    IplImage source=out;
    IplImage* dest=cvCreateImage(cvGetSize(&source), IPL_DEPTH_LABEL, 1);
    cvb::cvLabel(&source, dest, blobs);
    cvb::cvFilterByArea(blobs, areafilter, 1000000);
    if (blobs.size()==0)
    {
        inRange(HSV, redmin, redmax, out);
        source=out;
        cvb::cvLabel(&source, dest, blobs);
        cvb::cvFilterByArea(blobs, areafilter, 1000000);
    }
    if (show==true)
    {
        for (iter=blobs.begin(); iter!=blobs.end(); ++iter)
        {
            cvb::CvBlob* blob=(*iter).second;
            if (show==true)
            {
                rectangle(frame, Point(blob->minx, blob->maxy), Point(blob->maxx, blob->miny), Scalar(255,0,0), 2);
                putText(frame, convertInt((blob->maxx+blob->minx)/2)+", "+convertInt((blob->maxy+blob->miny)/2), Point(blob->maxx, blob->maxy), FONT_HERSHEY_SIMPLEX, 1, Scalar(0,0,255));
            }
        }
    }
    char outmessage[10];
    cvb::CvBlob* biggest=(blobs.find(cvb::cvLargestBlob(blobs))->second);
    sprintf(outmessage, "%d", (biggest->minx+biggest->maxx)/2);
    cvb::cvReleaseBlobs(blobs);
    if (show==true)
    {
        imshow("feed", frame);
        waitKey(30);
    }
    return string(outmessage);
}

void Ballfinder::show_raw_video()
{
    cap >> frame;
    imshow("feed", frame);
    waitKey(30);
}

void Ballfinder::initserver(string port)
{
    comms= new SocketServer(port);
}

void Ballfinder::runserver()
{
    while(1)
    {
        if (comms->waitmessage(30)==true)
        {
            comms->sendmessage(findballs());
        }
    }
}
