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
#ifndef _ddgStr_Class_
#define _ddgStr_Class_

#include "util/ddg.h"
/**
 * Basic string class, supports streaming to output.
 */
class WEXP ddgStr
{
	/// Assigns value to string.
	bool _assign( const char *b, int size);
public:
	/// String data.
	char *s;
	/// Length of string.
	unsigned int l;
	/// Create str initialize to zero.
	ddgStr(void);
	/// Create string initialized to ss, allocate with given size.
	ddgStr(const char *ss, int size = 0);
	/// Create string with a given buffer size.
	ddgStr(const int s);
	/// Create string based on the value of another.
	ddgStr(const ddgStr *str) { s = NULL; assign(str->s); }
	/// Create string based on the value of another.
	ddgStr(const ddgStr &str) { s = NULL; assign(str.s); }
	/// Free the string.
	~ddgStr();
	/// Assign a value to the string.
	bool assign( const char * b );
	/// Assign a value to the string.
	bool assign( const ddgStr * str ) { return assign(str->s); }
	/// Return the string managed by this object.
	inline operator char *() { return s; }
	/// Return the string managed by this object.
	inline operator const char *() { return s; }
	/// Return the pointer to this object.
	inline operator ddgStr *() { return this; }
	/// Concatenate another this to this one.
	ddgStr& operator  +=( const ddgStr& s);
	/// Return the length of this string.
	unsigned int length(void)
	{
		return l;
	}
	/// Return the length of the string.
	static unsigned int length( const char *s)
	{
		char *p = (char *)s;
		while( *p ) { p++;}
		return p-s;
	} 
	/// Compare two string objects.
	static bool equal(const char * a, const char *b)
	{
		while(*a && *b && *a == *b) { a++; b++; }
		if (!*a && !*b)
			return true;
		return false;
	}
	/// Compare this object to a character string.
	inline bool equal( const char *b) { return equal(s,b);}
	/// Convert a string to a floating point value.
	static float stof( const char * s);
	/// Convert a string to an integer value.
	static int stoi( const char * s);
	/// Find char in string.  Return null if char is not there.
	char *findChar( const char target );
};

#ifdef DDGSTREAM
///
WEXP ostream& WFEXP operator << ( ostream&s, const ddgStr v );
///
WEXP ostream& WFEXP operator << ( ostream&s, const ddgStr* v );
///
WEXP istream& WFEXP operator >> ( istream& s, ddgStr& v);
#endif // DDGSTREAM

#endif
