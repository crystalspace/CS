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

#ifndef __CS_IUTIL_STRINGARRAY_H__
#define __CS_IUTIL_STRINGARRAY_H__

#include "csutil/scf.h"

/**\file
 * String Array interface
 */
/**\addtogroup util
 * @{ */
SCF_VERSION (iStringArray, 0, 0, 2);

/// This is an SCF-compatible interface for csStringArray.
struct iStringArray : public iBase
{
  /// Query array length.
  virtual int Length () const = 0;

  /// Push a string onto the stack.
  virtual void Push (const char *value) = 0;

  /// Pop a string from the top of stack.
  virtual char *Pop () = 0;

  /// Get Nth string in vector.
  virtual char const *Get (int n) const = 0;

  /**
   * Find a string, case-sensitive. Returns -1 if not found, else item index.
   * Works with unsorted arrays.  For sorted arrays, FindSortedKey() is faster.
   */
  virtual int Find (const char *value) const = 0;

  /**
   * Find a string, case-insensitive. Returns -1 if not found, else item index.
   * Works with unsorted arrays.  For sorted arrays, FindSortedKey() is faster.
   */
  virtual int FindCaseInsensitive (const char *value) const = 0;

  /// Find index of a string in a pre-sorted string array.
  virtual int FindSortedKey (const char *value) const = 0;

  /// Sort the string array.
  virtual void Sort () = 0;

  /// Delete Nth string in the array.
  virtual bool DeleteIndex (int n) = 0;

  /// Insert a string before Nth string in the array.
  virtual bool Insert (int n, char const *value) = 0;

  /// Delete all strings in array.
  virtual void DeleteAll () = 0;
};

/** @} */

#endif // __CS_IUTIL_STRINGARRAY_H__

