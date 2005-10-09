/*
    Crystal Space String Array SCF interface
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

#ifndef __CS_SCFSTRINGARRAY_H__
#define __CS_SCFSTRINGARRAY_H__

/**\file
 * Implementation for iStringArray
 */

#include "csextern.h"
#include "csutil/scf_implementation.h"
#include "csutil/stringarray.h"
#include "iutil/stringarray.h"

/// This class is a thin wrapper around csStringArray with SCF capability
class CS_CRYSTALSPACE_EXPORT scfStringArray : 
  public scfImplementation1<scfStringArray, iStringArray>
{
  csStringArray v;

public:

  /// Create a iStringArray from scratch.
  scfStringArray (int limit = 16, int delta = 16) 
    : scfImplementationType (this), v (limit, delta)
  { }

  /// Destructor - nothing to do.
  virtual ~scfStringArray ()
  { }

  /// Get array length.
  virtual size_t GetSize () const
  {
    return v.Length ();
  }

  /**
   * Get array length.
   * \deprecated Use GetSize() instead.
   */
  virtual size_t Length () const
  {
    return GetSize ();
  }

  /// Push a string onto the stack.
  virtual void Push (char const *value)
  {
    v.Push ((char*)value);
  }

  /**
   * Pop an element from tail end of array.
   * \remarks Caller is responsible for invoking delete[] on the returned
   *   string when no longer needed.
   */
  virtual char *Pop ()
  {
    return v.Pop ();
  }

  /// Get a particular string from the array.
  virtual char const *Get (size_t n) const
  {
    return v.Get (n);
  }

  /**
   * Find a string, case-sensitive.
   * \return csArrayItemNotFound if not found, else item index.
   * \remarks Works with sorted and unsorted arrays, but FindSortedKey() is
   *   faster on sorted arrays.
   */
  virtual size_t Find (const char *value) const
  {
    return v.Find (value);
  }

  /**
   * Find a string, case-insensitive.
   * \return csArrayItemNotFound if not found, else item index.
   * \remarks Works with sorted and unsorted arrays, but FindSortedKey() is
   *   faster on sorted arrays.
   */
  virtual size_t FindCaseInsensitive (const char *value) const
  {
    return v.FindCaseInsensitive (value);
  }

  /**
   * Find an element based on some key, using a comparison function.
   * \return csArrayItemNotFound if not found, else item index.
   * \remarks The array must be sorted.
   */
  virtual size_t FindSortedKey (const char *value) const
  {
    return v.FindSortedKey ((char*)value);
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
  virtual size_t Contains(const char* str, bool case_sensitive = true) const
  {
    return v.Contains (str, case_sensitive);
  }

  /**
   * Sort array.
   * \param case_sensitive If true, consider case when performing comparison.
   *   (default: yes)
   */
  virtual void Sort (bool case_sensitive = true)
  {
    v.Sort (case_sensitive);
  }

  /// Delete string \c n from the array.
  virtual bool DeleteIndex (size_t n)
  {
    return v.DeleteIndex (n);
  }

  /// Insert a string before entry \c n in the array.
  virtual bool Insert (size_t n, char const *value)
  {
    return v.Insert (n, (char*)value);
  }

  /// Remove all strings from array, releasing allocated memory.
  virtual void Empty ()
  {
    v.Empty();
  }

  /**
   * Remove all strings from array.
   * \deprecated Use Empty() instead.
   */
  virtual void DeleteAll ()
  {
    Empty();
  }

  /**
   * Return true if the array is empty.
   * \remarks Rigidly equivalent to <tt>return GetSize() == 0</tt>, but more
   *   idiomatic.
   */
  virtual bool IsEmpty() const
  {
    return v.IsEmpty();
  }
};

#endif // __CS_SCFSTRINGARRAY_H__
