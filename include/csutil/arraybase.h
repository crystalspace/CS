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

/// This function prototype is used for Sort()
typedef int ArraySortCompareFunction (void const* item1,
	void const* item2);

/**
 * The base for all templated array classes in CS. Note that
 * there are no virtual functions here for performance reasons.
 */
template <class T, class ElementHandler>
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

  /**
   * Transfer the entire contents of one array to the other. The end
   * result will be that this array will be completely empty and the
   * other array will have all items that originally were in this array.
   * This operation is very efficient.
   */
  void TransferTo (csArrayBase<T, ElementHandler>& destination)
  {
    if (&destination == this) return;
    destination.DeleteAll ();
    destination.root = root;
    destination.count = count;
    destination.capacity = capacity;
    destination.threshold = threshold;
    root = 0;
    capacity = count = 0;
  }

  /// Copy from one array to this.
  void CopyFrom (const csArrayBase<T, ElementHandler>& source)
  {
    if (&source == this) return;
    DeleteAll ();
    threshold = source.threshold;
    SetLengthUnsafe (source.Length ());
    for (int i=0 ; i<source.Length() ; i++)
      ElementHandler::Construct (root + i, source[i]);
  }

public:
  ~csArrayBase ()
  {
    DeleteAll ();
  }

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

  /**
   * Sort array.
   */
  void Sort (ArraySortCompareFunction* compare)
  {
    qsort (root, Length (), sizeof (T), compare);
  }

  /// Get an element (const).
  T const& Get (int n) const
  {
    CS_ASSERT (n >= 0 && n < count);
    return root[n];
  }

  /// Get a const reference.
  T const& operator [] (int n) const
  {
    CS_ASSERT (n >= 0 && n < count);
    return root[n];
  }

  /**
   * Clear entire vector.
   */
  void DeleteAll ()
  {
    if (root)
    {
      int i;
      for (i = 0 ; i < count ; i++)
        ElementHandler::Destroy (root + i);
      free (root);
      root = 0;
      capacity = count = 0;
    }
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
        ElementHandler::Destroy (root + i);
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
      ElementHandler::InitRegion (root + old_len, n-old_len);
    }
  }

  /**
   * Set vector capacity to approximately 'n' elements.  Never sets the
   * capacity to fewer than the current number of elements in the array.  See
   * Truncate() or SetLength() if you need to adjust the number of actual
   * array elements.
   */
  void SetCapacity (int n)
  {
    if (n > Length ())
      AdjustCapacity (n);
  }

  /**
   * Make the array just as big as it needs to be. This is useful in cases
   * where you know the array isn't going to be modified anymore in order
   * to preserve memory.
   */
  void ShrinkBestFit ()
  {
    if (count == 0)
    {
      DeleteAll ();
    }
    else if (count != capacity)
    {
      capacity = count;
      root = (T*)realloc (root, capacity * sizeof(T));
    }
  }

  /// Delete element number 'n' from vector.
  bool DeleteIndex (int n)
  {
    if (n >= 0 && n < count)
    {
      int const ncount = count - 1;
      int const nmove = ncount - n;
      ElementHandler::Destroy (root + n);
      if (nmove > 0)
        memmove (root + n, root + n + 1, nmove * sizeof(T));
      SetLengthUnsafe (ncount);
      return true;
    }
    else
      return false;
  }

  /**
   * Delete a given range (inclusive). This routine will clamp start and end
   * to the size of the array.
   */
  void DeleteRange (int start, int end)
  {
    if (start >= count) return;
    if (end < 0) return;
    if (start < 0) start = 0;
    if (end >= count) end = count-1;
    int i;
    for (i = start ; i < end ; i++)
      ElementHandler::Destroy (root + i);

    int const range_size = end-start+1;
    int const ncount = count - range_size;
    int const nmove = count - end - 1;
    if (nmove > 0)
      memmove (root + start, root + start + range_size, nmove * sizeof(T));
    SetLengthUnsafe (ncount);
  }

  /// Delete the given element from vector.
  bool Delete (T const& item)
  {
    int const n = Find (item);
    if (n >= 0)
      return DeleteIndex (n);
    return false;
  }
};

#endif

