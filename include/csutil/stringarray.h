/*
  Crystal Space String Array
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

#ifndef __CS_STRINGARRAY_H__
#define __CS_STRINGARRAY_H__

#include <stdarg.h>
#include "csutil/util.h"
#include "csutil/arraybase.h"

typedef int csStringArrayCompareFunction (char const* item1, char const* item2);
typedef int csStringArraySortFunction (void const* item1, void const* item2);

/**
 * An array of strings. This array will properly make copies of the strings
 * and delete those copies using delete[] later.
 */
class csStringArray : private csArrayBase<char*>  // Note! Private.
{
public:
  // We take the following public functions from csArrayBase<T> and
  // make them public here.
  using csArrayBase<char*>::Length;
  using csArrayBase<char*>::Capacity;
  // using csArrayBase<char*>::Find; We have our own version

  /**
   * Initialize object to hold initially 'ilimit' elements, and increase
   * storage by 'ithreshold' each time the upper bound is exceeded.
   */
  csStringArray (int ilimit = 0, int ithreshold = 0)
  	: csArrayBase<char*> (ilimit, ithreshold)
  {
  }

  /**
   * Clear entire vector.
   */
  void DeleteAll ()
  {
    int i;
    for (i = 0 ; i < count ; i++)
      delete[] root[i];
    DeleteRoot ();
  }

  /**
   * Destroy the container.
   */
  ~csStringArray ()
  {
    DeleteAll ();
  }

  /**
   * Transfer the entire contents of one array to the other. The end
   * result will be that this array will be completely empty and the
   * other array will have all items that originally were in this array.
   * This operation is very efficient.
   */
  void TransferTo (csStringArray& destination)
  {
    destination.DeleteAll ();
    destination.root = root;
    destination.count = count;
    destination.capacity = capacity;
    destination.threshold = threshold;
    root = 0;
    capacity = count = 0;
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
        delete[] root[i];
      SetLengthUnsafe(n);
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
      memset (root+old_len, 0, (n-old_len)*sizeof (char*));
    }
  }

  /// Get a string.
  char const* Get (int n) const
  {
    CS_ASSERT (n >= 0 && n < count);
    return root[n];
  }

  /// Get a string.
  char const* operator [] (int n) const
  {
    CS_ASSERT (n >= 0 && n < count);
    return root[n];
  }

  /// Make a copy of a string and remember it.
  void Put (int n, char const* ptr)
  {
    CS_ASSERT (n >= 0);
    if (n >= count)
      SetLength (n + 1);
    delete root[n];
    root[n] = csStrNew (ptr);
  }

  /**
   * Find a element in array and return its index (or -1 if not found).
   * This version will compare pointers and does NOT do an actual string
   * compare. Use Find() for that.
   */
  int FindPointer (char const* which) const
  {
    int i;
    for (i = 0 ; i < Length () ; i++)
      if (root[i] == which)
        return i;
    return -1;
  }

  /**
   * Find a element in array and return its index (or -1 if not found).
   * This version will compare strings in a case-sensitive manner.
   */
  int Find (char const* which) const
  {
    int i;
    for (i = 0 ; i < Length () ; i++)
    {
      if (root[i] == which)
        return i;
      if (root[i] != 0 && which != 0 && !strcmp (root[i], which))
        return i;
    }
    return -1;
  }

  /// Push a element on 'top' of vector (makes a copy of the string).
  int Push (char const* what)
  {
    SetLength (count + 1);
    root [count - 1] = csStrNew (what);
    return (count - 1);
  }
  /**
   * Push a printf-style string onto the list (makes copy of string after
   * formatting).
   */
  int FormatPush (char const * fmt, ...)
  {
    char str[1000];
    va_list args;
    va_start(args, fmt);
    vsprintf(str, fmt, args);
    va_end(args);
    return Push(str);
  }

  /**
   * Push a element on 'top' of vector if it is not already there
   * (makes a copy of the string).
   * This function does a case-sensitive string compare.
   */
  int PushSmart (char const* what)
  {
    int n = Find (what);
    return (n == -1) ? Push (what) : n;
  }

  /**
   * Pop an element from vector 'top'. Caller is responsible for
   * deleting the object later (via delete[]).
   */
  char* Pop ()
  {
    char* ret = root [count - 1];
    root [count - 1] = 0;
    SetLength (count - 1);
    return ret;
  }

  /// Return the top element but don't remove it.
  char const* Top () const
  {
    return root [count - 1];
  }

  /**
   * Get and clear the element 'n' from vector. This spot in the array
   * will be set to 0. Caller is responsible for deleting the returned
   * pointer later (via delete[]).
   */
  char* GetAndClear (int n)
  {
    CS_ASSERT (n >= 0 && n < count);
    char* ret = root[n];
    root[n] = 0;
    return ret;
  }

  /**
   * Extract element number 'n' from vector. The element is deleted
   * from the array and returned. Caller is responsible for deleting the
   * pointer later (using delete[]).
   */
  char* Extract (int n)
  {
    char* rc = 0;
    if (n >= 0 && n < count)
    {
      rc = root[n];
      const int ncount = count - 1;
      const int nmove = ncount - n;
      if (nmove > 0)
      {
        memmove (&root [n], &root [n + 1], nmove * sizeof (char*));
      }
      root[ncount] = 0;	// Clear last element to prevent deletion.
      SetLength (ncount);
    }
    return rc;
  }

  /// Delete element number 'n' from vector.
  bool DeleteIndex (int n)
  {
    char* p = Extract (n);
    if (p)
    {
      delete[] p;
      return true;
    }
    else
    {
      return false;
    }
  }

  /**
   * Delete the given element from vector.
   * This function does a case sensitive string compare to find the element.
   */
  bool Delete (char const* item)
  {
    int n = Find (item);
    if (n == -1) return false;
    else return DeleteIndex (n);
  }

  /**
   * Insert element 'Item' before element 'n'. This function makes a
   * copy.
   */
  bool Insert (int n, char const* item)
  {
    if (n <= count)
    {
      SetLength (count + 1); // Increments 'count' as a side-effect.
      const int nmove = (count - n - 1);
      if (nmove > 0)
      {
        memmove (&root [n + 1], &root [n], nmove * sizeof (char*));
      }
      root [n] = csStrNew (item);
      return true;
    }
    else
     return false;
  }

  /**
   * Find an element based on some key.
   */
  int FindSorted (const char* key,
  	csStringArrayCompareFunction* compare) const
  {
    int l = 0, r = Length () - 1;
    while (l <= r)
    {
      int m = (l + r) / 2;
      int cmp = compare (root [m], key);

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
  int InsertSorted (char const* item, csStringArrayCompareFunction* compare)
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

  static int CaseSensitiveCompare (char const* item1, char const* item2)
  {
    return strcmp (item1, item2);
  }

  static int CaseInsensitiveCompare (char const* item1, char const* item2)
  {
    return strcasecmp (item1, item2);
  }
 
  static int CaseSensitiveCompare (void const* item1, void const* item2)
  {
    return strcmp (*(char const**)item1, *(char const**)item2);
  }

  static int CaseInsensitiveCompare (void const* item1, void const* item2)
  {
    return strcasecmp (*(char const**)item1, *(char const**)item2);
  }
 
  /**
   * Find an element in a case sensitive way.
   */
  int FindSorted (const char* key) const
  {
    return FindSorted (key, CaseSensitiveCompare);
  }

  /**
   * Insert an element at a sorted position.
   * Assumes array is already sorted.
   * Sorting assumes case-sensitive string compare.
   */
  int InsertSorted (char const* item)
  {
    return InsertSorted (item, CaseSensitiveCompare);
  }

  /**
   * Insert an element at a sorted position.
   * Assumes array is already sorted.
   * Sorting assumes case-insensitive string compare.
   */
  int InsertSortedCase (char const* item)
  {
    return InsertSorted (item, CaseInsensitiveCompare);
  }

  /**
   * Sort array.
   */
  void Sort (csStringArraySortFunction* compare)
  {
    qsort (root, Length (), sizeof (char*), compare);
  }

  /**
   * Sort array based on case sensitive string compare function.
   */
  void Sort ()
  {
    qsort (root, Length (), sizeof (char*), CaseSensitiveCompare);
  }

  /**
   * Sort array based on case insensitive string compare function.
   */
  void SortCase ()
  {
    qsort (root, Length (), sizeof (char*), CaseInsensitiveCompare);
  }
};

#endif // __CS_STRINGARRAY_H__
