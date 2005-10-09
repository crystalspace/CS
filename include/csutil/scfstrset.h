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

/**\file
 * Implementation for iStringSet
 */

#include "csextern.h"
#include "csutil/scf_implementation.h"
#include "csutil/strset.h"
#include "iutil/strset.h"

/**
 * The string set is a collection of unique strings. Each string has an ID
 * number. The most important operation is to request a string, which means to
 * return the ID for the string, adding it to the collection if not already
 * present.  This is useful when you need to work with strings but want the
 * performance characteristics of simple numeric comparisons.  Rather than
 * performing string comparisons, you instead compare the numeric string ID's.
 */
class CS_CRYSTALSPACE_EXPORT csScfStringSet : 
  public scfImplementation1<csScfStringSet, iStringSet>
{
private:
  csStringSet set;

public:
  /// Constructor.
  csScfStringSet (size_t size = 23) 
    : scfImplementationType (this), set(size)
  { }

  /// Destructor.
  virtual ~csScfStringSet()
  { }

  /**
   * Request the numeric ID for the given string.
   * \return The ID of the string.
   * \remarks Creates a new ID if the string is not yet present in the set,
   *   else returns the previously assigned ID.
   */
  virtual csStringID Request(const char* s)
  { return set.Request(s); }

  /**
   * Request the string corresponding to the given ID.
   * \return Null if the string has not been requested (yet), else the string
   *   corresponding to the ID.
   */
  virtual const char* Request(csStringID id) const
  { return set.Request(id); }

  /**
   * Check if the set contains a particular string.
   */
  virtual bool Contains(char const* s) const
  { return set.Contains(s); }

  /**
   * Check if the set contains a string with a particular ID.
   * \remarks This is rigidly equivalent to
   *   <tt>return Request(id) != NULL</tt>, but more idomatic.
   */
  virtual bool Contains(csStringID id) const
  { return set.Contains(id); }

  /**
   * Remove specified string.
   * \return True if a matching string was in thet set; else false.
   */
  virtual bool Delete(char const* s)
  { return set.Delete(s); }

  /**
   * Remove a string with the specified ID.
   * \return True if a matching string was in thet set; else false.
   */
  virtual bool Delete(csStringID id)
  { return set.Delete(id); }

  /**
   * Remove all stored strings. When new strings are registered again, new
   * ID values will be used; the old ID's will not be re-used.
   */
  virtual void Empty()
  { set.Empty(); }

  /**
   * Remove all stored strings.
   */
  virtual void Clear()
  { Empty(); }

  /// Get the number of elements in the hash.
  virtual size_t GetSize () const
  { return set.GetSize(); }

  /**
   * Return true if the hash is empty.
   * \remarks Rigidly equivalent to <tt>return GetSize() == 0</tt>, but more
   *   idiomatic.
   */
  virtual bool IsEmpty() const
  { return set.IsEmpty(); }
};

#endif // __CS_SCFSTRSET_H__
