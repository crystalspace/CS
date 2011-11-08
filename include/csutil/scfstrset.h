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

// For Clear()
#include "csutil/deprecated_warn_off.h"

namespace CS
{
/**
 * The string set is a collection of unique strings. Each string has an ID
 * number. The most important operation is to request a string, which means to
 * return the ID for the string, adding it to the collection if not already
 * present.  This is useful when you need to work with strings but want the
 * performance characteristics of simple numeric comparisons.  Rather than
 * performing string comparisons, you instead compare the numeric string ID's.
 *
 * Instances of the set are locked are for concurrent accesses.
 */

template<typename IF>
class ScfStringSet : public scfImplementation1<ScfStringSet<IF>, IF>
{
private:
  Utility::StringSet<typename IF::TagType, true> set;
  typedef StringID<typename IF::TagType> StringIDType;

  typedef scfImplementation1<ScfStringSet<IF>, IF> scfImplementationType_;
public:
  /// Constructor.
  ScfStringSet (size_t size = 23) 
    : scfImplementationType_ (this), set(size)
  { }

  /// Destructor.
  virtual ~ScfStringSet()
  { }

  /**
   * Request the numeric ID for the given string.
   * \return The ID of the string.
   * \remarks Creates a new ID if the string is not yet present in the set,
   *   else returns the previously assigned ID.
   */
  virtual StringIDType Request(const char* s)
  { return set.Request(s); }

  /**
   * Request the string corresponding to the given ID.
   * \return Null if the string has not been requested (yet), else the string
   *   corresponding to the ID.
   */
  virtual const char* Request(StringIDType id) const
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
  virtual bool Contains(StringIDType id) const
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
  virtual bool Delete(StringIDType id)
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

  /**
   * Return an iterator for the set which iterates over all strings.
   * \warning Modifying the set while you have open iterators will result
   *   undefined behaviour.
   * \warning The iterator will <b>not</b> respect locking of the string set!
   */
  typename Utility::StringSet<typename IF::TagType, true>::GlobalIterator GetIterator () const
  { return set.GetIterator(); }
};
} // namespace CS

typedef CS::ScfStringSet<iStringSet> csScfStringSet;

#include "csutil/deprecated_warn_on.h"

#endif // __CS_SCFSTRSET_H__
