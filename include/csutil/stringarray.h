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

/**\file
 * Array of strings
 */

#include <stdarg.h>
#include "csextern.h"
#include "csutil/array.h"
#include "csutil/util.h"
#include "csutil/csstring.h"

class csStringArrayElementHandler : public csArrayElementHandler<const char*>
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

namespace CS
{
namespace Utility
{
/**
 * An array of strings. This array will properly make copies of the strings
 * and later delete those copies via delete[].
 */
 
template <class Allocator = CS::Memory::AllocatorMalloc,
          class CapacityHandler = csArrayCapacityFixedGrow<16> >
class StringArray :
  public csArray<const char*, csStringArrayElementHandler, Allocator,
                 CapacityHandler>
{
private:
  typedef csArray<const char*, csStringArrayElementHandler, Allocator,
                  CapacityHandler> superclass;

public:
  /**
   * Initialize object to hold initially \c limit elements, and increase
   * storage by \c threshold each time the upper bound is exceeded.
   */
  StringArray (size_t limit = 0, const CapacityHandler& ch = CapacityHandler())
  	: superclass(limit, ch)
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
  size_t FindSortedKey (csArrayCmp<char const*, char const*> comparekey,
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
    CS_ASSERT (this->GetSize () > 0);
    size_t l = this->GetSize () - 1;
    char* ret = (char*)this->Get (l);
    this->InitRegion (l, 1);
    this->SetSize (l);
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
    for (size_t i = 0; i < this->GetSize (); i++)
      if (! strcmp (this->Get (i), str))
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
    for (size_t i = 0; i < this->GetSize (); i++)
      if (!csStrCaseCmp (this->Get (i), str))
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

  /**
   * Mode how SplitString() treats consecutive occurance of delimiters.
   */
  enum ConsecutiveDelimiterMode
  {
    /// Split at each different delimiter.
    delimSplitEach,
    /**
     * Ignore consecutive delimiters (except the first). In other words,
     * any number of consecutive delimiters is treated like there was only
     * one delimiter.
     */
    delimIgnore,
    /**
     * Only ignore consecutive delimiters if they're different.
     */
    delimIgnoreDifferent
  };

  /**
   * Initialize object to hold initially \c limit elements, and increase
   * storage by \c threshold each time the upper bound is exceeded.
   * Additionally load in this array the splitted string provided.
   * \param str The string to split and place in this array.
   * \param delimiters The delimiters to use to split the string.
   * \param delimMode The way to split this array
   */
  StringArray (const char* str, const char* delimiters, 
               ConsecutiveDelimiterMode delimMode = delimSplitEach,
               size_t limit = 0, const CapacityHandler& ch = CapacityHandler())
  	: superclass(limit, ch)
  {
    SplitString(str, delimiters, delimMode);
  }
  
  /**
   * Add a number of strings to this array by splitting \a str at characters
   * from \a delimiters. It will start from the first char and won't ignore
   * delimiters before the first word (in other words even with delimIgnore
   * you'll get at least an empty string if the string starts with delimiters).
   */
  size_t SplitString (const char* str, const char* delimiters, 
    ConsecutiveDelimiterMode delimMode = delimSplitEach)
  {
    size_t num = 0;
    csString currentString = "";
    int lastDelim = -1;

    const char* p = str;
    while (*p != 0)
    {
      if (strchr (delimiters, *p))
      {
        bool newString = true;
        switch (delimMode)
        {
          case delimSplitEach:
            break;
          case delimIgnore:
            newString = lastDelim == -1;
            break;
          case delimIgnoreDifferent:
            newString = (lastDelim == -1) || (lastDelim == *p);
            break;
        }
        if (newString)
        {
          this->Push (currentString);
          currentString.Empty();
          num++;
          lastDelim = *p;
        }
      }
      else
      {
        currentString.Append (*p);
        lastDelim = -1;
      }
      p++;
    }

    this->Push (currentString);
    return num + 1;
  }
};
} // namespace Utility
} // namespace CS

/**
 * An array of strings.
 */
class csStringArray : 
  public CS::Utility::StringArray<CS::Memory::AllocatorMalloc,
                                  csArrayCapacityDefault>
{
public:
  csStringArray (size_t limit = 0, size_t threshold = 0)
    : CS::Utility::StringArray<CS::Memory::AllocatorMalloc, 
                               csArrayCapacityDefault> (limit, threshold)
  {
  }
  csStringArray (const char* str, const char* delimiters, 
               ConsecutiveDelimiterMode delimMode = delimSplitEach,
               size_t limit = 0, size_t threshold = 0)
    : CS::Utility::StringArray<CS::Memory::AllocatorMalloc, 
                               csArrayCapacityDefault> (str, delimiters, 
                               delimMode, limit, threshold)
  {
  }
};

#endif // __CS_STRINGARRAY_H__
