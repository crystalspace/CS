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
#include "csextern.h"
#include "array.h"
#include "util.h"

class csStringArrayElementHandler
{
public:
  static void Construct (const char** address, const char* const& src)
  {
    *address = csStrNew (src);
  }

  static void Destroy (const char** address)
  {
    delete[] (char*)*address;
  }

  static void InitRegion (const char** address, size_t count)
  {
    memset (address, 0, count*sizeof (const char*));
  }
};

/**
 * An array of strings. This array will properly make copies of the strings
 * and later delete those copies via delete[].
 */
class csStringArray : public csArray<const char*, csStringArrayElementHandler>
{
private:
  typedef csArray<const char*, csStringArrayElementHandler> superclass;

public:
  /**
   * Initialize object to hold initially \c limit elements, and increase
   * storage by \c threshold each time the upper bound is exceeded.
   */
  csStringArray (size_t limit = 0, size_t threshold = 0)
  	: superclass(limit, threshold)
  {
  }

  /// Case-sensitive comparision function for strings.
  static int CaseSensitiveCompare (const char* const &item1,
				   const char* const &item2)
  {
    return strcmp (item1, item2);
  }

  /// Case-insensitive comparision function for strings.
  static int CaseInsensitiveCompare (const char* const &item1,
				     const char* const &item2)
  {
    return csStrCaseCmp (item1, item2);
  }

  /**
   * Sort array based on comparison function.
   */
  void Sort (int(*compare)(char const* const&, char const* const&))
  {
    superclass::Sort (compare);
  }

  /**
   * Sort array.
   * \param case_sensitive If true, consider case when performing comparison.
   *   (default: yes)
   */
  void Sort (bool case_sensitive = true)
  {
    if (case_sensitive)
      Sort (CaseSensitiveCompare);
    else
      Sort (CaseInsensitiveCompare);
  }

  /**
   * Find an element based on some key, using a comparison function.
   * \return csArrayItemNotFound if not found, else item index.
   * \remarks The array must be sorted.
   */
  size_t FindSortedKey (csArrayCmpDecl(char const*, char const*) comparekey,
    size_t* candidate = 0) const
  {
    return superclass::FindSortedKey(comparekey, candidate);
  }

  /**
   * Find an element.
   * \return csArrayItemNotFound if not found, else item index.
   * \remarks The array must be sorted.
   */
  size_t FindSortedKey (char const* key, bool case_sensitive = true,
    size_t* candidate = 0) const
  {
    int(*cf)(char const* const&, char const* const&) =
      case_sensitive ? CaseSensitiveCompare : CaseInsensitiveCompare;
    return FindSortedKey(csArrayCmp<char const*, char const*>(key, cf),
      candidate);
  }

  /**
   * Insert an element at a sorted position.
   * \remarks Assumes array is already sorted.
   */
  size_t InsertSorted (const char* item, bool case_sensitive = true,
    size_t* equal_index = 0)
  {
    int(*cf)(char const* const&, char const* const&) =
      case_sensitive ? CaseSensitiveCompare : CaseInsensitiveCompare;
    return superclass::InsertSorted (item, cf, equal_index);
  }

  /**
   * Pop an element from tail end of array.
   * \remarks Caller is responsible for invoking delete[] on the returned
   *   string when no longer needed.
   */
  char* Pop ()
  {
    CS_ASSERT (GetSize () > 0);
    size_t l = GetSize () - 1;
    char* ret = (char*)Get (l);
    InitRegion (l, 1);
    SetSize (l);
    return ret;
  }

  /**
   * Find a string, case-sensitive.
   * \return csArrayItemNotFound if not found, else item index.
   * \remarks Works with sorted and unsorted arrays, but FindSortedKey() is
   *   faster on sorted arrays.
   */
  size_t Find (const char* str) const
  {
    for (size_t i = 0; i < GetSize (); i++)
      if (! strcmp (Get (i), str))
        return i;
    return (size_t)-1;
  }

  /**
   * Find a string, case-insensitive.
   * \return csArrayItemNotFound if not found, else item index.
   * \remarks Works with sorted and unsorted arrays, but FindSortedKey() is
   *   faster on sorted arrays.
   */
  size_t FindCaseInsensitive (const char* str) const
  {
    for (size_t i = 0; i < GetSize (); i++)
      if (!csStrCaseCmp (Get (i), str))
        return i;
    return (size_t)-1;
  }

  /**
   * Alias for Find() and FindCaseInsensitive().
   * \param str String to look for in array.
   * \param case_sensitive If true, consider case when performing comparison.
   *   (default: yes)
   * \return csArrayItemNotFound if not found, else item index.
   * \remarks Works with sorted and unsorted arrays, but FindSortedKey() is
   *   faster on sorted arrays.
   * <p>
   * \remarks Some people find Contains() more idiomatic than Find().
   */
  size_t Contains(const char* str, bool case_sensitive = true) const
  {
    return case_sensitive ? Find(str) : FindCaseInsensitive(str);
  }
};

#endif // __CS_STRINGARRAY_H__
