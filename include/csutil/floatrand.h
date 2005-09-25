/*
    Copyright (C) 2004 by Jorrit Tyberghein
    Copyright (C) 2005 by Eric Sunshine

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

#ifndef __CS_CSUTIL_FLOATRAND_H__
#define __CS_CSUTIL_FLOATRAND_H__

/**\file
 * Random number generator for floating point values
 */

#include "csextern.h"

/**
 * Fast simple random number generator for floating point values.
 */
class CS_CRYSTALSPACE_EXPORT csRandomFloatGen
{
private:
  unsigned int seed;

public:
  /// Initialize the random number generator using current time().
  csRandomFloatGen ()
  { Initialize(); }
  /// Initialize the random number generator using given seed.
  csRandomFloatGen (unsigned int seed)
  { Initialize(seed); }

  /// Initialize the RNG using current time() as the seed value.
  void Initialize();
  /// Initialize the RNG using the supplied seed value.
  void Initialize(unsigned int new_seed)
  { seed = new_seed; }

  /// Get a floating-point random number in range 0 <= num < 1.
  inline float Get()
  {
    unsigned int const b = 1664525;
    unsigned int const c = 1013904223;
    seed = b * seed + c;
    uint32 temp = 0x3f800000 | (0x007fffff & seed);
    return (*(float*)&temp) - 1.0;
  }

  /// Get a floating point random number in range 0 <= num < \c max.
  inline float Get(float max)
  {
    return max * Get();
  }

  /// Get a floating point random number in range \c min <= num < \c max.
  inline float Get(float min, float max)
  {
    float const w = max - min;
    return min + w * Get();
  }

  /// Get a random angle in range 0 <= num < 2*PI radians.
  inline float GetAngle()
  {
    return TWO_PI * Get();
  }
};

#endif // __CS_CSUTIL_FLOATRAND_H__
