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
 
namespace CS
{
namespace Utility
{
/**
 * A string-to-ID hash table.  Useful when you need to work with strings but
 * want the performance characteristics of simple numeric comparisons.
 * Register a string with a unique numeric ID and then compare ID's rather than
 * comparing strings.  You can fetch a string's ID via Request().
 * \sa csStringSet
 */
template<typename Tag>
class CS_CRYSTALSPACE_EXPORT StringHash
{
private:
  typedef csHash<StringID<Tag>, char const*> HashType;
  HashType registry;
  csMemoryPool pool;

public:
  typedef typename HashType::ConstGlobalIterator GlobalIterator;

private:
  void Copy(StringHash const& h)
  {
    if (&h != this)
    {
      GlobalIterator it(h.GetIterator());
      while (it.HasNext())
      {
	char const* s;
	StringID<Tag> id = it.Next(s);
	this->Register(s, id);
      }
    }
  }

public:
  /// Constructor.
  StringHash (size_t size = 23) : registry (size) {}
  /// Copy constructor.
  StringHash (StringHash const& h) { Copy(h); }
  /// Destructor.
  ~StringHash () { Empty(); }
  /// Assignment operator.
  StringHash& operator=(StringHash const& h) { Copy(h); return *this; }

  /**
   * Register a string with an ID.
   * \param s The string with which to associate the ID.
   * \param id A numeric value with which to identify this string.
   * \return A pointer to the copy of the string in this hash.
   * \remarks If the string is already registered with a different ID, the old
   *   ID will be replaced with the one specified here. If you would like the
   *   convenience of having the ID assigned automatically, then consider using
   *   csStringSet, instead.
   * 
   * \remarks If you do not care about the ID, but instead simply want to use
   *   the hash as a string \e set which merely records if a string is present,
   *   then you can omit \c id. To find out if a string is contained in the
   *   set, invoke Contains(). The same functionality can be accomplished via
   *   csStringSet, however csStringSet is more heavyweight because it also
   *   maintains a reverse-mapping from ID to string. Omitting the \c id makes
   *   for a good alternative to csStringSet when you do not require its extra
   *   bulk.
   */
  const char* Register (const char* s, StringID<Tag> id = 0)
  {
    char const* t = pool.Store(s);
    registry.PutUnique(t, id);
    return t;
  }

  /**
   * Request the ID for the given string.
   * \return The string's ID or csInvalidStringID if the string has not yet
   *   been registered.
   */
  StringID<Tag> Request (const char* s) const
  {
    return registry.Get(s, CS::InvalidStringID<Tag> ());
  }

  /**
   * Request the string for a given ID.
   * \return The string associated with the given ID, or the null pointer if
   *   the string has not yet been registered. If more than one string is
   *   associated with the ID, then one is returned (but specifically which one
   *   is unspecified).
   * \warning This operation is slow.  If you need to perform reverse lookups
   *   frequently, then instead consider using csStringSet or csStringHashReversible,
   *   in which reverse lookups are optimized.
   */
  const char* Request (StringID<Tag> id) const
  {
    GlobalIterator it(GetIterator());
    while (it.HasNext())
    {
      char const* s;
      StringID<Tag> const x = it.Next(s);
      if (x == id)
	return s;
    }
    return 0;
  }

  /**
   * Check if the hash contains a particular string.
   * \remarks This is rigidly equivalent to
   *   <tt>return Request(s) != csInvalidStringID</tt>.
   */
  bool Contains(char const* s) const
  { return Request(s) != InvalidStringID<Tag> (); }

  /**
   * Check if the hash contains a string with a particular ID.
   * \remarks This is rigidly equivalent to
   *   <tt>return Request(id) != NULL</tt>, but more idiomatic.
   * \warning This operation is slow.  If you need to check containment of ID's
   *   frequently, then instead consider using csStringSet, in which such
   *   checks are optimized.
   */
  bool Contains(StringID<Tag> id) const
  { return Request(id) != 0; }

  /**
   * Remove specified string.
   * \return True if a matching string was in thet set; else false.
   */
  bool Delete(char const* s)
  {
    return registry.DeleteAll(s);
  }

  /**
   * Remove a string with the specified ID.
   * \return True if a matching string was in thet set; else false.
   * \remarks If more than one string is associated with the ID, then one is
   *   removed (but specifically which one is unspecified).
   */
  bool Delete(StringID<Tag> id)
  {
    char const* s = Request(id);
    return s != 0 ? Delete(s) : false;
  }

  /**
   * Remove all stored strings.
   */
  void Empty ()
  {
    registry.Empty();
    pool.Empty();
  }

  /**
   * Delete all stored strings.
   * \deprecated Use Empty() instead.
   */
  /*CS_DEPRECATED_METHOD("Use Empty() instead.")*/
  void Clear ()
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
} // namespace Utility
} // namespace CS

/**
 * A string-to-ID hash table.
 */
typedef CS::Utility::StringHash<CS::StringSetTag::General> csStringHash;

#endif // __CS_STRHASH_H__
