/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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

#ifndef __CS_STRSET_H__
#define __CS_STRSET_H__

#include "csextern.h"
#include "strhash.h"

/**\file
 */
 
class csStringSet;

/**
 * An iterator to iterate over elements in a csStringSet.
 * When you have an open iterator you should not alter the
 * string set that this object iterates over. 
 */
class CS_CSUTIL_EXPORT csStringSetIterator
{
  friend class csStringSet;

private:
  csStringHashIterator* hashIt;

public:

  /**
   * Constructor for an iterator to iterate over all elements in a hashmap.
   * Note that you should not do changes on the hashmap when you have
   * open iterators.
   */
  csStringSetIterator (csStringSet* hash);

  /// Is there a next element in this iterator?
  bool HasNext ();
  /// Get the next element.
  csStringID Next ();
};

/**
 * The string set is a list of strings, all with different content. Each
 * string has an ID number. The most important operation is to request a
 * string, which means to return the ID for the string, adding it to the
 * list if it is not already there.
 */
class CS_CSUTIL_EXPORT csStringSet
{
  friend class csStringSetIterator;

  csStringHash Registry;
  csHashMap reverse_mapping;	// Mapping from ID to string.
  csStringID IDCounter;
public:
  /// Constructor
  csStringSet (uint32 size = 211);
  /// Destructor
  ~csStringSet ();

  /**
   * Request the ID for the given string. Create a new ID
   * if the string was never requested before.
   */
  csStringID Request (const char *s);

  /**
   * Request the string for a given ID. Return 0 if the string
   * has not been requested (yet).
   */
  const char* Request (csStringID id) const;

  /**
   * Delete all stored strings. When new strings are registered again, new
   * ID values will be used, not the old ones reused.
   */
  void Clear ();
};

#endif // __CS_STRSET_H__
