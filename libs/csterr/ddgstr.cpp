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

#include "sysdef.h"
#include "csterr/ddgstr.h"


// ----------------------------------------------------------------------
//
// Str:
//
//  Memory allocation must remain in the same DLL, so this can not be inlined.
//
ddgStr::ddgStr(void)
{
	s = NULL;
	l = 0;
}

ddgStr::ddgStr(char *ss)
{
	s = NULL; assign(ss);
}

ddgStr::~ddgStr()
{
	delete [] s;
}

// OPTIM: Don't call delete/new if old string is same size as new string.
bool ddgStr::assign( char * b)
{
	if (s != b ) {
		if (b)
		{
			unsigned int i = len(b);
			if (i != l)
			{
				delete[] s;
				l = i;
				if (!(s = new char[l+1]))
					return false;
			}
			if (s)
				s[l] = 0;		// End the string.
			while (i)
			{
				i--;
				s[i]= b[i];
			}
		}
		else				// Null assignment.
		{
			delete[] s;
			s = NULL;
		}
	}
	return true;
}

float ddgStr::stof(char *s)
{
    float r = 0.0f;
    float b = 0.0f;
    bool neg = false;
    while(*s)
    {
        if (*s == '.') b = 1;
        else if (*s == '+') ;
        else if (*s == '-') neg = true;
        else
        {
            if (b) b *= 10.0f;
            r = r*10.0 + (float)(*s - '0');
        }
        s++;
    }
    return (neg?-1.0:1.0)*(b ? r/b : r);
}

int ddgStr::stoi(char *s)
{
    int b = 0;
    bool neg = false;
    while(*s)
    {
        if (*s == '+') ;
        else if (*s == '-') neg = true;
        else
        {
            b = (int)(b*10.0 + (float)(*s - '0'));
        }
        s++;
    }
    return (neg?-1*b:b) ;
}

// This method corrupts strings when used with.
/*
ostream& operator << ( ostream&strm, Str str )
{ 
	return strm << str.s;
}
*/
#if 0
// Define an input and output stream operator for Vector2.
// Output format is "abcd"
ostream& operator << ( ostream&s, ddgStr str )
     { return s << str.s; }
ostream& operator << ( ostream&s, ddgStr* str )
     { return s << str->s; }

// Input format is "abcd"
istream& operator >> ( istream& s, ddgStr& str)
     { char c[256]; s >> c; str.assign(c); return s; }

#endif


