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
class CS_CRYSTALSPACE_EXPORT csStringSetIterator
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
class CS_CRYSTALSPACE_EXPORT csStringSet
{
  friend class csStringSetIterator;

  csStringHash Registry;
  csHash<const char*, csStringID> reverse_mapping; // ID to string mapping.
  csStringID IDCounter;
public:
  /// Constructor
  csStringSet (int size = 23);
  /// Destructor
  ~csStringSet ();

  /**
   * Request the ID for the given string. Create a new ID
   * if the string was never requested before.
   */
  csStringID Request (const char*);

  /**
   * Request the string for a given ID. Return 0 if the string
   * has not been requested (yet).
   */
  char const* Request (csStringID) const;

  /**
   * Check if the set contains a particular string.
   */
  bool Contains(char const*) const;

  /**
   * Delete all stored strings. When new strings are registered again, new
   * ID values will be used; the old ID's will not be re-used.
   */
  void Clear ();
};

#endif // __CS_STRSET_H__
