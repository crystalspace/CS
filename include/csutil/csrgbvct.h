/*
    Crystal Space utility library: vector class interface
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


#ifndef __CS_RGB_VECTOR__
#define __CS_RGB_VECTOR__

#include "csvector.h"

/** This is a vector class that expects you to push
 * csRGBcolor structs onto it.  It has overrides for
 * Compare and CompareKey.  
 */
class csRGBVector : public csVector
{

public:
  /// Compare two csRGBcolor structs.
  virtual int Compare(csSome item1, csSome item2, int Mode)
  {
    csRGBcolor *i1 = STATIC_CAST(csRGBcolor *, item1);
    csRGBcolor *i2 = STATIC_CAST(csRGBcolor *, item2);

    if ((*i1)==(*i2))
      return 0;
    else if(i1->red < i2->red &&
	    i1->green < i2->green &&
	    i1->blue < i2->blue)
    {
      return -1;
    }
    else
      return 1;
  }

  /// Compare a key (csRGBcolor struct) with a csRGBcolor struct
  virtual int CompareKey(csSome item, csConstSome key, int Mode)
  {
    csRGBcolor *i1 = STATIC_CAST(csRGBcolor *, item);
    const csRGBcolor *i2 = STATIC_CAST(const csRGBcolor *, key);

    if ((*i1)==(*i2))
      return 0;
    else if(i1->red < i2->red &&
	    i1->green < i2->green &&
	    i1->blue < i2->blue)
    {
      return -1;
    }
    else
      return 1;
  }

};

#endif