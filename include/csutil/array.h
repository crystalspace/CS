/*
  Crystal Space Generic Array Template
  Copyright (C) 2003 by Matze Braun

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
#ifndef __CSUTIL_ARRAY_H__
#define __CSUTIL_ARRAY_H__

#include "csutil/arraybase.h"

// hack: work around problems caused by #defining 'new'
#ifdef CS_EXTENSIVE_MEMDEBUG
# undef new
#endif
#include <new>

template <class T>
class csArrayElementHandler
{
public:
  static void Construct (T* address, T const& src)
  {
    new (static_cast<void*>(address)) T(src);
  }

  static void Destroy (T* address)
  {
    address->T::~T();
  }

  static void InitRegion (T* address, int count)
  {
    for (int i = 0 ; i < count ; i++)
      Construct (root + i, T ());
  }
};

#undef ElementHandler
#undef ArraySuper
#define ElementHandler csArrayElementHandler<T>
#define ArraySuper csArrayBase<T, ElementHandler >

/**
 * A templated array class.  The objects in this class are constructed via
 * copy-constructor and are destroyed when they are removed from the array or
 * the array is destroyed.  Note: If you want to store reference-counted object
 * pointers (such as iSomething*), then you should look at csRefArray instead
 * of this class.
 */
template <class T>
class csArray : private ArraySuper	// Note! Private inheritance!
{
public:
  // We take the following public functions from csArrayBase<T> and
  // make them public here.
  using ArraySuper::Length;
  using ArraySuper::Capacity;
  using ArraySuper::Find;
  using ArraySuper::Sort;
  using ArraySuper::Get;
  using ArraySuper::operator[];
  using ArraySuper::DeleteAll;
  using ArraySuper::Truncate;
  using ArraySuper::Empty;
  using ArraySuper::SetLength;
  using ArraySuper::AdjustCapacity;
  using ArraySuper::ShrinkBestFit;
  using ArraySuper::DeleteIndex;
  using ArraySuper::DeleteRange;
  using ArraySuper::Delete;

  /// This function prototype is used for csArray::InsertSorted()
  typedef int ArrayCompareFunction (T const& item1, T const& item2);
  /// This function prototype is used for csArray::FindKey()
  typedef int ArrayCompareKeyFunction (T const& item1, void* item2);

  /**
   * Initialize object to have initial capacity of 'icapacity' elements, and to
   * increase storage by 'ithreshold' each time the upper bound is exceeded.
   */
  csArray (int icapacity = 0, int ithreshold = 0)
  	: ArraySuper (icapacity, ithreshold)
  {
  }

  /// Copy constructor.
  csArray (const csArray& source)
  {
    ArraySuper::CopyFrom ((ArraySuper&)source);
  }
  
  /// Assignment operator.
  csArray<T>& operator= (const csArray& other)
  {
    ArraySuper::CopyFrom ((ArraySuper&)other);
    return *this;
  }

  /**
   * Transfer the entire contents of one array to the other. The end
   * result will be that this array will be completely empty and the
   * other array will have all items that originally were in this array.
   * This operation is very efficient.
   */
  void TransferTo (csArray& destination)
  {
    ArraySuper::TransferTo ((ArraySuper&)destination);
  }

  /**
   * Set the actual number of items in this array. This can be used to
   * shrink an array (works like Truncate() then in which case it will properly
   * destroy all truncated objects) or to enlarge an array in which case
   * it will properly set the new capacity and construct all new items
   * based on the given item.
   */
  void SetLength (int n, T const& what)
  {
    if (n <= count)
    {
      Truncate (n);
    }
    else
    {
      int old_len = Length ();
      SetLengthUnsafe (n);
      for (int i = old_len ; i < n ; i++)
        ElementHandler::Construct (root + i, what);
    }
  }

  /// Get an element (non-const).
  T& Get (int n)
  {
    CS_ASSERT (n >= 0 && n < count);
    return root[n];
  }

  /// Get an element (non-const).
  T& operator [] (int n)
  {
    return Get(n);
  }

  /**
   * Find an element based on some key.
   */
  int FindKey (void* key, ArrayCompareKeyFunction* comparekey) const
  {
    int i;
    for (i = 0 ; i < Length () ; i++)
      if (comparekey (root[i], key) == 0)
        return i;
    return -1;
  }


  /// Push an element onto the tail end of the array. Returns index of element.
  int Push (T const& what)
  {
    SetLengthUnsafe (count + 1);
    ElementHandler::Construct (root + count - 1, what);
    return (count - 1);
  }

  /*
   * Push a element onto the tail end of the array if not already present.
   * Returns index of newly pushed element or index of already present element.
   */
  int PushSmart (T const& what)
  {
    int const n = Find (what);
    return (n < 0) ? Push (what) : n;
  }

  /// Pop an element from tail end of array.
  T Pop ()
  {
    CS_ASSERT (count > 0);
    T ret(root [count - 1]);
    ElementHandler::Destroy (root + count - 1);
    SetLengthUnsafe (count - 1);
    return ret;
  }

  /// Return the top element but do not remove it.
  T const& Top () const
  {
    CS_ASSERT (count > 0);
    return root [count - 1];
  }

  /// Insert element 'item' before element 'n'.
  bool Insert (int n, T const& item)
  {
    if (n <= count)
    {
      SetLengthUnsafe (count + 1); // Increments 'count' as a side-effect.
      int const nmove = (count - n - 1);
      if (nmove > 0)
        memmove (root + n + 1, root + n, nmove * sizeof(T));
      ElementHandler::Construct (root + n, item);
      return true;
    }
    else
      return false;
  }

  /**
   * Get the portion of the array between low and high inclusive
   */
  csArray<T> Section (int low, int high) const
  {
    CS_ASSERT (low >= 0 && high < count && high >= low);
    csArray<T> sect (high - low + 1);
    for (int i = low; i <= high; i++) sect.Push (root[i]);
    return sect;
  }

  /// The default ArrayCompareFunction for InsertSorted()
  static int DefaultCompare (T const &item1, T const &item2)
  {
    if (item1 < item2) return -1;
    else if (item1 > item2) return 1;
    else return 0;
  }

  /// The default ArrayCompareKeyFunction for FindKey()
  static int DefaultCompareKey (T const &item1, void *item2)
  {
    if (item1 < item2) return -1;
    else if (item1 > item2) return 1;
    else return 0;
  }

  /**
   * Find an element based on some key, using a csArrayCompareKeyFunction.
   * The array must be sorted. Returns -1 if element does not exist.
   */
  int FindSortedKey (void* key, ArrayCompareKeyFunction* comparekey
    = DefaultCompareKey, int *candidate = 0) const
  {
    int m = 0, l = 0, r = Length () - 1;
    while (l <= r)
    {
      m = (l + r) / 2;
      int cmp = comparekey (root [m], key);

      if (cmp == 0)
      {
        if (candidate) *candidate = -1;
        return m;
      }
      else if (cmp < 0)
        l = m + 1;
      else
        r = m - 1;
    }
    if (candidate) *candidate = m;
    return -1;
  }

  /**
   * Insert an element at a sorted position, using a csArrayCompareFunction.
   * Assumes array is already sorted.
   */
  int InsertSorted (const T &item, ArrayCompareFunction* compare
    = DefaultCompare, int* equal_index = 0)
  {
    int m = 0, l = 0, r = Length () - 1;
    while (l <= r)
    {
      m = (l + r) / 2;
      int cmp = compare (root [m], item);

      if (cmp == 0)
      {
	if (equal_index) *equal_index = m;
        Insert (++m, item);
        return m;
      }
      else if (cmp < 0)
        l = m + 1;
      else
        r = m - 1;
    }
    if (r == m)
      m++;
    if (equal_index) *equal_index = -1;
    Insert (m, item);
    return m;
  }

  /** Iterator for the Array object */
  class Iterator
  {
  public:
    /** Returns true if the next Next() call will return an element */
    bool HasNext()
    { return currentelem < array.Length(); }

    /** Returns the next element in the array. */
    const T& Next()
    { return array.Get(currentelem++); }

  protected:
    Iterator(const csArray<T>& newarray)
	: currentelem(0), array(newarray)
    { }
    friend class csArray<T>;
    
  private:
    int currentelem;
    const csArray<T>& array;
  };

  /** Returns an Iterator which traverses the Array */
  Iterator GetIterator() const
  { return Iterator(*this); }
};

#undef ElementHandler
#undef ArraySuper

#ifdef CS_EXTENSIVE_MEMDEBUG
# define new CS_EXTENSIVE_MEMDEBUG_NEW
#endif

#endif
