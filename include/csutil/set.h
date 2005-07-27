/*
    Copyright (C) 2003 by Mat Sutcliffe <oktal@gmx.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_UTIL_SET_H__
#define __CS_UTIL_SET_H__

#include "csutil/hash.h"

/**\file
 * A basic set for objects.
 */

/**\addtogroup util_containers
 * @{ */

/**
 * This class implements a basic set for objects.
 * You can basically use this to test for the occurrence
 * of some object quickly.
 */
template <class T> 
class csSet
{
public:
  typedef csHash<bool, T> HashType;

private:
  typedef typename_qualifier HashType::GlobalIterator ParentIter;
  HashType map;

public:
  /* Unfortunately, MSVC6 barfs if we derive this from ParentIter. */
  /// An iterator class for the set.
  class GlobalIterator
  {
  protected:
    ParentIter iter;
    GlobalIterator () {}
    GlobalIterator (const csSet<T>* s) : iter(s->map.GetIterator()) {}

  public:
    friend class csSet<T>;

    GlobalIterator (const GlobalIterator& o) : iter(o.iter) {}
    GlobalIterator& operator=(const GlobalIterator& o)
    { iter = o.iter; return *this; }

    /// Returns a boolean indicating whether or not the set has more elements.
    bool HasNext () const
    { return iter.HasNext(); }

    /// Get the next element's value.
    T Next()
    {
      T key;
      iter.Next(key);
      return key;
    }
  };
  friend class GlobalIterator;

  /**
   * Construct a new empty set.
   * \a size, \a grow_rate, and \a max_size allow fine-tuning of how the set
   *   manages its internal allocations.
   */
  csSet (int size = 23, int grow_rate = 5, int max_size = 20000)
  	: map (size, grow_rate, max_size)
  {
  }

  /**
   * Add an object to this set.
   * This will do nothing if the object is already present.
   */
  void Add (const T& object)
  {
    if (!Contains (object))
      AddNoTest (object);
  }

  /**
   * Add an object to this set.
   * This function does not test if the object is already
   * there. This is used for efficiency reasons. But use
   * with care!
   */
  void AddNoTest (const T& object)
  {
    map.Put (object, true);
  }

  /**
   * Test if an object is in this set.
   */
  bool Contains (const T& object) const
  {
    return map.Contains (object);
  }

  /**
   * Test if an object is in this set.
   * \remarks This is rigidly equivalent to Contains(object), but may be
   *   considered more idiomatic by some.
   */
  bool In (const T& object) const
  { return Contains(object); }

  /**
   * Delete all elements in the set.
   */
  void DeleteAll ()
  {
    map.DeleteAll ();
  }

  /// Delete all elements in the set. (Idiomatic alias for DeleteAll().)
  void Empty() { DeleteAll(); }

  /**
   * Delete an object from the set. This function
   * does nothing if the object is not in the set.
   * Return true if the object was present.
   */
  bool Delete (const T& object)
  {
    return map.Delete (object, true);
  }

  /// Get the number of elements in the set.
  size_t GetSize () const
  {
    return map.GetSize ();
  }

  /**
   * Return true if the set is empty.
   * \remarks Rigidly equivalent to <tt>return GetSize() == 0</tt>, but more
   *   idiomatic.
   */
  bool IsEmpty() const
  {
    return GetSize() == 0;
  }

  /**
   * Return an iterator for the set which iterates over all elements.
   * \warning Modifying the set while you have open iterators will cause
   *   undefined behaviour.
   */
  GlobalIterator GetIterator () const
  {
    return GlobalIterator(this);
  }
};

/** @} */

#endif // __CS_UTIL_SET_H__
