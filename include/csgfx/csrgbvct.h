/*
    Copyright (C) 1998,1999,2000 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_RGB_VECTOR_H__
#define __CS_RGB_VECTOR_H__

#include "csextern.h"
#include "csutil/array.h"
#include "csgfx/rgbpixel.h"

/**
 * This is a vector class that expects you to push
 * csRGBcolor structs onto it.  It has overrides for
 * Compare and CompareKey.  
 */
class csRGBVector : public csArray<csRGBcolor*>
{
public:
  /// Compare two csRGBcolor structs.
  static int Compare (csRGBcolor* const& i1, csRGBcolor* const& i2)
  {
    if ((*i1)==(*i2))
      return 0;
    else if(i1->red < i2->red &&
	    i1->green < i2->green &&
	    i1->blue < i2->blue)
      return -1;
    else
      return 1;
  }

  /// Compare a key (csRGBcolor struct) with a csRGBcolor struct
  static int CompareKey (csRGBcolor* const& c, csRGBcolor const& key)
  {
    return Compare(c, CS_CONST_CAST(csRGBcolor*,&key));
  }
};

#endif // __CS_RGB_VECTOR_H__
