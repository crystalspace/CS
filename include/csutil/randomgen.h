/*
    Copyright (C) 1998-2005 by Jorrit Tyberghein
  
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

#ifndef __CS_RNG_H__
#define __CS_RNG_H__

/**\file
 * Random number generator.
 * This random number generator originally appeared in "Toward a Universal
 * Random Number Generator" by George Marsaglia and Arif Zaman.
 * Florida State University Report: FSU-SCRI-87-50 (1987)
 * <p>
 * It was later modified by F. James and published in "A Review of Pseudo-
 * random Number Generators"
 * <p>
 * THIS IS THE BEST KNOWN RANDOM NUMBER GENERATOR AVAILABLE.
 * (However, a newly discovered technique can yield
 * a period of 10^600. But that is still in the development stage.)
 * <p>
 * It passes ALL of the tests for random number generators and has a period
 * of 2^144, is completely portable (gives bit identical results on all
 * machines with at least 24-bit mantissas in the floating point
 * representation).
 * <p>
 * The algorithm is a combination of a Fibonacci sequence (with lags of 97
 * and 33, and operation "subtraction plus one, modulo one") and an
 * "arithmetic sequence" (using subtraction).
 */

#include "csextern.h"
#include "cstypes.h"

/**
 * Portable random number generator class.
 * The reason for using this class if that you may want a
 * consistent random number generator across all platforms
 * supported by Crystal Space. Besides, in general it is a
 * better quality RNG than the one supplied in most C runtime
 * libraries. Personally I observed a significant improvement
 * in a random terrain generator I made after I switched to
 * this RNG.
 */
class CS_CRYSTALSPACE_EXPORT csRandomGen
{
  int i97, j97;
  float u [98];
  float c, cd, cm;

public:
  /// Initialize the random number generator using current time()
  csRandomGen ()
  { Initialize (); }
  /// Initialize the random number generator given a seed
  csRandomGen (uint32 iSeed)
  { Initialize (iSeed); }

  /// Initialize the RNG using current time() as the seed value
  void Initialize ();
  /// Select the random sequence number (942,438,978 sequences available)
  void Initialize (uint32 iSeed);

  /// Get a floating-point random number in range 0 <= num < 1
  float Get ()
  { return RANMAR (); }
  /// Get a uint32 integer random number in range 0 <= num < iLimit
  uint32 Get (uint32 iLimit);

  /// Perform a self-test
  bool SelfTest ();

private:
  /// Initialize the random number generator
  void InitRANMAR (uint32 ij, uint32 kl);
  /// Get the next random number in sequence
  float RANMAR ();
};

#endif // __CS_RNG_H__
