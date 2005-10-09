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
#include "csutil/scf_interface.h"


/// This is a SCF-compatible interface for csString.
struct iString : public virtual iBase
{
  SCF_INTERFACE(iString, 2,0,0);
  /**
   * Advise the string that it should allocate enough space to hold up to
   * NewSize characters.
   * \remarks After calling this method, the string's capacity will be at least
   *   NewSize + 1 (one for the implicit null terminator).  Never shrinks
   *   capacity.  If you need to actually reclaim memory, then use Free() or
   *   Reclaim().
   */
  virtual void SetCapacity (size_t NewSize) = 0;
  /// Return the current capacity.
  virtual size_t GetCapacity () const = 0;

  /**
   * Advise the string that it should grow its allocated buffer by
   * approximately this many bytes when more space is required. This is an
   * optimization to avoid excessive memory reallocation and heap management,
   * which can be quite slow.
   * \remarks This value is only a suggestion.  The actual value by which it
   *   grows may be rounded up or down to an implementation-dependent
   *   allocation multiple.
   * <p>
   * \remarks If the value is zero, then the internal buffer grows
   *   exponentially by doubling its size, rather than by fixed increments.
   */
  virtual void SetGrowsBy(size_t) = 0;
  /**
   * Return the number of bytes by which the string grows.
   * \remarks If the return value is zero, then the internal buffer grows
   *   exponentially by doubling its size, rather than by fixed increments.
   */
  virtual size_t GetGrowsBy() const = 0;

  /**
   * Truncate the string.
   * \param Len The number of characters to which the string should be
   *   truncated (possibly 0).
   * \remarks Will only make a string shorter; will never extend it.  This
   *   method may or may not reclaim memory depending upon the implementation.
   *   If you positively need to reclaim memory after truncating the string,
   *   then invoke Reclaim().
   */
  virtual void Truncate (size_t Len) = 0;

  /**
   * Set string buffer capacity to hold exactly the current content.
   * \remarks If the string length is greater than zero, then the buffer's
   *   capacity will be adjusted to exactly that size.  If the string length is
   *   zero, then the implementation may shrink the allocation so that it only
   *   holds the implicit null terminator, or it may free the string's memory
   *   completely.
   */
  virtual void ShrinkBestFit () = 0;

  /**
   * Set string buffer capacity to hold exactly the current content.
   * \remarks If the string length is greater than zero, then the buffer's
   *   capacity will be adjusted to exactly that size.  If the string length is
   *   zero, then the implementation may shrink the allocation so that it only
   *   holds the implicit null terminator, or it may free the string's memory
   *   completely.
   * \deprecated Use ShrinkBestFit() instead.
   */
  CS_DEPRECATED_METHOD virtual void Reclaim () = 0;

  /**
   * Clear the string (so that it contains only a null terminator).
   * \remarks This is typically shorthand for Truncate(0), but more idiomatic
   *   in terms of human language.
   */
  virtual void Empty () = 0;

  /**
   * Clear the string (so that it contains only a null terminator).
   * \remarks This is typically shorthand for Truncate(0), but more idiomatic
   *   in terms of human language.
   * \deprecated Use Empty() instead.
   */
  /* CS_DEPRECATED_METHOD */ virtual void Clear () = 0;

  /// Get a copy of this string
  virtual csRef<iString> Clone () const = 0;

  /**
   * Get a pointer to the null-terminated character array.
   * \return A C-string pointer to the null-terminated character array; or zero
   *   if the string represents a null-pointer.
   */
  virtual char const* GetData () const = 0;

  /**
   * Get a pointer to the null-terminated character array.
   * \return A C-string pointer to the null-terminated character array; or zero
   *   if the string represents a null-pointer.
   * \warning This returns a non-const pointer, so use this function with care!
   * \deprecated Use the 'const' version of GetData() instead.
   */
  /*CS_DEPRECATED_METHOD*/ 
  // @@@ GCC and VC always seem to prefer this GetData() and barf "deprecated".
  virtual char* GetData () = 0;

  /**
   * Query string length.
   * \return The string length.
   * \remarks The returned length does not count the implicit null terminator.
   */
  virtual size_t Length () const = 0;

  /**
   * Check if string is empty.
   * \return True if the string is empty; false if it is not.
   * \remarks This is equivalent to the expression 'Length() == 0'.
   */
  virtual bool IsEmpty () const = 0;

  /// Get a modifiable reference to n'th character.
  virtual char& operator [] (size_t n) = 0;

  /// Get n'th character.
  virtual char operator [] (size_t n) const = 0;

  /**
   * Set the n'th character.
   * \remarks The n'th character position must be a valid position in the
   *   string.  You can not expand the string by setting a character beyond the
   *   end of string.
   */
  virtual void SetAt (size_t n, char iChar) = 0;

  /// Get the n'th character.
  virtual char GetAt (size_t n) const = 0;

  /**
   * Insert another string into this one.
   * \param Pos Position at which to insert the other string (zero-based).
   * \param Str String to insert.
   */
  virtual void Insert (size_t Pos, iString const* Str) = 0;

  /**
   * Overlay another string onto a part of this string.
   * \param Pos Position at which to insert the other string (zero-based).
   * \param Str String which will be overlayed atop this string.
   * \remarks The target string will grow as necessary to accept the new
   *   string.
   */
  virtual void Overwrite (size_t Pos, iString const* Str) = 0;

  /**
   * Append a null-terminated C-string to this one.
   * \param Str String which will be appended.
   * \param Count Number of characters from Str to append; if -1 (the default),
   *   then all characters from Str will be appended.
   */
  virtual void Append (const char* Str, size_t Count = (size_t)-1) = 0;

  /**
   * Append a string to this one. 
   * \param Str String which will be appended.
   * \param Count Number of characters from Str to append; if -1 (the default),
   *   then all characters from Str will be appended.
   */
  virtual void Append (const iString* Str, size_t Count = (size_t)-1) = 0;

  /**
   * Copy and return a portion of this string.
   * \param start Start position of slice (zero-based).
   * \param len Number of characters in slice.
   * \return The indicated string slice.
   */
  virtual csRef<iString> Slice (size_t start, size_t len) const = 0;

  /**
   * Copy a portion of this string.
   * \param sub String which will receive the indicated substring copy.
   * \param start Start position of slice (zero-based).
   * \param len Number of characters in slice.
   * \remarks Use this method instead of Slice() for cases where you expect to
   *   extract many substrings in a tight loop, and want to avoid the overhead
   *   of allocation of a new string object for each operation.  Simply re-use
   *   'sub' for each operation.
   */
  virtual void SubString (iString* sub, size_t start, size_t len) const = 0;

  /**
   * Find the first occurrence of a character in the string.
   * \param c Character to locate.
   * \param p Start position of search (default 0).
   * \return First position of character, or (size_t)-1 if not found.
   */
  virtual size_t FindFirst (const char c, size_t p = (size_t)-1) const = 0;

  /**
   * Find the last occurrence of a character in the string.
   * \param c Character to locate.
   * \param p Start position of reverse search.  Specify (size_t)-1 if you want
   *   the search to begin at the very end of string.
   * \return Last position of character, or (size_t)-1 if not found.
   */
  virtual size_t FindLast (const char c, size_t p = (size_t)-1) const = 0;
  
  /**
   * Find the first occurrence of \p search in this string starting at \p pos.
   * \param search String to locate.
   * \param pos Start position of search (default 0).
   * \return First position of \p search, or (size_t)-1 if not found.
   */
  virtual size_t Find (const char* search, size_t pos = 0) const = 0;

  /**
   * Find all occurrences of \p search in this string and replace them with
   * \p replacement.
   */
  virtual void ReplaceAll (const char* search, const char* replacement) = 0;

  /**
   * Format this string using sprintf()-style formatting directives.
   * \remarks Automatically allocates sufficient memory to hold result.  Newly
   *   formatted string replaces previous string value.
   * \sa \ref FormatterNotes
   */
  virtual void Format (const char* format, ...) CS_GNUC_PRINTF (2, 3) = 0;

  /**
   * Format this string using sprintf() formatting directives in a va_list.
   * \return Reference to itself.
   * \remarks Automatically allocates sufficient memory to hold result.  Newly
   *   formatted string replaces previous string value.
   * \sa \ref FormatterNotes
   */
  virtual void FormatV (const char* format, va_list args) = 0;

  /**
   * Replace contents of this string with the contents of another.
   * \param str String from which new content of this string will be copied.
   * \param count Number of characters to copy.  If (size_t)-1 is specified,
   *   then all characters will be copied.
   */
  virtual void Replace (const iString* str, size_t count = (size_t)-1) = 0;

  /**
   * Replace contents of this string with the contents of another.
   * \param str String from which new content of this string will be copied.
   * \param count Number of characters to copy.  If (size_t)-1 is specified,
   *   then all characters will be copied.
   */
  virtual void Replace (const char* str, size_t count = (size_t)-1) = 0;

  /**
   * Check if another string is equal to this one.
   * \param Str Other string.
   * \return True if they are equal; false if not.
   * \remarks The comparison is case-sensitive.
   */
  virtual bool Compare (const iString* Str) const = 0;

  /**
   * Check if another string is equal to this one.
   * \param Str Other string.
   * \return True if they are equal; false if not.
   * \remarks The comparison is case-insensitive.
   */
  virtual bool CompareNoCase (const iString* Str) const = 0;

  /// Append another string to this one.
  virtual void operator += (const iString& iStr) = 0;

  /// Append a null-terminated C-string to this string
  virtual void operator += (const char* iStr) = 0;

  /// Concatenate two strings and return a third one.
  virtual csRef<iString> operator + (const iString& iStr) const = 0;

  /**
   * Get a pointer to the null-terminated character array.
   * \return A C-string pointer to the null-terminated character array; or zero
   *   if the string represents a null-pointer.
   */
  virtual operator char const* () const = 0;

  /**
   * Check if another string is equal to this one.
   * \param Str Other string.
   * \return True if they are equal; false if not.
   * \remarks The comparison is case-sensitive.
   */
  virtual bool operator == (const iString& Str) const = 0;

  /**
   * Check if another string is not equal to this one.
   * \param Str Other string.
   * \return False if they are equal; true if not.
   * \remarks The comparison is case-sensitive.
   */
  virtual bool operator != (const iString& Str) const = 0;

  /// Convert string to lowercase.
  virtual void Downcase() = 0;

  /// Convert string to uppercase.
  virtual void Upcase() = 0;
};

/** @} */

#endif // __CS_IUTIL_STRING_H__
