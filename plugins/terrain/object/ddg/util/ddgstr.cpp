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

#include "util/ddgerror.h"
#include "util/ddgstr.h"


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

ddgStr::ddgStr(const char *ss, int size)
{
	s = NULL;
	_assign(ss,size);
}

ddgStr::ddgStr(const int size)
{
	s = new char[size];
	s[0] = 0;
}

ddgStr::~ddgStr()
{
	if (s)
		delete [] s;
}

// OPTIM: Don't call delete/new if old string is same size as new string.
bool ddgStr::_assign( const char * b, int size )
{
	if (s == b )
		return ddgFailure;
	
	if (b)
	{
		unsigned int i = length(b);
		if (i != l)
		{
			delete[] s;
			l = i;
			if (!(s = new char[(size ? size : l+1)]))
				return ddgFailure;
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

	return ddgSuccess;
}

bool ddgStr::assign(const char *b)
{
	return _assign(b,0);
}
float ddgStr::stof(const char *s)
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
            r = r*10.0f + (float)(*s - '0');
        }
        s++;
    }
    return (neg?-1.0f:1.0f)*(b ? r/b : r);
}

int ddgStr::stoi(const char *s)
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

ddgStr& ddgStr::operator  +=( const ddgStr& s2)
{
	char *t = new char[l + s2.l + 1], *tp;
	unsigned int i;
	tp = t;
	for (i=0;s[i]; i++,tp++) *tp = s[i];
	for (i=0;s2.s[i]; i++,tp++) *tp = s2.s[i];
	tp = 0;
	delete[] s;
	s = t;
	return *this;
}

char *ddgStr::findChar(const char target)
{
	char *tmp = s;
	while (*tmp && *tmp !=target)
		tmp++;
	return tmp;
}
// This method corrupts strings when used with.
/*
ostream& operator << ( ostream&strm, Str str )
{ 
	return strm << str.s;
}
*/
// Define an input and output stream operator for Vector2.
// Output format is "abcd"
#ifdef DDGSTREAM
ostream& operator << ( ostream &stream, const ddgStr str )
     { return stream << str.s; }
ostream& operator << ( ostream &stream, const ddgStr *str )
     { return stream << str->s; }

// Input format is "abcd"
istream& operator >> ( istream& stream, ddgStr& str)
     { char c[256]; stream >> c; str.assign(c); return stream; }
#endif


