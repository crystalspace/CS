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

// hack: work around problems caused by #defining 'new'
#ifdef CS_EXTENSIVE_MEMDEBUG
# undef new
#endif
#include <new>

/// This function prototype is used for Sort()
typedef int ArraySortCompareFunction (void const* item1,
	void const* item2);

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
      Construct (address + i, T ());
  }
};

/**
 * A templated array class.  The objects in this class are constructed via
 * copy-constructor and are destroyed when they are removed from the array or
 * the array is destroyed.  Note: If you want to store reference-counted object
 * pointers (such as iSomething*), then you should look at csRefArray instead
 * of this class.
 */
template <class T, class ElementHandler = csArrayElementHandler<T> >
class csArray
{
private:
  int count;
  int capacity;
  int threshold;
  T* root;

protected:
  /**
   * Initialize a region. This is a dangerous function to use because it
   * does not properly destruct the items in the array.
   */
  void InitRegion (int start, int count)
  {
    ElementHandler::InitRegion (root+start, count);
  }

private:
  /// Copy from one array to this.
  void CopyFrom (const csArray& source)
  {
    if (&source != this)
    {
      DeleteAll ();
      threshold = source.threshold;
      SetLengthUnsafe (source.Length ());
      for (int i=0 ; i<source.Length() ; i++)
        ElementHandler::Construct (root + i, source[i]);
    }
  }

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

public:
  /// This function prototype is used for csArray::InsertSorted()
  typedef int ArrayCompareFunction (T const& item1, T const& item2);
  /// This function prototype is used for csArray::FindKey()
  typedef int ArrayCompareKeyFunction (T const& item1, void* item2);

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

  ~csArray ()
  {
    DeleteAll ();
  }

  /// Copy constructor.
  csArray (const csArray& source)
  {
    root = 0;
    capacity = 0;
    count = 0;
    CopyFrom (source);
  }
  
  /// Assignment operator.
  csArray<T>& operator= (const csArray& other)
  {
    CopyFrom (other);
    return *this;
  }

  /// Return the number of elements in the Array
  int Length () const
  {
    return count;
  }

  /// Query vector capacity.  Note that you should rarely need to do this.
  int Capacity () const
  {
    return capacity;
  }

  /**
   * Transfer the entire contents of one array to the other. The end
   * result will be that this array will be completely empty and the
   * other array will have all items that originally were in this array.
   * This operation is very efficient.
   */
  void TransferTo (csArray& destination)
  {
    if (&destination != this)
    {
      destination.DeleteAll ();
      destination.root = root;
      destination.count = count;
      destination.capacity = capacity;
      destination.threshold = threshold;
      root = 0;
      capacity = count = 0;
    }
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

  /// Get an element (non-const).
  T& Get (int n)
  {
    CS_ASSERT (n >= 0 && n < count);
    return root[n];
  }

  /**
   * Get an item from the array. If the number of elements in this
   * array is too small the array will be automatically extended.
   */
  T& GetExtend (int n)
  {
    CS_ASSERT (n >= 0);
    if (n >= count)
      SetLength (n+1);
    return root[n];
  }

  /// Get an element (non-const).
  T& operator [] (int n)
  {
    return Get(n);
  }

  /// Put an element at some position.
  void Put (int n, T const& what)
  {
    CS_ASSERT (n >= 0);
    if (n >= count)
      SetLength (n+1);
    ElementHandler::Destroy (root + n);
    ElementHandler::Construct (root + n, what);
  }

  /**
   * Find an element based on some key.
   */
  int FindKey (void* key, ArrayCompareKeyFunction* comparekey) const
  {
    for (int i = 0 ; i < Length () ; i++)
      if (comparekey (root[i], key) == 0)
        return i;
    return -1;
  }


  /// Push an element onto the tail end of the array. Returns index of element.
  int Push (T const& what)
  {
    SetLengthUnsafe (count + 1);
    ElementHandler::Construct (root + count - 1, what);
    return count - 1;
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
  static int DefaultCompareKey (T const &item1, void* p)
  {
    T const& item2 = *(T const*)p;
    if (item1 < item2) return -1;
    else if (item1 > item2) return 1;
    else return 0;
  }

  /**
   * Find an element based on some key, using a csArrayCompareKeyFunction.
   * The array must be sorted. Returns -1 if element does not exist.
   */
  int FindSortedKey (void* key, ArrayCompareKeyFunction* comparekey
    = DefaultCompareKey, int* candidate = 0) const
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

  /// Find a element in array and return its index (or -1 if not found).
  int Find (T const& which) const
  {
    for (int i = 0 ; i < Length () ; i++)
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

  /** Iterator for the Array object */
  class Iterator
  {
  public:
    /** Copy constructor. */
    Iterator(Iterator const& r) :
      currentelem(r.currentelem), array(r.array) {}

    /** Assignment operator. */
    Iterator& operator=(Iterator const& r)
    { currentelem = r.currentelem; array = r.array; return *this; }

    /** Returns true if the next Next() call will return an element */
    bool HasNext()
    { return currentelem < array.Length(); }

    /** Returns the next element in the array. */
    const T& Next()
    { return array.Get(currentelem++); }

    /** Reset the array to the first element */
    void Reset()
    { currentelem = 0; }
  protected:
    Iterator(const csArray<T, ElementHandler>& newarray)
	: currentelem(0), array(newarray)
    { }
    friend class csArray<T, ElementHandler>;
    
  private:
    int currentelem;
    const csArray<T, ElementHandler>& array;
  };

  /** Returns an Iterator which traverses the Array */
  Iterator GetIterator() const
  { return Iterator(*this); }
};

#ifdef CS_EXTENSIVE_MEMDEBUG
# define new CS_EXTENSIVE_MEMDEBUG_NEW
#endif

#endif
