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

#include <new>

/**
 * A templated array class.  The objects in this class are constructed via
 * copy-constructor and are destroyed when they are removed from the array or
 * the array is destroyed.  Note: If you want to store reference-counted object
 * pointers (such as iSomething*), then you should look at csRefArray instead
 * of this class.
 */
template <class T>
class csArray
{
private:
  int count;
  int capacity;
  int threshold;
  T* root;

  void ConstructElement (int n, T const& src)
  {
    new (static_cast<void*>(root + n)) T(src);
  }
  void DestroyElement (int n)
  {
    (root + n)->~T::T();
  }

  // Set array length.  NOTE: Do not make this public since it does not
  // properly construct/destroy elements.  To safely truncate the array, use
  // Truncate().  To safely set the capacity, use SetCapacity().
  void SetLengthUnsafe (int n)
  {
    count = n;
    if (n > capacity || (capacity > threshold && n < capacity - threshold))
    {
      n = ((n + threshold - 1) / threshold ) * threshold;
      if (root == 0)
        root = (T*)malloc (n * sizeof(T));
      else
        root = (T*)realloc (root, n * sizeof(T));
      capacity = n;
    }
  }

public:
  /**
   * Initialize object to have initial capacity of 'icapacity' elements, and to
   * increase storage by 'ithreshold' each time the upper bound is exceeded.
   */
  csArray (int icapacity = 0, int ithreshold = 0)
  {
    count = 0;
    capacity = (icapacity > 0 ? icapacity : 0);
    threshold = (ithreshold > 0 ? ithreshold : 16);
    if (capacity != 0)
      root = (T*)malloc (capacity * sizeof(T));
    else
      root = 0;
  }

  /// Clear entire vector, releasing all memory.
  void DeleteAll ()
  {
    for (int i = 0; i < count; i++)
      DestroyElement (i);
    if (root != 0)
    {
      free (root);
      root = 0;
      capacity = 0;
      count = 0;
    }
  }

  /// Truncate array to specified number of elements.
  void Truncate(int n)
  {
    CS_ASSERT(n >= 0);
    CS_ASSERT(n <= count);
    if (n < count)
    {
      for (int i = n; i < count; i++)
        DestroyElement(i);
      SetLengthUnsafe(n);
    }
  }

  /**
   * Remove all elements.  Similar to DeleteAll(), but does not release memory
   * used by the array itself, thus making it more efficient for cases when the
   * number of contained elements will fluctuate.
   */
  void Empty()
  { Truncate(0); }


  /// Destroy the container.
  ~csArray ()
  {
    DeleteAll ();
  }

  /// Query vector length.
  int Length () const
  {
    return count;
  }

  /// Query vector capacity.  Note that you should rarely need to do this.
  int Capacity () const
  {
    return capacity;
  }

  /*
   * Set vector capacity to approximately 'n' elements.  Never sets the
   * capacity to fewer than the current number of elements in the array.  See
   * Truncate() if you need to adjust the number of actual array elements.
   */
  void SetCapacity (int n)
  {
    if (n > Length())
      SetLengthUnsafe(n);
  }

  /// Get an element (const).
  T const& Get (int n) const
  {
    CS_ASSERT (n >= 0 && n < count);
    return root[n];
  }

  /// Get an element (non-const).
  T& Get (int n)
  {
    CS_ASSERT (n >= 0 && n < count);
    return root[n];
  }

  /// Get an element (const).
  T const& operator [] (int n) const
  {
    return Get(n);
  }

  /// Get an element (non-const).
  T& operator [] (int n)
  {
    return Get(n);
  }

  /// Find a element in array and return its index (or -1 if not found).
  int Find (T const& which) const
  {
    for (int i = 0, n = Length(); i < n; i++)
      if (root[i] == which)
        return i;
    return -1;
  }

  /// Push an element onto the tail end of the array. Returns index of element.
  int Push (T const& what)
  {
    SetLengthUnsafe (count + 1);
    ConstructElement (count - 1, what);
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
    CS_ASSERT (length > 0);
    T ret(root [count - 1]);
    DestroyElement (count - 1);
    SetLengthUnsafe (count - 1);
    return ret;
  }

  /// Return the top element but do not remove it.
  T const& Top () const
  {
    CS_ASSERT (length > 0);
    return root [count - 1];
  }

  /// Delete element number 'n' from vector.
  bool Delete (int n)
  {
    if (n >= 0 && n < count)
    {
      int const ncount = count - 1;
      int const nmove = ncount - n;
      DestroyElement (n);
      if (nmove > 0)
        memmove (root + n, root + n + 1, nmove * sizeof(T));
      SetLengthUnsafe (ncount);
      return true;
    }
    else
      return false;
  }

  /// Delete the given element from vector.
  bool Delete (T const& item)
  {
    int const n = Find (item);
    if (n >= 0)
      return Delete(n);
    return false;
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
      ConstructElement (n, item);
      return true;
    }
    else
      return false;
  }
};

#endif
