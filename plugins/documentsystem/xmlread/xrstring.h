/*
www.sourceforge.net/projects/tinyxml
Original file by Yves Berquin.

This software is provided 'as-is', without any express or implied 
warranty. In no event will the authors be held liable for any 
damages arising from the use of this software.

Permission is granted to anyone to use this software for any 
purpose, including commercial applications, and to alter it and 
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must 
not claim that you wrote the original software. If you use this 
software in a product, an acknowledgment in the product documentation 
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source 
distribution.
*/

#include "xr.h"


#ifndef TIXML_STRING_INCLUDED
#define TIXML_STRING_INCLUDED

#ifdef MSVC
#pragma warning( disable : 4514 )
#endif

#include <assert.h>

/*
   TrXmlString is an emulation of the std::string template.
   Its purpose is to allow compiling TinyXML on compilers with no or poor STL support.
   Only the member functions relevant to the TinyXML project have been implemented.
   The buffer allocation is made by a simplistic power of 2 like mechanism : if we increase
   a string and there's no more room, we allocate a buffer twice as big as we need.
*/
class TrXmlString
{
  public :
    // TrXmlString constructor, based on a string
    TrXmlString (const char * instring);

    // TrXmlString empty constructor
    TrXmlString ()
    {
        allocated = 0;
        cstring = NULL;
	clength = 0;
    }

    // TrXmlString copy constructor
    TrXmlString (const TrXmlString& copy);

    // TrXmlString destructor
    ~ TrXmlString ()
    {
        empty_it ();
    }

    // Convert a TrXmlString into a classical char *
    const char * c_str () const
    {
        if (allocated)
            return cstring;
        return "";
    }

    // Return the length of a TrXmlString
    unsigned length () const { return clength; }

    // TrXmlString = operator
    void operator = (const char * content);

    // = operator
    void operator = (const TrXmlString & copy);

    // += operator. Maps to append
    TrXmlString& operator += (const char * suffix)
    {
        append (suffix);
		return *this;
    }

    // += operator. Maps to append
    TrXmlString& operator += (char single)
    {
        append (single);
		return *this;
    }

    // += operator. Maps to append
    TrXmlString& operator += (const TrXmlString & suffix)
    {
        append (suffix);
		return *this;
    }
    bool operator == (const TrXmlString & compare) const;
    bool operator < (const TrXmlString & compare) const;
    bool operator > (const TrXmlString & compare) const;

    // Checks if a TrXmlString is empty
    bool empty () const
    {
        return length () ? false : true;
    }

    // Checks if a TrXmlString contains only whitespace (same rules as isspace)
	// Not actually used in tinyxml. Conflicts with a C macro, "isblank",
	// which is a problem. Commenting out. -lee
//    bool isblank () const;

    // single char extraction
    const char& at (unsigned index) const
    {
        //assert( index < length ());
        return cstring [index];
    }

    // find a char in a string. Return TrXmlString::notfound if not found
    unsigned find (char lookup) const
    {
        return find (lookup, 0);
    }

    // find a char in a string from an offset. Return TrXmlString::notfound if not found
    unsigned find (char tofind, unsigned offset) const;

    /*	Function to reserve a big amount of data when we know we'll need it. Be aware that this
		function clears the content of the TrXmlString if any exists.
    */
    void reserve (unsigned size)
    {
        empty_it ();
        if (size)
        {
            allocated = size;
            cstring = (char*)malloc (size);
            cstring [0] = 0;
        }
    }

    // [] operator 
    char& operator [] (unsigned index) const
    {
        //assert( index < length ());
        return cstring [index];
    }

    // Error value for find primitive 
    enum {	notfound = 0xffffffff,
            npos = notfound };

    void append (const char *str, int len );

  protected :

    // The base string
    char * cstring;
    // Length of string (not including implicit null pointer).
    unsigned clength;
    // Number of chars allocated (including implicit null pointer).
    unsigned allocated;

    // New size computation. It is simplistic right now : it returns twice the amount
    // we need
    unsigned assign_new_size (unsigned minimum_to_allocate)
    {
        return minimum_to_allocate * 2;
    }

    // Internal function that clears the content of a TrXmlString
    void empty_it ()
    {
        if (cstring)
	    free (cstring);
        cstring = NULL;
	clength = 0;
        allocated = 0;
    }

    void append (const char *suffix )
    {
    	append (suffix, strlen (suffix));
    }

    // append function for another TrXmlString
    void append (const TrXmlString & suffix)
    {
        append (suffix . c_str ());
    }

    // append for a single char.
    void append (char single);

} ;

#endif	// TIXML_STRING_INCLUDED

