/*
    Crystal Space utility library: vector class interface
    Copyright (C) 1998,1999,2000 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_GARRAY_H__
#define __CS_GARRAY_H__

/**\file 
 * vector class interface
 */

/**
 * An automatically growing array of objects. Warning! Do NOT use
 * this for objects that require a constructor. Do not use this
 * for pointers. For normal pointers you should use csPArray and for
 * reference counted pointers you should use csRefArray instead of this
 * class.
 */
template <class T>
class csGrowingArray
{
private:
  int count, limit, threshold;
  int shrinklimit;
  T* root;
  int RefCount;

public:
  /**
   * Initialize object to hold initially 'ilimit' elements, and increase
   * storage by 'ithreshold' each time the upper bound is exceeded.
   */
  csGrowingArray (int ilimit = 0, int ithreshold = 0, int ishrinklimit = 0)
  {
    RefCount = 0;
    count = 0;
    limit = (ilimit > 0 ? ilimit : 0);
    threshold = (ithreshold > 0 ? ithreshold : 16);
    shrinklimit = (ishrinklimit > 0 ? ishrinklimit : 1000);
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
    if (root)
    {
      free (root);
      root = NULL;
      count = 0;
    }
  }

  /**
   * Destroy the container.
   */
  ~csGrowingArray ()
  {
    DeleteAll ();
  }

  // Reference counting.
  void IncRef () { RefCount++; }

  // Reference counting. Delete array when reference reaches 0.
  void DecRef ()
  {
    if (RefCount == 1) { SetLimit (0); count = 0; }
    RefCount--;
  }

  /// Set maximum size of array.
  void SetLimit (int inlimit)
  {
    if (limit == inlimit) return;
    if ((limit = inlimit) != 0)
      root = (T*)realloc (root, limit * sizeof (T));
    else if (root) { free (root); root = NULL; }
  }

  /// Set vector length to n.
  void SetLength (int n)
  {
    count = n;
    int newlimit = ((count + (threshold - 1)) / threshold) * threshold;
    if (newlimit > limit || newlimit < limit-shrinklimit)
      SetLimit (newlimit);
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

  /// Get the pointer to the start of the array.
  T* GetArray ()
  {
    return root;
  }

  /// Get a const reference.
  const T& Get (int n) const
  {
    CS_ASSERT (n >= 0 && n < count);
    return root[n];
  }

  /// Get a const reference.
  const T& operator [] (int n) const
  {
    CS_ASSERT (n >= 0 && n < count);
    return root[n];
  }

  /// Get a reference.
  T& operator [] (int n)
  {
    CS_ASSERT (n >= 0);
    if (n >= count)
      SetLength (n + 1);
    return root[n];
  }

  /// Push a element on 'top' of vector.
  int Push (const T& what)
  {
    SetLength (count + 1);
    root [count - 1] = what;
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
    T ret = root [count - 1];
    SetLength (count - 1);
    return ret;
  }

  /// Return the top element but don't remove it.
  T& Top () const
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
      root [n] = item;
      return true;
    }
    else
     return false;
  }
};


#endif // __CS_GARRAY_H__
