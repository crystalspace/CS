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
#ifndef _ddgStr_Class_
#define _ddgStr_Class_

//#include <iostream.h> 

#include "csterr/ddg.h"
#include "types.h"

/**
 * Basic string class, supports streaming to output.
 */
class WEXP ddgStr
{
public:
	/// String data.
	char *s;
	/// Length of string.
	unsigned int l;
	/// Create str initialize to zero.
	ddgStr(void);
	/// Create string initialized to ss.
	ddgStr(char *ss);
	/// Free the string.
	~ddgStr();
	/// Assign a value to the string.
	bool assign( char * b );
	/// Return the string managed by this object.
	inline operator char *() { return s; }
	/// Return the string managed by this object.
	inline operator ddgStr *() { return this; }
	/// Return the length of this string.
	unsigned int len(void)
	{
		return l;
	}
	/// Return the length of the string.
	static unsigned int len( char *s)
	{
		unsigned int i = 0;
		while( *s ) { s++; i++; }
		return i;
	} 
	/// Compare two string objects.
	static bool equal(char * a, char *b)
	{
		while(*a && *b && *a == *b) { a++; b++; }
		if (!*a && !*b)
			return true;
		return false;
	}
	/// Compare this object to a character string.
	inline bool equal(char *b) { return equal(s,b);}
	/// Convert a string to a floating point value.
	static float stof( char * s);
	/// Convert a string to an integer value.
	static int stoi( char * s);
};

///
//WEXP ostream& WFEXP operator << ( ostream&s, ddgStr v );
///
//WEXP ostream& WFEXP operator << ( ostream&s, ddgStr* v );
///
//WEXP istream& WFEXP operator >> ( istream& s, ddgStr& v);


#endif
