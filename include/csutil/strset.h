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

#include "csutil/hashmap.h"

/**
 * An identifier for a string. This identifier is equivalent to the contents
 * of a string: If two strings have the same content, they have get the same
 * identifier. If they have different content, they get different identifiers.
 */
typedef uint32 csStringID;
/// this ID is the 'invalid' value
csStringID const csInvalidStringID = ~0;

/**
 * The string set is a list of strings, all with different content. Each
 * string has an ID number. The most important operation is to request a
 * string, which means to return the ID for the string, adding it to the
 * list if it is not already there.
 */
class csStringSet
{
  csHashMap Registry;
  csStringID IDCounter;
public:
  
  /// constructor
  csStringSet ();
  /// destructor
  ~csStringSet ();

  /// request the ID for the given string
  csStringID Request (const char *s);
  /**
   * Delete all stored strings. When new strings are registered again, new
   * ID values will be used, not the old ones reused.
   */
  void Clear ();
};

#endif // __CS_STRSET_H__
