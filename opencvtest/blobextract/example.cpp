#include <stdio.h>
#include <stdlib.h>

#include <cv.h>
#include <cxtypes.h>
#include <cxcore.h>
#include <highgui.h>

#include "BlobExtract.h"

int main( int argc, char *argv[] )
{	int errflg = 0;
	int i;
	BlobExtract extract;
	std::vector<BlobExtract::Blob> blobs;

	IplImage *img = cvLoadImage( "blobtest.png", 1 );
	IplImage *img_out = cvCreateImage( cvGetSize( img ), 8, 3 );

	cvNamedWindow( "in" );
	cvNamedWindow( "out" );

	while( cvWaitKey( 10 ) != 'q' )
	{	cvCopy( img, img_out );

		cvSetImageCOI( img, 3 );
		extract.process( img, &blobs );
		cvSetImageCOI( img, 0 );

		for( i=0; i<(int)blobs.size(); ++i )
		{	cvRectangle( img_out, blobs[i].point1(), blobs[i].point2(), CV_RGB(0,255,0) );
			cvCircle( img_out, blobs[i].com(), 5, CV_RGB(0,255,0), -1 );
		}

		cvShowImage( "in", img );
		cvShowImage( "out", img_out );
	}

	// cleanup
	cvDestroyWindow( "in" );
	cvDestroyWindow( "out" );

	cvReleaseImage( &img );
	cvReleaseImage( &img_out );

	return errflg;
}