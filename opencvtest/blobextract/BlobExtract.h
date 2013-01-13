#ifndef BLOBEXTRACT_H
#define BLOBEXTRACT_H

#include <vector>
#include <queue>

#include <cv.h>
#include <cxtypes.h>
#include <cxcore.h>
#include <highgui.h>

#define BE_PIXEL_I( IMG, X, Y, C ) (((Y)*(IMG)->widthStep)+(X)*(IMG)->nChannels+(C))
#define BE_PIXEL( IMG, X, Y, C ) (IMG)->imageData[PIXEL_I(IMG,X,Y,C)]

class BlobExtract
{	
public:
	class Blob
	{
	public:
		int l, r, t, b;
		unsigned int x_val, y_val;
		unsigned int pixels;
		int id;

		Blob( int _id=0 );

		/* adds a pixel to the blob, does not have to actually be connected to the rest 
		   of the blob. i cant image why you would need to use this */
	    void addPixel( int _x, int _y );

		int width();

		int height();

		int area();

		int x();

		int y();

		// returns the aspect ratio of the bounding box. for instand 16:9 would be 1.7778, 4:3 would be 1.3333
		float aspect();

		// returns the top left hand corner of the bounding box of the blob
		CvPoint point1();

		// returns the bottom right hand corner of the bounding box of the blob
		CvPoint point2();

		// returns the center of the bounding box of the blob
		CvPoint center();

		// returns the center of mass/centroid of the blob
		CvPoint com();

		int radius();
	};

private:
	struct Equivilence
	{	BlobExtract::Blob *blob;
		bool visited;
		Equivilence *equiv;
		std::vector<int> equals;
		int id;		
	};
	
	int *map;
	int map_size;

	void add_equivilence( std::vector<BlobExtract::Equivilence> *equivs, int a, int b );

	void create_map( IplImage *img );

	void clear_map();

public:
	BlobExtract();

	~BlobExtract();

	/* process the contents of img looking for contiguous blobs. blobs are defined as pixels that are >0.
	   this supports COI and ROI, if img is a multichannel image and no COI is set then just the first 
	   channel is processed */
	void process( IplImage *img, std::vector<BlobExtract::Blob> *blobs );
};

#endif