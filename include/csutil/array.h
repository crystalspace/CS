/*
  Crystal Space Template Arrays
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
#include "csutil/ref.h"

/**
 * A templated array class. The objects in this class are constructed via
 * copy-constructor and are delete when they're removed from the array or the
 * array goes.
 * Note: If you want to store iSomething*, then you should look at csRefArray
 * instead of this class! csRefArray additionally takes care of the
 * refcounting.
 */
template <class T>
class csArray
{
private:
  int count, limit, threshold;
  T* root;

  void ConstructElement (int n, const T& elem)
  {
    new (static_cast<void*>(root+n)) T(elem);
  }
  void DestructElement (int n)
  {
    (root+n)->~T ();
  }

public:
  /**
   * Initialize object to hold initially 'ilimit' elements, and increase
   * storage by 'ithreshold' each time the upper bound is exceeded.
   */
  csArray (int ilimit = 0, int ithreshold = 0)
  {
    count = 0;
    limit = (ilimit > 0 ? ilimit : 0);
    threshold = (ithreshold > 0 ? ithreshold : 16);
    if (limit != 0)
      root = (T*)malloc (limit * sizeof(T));
    else
      root = NULL;
  }

  /**
   * Clear entire vector.
   */
  void DeleteAll ()
  {
    for (int i=0; i<count; i++)
    {
      DestructElement (i);
    }
    if (root)
    {
      free (root);
      root = NULL;
      limit = count = 0;
    }
  }

  /**
   * Destroy the container.
   */
  ~csArray ()
  {
    DeleteAll ();
  }

  /// Set vector length to n.
  void SetLength (int n)
  {
    count = n;

    if (n > limit || (limit > threshold && n < limit - threshold))
    {
      n = ((n + threshold - 1) / threshold ) * threshold;
      if (!n)
        DeleteAll ();
      else if (root == NULL)
        root = (T*)malloc (n * sizeof(T));
      else
        root = (T*)realloc (root, n * sizeof(T));
      limit = n;
    }
  }

  /// Query vector length.
  int Length () const
  {
    return count;
  }

  /// Query vector limit.
  int Limit () const
  {
    return limit;
  }

  /// Get an element
  const T& Get (int n) const
  {
    CS_ASSERT (n >= 0 && n < count);
    return root[n];
  }

  /// Get a const reference.
  const T& operator [] (int n) const
  {
    return Get(n);
  }

  /// Find a element in array and return its index (or -1 if not found).
  int Find (T which) const
  {
    int i;
    for (i = 0 ; i < Length () ; i++)
      if (root[i] == which)
        return i;
    return -1;
  }

  /// Push a element on 'top' of vector.
  int Push (const T& what)
  {
    SetLength (count + 1);
    ConstructElement (count-1, what);
    return (count - 1);
  }

  /// Push a element on 'top' of vector if it is not already there.
  int PushSmart (const T& what)
  {
    int n = Find (what);
    return (n == -1) ? Push (what) : n;
  }

  /// Pop an element from vector 'top'.
  T Pop ()
  {
    CS_ASSERT (length>0);
    const T& ret = root [count - 1];
    DestructElement (count-1);
    SetLength (count - 1);
    return ret;
  }

  /// Return the top element but don't remove it.
  const T& Top () const
  {
    return root [count - 1];
  }

  /// Delete element number 'n' from vector.
  bool Delete (int n)
  {
    if (n >= 0 && n < count)
    {
      const int ncount = count - 1;
      const int nmove = ncount - n;
      DestructElement (n);
      if (nmove > 0)
      {
        memmove (&root [n], &root [n + 1], nmove * sizeof (T));
      }
      SetLength (ncount);
      return true;
    }
    else
      return false;
  }

  /// Delete the given element from vector.
  bool Delete (const T& item)
  {
    int n = Find (item);
    if (n == -1) return false;
    else return Delete (n);
  }

  /// Insert element 'Item' before element 'n'.
  bool Insert (int n, const T& item)
  {
    if (n <= count)
    {
      SetLength (count + 1); // Increments 'count' as a side-effect.
      const int nmove = (count - n - 1);
      if (nmove > 0)
      {
        memmove (&root [n + 1], &root [n], nmove * sizeof (T));
      }
      ConstructElement (n, item);
      return true;
    }
    else
     return false;
  }
};

#endif

