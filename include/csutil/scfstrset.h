/*
    Copyright (C) 2003 by Anders Stenberg

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

#ifndef __CS_SCFSTRSET_H__
#define __CS_SCFSTRSET_H__

#include "csextern.h"
#include "iutil/strset.h"
#include "strset.h"

/**
 * The string set is a list of strings, all with different content. Each
 * string has an ID number. The most important operation is to request a
 * string, which means to return the ID for the string, adding it to the
 * list if it is not already there.
 */
class CS_CSUTIL_EXPORT csScfStringSet : public iStringSet
{
  csStringSet set;

public:
  SCF_DECLARE_IBASE;

  /// Create an empty scfStringSet object
  csScfStringSet ()
  { SCF_CONSTRUCT_IBASE (0); }

  /// Create an scfStringSet object and set the size of the hash
  csScfStringSet (uint32 size) : set(size)
  { SCF_CONSTRUCT_IBASE (0); }

  /// Destructor.
  virtual ~csScfStringSet()
  { SCF_DESTRUCT_IBASE(); }

  /**
   * Request the ID for the given string. Create a new ID
   * if the string was never requested before.
   */
  virtual csStringID Request (const char *s);

  /**
   * Request the string for a given ID. Return 0 if the string
   * has not been requested (yet).
   */
  virtual const char* Request (csStringID id) const;

  /**
   * Check if the set contains a particular string.
   */
  virtual bool Contains(char const*) const;

  /**
   * Delete all stored strings. When new strings are registered again, new
   * ID values will be used, not the old ones reused.
   */
  virtual void Clear ();
};

#endif // __CS_SCFSTRSET_H__
