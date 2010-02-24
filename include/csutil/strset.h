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
#include "csutil/strhash.h"

/**\file
 * String-to-ID hash table.
 */

namespace CS
{
namespace Utility
{
/**
 * The string set is a collection of unique strings. Each string has an ID
 * number. The most important operation is to request a string, which means to
 * return the ID for the string, adding it to the collection if not already
 * present.  This is useful when you need to work with strings but want the
 * performance characteristics of simple numeric comparisons.  Rather than
 * performing string comparisons, you instead compare the numeric string ID's.
 *
 * If \a Locked is true operations on an instance of the set are locked are
 * for concurrent accesses.
 *
 * \sa csStringHash
 * \sa iStringSet
 */
template<typename Tag, bool Locked = false>
class StringSet
{
 private:
  typedef CS::Threading::OptionalMutex<Locked> MutexType;
  StringHash<Tag> registry;
  csHash<const char*, CS::StringID<Tag> > reverse; // ID to string mapping.
  /* Inherit from OptionalMutex<> to avoid spending extra memory if locking
     is disabled */
  struct LockAndId : public MutexType
  {
    unsigned int next_id;

    LockAndId() : next_id (0) {}
  };
  mutable LockAndId lockAndId;

  void Copy(StringSet const& s)
  {
    if (&s != this)
    {
      CS::Threading::ScopedLock<MutexType> l1 (lockAndId);
      CS::Threading::ScopedLock<MutexType> l2 (s.lockAndId);
      registry = s.registry;
      reverse  = s.reverse;
      lockAndId.next_id  = s.lockAndId.next_id;
    }
  }

public:
  typedef csStringHash::GlobalIterator GlobalIterator;

public:
  /// Constructor.
  StringSet (size_t size = 23) : registry(size), reverse(size) {}
  /// Copy constructor.
  StringSet (StringSet const& s) { Copy(s); }
  /// Destructor.
  ~StringSet () {}
  /// Assignment operator.
  StringSet& operator=(StringSet const& s) { Copy(s); return *this; }

  /**
   * Request the numeric ID for the given string.
   * \return The ID of the string.
   * \remarks Creates a new ID if the string is not yet present in the set,
   *   else returns the previously assigned ID.
   */
  CS::StringID<Tag> Request (const char* s)
  {
    CS::Threading::ScopedLock<MutexType> lock (lockAndId);
    CS::StringID<Tag> id = registry.Request(s);
    if (id == CS::InvalidStringID<Tag> ())
    {
      const char* t = registry.Register(s, lockAndId.next_id);
      id = lockAndId.next_id++;
      reverse.Put (id, t);
    }
    return id;
  }

  /**
   * Request the string corresponding to the given ID.
   * \return Null if the string * has not been requested (yet), else the string
   *   corresponding to the ID.
   */
  char const* Request (CS::StringID<Tag> id) const
  {
    CS::Threading::ScopedLock<MutexType> lock (lockAndId);
    return reverse.Get(id, 0);
  }

  /**
   * Check if the set contains a particular string.
   */
  bool Contains(char const* s) const
  {
    CS::Threading::ScopedLock<MutexType> lock (lockAndId);
    return registry.Request(s) != CS::InvalidStringID<Tag> ();
  }

  /**
   * Check if the set contains a string with a particular ID.
   * \remarks This is rigidly equivalent to
   *   <tt>return Request(id) != NULL</tt>, but more idomatic.
   */
  bool Contains(CS::StringID<Tag> id) const
  { 
    CS::Threading::ScopedLock<MutexType> lock (lockAndId);
    return Request(id) != 0; 
  }

  /**
   * Remove specified string.
   * \return True if a matching string was in thet set; else false.
   */
  bool Delete(char const* s)
  {
    CS::Threading::ScopedLock<MutexType> lock (lockAndId);
    CS::StringID<Tag> const id = registry.Request(s);
    bool const ok = (id != csInvalidStringID);
    if (ok)
    {
      registry.Delete(s);
      reverse.DeleteAll(id);
    }
    return ok;
  }

  /**
   * Remove a string with the specified ID.
   * \return True if a matching string was in thet set; else false.
   */
  bool Delete(CS::StringID<Tag> id)
  {
    CS::Threading::ScopedLock<MutexType> lock (lockAndId);
    char const* s = reverse.Get(id,0);
    bool const ok = (s != 0);
    if (ok)
    {
      registry.Delete(s);
      reverse.DeleteAll(id);
    }
    return ok;
  }

  /**
   * Remove all stored strings. When new strings are registered again, new
   * ID values will be used; the old ID's will not be re-used.
   */
  void Empty ()
  {
    CS::Threading::ScopedLock<MutexType> lock (lockAndId);
    registry.Empty();
    reverse.Empty();
  }

  /**
   * Remove all stored strings.
   * \deprecated Deprecated in 1.3. Use Empty() instead.
   */
  CS_DEPRECATED_METHOD_MSG("Use Empty() instead.")
  void Clear ()
  { Empty(); }

  /// Get the number of elements in the hash.
  size_t GetSize () const
  { 
    CS::Threading::ScopedLock<MutexType> lock (lockAndId);
    return registry.GetSize (); 
  }

  /**
   * Return true if the hash is empty.
   * \remarks Rigidly equivalent to <tt>return GetSize() == 0</tt>, but more
   *   idiomatic.
   */
  bool IsEmpty() const
  { 
    CS::Threading::ScopedLock<MutexType> lock (lockAndId);
    return GetSize() == 0; 
  }

  /**
   * Return an iterator for the set which iterates over all strings.
   * \warning Modifying the set while you have open iterators will result
   *   undefined behaviour.
   * \warning The iterator will <b>not</b> respect locking of the string set!
   */
  GlobalIterator GetIterator () const
  { return registry.GetIterator(); }
};
} // namespace Utility
} // namespace CS

typedef CS::Utility::StringSet<CS::StringSetTag::General> csStringSet;

#endif // __CS_STRSET_H__
