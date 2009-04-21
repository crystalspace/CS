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

#include "csgeom/vector3.h"

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
    union
    {
      uint32 jabi;
      float tastic;
    } pun;
    pun.jabi = 0x3f800000 | (0x007fffff & seed);
    return pun.tastic - 1.0;
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

/**
 * Random number generator that generates random vector with spherical distribution
 */
class CS_CRYSTALSPACE_EXPORT csRandomVectorGen
{
public:
public:
  /// Initialize the random number generator using current time().
  csRandomVectorGen ()
    : floatGen ()
  {}
  /// Initialize the random number generator using given seed.
  csRandomVectorGen (unsigned int seed)
    : floatGen (seed)
  {}

  /// Initialize the RNG using current time() as the seed value.
  void Initialize()
  {
    floatGen.Initialize ();
  }
  /// Initialize the RNG using the supplied seed value.
  void Initialize(unsigned int new_seed)
  { 
    floatGen.Initialize (new_seed);
  }

  /// Get a random vector within unit sphere
  inline csVector3 Get ()
  {
    /* 
    This is a variant of the algorithm for computing a random point
    on the unit sphere; the algorithm is suggested in Knuth, v2,
    3rd ed, p136; and attributed to Robert E Knop, CACM, 13 (1970),
    326. 
    */
    csVector3 ret (0.0f);
    float s, a;

    do 
    {
      ret.x = 2.0f * floatGen.Get () - 1.0f;
      ret.y = 2.0f * floatGen.Get () - 1.0f;
      s = ret.SquaredNorm ();
    } while(s > 1.0f);

    ret.z = 2.0f * s - 1.0f;

    a = 2.0f * sqrtf (1.0f - s);
    ret.x *= a;
    ret.y *= a;

//    float x1, x2, x1sq, x2sq;
//    while (!haveNrs)
//    {
//      x1 = floatGen.Get (-1, 1);
//      x2 = floatGen.Get (-1, 1);
//      x1sq = x1*x1;
//      x2sq = x2*x2;
//      if ( (x1sq)+(x2sq) < 1) haveNrs = true;
//    }
//    csVector3 ret;
//    ret.x = 2*x1*sqrtf(1-x1sq-x2sq);
//    ret.y = 2*x2*sqrtf(1-x1sq-x2sq);
//    ret.z = 1-2*(x1sq+x2sq);

    return ret;
  }

private:
  csRandomFloatGen floatGen;
};

#endif // __CS_CSUTIL_FLOATRAND_H__
