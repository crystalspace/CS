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

#ifndef __CSREFARR_H__
#define __CSREFARR_H__

#include "csutil/ref.h"

/**
 * An array of smart pointers.
 */
template <class T>
class csRefArray
{
private:
  int count, limit, threshold;
  csRef<T>* root;

public:
  /**
   * Initialize object to hold initially 'ilimit' elements, and increase
   * storage by 'ithreshold' each time the upper bound is exceeded.
   */
  csRefArray (int ilimit = 0, int ithreshold = 0)
  {
    count = 0;
    limit = (ilimit > 0 ? ilimit : 0);
    threshold = (ithreshold > 0 ? ithreshold : 16);
    if (limit != 0)
      root = (csRef<T>*)calloc (limit, sizeof(csRef<T>));
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
      int i;
      for (i = 0 ; i < limit ; i++)
        root[i] = NULL;	// Clear ref.
      free (root);
      root = NULL;
      count = 0;
    }
  }

  /**
   * Destroy the container and release all contained references.
   */
  ~csRefArray ()
  {
    DeleteAll ();
  }

  /// Set vector length to n.
  void SetLength (int n)
  {
    // Free all items between new count and old count.
    int i;
    for (i = n ; i < count ; i++) root[i] = NULL;

    int old_count = count;
    count = n;

    if (n > limit || (limit > threshold && n < limit - threshold))
    {
      n = ((n + threshold - 1) / threshold ) * threshold;
      if (!n)
      {
        DeleteAll ();
      }
      else if (root == NULL)
        root = (csRef<T>*)calloc (n, sizeof(csRef<T>));
      else
      {
        csRef<T>* newroot = (csRef<T>*)calloc (n, sizeof(csRef<T>));
	memcpy (newroot, root, old_count * sizeof (csRef<T>));
	free (root);
	root = newroot;
      }
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

  /// Get a const reference.
  const csRef<T>& Get (int n) const
  {
    CS_ASSERT (n >= 0 && n < count);
    return root[n];
  }

  /// Get a const reference.
  const csRef<T>& operator [] (int n) const
  {
    CS_ASSERT (n >= 0 && n < count);
    return root[n];
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

  /// Push a element on 'top' of vector.
  int Push (T* what)
  {
    SetLength (count + 1);
    root [count - 1] = what;
    return (count - 1);
  }

  /// Pop an element from vector 'top'.
  csRef<T> Pop ()
  {
    csRef<T> ret = root [count - 1];
    SetLength (count - 1);
    return ret;
  }

  /// Return the top element but don't remove it.
  T* Top () const
  {
    return root [count - 1];
  }

  /// Delete element number 'n' from vector.
  bool Delete (int n)
  {
    if (n >= 0 && n < count)
    {
      root[n] = NULL;	// Clear element.
      const int ncount = count - 1;
      const int nmove = ncount - n;
      if (nmove > 0)
      {
        memmove (&root [n], &root [n + 1], nmove * sizeof (csRef<T>));
	// The following manual IncRef() is to make sure that
	// the element will not get deleted by the SetLength() below.
	if (root[ncount])
	  root[ncount]->IncRef ();
      }
      SetLength (ncount);
      return true;
    }
    else
      return false;
  }

  /// Delete the given element from vector.
  bool Delete (T* item)
  {
    int n = Find (item);
    if (n == -1) return false;
    else return Delete (n);
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
        memmove (&root [n + 1], &root [n], nmove * sizeof (csSome));
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
};

#endif // __CSREFARR_H__

