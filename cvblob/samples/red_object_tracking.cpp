// Cristóbal Carnero Liñán <grendel.ccl@gmail.com>

#include <iostream>
#include <iomanip>
#include <fstream>

#if (defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__) || (defined(__APPLE__) & defined(__MACH__)))
#include <cv.h>
#include <highgui.h>
#else
#include <opencv/cv.h>
#include <opencv/highgui.h>
#endif

#include "cvblob/cvblob.h"
#include <time.h>
using namespace cvb;
using namespace std;

void sleep(unsigned int mseconds)
{
      clock_t goal = mseconds + clock();
          while (goal > clock());
}


int main()
{
  CvTracks tracks;

  cvNamedWindow("red_object_tracking", CV_WINDOW_AUTOSIZE);

  CvCapture *capture = cvCaptureFromCAM(1);
  cvGrabFrame(capture);
  IplImage *img = cvRetrieveFrame(capture);

  //CvSize imgSize = cvGetSize(img);
  //imgSize.width=200;
  //imgSize.height=400;
  CvSize imgSize=cvSize(300,300);
  IplImage *resized=cvCreateImage(imgSize, img->depth, img->nChannels);
 // cvResize(img, resized, CV_INTER_LINEAR);
  //CvSize size=cvGetSize(resized);

  //IplImage *frame = cvCreateImage(size, resize->depth, resize->nChannels);
  IplImage* frame=img;

  IplConvKernel* morphKernel = cvCreateStructuringElementEx(5, 5, 1, 1, CV_SHAPE_RECT, NULL);

  //unsigned int frameNumber = 0;
  unsigned int blobNumber = 0;

  ofstream outfile;
  //outfile.open("coords.txt", ios_base::openmode mode = ios_base::trunc);
  bool quit = false;
  map<CvLabel, CvBlob*>::iterator iter;
  while (!quit&&cvGrabFrame(capture))
  {
    IplImage *img = cvRetrieveFrame(capture);

    cvConvertScale(img, frame, 1, 0);

    IplImage *segmentated = cvCreateImage(imgSize, 8, 1);
    
    // Detecting red pixels:
    // (This is very slow, use direct access better...)
    for (unsigned int j=0; j<imgSize.height; j++)
      for (unsigned int i=0; i<imgSize.width; i++)
      {
	CvScalar c = cvGet2D(frame, j, i);

	double b = ((double)c.val[0])/255.;
	double g = ((double)c.val[1])/255.;
	double r = ((double)c.val[2])/255.;
	unsigned char f = 255*((r>0.2+g)&&(r>0.2+b));

	cvSet2D(segmentated, j, i, CV_RGB(f, f, f));
      }

    cvMorphologyEx(segmentated, segmentated, NULL, morphKernel, CV_MOP_OPEN, 1);

    //cvShowImage("segmentated", segmentated);

    IplImage *labelImg = cvCreateImage(cvGetSize(frame), IPL_DEPTH_LABEL, 1);

    CvBlobs blobs;
    unsigned int result = cvLabel(segmentated, labelImg, blobs);
    cvFilterByArea(blobs, 500, 1000000);
    if (blobs.size()>=1)
    {
     // map<CvLabel, CvBlob*>::iterator iter;
      iter=blobs.find(1);
      if (iter->second->area>40)
      {
      cvCentroid(iter->second);
      CvBlob blob=*blobs.at(1);
      if (iter->second->centroid.x>0)
      {
      outfile.open("coord", ios_base::trunc);
      outfile << iter->second->centroid.x << "\n";
      outfile.close();
      }
      }
    }
    cvRenderBlobs(labelImg, blobs, frame, frame, CV_BLOB_RENDER_BOUNDING_BOX);
    cvUpdateTracks(blobs, tracks, 200., 5);
    cvRenderTracks(tracks, frame, frame, CV_TRACK_RENDER_ID|CV_TRACK_RENDER_BOUNDING_BOX);

    cvShowImage("red_object_tracking", frame);

    /*std::stringstream filename;
    filename << "redobject_" << std::setw(5) << std::setfill('0') << frameNumber << ".png";
    cvSaveImage(filename.str().c_str(), frame);*/

    cvReleaseImage(&labelImg);
    cvReleaseImage(&segmentated);

    char k = cvWaitKey(10)&0xff;
    switch (k)
    {
      case 27:
      case 'q':
      case 'Q':
        quit = true;
        break;
      case 's':
      case 'S':
        for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
        {
          std::stringstream filename;
          filename << "redobject_blob_" << std::setw(5) << std::setfill('0') << blobNumber << ".png";
          cvSaveImageBlob(filename.str().c_str(), img, it->second);
          blobNumber++;

          std::cout << filename.str() << " saved!" << std::endl;
        }
        break;
    }

    cvReleaseBlobs(blobs);
    sleep(1000);
    //frameNumber++;
  }

  cvReleaseStructuringElement(&morphKernel);
  cvReleaseImage(&frame);

  cvDestroyWindow("red_object_tracking");
  //outfile.close();
  return 0;
}
