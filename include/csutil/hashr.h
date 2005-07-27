/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

/**\file
 * Reversible hash.
 */

#ifndef __CS_UTIL_HASHR_H__
#define __CS_UTIL_HASHR_H__

#include "csutil/hash.h"
 
/**\addtogroup util_containers
 * @{ */

/// A csHash<> that maintains a reverse hash for indexing keys by values.
template <class T, class K = unsigned int> 
class csHashReversible : public csHash<T, K>
{
  csHash<K, T> reverse;
public:
  /**
   * Construct a hash table with an array of the given size,
   * which for optimisation reasons should be a prime number.
   * 
   * Grow_rate is the rate at which the hash table grows:
   * Size doubles once there are size/grow_rate collisions.
   * It will not grow after it reaches max_size.
   *
   * Here are a few primes: 7, 11, 19, 29, 59, 79, 101, 127, 151, 199, 251,
   * 307, 401, 503, 809, 1009, 1499, 2003, 3001, 5003, 12263, 25247, 36923,
   * 50119, 70951, 90313, 104707.
   * For a bigger list go to http://www.utm.edu/research/primes/
   */
  csHashReversible (int size = 23, int grow_rate = 5, int max_size = 20000) :
    csHash<T, K> (size, grow_rate, max_size), 
    reverse (size, grow_rate, max_size)
  {
  }
  /**
    * Add an element to the hash and reverse table.
    */
  void Put (const K& key, const T &value)
  {
    csHash<T, K>::Put (key, value);
    reverse.Put (value, key);
  }

  /**
   * Get a pointer to the first key matching the given value, 
   * or 0 if there is none.
   */
  const K* GetKeyPointer (const T& key) const
  {
    return reverse.GetElementPointer (key);
  }
 
  /**
   * Get the first key matching the given value, or \p fallback if there is 
   * none.
   */
  const K& GetKey (const T& key, const K& fallback) const
  {
    return reverse.Get (key, fallback);
  }
};

/** @} */

#endif // __CS_UTIL_HASHR_H__
