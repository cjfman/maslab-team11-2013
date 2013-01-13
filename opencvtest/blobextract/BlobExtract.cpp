#include "BlobExtract.h"

BlobExtract::Blob::Blob( int _id )
{   this->id = _id;

	this->l = -1;
	this->r = -1;
	this->t = -1;
	this->b = -1;

	this->x_val = 0;
	this->y_val = 0;

	this->pixels = 0;
}

void BlobExtract::Blob::addPixel( int _x, int _y )
{   if( _x < this->l || this->l == -1 ) this->l = _x;
    if( _x > this->r ) this->r = _x;

	if( _y < this->t || this->t == -1 ) this->t = _y;
	if( _y > this->b ) this->b = _y;

	this->pixels++;

	this->x_val += _x;
	this->y_val += _y;
}

int BlobExtract::Blob::width()
{	return this->r - this->l;
}

int BlobExtract::Blob::height()
{	return this->b - this->t;
}

int BlobExtract::Blob::area()
{	return this->width() * this->height();
}

int BlobExtract::Blob::x()
{	//return ( this->l + this->r ) /2;
	return (int)( this->x_val / this->pixels );
}

int BlobExtract::Blob::y()
{	//return ( this->t + this->b ) /2;
	return (int)( this->y_val / this->pixels );
}

float BlobExtract::Blob::aspect()
{	return float(this->width()) / float(this->height());
}

CvPoint BlobExtract::Blob::point1()
{	return cvPoint( this->l, this->t );
}

CvPoint BlobExtract::Blob::point2()
{	return cvPoint( this->r, this->b );
}

CvPoint BlobExtract::Blob::center()
{	return cvPoint( (this->l+this->r)/2, (this->t+this->b)/2 ); 
}

CvPoint BlobExtract::Blob::com()
{	return cvPoint( this->x(), this->y() );
}

int BlobExtract::Blob::radius()
{	return ( this->width() < this->height() ? this->width() : this->height() ) /2;
}

BlobExtract::BlobExtract()
{	this->map = 0;
	this->map_size = 0;
}

BlobExtract::~BlobExtract()
{	if( this->map ) delete this->map;
}

void BlobExtract::add_equivilence( std::vector<BlobExtract::Equivilence> *equivs, int a, int b )
{	unsigned int i;
	bool found = false;

	for( i=0; !found && i<(*equivs)[a].equals.size(); ++i )
		if( (*equivs)[a].equals[i] == b )
			found = true;
		
	if( !found )
	{	(*equivs)[a].equals.push_back( b );
		(*equivs)[b].equals.push_back( a );
	}
}

void BlobExtract::clear_map()
{	int i;
	
	for( i=0; i<this->map_size; ++i )
		this->map[i] = -1;
}

void BlobExtract::create_map( IplImage *img )
{	if( this->map == 0 || this->map_size != img->imageSize )
	{
		if( this->map ) delete this->map;

		this->map_size = img->imageSize;
		this->map = new int[this->map_size];
	}		
}

void BlobExtract::process( IplImage *img, std::vector<BlobExtract::Blob> *blobs )
{	bool new_blob;
	BlobExtract::Equivilence *e;
	int x, y;
	std::queue<BlobExtract::Equivilence*> to_visit;
	std::vector<BlobExtract::Equivilence> equivs;
	int l[5];
	int i, j;
	int blob_counter = 0;
	int channel;
	CvRect roi;
	int x_min, y_min, x_max, y_max;

	roi = cvGetImageROI( img );
	x_min = roi.x;
	y_min = roi.y;
	x_max = roi.x + roi.width;
	y_max = roi.y + roi.height;

	/* get the channel that is going to be processed, cvGetImageCOI returns 0
	   for all channels, if that is the case then just process the first channel */
	channel = cvGetImageCOI( img );
	if( channel != 0 )
		channel -= 1;

#if defined _DEBUG
	assert( channel < img->nChannels );
#endif

	this->create_map( img );
	this->clear_map();
	blobs->clear();

	for( y=y_min; y<y_max; ++y )
	{	for( x=x_min; x<x_max; ++x )
		{	l[4] = BE_PIXEL_I( img, x,   y,   channel ); // +------+------+------+
			l[3] = BE_PIXEL_I( img, x-1, y,   channel ); // | l[0] | l[1] | l[2] |
			l[1] = BE_PIXEL_I( img, x,   y-1, channel ); // +------+------+------+
			l[2] = BE_PIXEL_I( img, x+1, y-1, channel ); // | l[3] | l[4] |
			l[0] = BE_PIXEL_I( img, x-1, y-1, channel ); // +------+------+

			if( (uchar)img->imageData[l[4]] != 0 )
			{	new_blob = true;

				if( y>y_min )
				{	if( x>x_min && this->map[l[0]] != -1 )
					{	new_blob = false;

						this->map[l[4]] = this->map[l[0]];
					}

					if( this->map[l[1]] != -1 )
					{	new_blob = false;

						if( this->map[l[4]] == -1 )
							this->map[l[4]] = this->map[l[1]];

						if( this->map[l[4]] != this->map[l[1]] )
							this->add_equivilence( &equivs, this->map[l[4]], this->map[l[1]] );
					}

					if( x<x_max-1 && this->map[l[2]] != -1 )
					{	new_blob = false;

						if( this->map[l[4]] == -1 )
							this->map[l[4]] = this->map[l[2]];

						if( this->map[l[4]] != this->map[l[2]] )
							this->add_equivilence( &equivs, this->map[l[4]], this->map[l[2]] );
					}
				}

				if( x>x_min && this->map[l[3]] != -1 )
				{	new_blob = false;

					if( this->map[l[4]] == -1 )
						this->map[l[4]] = this->map[l[3]];

					if( this->map[l[4]] != this->map[l[3]] )
						this->add_equivilence( &equivs, this->map[l[4]], this->map[l[3]] );
				}

				if( new_blob )
				{	this->map[l[4]] = blob_counter;

					equivs.push_back( Equivilence() );
					equivs.back().id = blob_counter;

					blob_counter++;
				}
			}
		}
	}

	for( i=0; i<(int)equivs.size(); ++i )
		equivs[i].equiv = &(equivs[i]);

	// collapse the equiv values down
	for( i=0; i<(int)equivs.size(); ++i )
	{	for( j=0; j<(int)equivs.size(); ++j )
			equivs[j].visited = false;

		to_visit.push( &(equivs[i]) );

		while( !to_visit.empty() )
		{	e = to_visit.front();
			to_visit.pop();

			if( !e->visited )
			{	e->visited = true;

				if( e->id < equivs[i].equiv->id )
					equivs[i].equiv = e;

				for( j=0; j<(int)e->equals.size(); ++j )
				{	to_visit.push( &(equivs[e->equals[j]]) );
				}
			}
		}
	}

	// create the blobs
	for( i=0; i<(int)equivs.size(); ++i )
		if( &(equivs[i]) == equivs[i].equiv )
			blobs->push_back( BlobExtract::Blob( equivs[i].equiv->id ) );

	// store pointers to the correct blob in each equiv
	for( i=0; i<(int)blobs->size(); ++i )
	{	equivs[ (*blobs)[i].id ].blob = &((*blobs)[i]);
		(*blobs)[i].id = 0;
	}
			
	// get the information for each blob
	for( y=y_min; y<y_max; ++y )
		for( x=x_min; x<x_max; ++x )
		{	i = BE_PIXEL_I(img,x,y,channel);
			i = this->map[i];

			if( i != -1 )
				equivs[i].equiv->blob->addPixel( x, y );
		}
}