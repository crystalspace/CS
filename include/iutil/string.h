/*
    Crystal Space String interface
    Copyright (C) 1999 by Brandon Ehle (Azverkan)

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

#ifndef __CS_IUTIL_STRING_H__
#define __CS_IUTIL_STRING_H__

/**\file
 * String interface
 */
/**\addtogroup util
 * @{ */
#include <stdarg.h>
#include "csutil/scf.h"
#include "csutil/ref.h"

SCF_VERSION (iString, 0, 2, 0);

/// This is a SCF-compatible interface for csString.
struct iString : public iBase
{
  /**
   * Advise the string that it should allocate enough space to hold up to
   * NewSize characters.  After calling this method, the string's capacity will
   * be at least NewSize + 1 (one for the implicit null terminator).  Never
   * shrinks capacity.  If you need to actually reclaim memory, then use Free()
   * or Reclaim().
   */
  virtual void SetCapacity (size_t NewSize) = 0;
  /// Get string capacity.
  virtual size_t GetCapacity () const = 0;

  /**
   * Advise the string that it should grow by approximately this many bytes
   * when more space is required.  This value is only a suggestion.  The actual
   * value by which it grows may be rounded up or down to an
   * implementation-dependent allocation multiple.
   */
  virtual void SetGrowsBy(size_t) = 0;
  /// Get the allocation growth increment.
  virtual size_t GetGrowsBy() const = 0;

  /**
   * Tell the string to re-size its buffer exponentially as needed.  If set to
   * true, the GetGrowsBy() setting is ignored.
   */
  virtual void SetGrowsExponentially(bool) = 0;

  /// Returns true if exponential growth is enabled.
  virtual bool GetGrowsExponentially() const = 0;

  /// Truncate the string
  virtual void Truncate (size_t iPos) = 0;

  /// Set string maximal capacity to current string length
  virtual void Reclaim () = 0;

  /// Clear the string (so that it contains only ending 0 character)
  virtual void Clear () = 0;

  /// Get a copy of this string
  virtual csRef<iString> Clone () const = 0;

  /// Get a pointer to null-termianted character array.
  virtual char const* GetData () const = 0;

  /// Get a pointer to null-termianted character array.
  virtual char* GetData () = 0;

  /// Query string length
  virtual size_t Length () const = 0;

  /// Check if string is empty
  virtual bool IsEmpty () const = 0;

  /// Get a reference to iPos'th character
  virtual char& operator [] (size_t iPos) = 0;

  /// Get the iPos'th character
  virtual char operator [] (size_t iPos) const = 0;

  /// Set character number iPos to iChar
  virtual void SetAt (size_t iPos, char iChar) = 0;

  /// Get character at position iPos
  virtual char GetAt (size_t iPos) const = 0;

  /// Insert another string into this one at position iPos
  virtual void Insert (size_t iPos, iString const* iStr) = 0;

  /// Overlay another string onto a part of this string
  virtual void Overwrite (size_t iPos, iString const* iStr) = 0;

  /// Append a null-terminated string to this one (up to Count characters)
  virtual void Append (const char* iStr, size_t Count = (size_t)-1) = 0;

  /// Append a string to this one (possibly Count characters from the string)
  virtual void Append (const iString* iStr, size_t Count = (size_t)-1) = 0;

  /**
   * Copy and return a portion of this string.  The substring runs from `start'
   * for `len' characters.
   */
  virtual csRef<iString> Slice (size_t start, size_t len) const = 0;

  /**
   * Copy a portion of this string.  The result is placed in 'sub'.  The
   * substring is from 'start', of length 'len'.
   */
  virtual void SubString (iString* sub, size_t start, size_t len) const = 0;

  /**
   * Find first character 'c' from position 'p'.  If the character cannot be
   * found, this function returns (size_t)-1
   */
  virtual size_t FindFirst (const char c, size_t p = (size_t)-1) const = 0;

  /**
   * Find last character 'c', counting backwards from position 'p'.  Default
   * position is the end of the string.  If the character cannot be found, this
   * function returns (size_t)-1
   */
  virtual size_t FindLast (const char c, size_t p = (size_t)-1) const = 0;
  
  /**
   * Format this string using sprintf() formatting directives.  Automatically
   * allocates sufficient memory to hold result.  Newly formatted string
   * overwrites previous string value.
   */
  virtual void Format (const char* format, ...) CS_GNUC_PRINTF (2, 3) = 0;

  /**
   * Format this string using sprintf() formatting directives in a va_list.
   * Automatically allocates sufficient memory to hold result.  Newly
   * formatted string overwrites previous string value.
   */
  virtual void FormatV (const char* format, va_list args) = 0;

  /// Replace contents of this string with the contents of another
  virtual void Replace (const iString* iStr, size_t iCount = (size_t)-1) = 0;

  /// Check if two strings are equal
  virtual bool Compare (const iString* iStr) const = 0;

  /// Compare two strings ignoring case
  virtual bool CompareNoCase (const iString* iStr) const = 0;

  /// Append another string to this
  virtual void operator += (const iString& iStr) = 0;

  /// Append an ASCIIZ to this string
  virtual void operator += (const char* iStr) = 0;

  /// Concatenate two strings and return a third one
  virtual csRef<iString> operator + (const iString& iStr) const = 0;

  /// Get the null-terminated C string represented by this iString.
  virtual operator char const* () const = 0;

  /// Check if two strings are equal
  virtual bool operator == (const iString &iStr) const = 0;

  /// Convert string to lowercase.
  virtual void Downcase() = 0;

  /// Convert string to uppercase.
  virtual void Upcase() = 0;
};

/** @} */

#endif // __CS_IUTIL_STRING_H__
