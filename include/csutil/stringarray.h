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

#include "csutil/util.h"

typedef int csStringArrayCompareFunction (char const* item1, char const* item2);
typedef int csStringArrayCompareKeyFunction (char const* item, void* key);
typedef int csStringArraySortFunction (void const* item1, void const* item2);

/**
 * An array of strings. This array will properly make copies of the strings
 * and delete those copies using delete[] later.
 */
class csStringArray
{
private:
  int count, limit, threshold;
  char** root;

  // Set vector length to n.  If 'dealloc' is true and the array is being
  // shortened, then the excess items are deallocated.  If it is false, then
  // the items are not deallocated.  Passing false for this argument is unsafe.
  // Do so only if you know precisely what you are doing.  Obvious examples of
  // functions which might want to partake of this unsafe behavior are those
  // which return the excess strings to the caller with the expectation that
  // the caller will be responsible for freeing them with delete[].
  void SetLength (int n, bool dealloc)
  {
    // Free all items between new count and old count.
    int i;
    for (i = n ; i < count ; i++)
    {
      if (dealloc)
	  delete[] root[i];
      root[i] = 0;
    }

    int old_count = count;
    count = n;

    if (n > limit || (limit > threshold && n < limit - threshold))
    {
      n = ((n + threshold - 1) / threshold ) * threshold;
      if (!n)
      {
        DeleteAll ();
      }
      else if (root == 0)
      {
        root = (char**)calloc (n, sizeof(char*));
      }
      else
      {
        char** newroot = (char**)calloc (n, sizeof(char*));
	memcpy (newroot, root, old_count * sizeof (char*));
	free (root);
	root = newroot;
      }
      limit = n;
    }
  }

public:
  /**
   * Initialize object to hold initially 'ilimit' elements, and increase
   * storage by 'ithreshold' each time the upper bound is exceeded.
   */
  csStringArray (int ilimit = 0, int ithreshold = 0)
  {
    count = 0;
    limit = (ilimit > 0 ? ilimit : 0);
    threshold = (ithreshold > 0 ? ithreshold : 16);
    if (limit != 0)
      root = (char**)calloc (limit, sizeof(char*));
    else
      root = 0;
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
        delete[] root[i];
      free (root);
      root = 0;
      limit = count = 0;
    }
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
    destination.limit = limit;
    destination.threshold = threshold;
    root = 0;
    limit = count = 0;
  }

  /// Set vector length to n.
  void SetLength (int n)
  {
    SetLength(n, true);
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
      SetLength (n + 1, true);
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
    SetLength (count + 1, true);
    root [count - 1] = csStrNew (what);
    return (count - 1);
  }
  /// Push a printf-style string onto the list (makes copy of string after formatting).
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
    SetLength (count - 1, false);
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
	root[count-1] = 0;	// Clear last element to prevent deletion.
      }
      SetLength (ncount, false);
    }
    return rc;
  }

  /// Delete element number 'n' from vector.
  bool Delete (int n)
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
    else return Delete (n);
  }

  /**
   * Insert element 'Item' before element 'n'. This function makes a
   * copy.
   */
  bool Insert (int n, char const* item)
  {
    if (n <= count)
    {
      SetLength (count + 1, true); // Increments 'count' as a side-effect.
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
  int FindSortedKey (void* key,
  	csStringArrayCompareKeyFunction* comparekey) const
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
