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
#ifndef _ddgImage_Class_
#define _ddgImage_Class_

#include "util/ddgerror.h"
#include "util/ddgutil.h"

/**
 * A convenience object to store a 2D image.
 * Images can be read from .rgb and TGA files.
 * An image can store an arbitrary 2D set of data.
 */
class WEXP ddgImage {
	/// Memory block of image data row major.
	unsigned char*	_pixbuffer;
	/// 1,2,3,4 == 8,16,24,32 bits per pixel.
	unsigned short	_channels;
	/// Allocated size.
	unsigned short	_cols;
	/// Allocated size.
	unsigned short	_rows;
    /// Limits for a given channel.
    static unsigned int _maxval[5];
	/** Load an image buffer from a file. (Special case)
	 * Return true on error.
	 */
	bool _readBWold(char *filename);
public:
	/**
	 *  Create an image but don't allocate memory.
	 * ch is the number of channels (1-4).
	 * The t flag is true if this is an OpenGL texture which needs to be a power of 2.
	 */
	ddgImage( unsigned short ch = 4)
		: _pixbuffer(0), _channels(ch) {}
	/// Create an image and allocate memory.
	ddgImage( unsigned short r, unsigned short c, unsigned short ch = 4)
		: _pixbuffer(0), _channels(ch) { allocate(r,c); }
	/// Destroy and image.
	~ddgImage();
	/// Return the data block.
	unsigned char *buffer(void) { return _pixbuffer; }
	/// Return the number of channels in the image.
	unsigned short channels(void) { return _channels; }
	/// Set the number of channels in the image.
	void channels(unsigned short c ) { _channels = c; }
	/// Return the width of the image.
	unsigned short cols(void) { return _cols; }
	/// Return the height of the image.
	unsigned short rows(void) { return _rows; }
	/// Allocate a image buffer to be filled by the user.  Return true on error.
	bool allocate( unsigned short r, unsigned short c, unsigned short ch = 0 );
	/// Set a data entry in the image.
	void set(unsigned short r, unsigned short c,
		unsigned char d1,
		unsigned char d2 = 0,
		unsigned char d3 = 0,
		unsigned char d4 = 0)
	{
		switch (_channels)
		{
		case 4:
			_pixbuffer[_channels*(r*_cols+c)+3] = d4;
		case 3:
			_pixbuffer[_channels*(r*_cols+c)+2] = d3;
		case 2:
			_pixbuffer[_channels*(r*_cols+c)+1] = d2;
		case 1:
			_pixbuffer[_channels*(r*_cols+c)+0] = d1;
			break;
		}
	}
	/// Get a data entry in the image.
	void get(unsigned short r, unsigned short c,
		unsigned char *d1,
		unsigned char *d2 = 0,
		unsigned char *d3 = 0,
		unsigned char *d4 = 0)
	{
		switch (_channels)
		{
		case 4:
			*d4 = _pixbuffer[_channels*(r*_cols+c)+3];
		case 3:
			*d3 = _pixbuffer[_channels*(r*_cols+c)+2];
		case 2:
			*d2 = _pixbuffer[_channels*(r*_cols+c)+1];
		case 1:
			*d1 = _pixbuffer[_channels*(r*_cols+c)+0];
			break;
		}
	}
	/// Get a pointer to a data entry in the image.
	unsigned char *get(unsigned short r, unsigned short c, unsigned char **p)
	{
		return *p = &_pixbuffer[_channels*(r*_cols+c)];
	}
    /// Copy a value from one place to another.
    void copy(unsigned short sr, unsigned short sc,unsigned short dr, unsigned short dc)
    {
		unsigned int sb = _channels*(sr*_cols+sc),
					 db = _channels*(dr*_cols+dc);
		switch (_channels)
		{
		case 4:
			_pixbuffer[db+3] = _pixbuffer[sb+3];
		case 3:
			_pixbuffer[db+2] = _pixbuffer[sb+2];
		case 2:
			_pixbuffer[db+1] = _pixbuffer[sb+1];;
		case 1:
			_pixbuffer[db+0] = _pixbuffer[sb+0];;
			break;
		default:
			ddgAsserts(0,"Invalid number of channels in image");
		}
    }

	/// Copy block from one image to another.
	void import( ddgImage * isrc,
		unsigned int sc, unsigned int sr,
		unsigned int dc, unsigned int dr,
		unsigned int nc, unsigned int nr)
	{
		unsigned char d1, d2, d3, d4;
		for (unsigned int r = 0; r < nr; r++)
			for (unsigned int c = 0; c < nc; c++)
			{
				// TODO: Optimize using memcpy.
				isrc->get(sc+c,sr+r,&d1,&d2,&d3,&d4);
				set(dc+c,dr+r,d1,d2,d3,d4);
			}
	}

	/** Load an image buffer from a memory buffer.
	 * Return true on error.
	 */
	bool readMemory( unsigned char *buf );

	/** Load an image buffer from a file try to determine the file
	 *  automatically.
	 */
	bool readFile( ddgStr *file );

	/** Load an image buffer from an SGI RGB file.
	 */
	bool readRGB( const char *filename);
	/** Load an image buffer from a TGA file.
	 * supports 8 bit color index black and white
	 * and 24 bit rgb images.
	 */
	bool readTGA(const char *filename);
    /** Load an image buffer from a PNG file.
     */
    bool readPNG(const char *filename);
	/** Load an image buffer from a file.
	 */
	bool readBW(const char *filename);
	/**
	 * Save an image buffer from a file.
	 */
	bool writeTGA(const char *filename);
	/**
	 * Save an image buffer to a file.
	 */
	bool writePNG(const char *filename);
	/**
	 * Add an alpha component to an RGB image
	 * alpha map will be transparent where the specific
	 * color is present and solid otherwise.
	 * If no color is specified, the color at location
	 * 0,0 is used.
	 */
	bool mapColorToAlpha( unsigned char *color = NULL);
	/**
	 * Convert a LUMINANCE image to a full RGBA texture
	 */
	bool mapLuminanceToRGBA( void );
	/**
	 * Convert a LUMINANCE image to a full RGB texture
	 */
	bool mapLuminanceToRGB( void );
};

#endif
