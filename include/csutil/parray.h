/*
  Crystal Space Pointer Array
  Copyright (C) 2003 by Jorrit Tyberghein

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

#ifndef __CS_PTRARR_H__
#define __CS_PTRARR_H__

#include "csutil/arraybase.h"

/**
 * An array of pointers. No ref counting is done on the elements in this
 * array. Use csRefArray if you want ref counting to happen.
 * This array will delete elements (using 'delete') as needed.
 * This array properly initializes new elements in the array to 0 (the NULL
 * pointer).
 */
template <class T>
class csPDelArray : private csArrayBase<T*>  // Note! Private inheritance here!
{
public:
  // We take the following public functions from csArrayBase<T> and
  // make them public here.
  using csArrayBase<T*>::Length;
  using csArrayBase<T*>::Capacity;
  using csArrayBase<T*>::Find;
  using csArrayBase<T*>::Sort;
  using csArrayBase<T*>::Get;
  using csArrayBase<T*>::operator[];

  /// This function prototype is used for csPDelArray::InsertSorted()
  typedef int ArrayCompareFunction (T const* item1, T const* item2);
  /// This function prototype is used for csPDelArray::FindKey()
  typedef int ArrayCompareKeyFunction (T const* item1, void* item2);

  /**
   * Initialize object to hold initially 'ilimit' elements, and increase
   * storage by 'ithreshold' each time the upper bound is exceeded.
   */
  csPDelArray (int ilimit = 0, int ithreshold = 0)
  	: csArrayBase<T*> (ilimit, ithreshold)
  {
  }

  /**
   * Clear entire vector.
   */
  void DeleteAll ()
  {
    int i;
    for (i = 0 ; i < count ; i++)
      delete root[i];
    DeleteRoot ();
  }

  /**
   * Truncate array to specified number of elements. The new number of
   * elements cannot exceed the current number of elements. Use SetLength()
   * for a more general way to enlarge the array.
   */
  void Truncate (int n)
  {
    CS_ASSERT(n >= 0);
    CS_ASSERT(n <= count);
    if (n < count)
    {
      for (int i = n; i < count; i++)
        delete root[i];
      SetLengthUnsafe(n);
    }
  }

  /**
   * Remove all elements.  Similar to DeleteAll(), but does not release memory
   * used by the array itself, thus making it more efficient for cases when the
   * number of contained elements will fluctuate.
   */
  void Empty ()
  {
    Truncate (0);
  }

  /**
   * Destroy the container.
   */
  ~csPDelArray ()
  {
    DeleteAll ();
  }

  /**
   * Transfer the entire contents of one array to the other. The end
   * result will be that this array will be completely empty and the
   * other array will have all items that originally were in this array.
   * This operation is very efficient.
   */
  void TransferTo (csPDelArray<T>& destination)
  {
    destination.DeleteAll ();
    destination.root = root;
    destination.count = count;
    destination.capacity = capacity;
    destination.threshold = threshold;
    root = 0;
    capacity = count = 0;
  }

  /// Set vector length to n.
  void SetLength (int n)
  {
    if (n <= count)
    {
      Truncate (n);
    }
    else
    {
      int old_len = Length ();
      SetLengthUnsafe (n);
      memset (root+old_len, 0, (n-old_len)*sizeof (T*));
    }
  }

  /// Put a pointer.
  void Put (int n, T* ptr)
  {
    CS_ASSERT (n >= 0);
    if (n >= count)
      SetLength (n + 1);
    delete root[n];
    root[n] = ptr;
  }

  /// Push a element on 'top' of vector.
  int Push (T* what)
  {
    SetLength (count + 1);
    root [count - 1] = what;
    return (count - 1);
  }

  /// Push a element on 'top' of vector if it is not already there.
  int PushSmart (T* what)
  {
    int n = Find (what);
    return (n == -1) ? Push (what) : n;
  }

  /**
   * Pop an element from vector 'top'. You are responsible for
   * deleting the object later.
   */
  T* Pop ()
  {
    T* ret = root [count - 1];
    root [count-1] = 0;
    SetLength (count - 1);
    return ret;
  }

  /// Return the top element but don't remove it.
  T* Top () const
  {
    return root [count - 1];
  }

  /**
   * Get and clear the element 'n' from vector. This spot in the array
   * will be set to 0. You are responsible for deleting the returned
   * pointer later.
   */
  T* GetAndClear (int n)
  {
    CS_ASSERT (n >= 0 && n < count);
    T* ret = root[n];
    root[n] = 0;
    return ret;
  }

  /**
   * Extract element number 'n' from vector. The element is deleted
   * from the array and returned. You are responsible for deleting the
   * pointer later.
   */
  T* Extract (int n)
  {
    T* rc = 0;
    if (n >= 0 && n < count)
    {
      rc = root[n]; root[n] = 0;
      const int ncount = count - 1;
      const int nmove = ncount - n;
      if (nmove > 0)
      {
        memmove (&root [n], &root [n + 1], nmove * sizeof (T*));
	root[count-1] = 0;	// Clear last element to prevent deletion.
      }
      SetLength (ncount);
    }
    return rc;
  }

  /// Delete element number 'n' from vector.
  bool DeleteIndex (int n)
  {
    T* p = Extract (n);
    if (p)
    {
      delete p;
      return true;
    }
    else
    {
      return false;
    }
  }

  /// Delete the given element from vector.
  bool Delete (T* item)
  {
    int n = Find (item);
    if (n == -1) return false;
    else return DeleteIndex (n);
  }

  /// Insert element 'Item' before element 'n'.
  bool Insert (int n, T* item)
  {
    if (n <= count)
    {
      SetLength (count + 1); // Increments 'count' as a side-effect.
      const int nmove = (count - n - 1);
      if (nmove > 0)
      {
        memmove (&root [n + 1], &root [n], nmove * sizeof (T*));
      }
      root [n] = item;
      return true;
    }
    else
     return false;
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

  /**
   * Find an element based on some key.
   * Assumes array is sorted.
   */
  int FindSortedKey (void* key,
    ArrayCompareKeyFunction* comparekey) const
  {
    int l = 0, r = Length () - 1;
    while (l <= r)
    {
      int m = (l + r) / 2;
      int cmp = comparekey (root [m], key);

      if (cmp == 0)
        return m;
      else if (cmp < 0)
        l = m + 1;
      else
        r = m - 1;
    }
    return -1;
  }

  /**
   * Insert an element at a sorted position.
   * Assumes array is already sorted.
   * If 'equal_index' is not 0 then this integer will be set
   * to the index of a duplicate item (if found). If no such
   * duplicate item exists it will be set to -1.
   */
  int InsertSorted (T* item, ArrayCompareFunction* compare,
	int* equal_index = 0)
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
};

#endif // __CS_PTRARR_H__

