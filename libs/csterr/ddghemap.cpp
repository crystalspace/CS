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
// ----------------------------------------------------------------------
// General includes
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#endif
// DDG includes
#ifdef DDG
#include "ddghemap.h"
#include "ddgvec.h"
#include "ddgnoise.h"
#include "ddgimage.h"
#include "ddgerror.h"
#else
#include "sysdef.h"
// DDG includes
#include "csterr/ddgutil.h"
#include "csterr/ddghemap.h"
#include "csterr/ddgnoise.h"
#endif

// ----------------------------------------------------------------------
unsigned long ddgEndianTest = 0x12345678L;

ddgHeightMap::~ddgHeightMap(void)
{
	delete _pixbuffer;
}

bool ddgHeightMap::allocate( unsigned short r, unsigned short  c )
{
	_cols = c;
	_rows = r;
	delete _pixbuffer;
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
	FILE *fptr = filename && filename[0] ? fopen(filename,"wb") : 0;
	if (!fptr) {
		ddgErrorSet(FileWrite,(filename ? filename : "(null)"));
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
			s = _pixbuffer[i];
			_pixbuffer[i] = _pixbuffer[i+1];
			_pixbuffer[i+1] = s;
		}
	}
	int r;
	int cnt;
	r = 0;
	while (r < _rows)
	{
		if ((cnt = fwrite(&(_pixbuffer[r*_cols]),2,_cols,fptr)) <(int)_cols)
		{
			ddgErrorSet(FileWrite,(char *) (filename ? filename : "(null)"));
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
#ifdef DDG
	ddgError::report();
#endif
	return ddgFailure;
}

bool ddgHeightMap::readTGN(const void *buf, unsigned long size)
{
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
		return true;
	}

	// Segment 4 bytes.
	segment[4] = '\0';
	memcpy(segment, ptr, 4);
	ptr += 4;

	if (strcmp(segment,"SIZE")==0)
	{
		// SIZE 2 bytes.
		ch1 = *ptr++;
		ch2 = *ptr++;
		_rows = _cols = ch2*256+ch1 +1;
		// Padding 2 bytes.
		ptr += 2;
		memcpy(segment, ptr, 4);
		ptr += 4;
	}

	if (strcmp(segment,"XPTS")==0)
	{
		// XPTS.
		ch1 = *ptr++;
		ch2 = *ptr++;
		_cols = ch2*256+ch1;
		// Padding 2 bytes.
		ptr += 2;

		memcpy(segment, ptr, 4);
		ptr += 4;
		// YPTS.
		if (strcmp(segment,"YPTS")==0)
		{
			ch1 = *ptr++;
			ch2 = *ptr++;
			_rows = ch2*256+ch1;
			// Padding 2 bytes.
			ptr += 2;
			memcpy(segment, ptr, 4);
			ptr += 4;
		}
		else
		{
			return true;
		}
	}

	// ALTW.
	// Absolute elevation for a given point is
	// BaseHeight + elevation* Scale / 65536.
	if (strcmp(segment,"ALTW")==0)
	{
		ch1 = *ptr++;
		ch2 = *ptr++;
		_scale = (ch2*256+ch1)/ 65536.0;

		ch1 = *ptr++;
		ch2 = *ptr++;
		_base = ch2*256+ch1;
	}
	else
	{
		return true;
	}

	// Read height data
	// The absolute altitude of a particular point (in the same scale as x and y)
	// is equal to BaseHeight + Elevation * HeightScale / 65536
	allocate(_cols,_rows);
	int r = 0;
	while (r < _rows)
	{
		if (ptr + _cols * 2 > bufend)
			return true;	// Not enough data to fill column
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

	return false;
}



bool ddgHeightMap::readTGN(const char *file)
{
	FILE *fptr = file && file[0] ? fopen(file,"rb") : 0;
	if (!fptr)
	{
		ddgErrorSet(FileRead,(char *) (file ? file : "(null)"));
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

	fread(segment,4,1,fptr);

	if (!strcmp(segment,"SIZE"))
	{
		// SIZE 2 bytes.
		ch1 = fgetc(fptr);
		ch2 = fgetc(fptr);
		_rows = _cols = ch2*256+ch1 +1;
		// Padding 2 bytes.
		fread(pad,2,1,fptr);
		fread(segment,4,1,fptr);
	}

	if (!strcmp(segment,"XPTS"))
	{
		// XPTS.
		ch1 = fgetc(fptr);
		ch2 = fgetc(fptr);
		_cols = ch2*256+ch1;
		// Padding 2 bytes.
		fread(pad,2,1,fptr);

		fread(segment,4,1,fptr);
		// YPTS.
		if (!strcmp(segment,"YPTS"))
		{
			ch1 = fgetc(fptr);
			ch2 = fgetc(fptr);
			_rows = ch2*256+ch1;
			// Padding 2 bytes.
			fread(pad,2,1,fptr);
			fread(segment,4,1,fptr);
		}
		else
		{
			ddgErrorSet(FileRead,(char *) "Expected YPTS segment, but it was not found.");
			goto error;
		}
	}

	// ALTW.
	// Absolute elevation for a given point is
	// BaseHeight + elevation* Scale / 65536.
	if (!strcmp(segment,"ALTW"))
	{
		ch1 = fgetc(fptr);
		ch2 = fgetc(fptr);
		_scale = (ch2*256+ch1)/ 65536.0;

		ch1 = fgetc(fptr);
		ch2 = fgetc(fptr);
		_base = ch2*256+ch1;
	}
	else
	{
		ddgErrorSet(FileRead,(char *) "Expected ALTW segment, but it was not found.");
		goto error;
	}

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
			ddgErrorSet(FileRead,(char *) (file ? file : "(null)"));
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
			s = _pixbuffer[i];
			_pixbuffer[i] = _pixbuffer[i+1];
			_pixbuffer[i+1] = s;
		}
	}

	return ddgSuccess;

error:
	if (fptr)
		fclose(fptr);
#ifdef DDG
	ddgError::report();
#endif
	return ddgFailure;
}



bool ddgHeightMap::generateHeights(unsigned int r, unsigned int c, float oct )
{
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
		set(r,0,short(l));
		set(r,_cols-1,short(l));
	}
	for (unsigned c = 0; c < _cols; c++ )
	{
		set(0,c,short(l));
		set(_rows-1,c,short(l));
	}
}

void ddgHeightMap::sin(void)
{
	for (unsigned r = 0; r < _rows; r++)
		for (unsigned c = 0; c < _cols; c++ )
		{
			set(r,c,short(10000*(ddgAngle::sin(180.0*(float)r/(float)_rows)
				         + ddgAngle::sin(180.0*(float)c/(float)_cols))));
		}
}

// Save image of height field.
bool ddgHeightMap::saveAsTGA(const char *s)
{
#ifdef DDG
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
#else
  (void) s;
	return ddgSuccess;
#endif
}

#ifdef DDGDEM
// Terrain loaders based on Ben Discoe's code.
#include "ElevationGrid.h"
#endif

bool ddgHeightMap::loadTerrain(const char *filename)
{
	if ( (readTGN(filename) == ddgFailure)
	/*	&& (readPGM(filename,0x7FFF) == ddgFailure )*/ )
#ifdef DDGDEM
	{
		// Try one of the other formats.
		ElevationGrid *eg = new ElevationGrid;

		// Try to get one of the formats to load the file.
		if (eg->LoadFromBT(filename)
		 &&	eg->LoadFromDEM( filename)
		 && eg->LoadFromDTED(filename)
		 && eg->LoadFromGTOPO30(filename)
		)
			goto error;

		int		gr,gc, i,j;
		float	v;
		unsigned short r,c;
		eg->GetDimensions(gc, gr);
		if (gc == 0 || gr == 0)
			goto error;

		// TODO calc based on grid corners/heightdelta.

		cerr << "Loaded DEM " << eg->GetName( ) << endl;

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
		cerr << "Terrain loaded " << rows() << "x" << cols() << endl;
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
#include "Generators/HeightField.h"
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
	}
  // Success
  return ddgSuccess;
#else
  (void) level;
  (void) smoothness;
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
  (void) data;
  (void) desired_max;
	return ddgFailure;
#endif
};

