/*
    Copyright (C) 2004 by Jorrit Tyberghein

    Fast simple floating point generator for a floating point value
    between 0 and 1.

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

#ifndef __CS_FLOATRAND_H__
#define __CS_FLOATRAND_H__

#include "csextern.h"

CS_CSUTIL_EXPORT_VAR unsigned int cs_floatrand_seed;

static inline float csFastRandFloat ()
{
  static unsigned int b = 1664525;
  static unsigned int c = 1013904223;
  cs_floatrand_seed = b*cs_floatrand_seed + c;
  uint32 temp = 0x3f800000 | (0x007fffff & cs_floatrand_seed);
  float r = (*(float*)&temp)-1.0;
  return r;
}

#endif // __CS_FLOATRAND_H__

