/*
  Crystal Space Generic Base for all array classes
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
#ifndef __CSUTIL_BASEARRAY_H__
#define __CSUTIL_BASEARRAY_H__

/**
 * The base for all templated array classes in CS. Note that
 * there are no virtual functions here for performance reasons.
 */
template <class T>
class csArrayBase
{
protected:
  int count;
  int capacity;
  int threshold;
  T* root;

  // Adjust internal capacity of this array.
  void AdjustCapacity (int n)
  {
    if (n > capacity || (capacity > threshold && n < capacity - threshold))
    {
      n = ((n + threshold - 1) / threshold ) * threshold;
      capacity = n;
      if (root == 0)
        root = (T*)malloc (capacity * sizeof(T));
      else
        root = (T*)realloc (root, capacity * sizeof(T));
    }
  }

  // Completely delete the root array and reset the count.
  // Doesn't delete any items in it.
  void DeleteRoot ()
  {
    if (root)
    {
      free (root);
      root = 0;
      capacity = count = 0;
    }
  }

  // Set array length.  NOTE: Do not make this public since it does not
  // properly construct/destroy elements.  To safely truncate the array, use
  // Truncate().  To safely set the capacity, use SetCapacity().
  void SetLengthUnsafe (int n)
  {
    if (n > capacity)
      AdjustCapacity (n);
    count = n;
  }

  /**
   * Initialize object to have initial capacity of 'icapacity' elements, and to
   * increase storage by 'ithreshold' each time the upper bound is exceeded.
   */
  csArrayBase (int icapacity = 0, int ithreshold = 0)
  {
    count = 0;
    capacity = (icapacity > 0 ? icapacity : 0);
    threshold = (ithreshold > 0 ? ithreshold : 16);
    if (capacity != 0)
      root = (T*)malloc (capacity * sizeof(T));
    else
      root = 0;
  }

public:
  /// return the number of elements in the Array
  int Length () const
  {
    return count;
  }

  /// Query vector capacity.  Note that you should rarely need to do this.
  int Capacity () const
  {
    return capacity;
  }

  /// Find a element in array and return its index (or -1 if not found).
  int Find (T const& which) const
  {
    int i;
    for (i = 0 ; i < Length () ; i++)
      if (root[i] == which)
        return i;
    return -1;
  }
};

#endif
