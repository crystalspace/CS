/*
    Copyright (C) 1997, 1998, 1999 by Alex Pfaffe
	(Digital Dawn Graphics Inc)
  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
  
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef _ddgHeightMap_Class_
#define _ddgHeightMap_Class_

#include "csterr/ddg.h"
#include "types.h"

/**
 * A 16bit heightmap object which can be read from terragen file.
 * Also includes the ability to generate noise based maps.
 */
class WEXP ddgHeightMap {
	/// Memory block of image data row major.
	short*	_pixbuffer;
	/// Allocated size.
	unsigned short	_cols;
	/// Allocated size.
	unsigned short	_rows;
	/// Base offset.
	float _base;
	/// Height/Width ratio.
	float _scale;
public:
	/**
	 *  Create an image but don't allocate memory.
	 */
	ddgHeightMap( void )
		: _pixbuffer(0),_base(0), _scale(1) {}
	/// Create an image and allocate memory.
	ddgHeightMap( unsigned short r, unsigned short c)
		: _pixbuffer(0),_base(0), _scale(1) { allocate(r,c); }
	/// Destroy and image.
	~ddgHeightMap();
	/// Return the data block.
	short *buffer(void) { return _pixbuffer; }
	/// Return the width of the image.
	unsigned short cols(void) { return _cols; }
	/// Return the height of the image.
	unsigned short rows(void) { return _rows; }
	/// Allocate a image buffer to be filled by the user.
	void allocate( unsigned short r, unsigned short c);
	/// Set a data entry in the image.
	void set(unsigned short r, unsigned short c,
		short d1)
	{
		_pixbuffer[(r*_cols+c)+0] = d1;
	}
	/// Get a data entry in the image.
	short get(unsigned short r, unsigned short c,
		short *d1 = 0)
	{
		short s = _pixbuffer[(r*_cols+c)+0];
		if (d1) *d1 = s;
		return s;
	}
	/// Get a transformed value in the image.
	float getf(unsigned short r, unsigned short c,
		float *d1 = 0)
	{
		float s = _base + _pixbuffer[(r*_cols+c)+0] *_scale;
		if (d1) *d1 = s;
		return s;
	}
	/// Transform a value.
	float convert(float n)
	{
		return _base + n *_scale;
	}
	/// Inverse transform a value.
	float iconvert(float n)
	{
		return (n - _base) /_scale;
	}
	/** Load an image buffer from a memory buffer.
	 * Return true on error.
	 */
	bool readMemory( unsigned short *buf );

	/** Read an image buffer from a Terragen file.
	 * Return true on error.
	 */
	bool readTGN(char *file);

	/** Write an image buffer to a Terragen file.
	 * Return true on error.
	 */
	bool writeTGN(char *file, unsigned int base, unsigned int scale);
	/** Generate a height map and fill this image with the
	 *  contents.
	 */
	bool generateHeights(unsigned int r, unsigned int c, float octaves);

	/// Process the height map to produce rougher features. strength = 0-1.
	void canyonize(float strength );
	/// Process the height map to produce smoother features. strength = 0-1.
	void glaciate(float strength );
	/// Set scale and base.
	void setScaleAndBase(float s, float b = 0) { _scale = s; _base = b; }
	/// Scale the height map.
	void scale(float s );
	/// Translate the height map.
	void translate(float t );
	/// Find min value.
	int mini(void);
	/// Find max value.
	int maxi(void);
	/// Set max value.
	void setmax( int m);
	/// Set min value.
	void setmin( int m);
	/// Scale the height map.
	void scalef(float s ) { scale( iconvert(s)); }
	/// Translate the height map.
	void translatef(float t ) { translate( iconvert(t)); }
	/// Find min value.
	float minf(void)  { return convert(mini()); }
	/// Find max value.
	float maxf(void)  { return convert(maxi()); }
	/// Set max value.
	void setmaxf( float m) { setmax( int (iconvert(m))); }
	/// Set min value.
	void setminf( float m) { setmin( int (iconvert(m))); }
	/// Pull the edge of the heightmap down to 0.
	void closeEdge(void);
	/// Initialize with a sin curve
	void sin(void);

};

#endif
