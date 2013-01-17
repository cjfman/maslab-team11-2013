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
    pthread_join(listener, NULL);
    delete comms;
}

void Ballfinder::findballs()
{
    static int count=0;
    Mat out;
    cvb::CvBlobs blobs;
    cap >> frame;
    cvtColor(frame, HSV, CV_BGR2HSV, 0);
    if (comms->findwall==false)
    {
        Mat green, red1, red2;
        inRange(HSV, greenmin, greenmax, green);
        inRange(HSV, redmin[0], redmax[0], red1);
        inRange(HSV, redmin[1], redmax[1], red2);
        addWeighted(red1, 1, red2, 1, 0, out);
        addWeighted(out, 1, green, 1, 0, out);
        IplImage source=out;
        IplImage* dest=cvCreateImage(cvGetSize(&source), IPL_DEPTH_LABEL, 1);
        cvb::cvLabel(&source, dest, blobs);
        cvb::cvFilterByArea(blobs, areafilter, 1000000);
    }
    else
    {
        Mat bluewall;
        inRange(HSV, bluewallmin, bluewallmax, bluewall);
        IplImage source=out;
        IplImage* dest=cvCreateImage(cvGetSize(&source), IPL_DEPTH_LABEL, 1);
        cvb::cvLabel(&source, dest, blobs);
        cvb::cvFilterByArea(blobs, wallareafilter, 1000000);
       // for (iter=blobs.begin(); iter!=blobs.end(); ++iter)
       // {
            //image.at<cv::Vec3b>(x,y)[0] = newval[0];
            cvb::CvBlob* blob=(blobs.find(cvb::cvLargestBlob(blobs))->second);
            bool onwall=false;
            for (int x=blob->minx; x<=blob->maxx; ++x)
            {
              int hue=HSV.at<Vec3b>(x, blob->miny)[0];
              if ((hue>=yellowwall[0])&&(hue<=yellowwall[1])&&onwall==false)
              {
                onwall=true;
                wall_locations.push_back(x);
              }
              else if ((hue<yellowwall[0])||(hue>yellowwall[1])&&onwall==true)
              {
                onwall=false;
                wall_locations.push_back(x-1);
              }
            }
       // }
    //comms->findwall=false;
            std::cout << "exiting blue wall code\n";
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
    char outmessage[25]= {"none\0"};
    if (blobs.size()>0)
    {
        cvb::CvBlob* biggest=(blobs.find(cvb::cvLargestBlob(blobs))->second);
        snprintf(outmessage, 25,  "%d:%d\0", biggest->maxx-biggest->minx, (biggest->maxx+biggest->minx)/2);
        //outmessage=std::to_string(biggest->maxx-biggest->minx)+":"+std::to_string((biggest->maxx+biggest->minx)/2);
        //std::cout << string(outmessage) << "\n";
//    sprintf(outmessage, "%d", biggest->maxx-biggest->minx);
    }
    //std::cout << string(outmessage) << "\n";
    cvb::cvReleaseBlobs(blobs);
    if (show==true)
    {
        imshow("feed", frame);
        waitKey(20);
    }
    if (comms->pic==true)
    {
      imwrite("test"+convertInt(count)+".jpg", frame);
      ++count;
      comms->pic=false;
    }
    //std::cout << "writing frame\n";
    //++count;
    if (comms->sendnow==true)
    {
        comms->sendnow=false;
        //return string(outmessage);
        comms->sendmessage(outmessage);
        //return outmessage;
    }
    if (comms->findwall==true)
    {
      comms->findwall=false;
      comms->sendmessage(outmessage);
    }
    //std::cout << time(NULL)-time_ref << "\n";
    //else
    //return string("none");
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

extern "C" void *thread_func(void* arg)
{
    static_cast<SocketServer *>(arg)->waitmessage();
    //return;
}

void Ballfinder::runserver()
{
    pthread_create(&listener, NULL, thread_func, comms);
    while(comms->bye==false)
    {
        findballs();
    }
}
