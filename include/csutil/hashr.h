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

#include "csutil/hash.h"
 
/// A csHash<> that maintains a reverse hash for indexing keys by values.
template <class T, class K = uint32, 
  class KeyHandler = csIntegralHashKeyHandler<K>,
  class ReverseKeyHandler = csIntegralHashKeyHandler<T> > 
class csHashReversible : public csHash<T, K, KeyHandler>
{
  csHash<K, T, ReverseKeyHandler> reverse;
public:
  /**
    * Add an element to the hash and reverse table.
    */
  void Put (const K& key, const T &value)
  {
    csHash<T, K, KeyHandler>::Put (key, value);
    reverse.Put (value, key);
  }

  /// Get the first key matching the given value, or 0 if there is none.
  const K& GetKey (const T& key) const
  {
    return reverse.Get (key);
  }
 
};

