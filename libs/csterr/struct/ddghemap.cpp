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
// ----------------------------------------------------------------------
// General includes
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#endif

// DDG includes
#include "util/ddgerror.h"
#include "math/ddgvec.h"
#ifdef DDG
#include "math/ddgnoise.h"
#endif
#include "struct/ddghemap.h"
#include "struct/ddgimage.h"

// ----------------------------------------------------------------------
unsigned long ddgEndianTest = 0x12345678L;

ddgHeightMap::~ddgHeightMap(void)
{
	ddgMemoryFree(short,_cols*_rows*(_pixbuffer?1:0));
	delete [] _pixbuffer;
}

bool ddgHeightMap::allocate( unsigned short r, unsigned short  c )
{
	ddgMemoryFree(short,_cols*_rows*(_pixbuffer?1:0));
	_cols = c;
	_rows = r;
	delete [] _pixbuffer;
	ddgAssert(sizeof(short) == 2);
	_pixbuffer = new short[_cols*_rows];
	ddgAsserts(_pixbuffer,"Failed to Allocate memory");
	ddgMemorySet(short,_cols*_rows);
	return (_pixbuffer ? ddgSuccess : ddgFailure);
}

// ----------------------------------------------------------------------
// load:
bool ddgHeightMap::readMemory( unsigned short *buf )
{
	int i;
	for (i=0; i < _cols*_rows; i++)
		_pixbuffer[i] = buf[i];
	return ddgSuccess;
}

// ----------------------------------------------------------------------
// load:

bool ddgHeightMap::writeTGN(const char *filename)
{
	char *fullName = ddgFileHelper::getFullFileName(filename);
	FILE *fptr = filename && filename[0] ? fopen(fullName,"wb") : 0;
	if (!fptr) {
		ddgErrorSet(FileWrite,(fullName ? fullName : "(null)"));
		goto error;
	}

	// Header
	if (fwrite("TERRAGENTERRAIN SIZE",1,20,fptr) < 20)
	{
		ddgErrorSet(FileWrite,"Couldn't write header.");
		goto error;
	}

	// SIZE.
	unsigned char ch1, ch2;

	ch1 = (_cols-1) % 256;
	ch2 = ((_cols-1) - ch1) / 256;
	fputc(ch1,fptr);
	fputc(ch2,fptr);
	// Padding 2 bytes.
	fputc(0,fptr);
	fputc(0,fptr);

	// XPTS.
	if (fwrite("XPTS",1,4,fptr) < 4)
	{
		ddgErrorSet(FileWrite,"Couldn't write XPTS.");
		goto error;
	}

	ch1 = _cols % 256;
	ch2 = (_cols - ch1) / 256;
	fputc(ch1,fptr);
	fputc(ch2,fptr);
	// Padding 2 bytes.
	fputc(0,fptr);
	fputc(0,fptr);

	// YPTS.
	if (fwrite("YPTS",1,4,fptr) < 4)
	{
		ddgErrorSet(FileWrite,"Couldn't write YPTS.");
		goto error;
	}

	ch1 = _rows % 256;
	ch2 = (_rows - ch1) / 256;
	fputc(ch1,fptr);
	fputc(ch2,fptr);
	// Padding 2 bytes.
	fputc(0,fptr);
	fputc(0,fptr);

	// ALTW.
	if (fwrite("ALTW",1,4,fptr) < 4)
	{
		ddgErrorSet(FileWrite,"Couldn't write ALTW.");
		goto error;
	}

	unsigned int scale, base;
	scale = (unsigned int)(_scale * 65536);
	base = (unsigned int)_base;
	ch1 = scale % 256;
	ch2 = (scale - ch1) / 256;
	fputc(ch1,fptr);
	fputc(ch2,fptr);

	ch1 = base % 256;
	ch2 = (base - ch1) / 256;
	fputc(ch1,fptr);
	fputc(ch2,fptr);

	// Write data
	if  (*((unsigned char*)&ddgEndianTest) == 0x12)
	{
		char s;
		int i;

		for (i = 0; i < 2 * _rows * _cols; i=i+2)
		{
			s = ((char*)_pixbuffer)[i];
			((char*)_pixbuffer)[i] = ((char*)_pixbuffer)[i+1];
			((char*)_pixbuffer)[i+1] = s;
		}
	}
	int r;
	int cnt;
	r = 0;
	while (r < _rows)
	{
		if ((cnt = fwrite(&(_pixbuffer[r*_cols]),2,_cols,fptr)) <(int)_cols)
		{
			ddgErrorSet(FileWrite,(char *) (fullName ? fullName : "(null)"));
			if (feof(fptr))
			{
				ddgErrorSet(FileAccess,"Error writing to file!");
			}
			goto error;
		}
		r++;
	}
	// Swap byte order for machines with different endian.
	if  (*((unsigned char*)&ddgEndianTest) == 0x12)
	{
		char s;
		int i;

		for (i = 0; i < 2 * _rows * _cols; i=i+2)
		{
			s = _pixbuffer[i];
			_pixbuffer[i] = _pixbuffer[i+1];
			_pixbuffer[i+1] = s;
		}
	}
	// File termination segment.
	fputc(0,fptr);
	fputc(0,fptr);
	fputc(0,fptr);
	fputc(0,fptr);

	fclose(fptr);
	return ddgSuccess;

error:
	if (fptr)
		fclose(fptr);

	ddgError::report();
	return ddgFailure;
}

bool ddgHeightMap::readTGN(const void *buf, unsigned long size)
{
	bool done = false;
	unsigned char ch1, ch2;
	char name[9],type[9], segment[5];
	const char* ptr = (const char*)buf;
	const char* const bufend = ptr + size;

	// Name 8 bytes.
	memcpy(name, ptr, 8);
	ptr += 8;
	name[8] = '\0';
	memcpy(type, ptr, 8);
	ptr += 8;
	type[8] = '\0';
	if (strcmp(name,"TERRAGEN") || strcmp(type,"TERRAIN "))
	{
		return ddgFailure;
	}

	while (!done)
	{
		// Segment 4 bytes.
		segment[4] = '\0';
		memcpy(segment,ptr,4);
		ptr += 4;

		if (!strcmp(segment,"SIZE"))
		{
			// SIZE 2 bytes.
			ch1 = *ptr++;
			ch2 = *ptr++;
			_rows = _cols = ch2*256+ch1 +1;
			// Padding 2 bytes.
			ptr += 2;
		}
		else if (!strcmp(segment,"XPTS"))
		{
			// XPTS.
			ch1 = *ptr++;
			ch2 = *ptr++;
			_cols = ch2*256+ch1;
			// Padding 2 bytes.
			ptr += 2;
		}
		else if (!strcmp(segment,"YPTS"))
		{
			// YPTS.
			ch1 = *ptr++;
			ch2 = *ptr++;
			_rows = ch2*256+ch1;
			// Padding 2 bytes.
			ptr += 2;
		}
		else if (!strcmp(segment,"SCAL"))
		{
			// Ignore
			ptr += 3*sizeof(float);
 		}
		else if (!strcmp(segment,"CRAD"))
		{
			// Ignore
			ptr += 1*sizeof(float);
		}
		else if (!strcmp(segment,"CRVM"))
		{
			// Ignore
			ptr += 1*sizeof(unsigned int);
		}
		else if (!strcmp(segment,"ALTW"))
		{
			// ALTW.
			// Absolute elevation for a given point is
			// BaseHeight + elevation* Scale / 65536.
			ch1 = *ptr++;
			ch2 = *ptr++;
			_scale = (ch2*256+ch1)/ 65536.0;

			ch1 = *ptr++;
			ch2 = *ptr++;
			_base = ch2*256+ch1;

			// Read height data
			// The absolute altitude of a particular point (in the same scale as x and y)
			// is equal to BaseHeight + Elevation * HeightScale / 65536
			if (allocate(_cols,_rows) != ddgSuccess)
				goto error;

			int r = 0;
			while (r < _rows)
			{
				if (ptr + _cols * 2 > bufend)
					return ddgFailure;	// Not enough data to fill column
				memcpy(_pixbuffer + r * _cols, ptr, _cols * 2);
				ptr += _cols * 2;
				r++;
			}
			// Swap byte order for machines with different endian.
			if  (*((unsigned char*)&ddgEndianTest) == 0x12)
			{
				char s;
				int i;
				char* _pixchar = (char*)_pixbuffer;

				for (i = 0; i < 2 * _rows * _cols; i=i+2)
				{
					s = _pixchar[i];
					_pixchar[i] = _pixchar[i+1];
					_pixchar[i+1] = s;
				}
			}
			done = true;
		} // ALTW
	} // End while

	return ddgSuccess;

error:
	return ddgFailure;
}

/*
"XPTS" 4-byte marker. Must appear after the "SIZE" marker. Must appear before any altitude data. 

     The "XPTS" marker is followed by a 2-byte integer value xpts, followed by 2 bytes of padding. xpts is equal to the number of data
     points in the x-direction in the elevation image. 

"YPTS" 4-byte marker. Must appear after the "SIZE" marker. Must appear before any altitude data. 

     The "YPTS" marker is followed by a 2-byte integer value ypts, followed by 2 bytes of padding. ypts is equal to the number of data points
     in the y-direction in the elevation image. 

"SIZE" 4-byte marker, necessary. Must appear before any altitude data. 

     The "SIZE" marker is followed by a 2-byte integer value equal to n - 1, followed by 2 bytes of padding. In square terrains, n is the
     number of data points on a side of the elevation image. In non-square terrains, n is equal to the number of data points along the shortest
     side. 
     Example: a terrain with a heightfield 300 points in the x-direction and 400 points in the y-direction would have a size value of 299. 

"SCAL" 4-byte marker, optional. Must appear before any altitude data. 

     The "SCAL" marker is followed by three intel-ordered 4-byte floating point values (x,y,z). It represents the scale of the terrain in metres
     per terrain unit. Default scale is currently (30,30,30). At present, Terragen can not use non-cuboid scaling, so x, y and z must be equal. 

"CRAD" 4-byte marker, optional. Must appear before any altitude data. 

     The "CRAD" marker is followed by one intel-ordered 4-byte floating point value. It represents the radius of the planet being rendered
     and is measured in metres. The default value is 6370, which is the approximate radius of the Earth. 

"CRVM" 4-byte marker, optional. Must appear before any altitude data. 

     The "CRVM" marker is followed by one unsigned integer. Mode 0 means the terrain is rendered flat (default). Mode 1 means the terrain
     is draped (and stretched) over a sphere of radius CRAD*1000/zscale, centred at (midx, midy, -CRAD*1000/zscale), where
     midx=XSIZE/2 and midy=YSIZE/2. Terrain sizes are one less than the number of points on the side. 
     The x and y values are undistorted, therefore the map will still look normal when viewed from above, but geographic distances will be
     stretched towards the edge of the map if there is a lot of curvature. There is also an implicit limit on the size/curvature radius of a
     landscape before the landscape becomes unacceptably distorted (and wraps around onto itself!) 
     Other curve modes are currently undefined, and reserved for the future. 

"ALTW" 4-byte marker. Must appear after the "SIZE" marker. Must appear after the "XPTS" and "YPTS" markers (if they exist). 

     ALTW stands for 'Altitude in 16-bit Words'. After The "ALTW" marker, the following appear in order: 

          HeightScale, a 2-byte signed integer value. 
          BaseHeight, a 2-byte signed integer value. 
          Elevations, a sequence of 2-byte signed integers. 

     There are (xpts * ypts) elevation integers, where xpts and ypts will have been set earlier in the "SIZE" segment or the "XPTS" and
     "YPTS" segments. The elevations are ordered such that the first row (y = 0) is read first from left to right, then the second (y = 1), and so
     on. The values in Elevations are not absolute altitudes. The absolute altitude of a particular point (in the same scale as x and y) is equal to
     BaseHeight + Elevation * HeightScale / 65536. 
*/
bool ddgHeightMap::readTGN(const char *filename)
{
	bool done = false;
	char *fullName = ddgFileHelper::getFullFileName(filename);
	FILE *fptr = fullName && fullName[0] ? fopen(fullName,"rb") : 0;
	if (!fptr)
	{
		ddgErrorSet(FileRead,(char *) (fullName ? fullName : "(null)"));
		goto error;
	}
	unsigned char ch1, ch2;

	char name[9],type[9], segment[5], pad[2];

	// Name 8 bytes.
	fread(name,8,1,fptr);
	name[8] = '\0';
	fread(type,8,1,fptr);
	type[8] = '\0';
	if (strcmp(name,"TERRAGEN") || strcmp(type,"TERRAIN "))
	{
		ddgErrorSet(FileRead,(char *) "File is not a TERRAGEN TERRAIN file");
		goto error;
	}

	// Segment 4 bytes.
	segment[4] = '\0';

	while (!done)
	{
		fread(segment,4,1,fptr);

		if (!strcmp(segment,"SIZE"))
		{
			// SIZE 2 bytes.
			ch1 = fgetc(fptr);
			ch2 = fgetc(fptr);
			_rows = _cols = ch2*256+ch1 +1;
			// Padding 2 bytes.
			fread(pad,2,1,fptr);
		}

		else if (!strcmp(segment,"XPTS"))
		{
			// XPTS.
			ch1 = fgetc(fptr);
			ch2 = fgetc(fptr);
			_cols = ch2*256+ch1;
			// Padding 2 bytes.
			fread(pad,2,1,fptr);
		}
		else if (!strcmp(segment,"YPTS"))
		{
			// YPTS.
			ch1 = fgetc(fptr);
			ch2 = fgetc(fptr);
			_rows = ch2*256+ch1;
			// Padding 2 bytes.
			fread(pad,2,1,fptr);
		}
		else if (!strcmp(segment,"SCAL"))
		{
			// Ignore
			fread(pad,3,sizeof(float),fptr);
 		}
		else if (!strcmp(segment,"CRAD"))
		{
			// Ignore
			fread(pad,1,sizeof(float),fptr);
		}
		else if (!strcmp(segment,"CRVM"))
		{
			// Ignore
			fread(pad,1,sizeof(unsigned int),fptr);
		}
		else if (!strcmp(segment,"ALTW"))
		{
			// ALTW.
			// Absolute elevation for a given point is
			// BaseHeight + elevation* Scale / 65536.
			ch1 = fgetc(fptr);
			ch2 = fgetc(fptr);
			_scale = (ch2*256+ch1)/ 65536.0;

			ch1 = fgetc(fptr);
			ch2 = fgetc(fptr);
			_base = ch2*256+ch1;

			// Read height data
			// The absolute altitude of a particular point (in the same scale as x and y)
			// is equal to BaseHeight + Elevation * HeightScale / 65536
			if (allocate(_cols,_rows) != ddgSuccess)
				goto error;

			int r;
			int cnt;
			r = 0;
			while (r < _rows)
			{
				if ((cnt = fread(&(_pixbuffer[r*_cols]),2,_cols,fptr)) <
				(int)_cols)
				{
					ddgErrorSet(FileRead,(char *) (fullName ? fullName : "(null)"));
					if (feof(fptr))
					{
						ddgErrorSet(FileAccess,"Reached end of file!");
					}
					goto error;
				}
				r++;
			}
			// Swap byte order for machines with different endian.
			if  (*((unsigned char*)&ddgEndianTest) == 0x12)
			{
				char s;
				int i;

				for (i = 0; i < 2 * _rows * _cols; i=i+2)
				{
					s = ((char*)_pixbuffer)[i];
					((char*)_pixbuffer)[i] = ((char*)_pixbuffer)[i+1];
					((char*)_pixbuffer)[i+1] = s;
				}
			}
			done = true;
		} // ALTW
	} // End while
	return ddgSuccess;

error:
	if (fptr)
		fclose(fptr);
	ddgError::report();

	return ddgFailure;
}



bool ddgHeightMap::generateHeights(unsigned int r, unsigned int c, float oct )
{
#ifndef DDG
	(void)r; (void)c; (void)oct;
#else
	float lacun = 2.0;
	float fractalDim = 0.25;
	float offset = 0.7;
	double size = 3.0*r/256.0;  // sample range = 0 to 3 
	// Read data
    allocate(r,c);
	unsigned int ci, ri;
	short cc;
	double n;
	double v[3];
	v[0] = 0; v[1] = 0; v[2] = 0;
	setMinMax(0,4);
	for (ri = 0; ri < r; ri++)
	{
		v[1] = 0.0f;
		for (ci = 0; ci < c; ci++)
		{
			n = ddgNoise::hybridmultifractal(v,fractalDim,lacun,oct,offset);	// Returns 0 to 4.
			cc = iconvert(n);
			set(ri,ci, cc);
			v[1]+= size/c;
		}
	v[0]+= size/r;
	}
#endif
	return ddgSuccess;
}

void ddgHeightMap::canyonize(float f )
{
	f += 1;
	for (unsigned r = 0; r < _rows; r++)
		for (unsigned c = 0; c < _cols; c++ )
		{
			unsigned int d = get(r,c);
            unsigned int d2 = (unsigned int)ddgUtil::clamp(pow(d,f),-0x7FFF,0x7FFF);
			set(r,c,d2);
		}
}

void ddgHeightMap::glaciate(float f )
{
	f *= 0xFFFF;
	for (unsigned r = 1; r+1 < _rows; r++)
		for (unsigned c = 1; c+1 < _cols; c++ )
		{
			unsigned int d = get(r,c),
                        dn = (get(r-1,c)+ get(r+1,c)+ get(r,c-1)+ get(r,c+1))/4;
            if (fabs(d-dn) < f)
            {
//                unsigned int d2 = Util::clamp(pow(d,f),-0x7FFF,0x7FFF);
	    		set(r,c,dn);
            }
		}
}

void ddgHeightMap::scale(float s )
{
	for (unsigned r = 0; r < _rows; r++)
		for (unsigned c = 0; c < _cols; c++ )
		{
            unsigned int d = (unsigned int)ddgUtil::clamp(s * get(r,c),-0x7FFF,0x7FFF);
			set(r,c,d);
		}
}

void ddgHeightMap::translate(float t )
{
	for (unsigned r = 0; r < _rows; r++)
		for (unsigned c = 0; c < _cols; c++ )
		{
            unsigned int d = (unsigned int)ddgUtil::clamp(t + get(r,c),-0x7FFF,0x7FFF);
			set(r,c,d);
		}
}

int ddgHeightMap::mini(void )
{
	int m = get(0,0);
	for (unsigned r = 0; r < _rows; r++)
		for (unsigned c = 0; c < _cols; c++ )
		{
			int d = get(r,c);
			if (d < m) m = d;
		}
	return m;
}

int ddgHeightMap::maxi(void )
{
	int m = get(0,0);
	for (unsigned r = 0; r < _rows; r++)
		for (unsigned c = 0; c < _cols; c++ )
		{
			int d = get(r,c);
			if (d > m) m = d;
		}
	return m;
}

void ddgHeightMap::setmin( int m)
{
	for (unsigned r = 0; r < _rows; r++)
		for (unsigned c = 0; c < _cols; c++ )
		{
			if (get(r,c) < m) set(r,c,m);
		}
}

void ddgHeightMap::setmax( int m)
{
	for (unsigned r = 0; r < _rows; r++)
		for (unsigned c = 0; c < _cols; c++ )
		{
			if (get(r,c) > m) set(r,c,m);
		}
}

void ddgHeightMap::closeEdge(float l)
{
	l = iconvert(l);
	for (unsigned r = 0; r < _rows; r++)
	{
		set(r,0,(short)l);
		set(r,_cols-1,(short)l);
	}
	for (unsigned c = 0; c < _cols; c++ )
	{
		set(0,c,(short)l);
		set(_rows-1,c,(short)l);
	}
}

void ddgHeightMap::sin(void)
{
	for (unsigned r = 0; r < _rows; r++)
		for (unsigned c = 0; c < _cols; c++ )
		{
			set(r,c,(short)(10000*(ddgAngle::sin(180.0*(float)r/(float)_rows)
				         + ddgAngle::sin(180.0*(float)c/(float)_cols))));
		}
}

// Save image of height field.
bool ddgHeightMap::saveAsTGA(const char *s)
{
	ddgImage * img = new ddgImage(rows(),cols(),1);
	unsigned int r, c;
	unsigned short f;
	for (r = 0; r < rows(); r++)
	for (c = 0; c < cols(); c++)
	{
		f = get(r,c);
		img->set(r,c,(unsigned char) (f/256.0));
	}
	return img->writeTGA(s);
	return ddgSuccess;
}

#ifdef DDGDEM
// Terrain loaders based on Ben Discoe's code.
#include "dem/ddgegrid.h"
#endif

bool ddgHeightMap::loadTerrain(const char *filename)
{
	if ( (readTGN(filename) == ddgFailure)
	/*	&& (readPGM(filename,0x7FFF) == ddgFailure )*/ )
#ifdef DDGDEM
	{
		// Try one of the other formats.
		ElevationGrid *eg = new ElevationGrid;
		char *fullName = ddgFileHelper::getFullFileName(filename);

		// Try to get one of the formats to load the file.
		if (eg->LoadFromBT(fullName)
		 &&	eg->LoadFromDEM( fullName)
		 && eg->LoadFromDTED(fullName)
		 && eg->LoadFromGTOPO30(fullName)
		)
			goto error;

		int		gr,gc, i,j;
		float	v;
		unsigned short r,c;
		eg->GetDimensions(gc, gr);
		if (gc == 0 || gr == 0)
			goto error;

		// TODO calc based on grid corners/heightdelta.
		ddgConsole::s("Loaded DEM "); ddgConsole::s(eg->GetName() ); ddgConsole::end();

		setMinMax(eg->m_fMinHeight, eg->m_fMaxHeight);

		r = gr;
		c = gc;
		if (allocate(r,c)== ddgFailure)
			goto error;

		for (i = 0; i < r; i++)
		{
			for (j = 0; j < c; j++)
			{
				// Scale and rebase to 0.
				v = eg->GetFValue(i,j);
				if (v == INVALID_ELEVATION) v = 0;
				set(i,j,iconvert(v));
			}
		}
		ddgConsole::s("Terrain loaded "); ddgConsole::i(rows()); ddgConsole::s("x");
		ddgConsole::i(cols()); ddgConsole::end();
		delete eg;
	}
	return ddgSuccess;
#else
		goto error;
	else
		return ddgSuccess;
#endif
error:
	return ddgFailure;
}

//
// The following code uses code from Klaus Hartman's heighfield generator demo.
//
#ifdef DDGSPECTRAL
#include "ext/ddghfld.h"
#endif
bool ddgHeightMap::createSpectralMap(int level, float smoothness)
{
#ifdef DDGSPECTRAL
	CHeightField	m_heightField;
	int		i,j;
	float	v ;

	unsigned short r,c;

	m_heightField.CreateSpectral(level, smoothness);
	r = m_heightField.GetFieldWidth();
	c = r;
	allocate(r,c);
	setMinMax(m_heightField.GetMinHeight(), m_heightField.GetMaxHeight());

	for (i = 0; i < r; i++)
	{
		for (j = 0; j < c; j++)
		{
			// Scale and rebase to 0.
			v = m_heightField.GetField()[i * c + j];
			set(i,j,iconvert(v));
		}
		ddgConsole::progress("Generating Height field",i,r-1);
	}
	ddgConsole::end();
    // Success
    return ddgSuccess;
#else
    (void)level; (void)smoothness;
    return ddgFailure;
#endif
}

/*
    Copyright (C) 1999 Ryan Willhoit
*/
#ifdef DDGPGM
#define PGM_MAGIC "P2"

class WordIterator
{
	protected:
		const char* string_pos;
		char* token;
		int token_length;
		const char* delimiters;
		
	public:
		/*
			Creates a new iterator using string as the source
			and delim as the list of characters that seperate
			words
		*/
		WordIterator(const char* string, const char* delim);
		~WordIterator();
		
		/*
			Returns the next word, or 0 if there are none.
		*/
		char* nextWord();
};
WordIterator::WordIterator(const char* string, const char* delim)
{
	string_pos = string;
	delimiters = delim;
	token_length = 64;
	token = new char[token_length];
};

WordIterator::~WordIterator()
{
	if (token)
	{
		delete token;
	};
};

char* WordIterator::nextWord()
{
	if (string_pos)
	{
		int len = 0;

		//Strip delims
		len = strspn(string_pos, delimiters);
		string_pos += len;

		//Read in non-delims
		len = strcspn(string_pos, delimiters);
		if (token_length <= len)
		{
			delete token;
			token = new char[len + 1];
		};

		if (len == 0)
		{
			return 0;
		};
		
		strncpy(token, string_pos, len);
		token[len] = '\0';

		string_pos += len;

		return token;
	}
	else
	{
		return 0;
	};
};
#endif
/*
	Some notes about this function:
	1. It sets scale to height/width and base to 0
	2. The magic for an ascii PGM is that the first two characters should be "P2"
	3. The values in the map are between 0 and max_grey and are then scaled
	   to fit between 0 and desired_max
	4. The reason you have to work from a char* is that the VFS 
	   provides only a read function, which while this is enough to implement
	   a nice interface on top of it, that's not the focus of this code
	   ScanStr could have been used but in general is very limited and would
	   have required a lot of extra work...
	5. The PGM spec doesn't set limits on the values in the file, this function
	   can only accept up to something small depending on the value
	   of desired_max
	6. Error reporting is only via the return value but is commented :)
	7. PGMs aren't really meant to be fast, and this function isn't either
*/
bool ddgHeightMap::readPGM(const char* data, int desired_max)
{
#ifdef DDGPGM
	int	max_grey;
	int width;
	int height;
	
	char* token;
	
	WordIterator st(data, " \n\t");

	token = st.nextWord();
	if (!strncmp(token, PGM_MAGIC, strlen(PGM_MAGIC)))
	{
		token = st.nextWord();
		width = token ? atoi(token) : 0;
		
		token = st.nextWord();
		height = token ? atoi(token) : 0;
		
		token = st.nextWord();
		max_grey = token ? atoi(token) : 0;

		if ((max_grey < 1) || (width < 1) || (height < 1))
		{
			//ERROR: Obviously not well formed...
			return ddgFailure;
		};
		
		allocate(height, width);
		setMinMax( 0, max_grey);
	
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				int temp;
				short val;
				
				token = st.nextWord();
				temp = token ? atoi(token) : 0;

				/* @@@ This could overflow... how do we want to solve this?*/
				val = (short)((temp * desired_max) / max_grey);

				set(x, y, val);
			};
		};
		
	}
	else
	{
		//ERROR: Invalid magic, not an ASCII PGM
		return ddgFailure;
	};

	return ddgSuccess;
#else
	(void)data; (void)desired_max;
	return ddgFailure;
#endif
};

void ddgHeightMap::vertexNormal(ddgVector3& normal, long row, long col, float gridSpacing)
{
    float denom, nx, nz;

    // Compute x-component of normal
    if (col > 0 && col < _cols - 1)
    {
        nx = getf(row, col - 1) - getf(row, col + 1);
    }
    else if (col > 0) 
    {
        nx = 2.0f * (getf(row, col - 1) - getf(row, col));
    }
    else 
    {
        nx = 2.0f * (getf(row, col) - getf(row, col + 1));
    }

    // Compute z-component of normal
    if (row > 0 && row < _rows - 1)
    {
        nz = getf(row - 1, col) - getf(row + 1, col);
    }
    else if (row > 0)
    {
        nz = 2.0f * (getf(row - 1, col) - getf(row, col));
    }
    else 
    {
        nz = 2.0f * (getf(row, col) - getf(row + 1, col));
    }

    // Normalize the normal
    gridSpacing *= 2.0f;
    denom = 1.0f / ::sqrtf(nx * nx + gridSpacing * gridSpacing + nz * nz);
    normal.v[0] = nx * denom;
    normal.v[1] = gridSpacing * denom;
    normal.v[2] = nz * denom;
}
