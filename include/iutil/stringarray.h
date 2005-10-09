/*
    Crystal Space String Array SCF Interface
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

#ifndef __CS_IUTIL_STRINGARRAY_H__
#define __CS_IUTIL_STRINGARRAY_H__

#include "csutil/scf_interface.h"

/**\file
 * String Array interface
 */
/**\addtogroup util
 * @{ */


/// This is an SCF-compatible interface for csStringArray.
struct iStringArray : public virtual iBase
{
  SCF_INTERFACE(iStringArray, 2,0,0);
  /// Get array length.
  virtual size_t GetSize () const = 0;

  /**
   * Get array length.
   * \deprecated Use GetSize() instead.
   */
  virtual size_t Length () const = 0;

  /// Push a string onto the stack.
  virtual void Push (const char *value) = 0;

  /**
   * Pop an element from tail end of array.
   * \remarks Caller is responsible for invoking delete[] on the returned
   *   string when no longer needed.
   */
  virtual char *Pop () = 0;

  /// Get a particular string from the array.
  virtual char const *Get (size_t n) const = 0;

  /**
   * Find a string, case-sensitive.
   * \return csArrayItemNotFound if not found, else item index.
   * \remarks Works with sorted and unsorted arrays, but FindSortedKey() is
   *   faster on sorted arrays.
   */
  virtual size_t Find (const char *value) const = 0;

  /**
   * Find a string, case-insensitive.
   * \return csArrayItemNotFound if not found, else item index.
   * \remarks Works with sorted and unsorted arrays, but FindSortedKey() is
   *   faster on sorted arrays.
   */
  virtual size_t FindCaseInsensitive (const char *value) const = 0;

  /**
   * Find an element based on some key, using a comparison function.
   * \return csArrayItemNotFound if not found, else item index.
   * \remarks The array must be sorted.
   */
  virtual size_t FindSortedKey (const char *value) const = 0;

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
  virtual size_t Contains(const char* str, bool case_sensitive = true) const=0;

  /**
   * Sort array.
   * \param case_sensitive If true, consider case when performing comparison.
   *   (default: yes)
   */
  virtual void Sort (bool case_sensitive = true) = 0;

  /// Delete string \c n from the array.
  virtual bool DeleteIndex (size_t n) = 0;

  /// Insert a string before entry \c n in the array.
  virtual bool Insert (size_t n, char const *value) = 0;

  /// Remove all strings from array, releasing allocated memory.
  virtual void Empty () = 0;

  /**
   * Remove all strings from array.
   * \deprecated Use Empty() instead.
   */
  virtual void DeleteAll () = 0;

  /**
   * Return true if the array is empty.
   * \remarks Rigidly equivalent to <tt>return GetSize() == 0</tt>, but more
   *   idiomatic.
   */
  virtual bool IsEmpty() const = 0;
};

/** @} */

#endif // __CS_IUTIL_STRINGARRAY_H__
