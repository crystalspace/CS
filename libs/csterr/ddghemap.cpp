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

#include "sysdef.h"
// DDG includes
#include "csterr/ddgutil.h"
#include "csterr/ddghemap.h"
#include "csterr/ddgnoise.h"
#include "csterr/ddgerror.h"
#include "csterr/worditer.h"

#define PGM_MAGIC "P2"

// ----------------------------------------------------------------------

ddgHeightMap::~ddgHeightMap(void)
{
	delete _pixbuffer;
}

void ddgHeightMap::allocate( unsigned short r, unsigned short  c )
{
	_cols = c;
	_rows = r;
	delete _pixbuffer;
	assert(sizeof(short) == 2);
	_pixbuffer = new short[_cols*_rows];
}

// ----------------------------------------------------------------------
// load:
bool ddgHeightMap::readMemory( unsigned short *buf )
{
	int i;
	for (i=0; i < _cols*_rows; i++)
		_pixbuffer[i] = buf[i];
	return false;
}

// ----------------------------------------------------------------------
// load:

bool ddgHeightMap::writeTGN(char *filename, unsigned int base, unsigned int scale)
{
	FILE *fptr = filename && filename[0] ? fopen(filename,"wb") : 0;

	if (!fptr) {
		ddgErrorSet(FileWrite,(char *) (filename ? filename : "(null)"));
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
	ch1 = _cols % 256;
	ch2 = _cols - ch1 * 256;
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
	ch2 = _cols - ch1 * 256;
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
	ch2 = _rows - ch1 * 256;
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

	ch1 = scale % 256;
	ch2 = scale - ch1 * 256;
	fputc(ch1,fptr);
	fputc(ch2,fptr);

	ch1 = base % 256;
	ch2 = base - ch1 * 256;
	fputc(ch1,fptr);
	fputc(ch2,fptr);

	// Write data
	int r, c;
	r = 0;

	while (r < _rows)
	{
		for (c = 0; c < _cols; c++)
		{
			ch1 = _pixbuffer[c + r * _cols] % 256;
			ch2 = _pixbuffer[c + r * _cols] - ch1 * 256;
			fputc(ch1,fptr);
			fputc(ch2,fptr);
		}
		r++;
	}
	// File termination segment.
	fputc(0,fptr);
	fputc(0,fptr);
	fputc(0,fptr);
	fputc(0,fptr);

	fclose(fptr);
	return true;

error:
	fclose(fptr);
	ddgError::report();
	return false;
}

bool ddgHeightMap::readTGN(char *file)
{
	FILE *fptr = file && file[0] ? fopen(file,"rb") : 0;
	unsigned long test_value = 0x12345678L;
	bool endian = (*((unsigned char*)&test_value) == 0x12);

	if (!fptr)
	{
		ddgErrorSet(FileRead,(char *) (file ? file : "(null)"));
		ddgError::report();
		return true;
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
		ddgError::report();
		return true;
	}

	// Segment 4 bytes.
	segment[4] = '\0';

	fread(segment,4,1,fptr);

	if (strcmp(segment,"SIZE")==0)
	{
		// SIZE 2 bytes.
		ch1 = fgetc(fptr);
		ch2 = fgetc(fptr);
		_rows = _cols = ch2*256+ch1 +1;
		// Padding 2 bytes.
		fread(pad,2,1,fptr);
		fread(segment,4,1,fptr);
	}

	if (strcmp(segment,"XPTS")==0)
	{
		// XPTS.
		ch1 = fgetc(fptr);
		ch2 = fgetc(fptr);
		_cols = ch2*256+ch1;
		// Padding 2 bytes.
		fread(pad,2,1,fptr);

		fread(segment,4,1,fptr);
		// YPTS.
		if (strcmp(segment,"YPTS")==0)
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
			ddgError::report();
			return true;
		}
	}

	// ALTW.
	// Absolute elevation for a given point is
	// BaseHeight + elevation* Scale / 65536.
	if (strcmp(segment,"ALTW")==0)
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
		ddgError::report();
		return true;
	}

	// Read height data
	// The absolute altitude of a particular point (in the same scale as x and y)
	// is equal to BaseHeight + Elevation * HeightScale / 65536
	allocate(_cols,_rows);
	int r = 0;
	int cnt;
	while (r < _rows)
	{
		if ((cnt = fread(&(_pixbuffer[r*_cols]),2,_cols,fptr)) <
		(int)_cols)
		{
			ddgErrorSet(FileRead,(char *) (file ? file : "(null)"));
			ddgError::report();
			if (feof(fptr))
			{
				ddgErrorSet(FileAccess,"Reached end of file!");
				ddgError::report();
			}
			fclose(fptr);
			return true;
		}
		r++;
	}
	// Swap byte order for machines with different endian.
	if  (endian)
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

	fclose(fptr);
	return false;
}



bool ddgHeightMap::generateHeights(unsigned int r, unsigned int c, float oct )
{
    allocate(r,c);
	unsigned int ci, ri;
	float lacun = 2.0;
	float fractalDim = 0.25;
	float offset = 0.7;
	double size = 3.0;  // sample range = 0 to 3 
	short cc;
	double n;
	double v[3];
	v[0] = 0; v[1] = 0; v[2] = 0;
	for (ri = 0; ri < r; ri++)
	{
		v[1] = 0.0f;
		for (ci = 0; ci < c; ci++)
		{
			n = ddgNoise::hybridmultifractal(v,fractalDim,lacun,oct,offset);	// Returns 0 to 1.
			cc = (short) ddgUtil::clamp(n*75.0,0,255);	// Scale to fill range.
			set(ri,ci, cc);
			v[1]+= size/c;
		}
	v[0]+= size/r;
	}

  
	return true;
}

void ddgHeightMap::canyonize(float f )
{
	f += 1;
	for (unsigned r = 0; r < _rows; r++)
		for (unsigned c = 0; c < _cols; c++ )
		{
			unsigned int d = get(r,c);
            unsigned int d2 = (unsigned int)ddgUtil::clamp(pow(d,f),0,0xFFFF);
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
//                unsigned int d2 = Util::clamp(pow(d,f),0,0xFFFF);
	    		set(r,c,dn);
            }
		}
}

void ddgHeightMap::scale(float s )
{
	for (unsigned r = 0; r < _rows; r++)
		for (unsigned c = 0; c < _cols; c++ )
		{
            unsigned int d = (unsigned int)ddgUtil::clamp(s * get(r,c),0,0xFFFF);
			set(r,c,d);
		}
}

void ddgHeightMap::translate(float t )
{
	for (unsigned r = 0; r < _rows; r++)
		for (unsigned c = 0; c < _cols; c++ )
		{
            unsigned int d = (unsigned int)ddgUtil::clamp(t + get(r,c),0,0xFFFF);
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

void ddgHeightMap::closeEdge(void)
{
	for (unsigned r = 0; r < _rows; r++)
	{
		set(r,0,0);
		set(r,_cols-1,0);
	}
	for (unsigned c = 0; c < _cols; c++ )
	{
		set(0,c,0);
		set(_rows-1,c,0);
	}
}

void ddgHeightMap::sin(void)
{
	for (unsigned r = 0; r < _rows; r++)
		for (unsigned c = 0; c < _cols; c++ )
		{
			set(r,c,(short)(10000*(ddgAngle::sin(180.0*(float)r/(float)_rows)
				         +ddgAngle::sin(180.0*(float)c/(float)_cols))));
		}
}

/*
	Dumb little utility function to make sure that 
	atoi doesn't get a null argument...
*/
int stringToInt(char *str)
{
	if (str)
	{
		return atoi(str);
	};
	
	return 0;
};

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
bool ddgHeightMap::readPGM(char* data, int desired_max)
{
	int	max_grey;
	int width;
	int height;
	
	char* token;
	
	WordIterator st(data, " \n\t");
	
	token = st.nextWord();
	if (!strncmp(token, PGM_MAGIC, strlen(PGM_MAGIC)))
	{
		token = st.nextWord();
		width = stringToInt(token);
		
		token = st.nextWord();
		height = stringToInt(token);
		
		token = st.nextWord();
		max_grey = stringToInt(token);

		if ((max_grey < 1) || (width < 1) || (height < 1))
		{
			//ERROR: Obviously not well formed...
			return false;
		};
		
		allocate(height, width);
		setScaleAndBase((float)((float)height / (float)width), 0);
	
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				int temp;
				short val;
				
				token = st.nextWord();
				temp = stringToInt(token);

				/* @@@ This could overflow... how do we want to solve this?*/
				val = (short)((temp * desired_max) / max_grey);

				set(x, y, val);
			};
		};
		
	}
	else
	{
		//ERROR: Invalid magic, not an ASCII PGM
		return false;
	};

	return true;	
};

