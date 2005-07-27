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
#include "csutil/hash.h"
#include "csutil/mempool.h"
#include "iutil/strset.h"

/**\file
 * String-to-ID hash table.
 */
 
/**
 * A string-to-ID hash table.  Useful when you need to work with strings but
 * want the performance characteristics of simple numeric comparisons.
 * Register a string with a unique numeric ID and then compare ID's rather than
 * comparing strings.  You can fetch a string's ID via Request().
 * \sa csStringSet
 */
class CS_CRYSTALSPACE_EXPORT csStringHash
{
private:
  typedef csHash<csStringID, char const*> HashType;
  HashType registry;
  csMemoryPool pool;

  void Copy(csStringHash const&);

public:
  typedef HashType::GlobalIterator GlobalIterator;

public:
  /// Constructor.
  csStringHash (size_t size = 23);
  /// Copy constructor.
  csStringHash (csStringHash const& h) { Copy(h); }
  /// Destructor.
  ~csStringHash ();
  /// Assignment operator.
  csStringHash& operator=(csStringHash const& h) { Copy(h); return *this; }

  /**
   * Register a string with an ID.
   * \param s The string with which to associate the ID.
   * \param id A numeric value with which to identify this string.
   * \return A pointer to the copy of the string in this hash.
   * \remarks If the string is already registered with a different ID, the old
   *   ID will be replaced with the one specified here. If you would like the
   *   convenience of having the ID assigned automatically, then consider using
   *   csStringSet, instead.
   * <p>
   * \remarks If you do not care about the ID, but instead simply want to use
   *   the hash as a string \e set which merely records if a string is present,
   *   then you can omit \c id. To find out if a string is contained in the
   *   set, invoke Contains(). The same functionality can be accomplished via
   *   csStringSet, however csStringSet is more heavyweight because it also
   *   maintains a reverse-mapping from ID to string. Omitting the \c id makes
   *   for a good alternative to csStringSet when you do not require its extra
   *   bulk.
   */
  const char* Register (const char* s, csStringID id = 0);

  /**
   * Request the ID for the given string.
   * \return The string's ID or csInvalidStringID if the string has not yet
   *   been registered.
   */
  csStringID Request (const char* s) const;

  /**
   * Request the string for a given ID.
   * \return The string associated with the given ID, or the null pointer if
   *   the string has not yet been registered. If more than one string is
   *   associated with the ID, then one is returned (but specifically which one
   *   is unspecified).
   * \warning This operation is slow.  If you need to perform reverse lookups
   *   frequently, then instead consider using csStringSet, in which reverse
   *   lookups are optimized.
   */
  const char* Request (csStringID id) const;

  /**
   * Check if the hash contains a particular string.
   * \remarks This is rigidly equivalent to
   *   <tt>return Request(s) != csInvalidStringID</tt>.
   */
  bool Contains(char const* s) const
  { return Request(s) != csInvalidStringID; }

  /**
   * Check if the hash contains a string with a particular ID.
   * \remarks This is rigidly equivalent to
   *   <tt>return Request(id) != NULL</tt>, but more idiomatic.
   * \warning This operation is slow.  If you need to check containment of ID's
   *   frequently, then instead consider using csStringSet, in which such
   *   checks are optimized.
   */
  bool Contains(csStringID id) const
  { return Request(id) != 0; }

  /**
   * Remove specified string.
   * \return True if a matching string was in thet set; else false.
   */
  bool Delete(char const* s);

  /**
   * Remove a string with the specified ID.
   * \return True if a matching string was in thet set; else false.
   * \remarks If more than one string is associated with the ID, then one is
   *   removed (but specifically which one is unspecified).
   */
  bool Delete(csStringID id);

  /**
   * Remove all stored strings.
   */
  void Empty ();

  /**
   * Delete all stored strings.
   * \deprecated Use Empty() instead.
   */
  /*CS_DEPRECATED_METHOD*/ void Clear ()
  { Empty(); }

  /// Get the number of elements in the hash.
  size_t GetSize () const
  { return registry.GetSize (); }

  /**
   * Return true if the hash is empty.
   * \remarks Rigidly equivalent to <tt>return GetSize() == 0</tt>, but more
   *   idiomatic.
   */
  bool IsEmpty() const
  { return GetSize() == 0; }

  /**
   * Return an iterator for the string hash which iterates over all elements.
   * \warning Modifying the hash while you have open iterators will result
   *   undefined behaviour.
   */
  GlobalIterator GetIterator () const
  { return registry.GetIterator(); }
};

#endif // __CS_STRHASH_H__
