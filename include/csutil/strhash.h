/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_STRHASH_H__
#define __CS_STRHASH_H__

#include "csextern.h"
#include "hashmap.h"

/**\file
 */
 
/**
 * An identifier for a string. This identifier is equivalent to the contents
 * of a string: If two strings have the same content, they have get the same
 * identifier. If they have different content, they get different identifiers.
 */
typedef uint32 csStringID;
/// this ID is the 'invalid' value
csStringID const csInvalidStringID = (csStringID) ~0;

class csStringHash;

/**
 * An iterator to iterate over elements in a csStringHash.
 * When you have an open iterator you should not alter the
 * string hash that this object iterates over. 
 */
class CS_CSUTIL_EXPORT csStringHashIterator
{
  friend class csStringHash;

private:
  csGlobalHashIterator* hashIt;

public:

  /**
   * Constructor for an iterator to iterate over all elements in a hashmap.
   * Note that you should not do changes on the hashmap when you have
   * open iterators.
   */
  csStringHashIterator (csStringHash* hash);
  virtual ~csStringHashIterator ();

  /// Is there a next element in this iterator?
  bool HasNext ();
  /// Get the next element.
  csStringID Next ();
};

/**
 * The string hash is a hash of strings, all with different content. Each
 * string has an ID number.
 */
class CS_CSUTIL_EXPORT csStringHash
{
private:
  friend class csStringHashIterator;
  
  csHashMap Registry;

public:
  /// Constructor
  csStringHash (uint32 size = 211);
  /// Destructor
  ~csStringHash ();

  /**
   * Register a string with an id. Returns the pointer to the copy
   * of the string in this hash.
   */
  const char* Register (const char *s, csStringID id);

  /**
   * Request the ID for the given string. Return csInvalidStringID
   * if the string was never requested before.
   */
  csStringID Request (const char *s);

  /**
   * Request the string for a given ID. Return 0 if the string
   * has not been requested (yet).
   */
  const char* Request (csStringID id);

  /**
   * Delete all stored strings.
   */
  void Clear ();
};

#endif // __CS_STRHASH_H__
