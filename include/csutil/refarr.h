/*
  Crystal Space Smart Pointers
  Copyright (C) 2002 by Jorrit Tyberghein and Matthias Braun

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

#ifndef __CS_REFARR_H__
#define __CS_REFARR_H__

#include "csutil/arraybase.h"
#include "csutil/ref.h"

template <class T>
class csRefArrayElementHandler
{
public:
  static void Construct (T* address, T const& src)
  {
  }

  static void Destroy (T* address)
  {
    *address = 0;	// Clear reference.
  }

  static void InitRegion (T* address, int count)
  {
    memset (address, 0, count*sizeof (T));
  }
};

#undef ElementHandler
#undef ArraySuper
#define ElementHandler csRefArrayElementHandler<csRef<T> >
#define ArraySuper csArrayBase<csRef<T>, ElementHandler >

/**
 * An array of smart pointers.
 */
template <class T>
class csRefArray : private ArraySuper	// Note! Private!
{
public:
  // We take the following public functions from csArrayBase<T> and
  // make them public here.
  using ArraySuper::Length;
  using ArraySuper::Capacity;
  //using ArraySuper::Find;
  //using ArraySuper::Sort;
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

  typedef int ArrayCompareFunction (T* item1, T* item2);
  typedef int ArrayCompareKeyFunction (T* item, void* key);
  
  /**
   * Initialize object to hold initially 'ilimit' elements, and increase
   * storage by 'ithreshold' each time the upper bound is exceeded.
   */
  csRefArray (int ilimit = 0, int ithreshold = 0)
  	: ArraySuper (ilimit, ithreshold)
  {
  }

  /// Copy constructor.
  csRefArray (const csRefArray<T>& source)
  {
    capacity = source.capacity;
    threshold = source.threshold;
    root = (csRef<T>*)calloc (capacity, sizeof(csRef<T>));
    count = source.Length ();
    for (int i = 0 ; i < count ; i++)
      root [i] = source[i];
  }
  
  /**
   * Transfer the entire contents of one array to the other. The end
   * result will be that this array will be completely empty and the
   * other array will have all items that originally were in this array.
   * This operation is very efficient.
   */
  void TransferTo (csRefArray<T>& destination)
  {
    ArraySuper::TransferTo ((ArraySuper&)destination);
  }

  /**
   * Destroy the container and release all contained references.
   */
  ~csRefArray ()
  {
    DeleteAll ();
  }

  /// Get a reference.
  csRef<T>& operator [] (int n)
  {
    CS_ASSERT (n >= 0);
    if (n >= count)
      SetLength (n + 1);
    return root[n];
  }

  /// Find a element in array and return its index (or -1 if not found).
  int Find (T* which) const
  {
    int i;
    for (i = 0 ; i < Length () ; i++)
      if (root[i] == which)
        return i;
    return -1;
  }

  /**
   * Find an element based on some key.
   */
  int FindKey (void* key,
    ArrayCompareKeyFunction* comparekey) const
  {
    int i;
    for (i = 0 ; i < Length () ; i++)
      if (comparekey (root[i], key) == 0)
        return i;
    return -1;
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

  /// Pop an element from vector 'top'.
  csPtr<T> Pop ()
  {
    csRef<T> ret = root [count - 1];
    SetLength (count - 1);
    return csPtr<T> (ret);
  }

  /// Return the top element but don't remove it.
  T* Top () const
  {
    return root [count - 1];
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
        memmove (&root [n + 1], &root [n], nmove * sizeof (csRef<T>));
	// The following manual IncRef() is to make sure that
	// the element will not get deleted later. This element is
	// currently (temporarily) duplicated in the root array so
	// that's why it needs an additional ref.
	if (root[n])
	  root[n]->IncRef ();
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
   */
  int InsertSorted (T* item, ArrayCompareFunction* compare)
  {
    int m = 0, l = 0, r = Length () - 1;
    while (l <= r)
    {
      m = (l + r) / 2;
      int cmp = compare (root [m], item);

      if (cmp == 0)
      {
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
    Insert (m, item);
    return m;
  }

  /// Same but for all elements
  void QuickSort (ArrayCompareFunction* compare)
  {
    if (count > 0)
      QuickSort (0, count - 1, compare);
  }
  
  /// Partially sort the array
  void QuickSort (int Left, int Right,
    ArrayCompareFunction* compare)
  {
  recurse:
    int i = Left, j = Right;
    int x = (Left + Right) / 2;
    do
    {
      while ((i != x) && (compare (root[i], root[x]) < 0))
	i++;
      while ((j != x) && (compare (root[j], root[x]) > 0))
	j--;
      if (i < j)
      {
	csRef<T> swap;
	swap = root[i];
	root[i] = root[j];
	root[j] = swap;
	if (x == i)
	  x = j;
	else if (x == j)
	  x = i;
      }
      if (i <= j)
      {
	i++;
	if (j > Left)
	  j--;
      }
    } while (i <= j);

    if (j - Left < Right - i)
    {
      if (Left < j)
	QuickSort (Left, j, compare);
      if (i < Right)
      {
	Left = i;
	goto recurse;
      }
    }
    else
    {
      if (i < Right)
	QuickSort (i, Right, compare);
      if (Left < j)
      {
	Right = j;
	goto recurse;
      }
    }
  }
  
};

#undef ElementHandler
#undef ArraySuper

#endif // __CS_REFARR_H__

