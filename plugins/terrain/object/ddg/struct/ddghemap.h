/*
    Copyright (C) 1997, 1998, 1999, 2000 by Alex Pfaffe
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

#include "util/ddgerror.h"
#include "math/ddgvec.h"

/**
 * A 16bit heightmap object which can be read from various file formats.
 * Also includes the ability to generate noise based maps.
 */
class WEXP ddgHeightMap {
	/// Memory block of image data row major.
	short*	_pixbuffer;
	/// Allocated size.
	unsigned short	_cols;
	/// Allocated size.
	unsigned short	_rows;
	/// Height/Width ratio.
	float _scale;
	/// Base offset.
	float _base;
public:
	/**
	 *  Create an image but don't allocate memory.
	 */
	ddgHeightMap( void )
		: _pixbuffer(0), _scale(1), _base(0) {}
	/// Create an image and allocate memory.
	ddgHeightMap( unsigned short r, unsigned short c)
		: _pixbuffer(0), _scale(1), _base(0) { allocate(r,c); }
	/// Destroy and image.
	~ddgHeightMap();
	/// Return the data block.
	short *buffer(void) { return _pixbuffer; }
	/// Return the width of the image.
	unsigned short cols(void) { return _cols; }
	/// Return the height of the image.
	unsigned short rows(void) { return _rows; }
	/// Allocate a image buffer to be filled by the user.
	bool allocate( unsigned short r, unsigned short c);
	/// Set a data entry in the image.
	void set(unsigned short r, unsigned short c,
		short d1)
	{
		ddgAssert(r < _rows && c < _cols);
		_pixbuffer[(r*_cols+c)+0] = d1;
	}
	/// Get a data entry in the image.
	short get(unsigned short r, unsigned short c,
		short *d1 = 0)
	{
		ddgAssert(r < _rows && c < _cols);
		short s = _pixbuffer[(r*_cols+c)+0];
		if (d1) *d1 = s;
		return s;
	}
	/// Transform a value from  short space back to real float space.
	static inline float sconvert(short n, float base, float scale)
	{
		return (n + 0x7FFF) * scale + base;
	}
	/// Inverse transform a value from real float to short space.
	static inline short siconvert(float n, float base, float scale)
	{
		return (short) ddgUtil::clamp((n - base ) / scale - 0x7FFF,-0x7FFF,0x7FFF);
	}
	/// Inverse transform a value from real float to short space.
	inline short iconvert(float n)
	{
		return siconvert(n, _base, _scale );
	}
	/// Transform a value from short space back to real float space.
	inline float convert(short n)
	{
		return sconvert(n, _base, _scale);
	}
	/// Get a transformed value in the image.
	float getf(unsigned short r, unsigned short c,
		float *d1 = 0)
	{
		ddgAssert(r < _rows && c < _cols);
		float s = convert( _pixbuffer[(r*_cols+c)+0] );
		if (d1) *d1 = s;
		return s;
	}
	/** Load an heightmap buffer from a memory buffer.
	 * Return true on error.
	 */
	bool readMemory( unsigned short *buf );

	/// Try all ways to load height map from TGN/BT/DEM/DTED/GTOPO30 file
	bool loadTerrain(const char *filename);

	/** Read an image buffer from a Terragen file image.
	 * Return true on error.
	 */
	bool readTGN(const void *buf, unsigned long size);

	/** Read an image buffer from a Terragen file.
	 * Return true on error.
	 */
	bool readTGN(const char *filename);

	/** Write an image buffer to a Terragen file.
	 * Return true on error.
	 */
	bool writeTGN(const char *filename);
	/**
	 * Load a PGM file as a height map.
	 */
	bool readPGM(const char* data, int desired_max);

	/** Generate a height map and fill this image with the
	 *  contents.
	 */
	bool generateHeights(unsigned int r, unsigned int c, float octaves);

	/// Process the height map to produce rougher features. strength = 0-1.
	void canyonize(float strength );
	/// Process the height map to produce smoother features. strength = 0-1.
	void glaciate(float strength );
	/// Set scale and base based on the Min Max of the input values, call before loading pixbuffer.
	void setMinMax(float b = 0, float d = 1) { _scale = (d-b)/0xFFFF; _base = b;  }
	/// Set scale and base.
	void setScaleAndBase(float s, float b = 0) { _scale = s*_scale; _base += b;  }
	/// Return the scale of the height map.
	float scale(void) { return _scale; }
	/// Return the base height of the map.
	float base(void) { return _base; }
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
	void setmaxf( float m) { setmax( iconvert(m)); }
	/// Set min value.
	void setminf( float m) { setmin( iconvert(m)); }
	/// Pull the edge of the heightmap down to l.
	void closeEdge(float l);
	/// Initialize with a sin curve
	void sin(void);
	/// Save height map as TGA image.
	bool saveAsTGA(const char *filename);
	/// Create height map from spectral wave generator.
	bool createSpectralMap(int level, float smoothness);
	/// Calculate the surface normal at a location on the map.
	void vertexNormal(ddgVector3& normal, long row, long col, float gridSpacing);

};

#endif
