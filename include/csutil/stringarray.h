/*
  Crystal Space String Array
  Copyright (C) 2003 by Jorrit Tyberghein

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

#ifndef __CS_STRINGARRAY_H__
#define __CS_STRINGARRAY_H__

#include <stdarg.h>
#include "csutil/util.h"
#include "csutil/array.h"

class csStringArrayElementHandler
{
public:
  static void Construct (const char** address, const char* const& src)
  {
    *address = csStrNew (src);
  }

  static void Destroy (const char** address)
  {
    delete[] *address;
  }

  static void InitRegion (const char** address, int count)
  {
    memset (address, 0, count*sizeof (const char*));
  }
};

/**
 * An array of strings. This array will properly make copies of the strings
 * and delete those copies using delete[] later.
 */
class csStringArray : public csArray<const char*, csStringArrayElementHandler>
{
public:
  /**
   * Initialize object to hold initially 'ilimit' elements, and increase
   * storage by 'ithreshold' each time the upper bound is exceeded.
   */
  csStringArray (int ilimit = 0, int ithreshold = 0)
  	: csArray<const char*, csStringArrayElementHandler> (ilimit, ithreshold)
  {
  }

  static int CaseSensitiveCompare (char const* item1, char const* item2)
  {
    return strcmp (item1, item2);
  }

  static int CaseInsensitiveCompare (char const* item1, char const* item2)
  {
    return strcasecmp (item1, item2);
  }
 
  static int CaseSensitiveCompare (void const* item1, void const* item2)
  {
    return strcmp (*(char const**)item1, *(char const**)item2);
  }

  static int CaseInsensitiveCompare (void const* item1, void const* item2)
  {
    return strcasecmp (*(char const**)item1, *(char const**)item2);
  }
 
  /**
   * Sort array based on case sensitive string compare function.
   */
  void Sort (ArraySortCompareFunction* compare)
  {
    csArray<const char*, csStringArrayElementHandler>::Sort (compare);
  }

  /**
   * Sort array based on case sensitive string compare function.
   */
  void Sort ()
  {
    csArray<const char*,
    	csStringArrayElementHandler>::Sort (CaseSensitiveCompare);
  }

  /// Pop an element from tail end of array.
  char* Pop ()
  {
    CS_ASSERT (Length () > 0);
    int l = Length () - 1;
    char* ret = (char*)Get (l);
    InitRegion (l, 1);
    SetLength (l);
    return ret;
  }
};

#endif // __CS_STRINGARRAY_H__
