/*
    Crystal Space utility library: string class
    Copyright (C) 1999,2000 by Andrew Zabolotny <bit@eltech.ru>

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
#ifndef __CS_CSSTRING_H__
#define __CS_CSSTRING_H__

/**\file
 * String utility class
 */

#include <stdarg.h>
#include <ctype.h>
#include "csextern.h"
#include "snprintf.h"
#include "util.h"

/**
 * This is a string class with a range of useful operators and type-safe
 * overloads.  Strings may contain arbitary binary data, including null bytes.
 * It also guarantees that a null-terminator always follows the last stored
 * character, thus you can safely use the return value from GetData() and
 * `operator char const*()' in calls to functions expecting C strings.  The
 * implicit null terminator is not included in the character count returned by
 * Length().
 *
 * Like a typical C character string pointer, csStringBase can also represent a
 * null pointer.  This allows a non-string to be distinguished from an empty
 * (zero-length) string.  The csStringBase will represent a null-pointer in the
 * following cases:
 * <ul>
 * <li>When constructed with no arguments (the default constructor).
 * <li>When constructed with an explicit null-pointer.
 * <li>When assigned a null-pointer via operator=((char const*)0).
 * <li>After an invocation of Replace((char const*)0).
 * <li>After invocation of csStringBase::Free() or any method which is documented
 *     as invoking Free() as a side-effect, such as Reclaim().
 * <li>After invocation of csStringBase::Detach().
 * </ul>
 */
class CS_CSUTIL_EXPORT csStringBase
{
protected:
  // Default number of bytes by which allocation should grow.
  // *** IMPORTANT *** This must be a power of two (i.e. 8, 16, 32, 64, etc.).
  enum { DEFAULT_GROW_BY = 64 };

  // String buffer.
  char* Data;
  // Length of string; not including null terminator.
  size_t Size;
  // Size in bytes of allocated string buffer.
  size_t MaxSize;
  // Size in bytes by which allocated buffer is increased when needed.
  size_t GrowBy;
  // Controls if allocated buffer grows exponentially (overrides GrowBy).
  bool GrowExponentially;

  // If necessary, increase the buffer capacity enough to hold NewSize bytes.
  // Buffer capacity is increased in GrowBy increments or exponentially
  // depending upon configuration.
  void ExpandIfNeeded (size_t NewSize);

  /**
   * Set the buffer to hold NewSize bytes. If \a extraSpace is true it means
   * the buffer can be larger to reduce the number of allocations needed.
   */
  virtual void SetCapacityInternal (size_t NewSize, bool extraSpace);
  /**
   * Compute a new buffer size. Takes GrowBy and GrowExponentially into
   * consideration.
   */
  size_t ComputeNewSize (size_t NewSize);
public:
  /**
   * Advise the string that it should allocate enough space to hold up to
   * NewSize characters.
   * \remarks After calling this method, the string's capacity will be at least
   *   NewSize + 1 (one for the implicit null terminator).  Never shrinks
   *   capacity.  If you need to actually reclaim memory, then use Free() or
   *   Reclaim().
   */
  void SetCapacity (size_t NewSize);

  /// Return the current capacity.
  size_t GetCapacity() const
  { return MaxSize; }

  /**
   * Advise the string that it should grow by approximately this many bytes
   * when more space is required.
   * \remarks This value is only a suggestion.  The actual value by which it
   *   grows may be rounded up or down to an implementation-dependent
   *   allocation multiple.  This method turns off exponential growth.
   */
  void SetGrowsBy(size_t);

  /// Return the number of bytes by which the string grows.
  size_t GetGrowsBy() const
  { return GrowBy; }

  /**
   * Tell the string to re-size its buffer exponentially as needed.
   * \remarks If set to true, the GetGrowsBy() setting is ignored.
   */
  void SetGrowsExponentially(bool b)
  { GrowExponentially = b; }

  /// Returns true if exponential growth is enabled.
  bool GetGrowsExponentially() const
  { return GrowExponentially; }

  /**
   * Free the memory allocated for the string.
   * \remarks Following a call to this method, invocations of GetData() and
   *   'operator char const*' will return a null pointer (until some new
   *   content is added to the string).
   */
  virtual void Free ();

  /**
   * Truncate the string.
   * \param Len The number of characters to which the string should be
   *   truncated (possibly 0).
   * \return Reference to itself.
   * \remarks Will only make a string shorter; will never extend it.
   *   This method does not reclaim memory; it merely shortens the string,
   *   which means that Truncate(0) is a handy method of clearing the string,
   *   without the overhead of slow heap management.  This may be important if
   *   you want to re-use the same string many times over.  If you need to
   *   reclaim memory after truncating the string, then invoke Reclaim().
   *   GetData() and 'operator char const*' will return a non-null zero-length
   *   string if you truncate the string to 0 characters, unless the string
   *   had already represented a null-pointer, in which case it will continue
   *   to represent a null-pointer after truncation.
   */
  csStringBase& Truncate (size_t Len);

  /**
   * Set string buffer capacity to hold exactly the current content.
   * \return Reference to itself.
   * \remarks If the string length is greater than zero, then the buffer's
   *   capacity will be adjusted to exactly that size.  If the string length is
   *   zero, then this is equivalent to an invocation of Free(), which means
   *   that GetData() and 'operator char const*' will return a null pointer
   *   after reclamation.
   */
  csStringBase& Reclaim ();

  /**
   * Clear the string (so that it contains only a null terminator).
   * \return Reference to itself.
   * \remarks This is rigidly equivalent to Truncate(0), but more idiomatic in
   *   terms of human language.
   */
  csStringBase& Clear ()
  { return Truncate (0); }

  /**
   * Get a pointer to the null-terminated character array.
   * \return A C-string pointer to the null-terminated character array; or zero
   *   if the string represents a null-pointer.
   * \remarks See the class description for a discussion about how and when the
   *   string will represent a null-pointer.
   * \warning This returns a non-const pointer, so use this function with care!
   * \deprecated Use the 'const' version of GetData() instead.
   */
  /*CS_DEPRECATED_METHOD*/ 
    // @@@ GCC and VC always seem to prefer this GetData() and barf "deprecated".
  char* GetData ()
  { return Data; }

  /**
   * Get a pointer to the null-terminated character array.
   * \return A C-string pointer to the null-terminated character array; or zero
   *   if the string represents a null-pointer.
   * \remarks See the class description for a discussion about how and when the
   *   string will represent a null-pointer.
   */
  char const* GetData () const
  { return Data; }

  /**
   * Get a pointer to the null-terminated character array.
   * \return A C-string pointer to the null-terminated character array.
   * \remarks Unlike GetData(), this will always return a valid, non-null
   *   C-string, even if the underlying representation is that of a
   *   null-pointer (in which case, it will return a zero-length C-string.
   *   This is a handy convenience which makes it possible to use the result
   *   directly without having to perform a null check first.
   */
   char const* GetDataSafe() const
   { return Data != 0 ? Data : ""; }

  /**
   * Query string length.
   * \return The string length.
   * \remarks The returned length does not count the implicit null terminator.
   */
  size_t Length () const
  { return Size; }

  /**
   * Check if string is empty.
   * \return True if the string is empty; false if it is not.
   * \remarks This is rigidly equivalent to the expression 'Length() == 0'.
   */
  bool IsEmpty () const
  { return (Size == 0); }

  /// Get a modifiable reference to n'th character.
  char& operator [] (size_t n)
  {
    CS_ASSERT (n < Size);
    return Data [n];
  }

  /// Get n'th character.
  char operator [] (size_t n) const
  {
    CS_ASSERT (n < Size);
    return Data[n];
  }

  /**
   * Set the n'th character.
   * \remarks The n'th character position must be a valid position in the
   *   string.  You can not expand the string by setting a character beyond the
   *   end of string.
   */
  void SetAt (size_t n, const char c)
  {
    CS_ASSERT (n < Size);
    Data [n] = c;
  }

  /// Get the n'th character.
  char GetAt (size_t n) const
  {
    CS_ASSERT (n < Size);
    return Data [n];
  }

  /**
   * Delete a range of characters from the string.
   * \param Pos Beginning of range to be deleted (zero-based).
   * \param Count Number of characters to delete.
   * \return Reference to itself.
   */
  csStringBase& DeleteAt (size_t Pos, size_t Count = 1);

  /**
   * Insert another string into this one.
   * \param Pos Position at which to insert the other string (zero-based).
   * \param Str String to insert.
   * \return Reference to itself.
   */
  csStringBase& Insert (size_t Pos, const csStringBase& Str);

  /**
   * Insert another string into this one.
   * \param Pos Position at which to insert the other string (zero-based).
   * \param Str String to insert.
   * \return Reference to itself.
   */
  csStringBase& Insert (size_t Pos, const char* Str);

  /**
   * Insert another string into this one.
   * \param Pos Position at which to insert the other string (zero-based).
   * \param C Character to insert.
   * \return Reference to itself.
   */
  csStringBase& Insert (size_t Pos, char C);

  /**
   * Overlay another string onto a part of this string.
   * \param Pos Position at which to insert the other string (zero-based).
   * \param Str String which will be overlayed atop this string.
   * \return Reference to itself.
   * \remarks The target string will grow as necessary to accept the new
   *   string.
   */
  csStringBase& Overwrite (size_t Pos, const csStringBase& Str);

  /**
   * Append a null-terminated C-string to this one.
   * \param Str String which will be appended.
   * \param Count Number of characters from Str to append; if -1 (the default),
   *   then all characters from Str will be appended.
   * \return Reference to itself.
   */
  csStringBase& Append (const char* Str, size_t Count = (size_t)-1);

  /**
   * Append a string to this one. 
   * \param Str String which will be appended.
   * \param Count Number of characters from Str to append; if -1 (the default),
   *   then all characters from Str will be appended.
   * \return Reference to itself.
   */
  csStringBase& Append (const csStringBase& Str, size_t Count = (size_t)-1);

  /**
   * Append a signed character to this string.
   * \return Reference to itself.
   */
  csStringBase& Append (char c)
  { char s[2]; s[0] = c; s[1] = '\0'; return Append(s); }

  /**
   * Append an unsigned character to this string.
   * \return Reference to itself.
   */
  csStringBase& Append (unsigned char c)
  { return Append(char(c)); }

  /**
   * Copy and return a portion of this string.
   * \param start Start position of slice (zero-based).
   * \param len Number of characters in slice.
   * \return The indicated string slice.
   */
  csStringBase Slice (size_t start, size_t len) const;

  /**
   * Copy a portion of this string.
   * \param sub Strign which will receive the indicated substring copy.
   * \param start Start position of slice (zero-based).
   * \param len Number of characters in slice.
   * \remarks Use this method instead of Slice() for cases where you expect to
   *   extract many substrings in a tight loop, and want to avoid the overhead
   *   of allocation of a new string object for each operation.  Simply re-use
   *   'sub' for each operation.
   */
  void SubString (csStringBase& sub, size_t start, size_t len) const;

  /**
   * Find the first occurrence of a character in the string.
   * \param c Character to locate.
   * \param pos Start position of search (default 0).
   * \return First position of character, or (size_t)-1 if not found.
   */
  size_t FindFirst (char c, size_t pos = 0) const;

  /**
   * Find the first occurrence of any of a set of characters in the string.
   * \param c Characters to locate.
   * \param pos Start position of search (default 0).
   * \return First position of character, or (size_t)-1 if not found.
   */
  size_t FindFirst (const char *c, size_t pos = 0) const;

  /**
   * Find the last occurrence of a character in the string.
   * \param c Character to locate.
   * \param pos Start position of reverse search.  Specify (size_t)-1 if you
   *   want the search to begin at the very end of string.
   * \return Last position of character, or (size_t)-1 if not found.
   */
  size_t FindLast (char c, size_t pos = (size_t)-1) const;
  
  /**
   * Find the occurrence of a substring in the string.
   * \param str String to locate.
   * \param pos Start position of search (default 0).
   * \return First position of string, or (size_t)-1 if not found.
   */
  size_t FindStr (const char* str, size_t pos = 0) const;

  /**
   * Find the occurrence of a substring in the string and replace it with
   * another string.
   */
  void FindReplace (const char* str, const char* replaceWith);

#define STR_APPEND(TYPE,FMT,SZ) csStringBase& Append(TYPE n) \
  { char s[SZ]; cs_snprintf(s, SZ, FMT, n); return Append(s); }
  /** @{ */
  /**
   * Append the value, in formatted form, to this string.
   */
  STR_APPEND(short, "%hd", 32)
  STR_APPEND(unsigned short, "%hu", 32)
  STR_APPEND(int, "%d", 32)
  STR_APPEND(unsigned int, "%u", 32)
  STR_APPEND(long, "%ld", 32)
  STR_APPEND(unsigned long, "%lu", 32)
  STR_APPEND(float, "%g", 64)
  STR_APPEND(double, "%g", 64)
#undef STR_APPEND
  /** @} */

  /// Append a boolean (as a number -- 1 or 0) to this string.
  csStringBase& Append (bool b) { return Append (b ? "1" : "0"); }

  /**
   * Replace contents of this string with the contents of another.
   * \param Str String from which new content of this string will be copied.
   * \param Count Number of characters to copy.  If (size_t)-1 is specified,
   *   then all characters will be copied.
   * \return Reference to itself.
   * \remarks This string will represent a null-pointer after replacement if
   *   and only if Str represents a null-pointer.
   */
  csStringBase& Replace (const csStringBase& Str, size_t Count = (size_t)-1);

  /**
   * Replace contents of this string with the contents of another.
   * \param Str String from which new content of this string will be copied.
   * \param Count Number of characters to copy.  If (size_t)-1 is specified,
   *   then all characters will be copied.
   * \return Reference to itself.
   * \remarks This string will represent a null-pointer after replacement if
   *   and only if Str is a null pointer.
   */
  csStringBase& Replace (const char* Str, size_t Count = (size_t)-1);

#define STR_REPLACE(TYPE) \
csStringBase& Replace (TYPE s) { Size = 0; return Append(s); }
  /** @{ */
  /**
   * Replace contents of this string with the value in formatted form.
   * \remarks Internally uses the various flavours of Append().
   */
  STR_REPLACE(char)
  STR_REPLACE(unsigned char)
  STR_REPLACE(short)
  STR_REPLACE(unsigned short)
  STR_REPLACE(int)
  STR_REPLACE(unsigned int)
  STR_REPLACE(long)
  STR_REPLACE(unsigned long)
  STR_REPLACE(float)
  STR_REPLACE(double)
  STR_REPLACE(bool)
#undef STR_REPLACE
  /** @} */
  
  /**
   * Check if another string is equal to this one.
   * \param iStr Other string.
   * \return True if they are equal; false if not.
   * \remarks The comparison is case-sensitive.
   */
  bool Compare (const csStringBase& iStr) const
  {
    if (&iStr == this)
      return true;
    size_t const n = iStr.Length();
    if (Size != n)
      return false;
    if (Size == 0 && n == 0)
      return true;
    return (memcmp (Data, iStr.GetData (), Size) == 0);
  }

  /**
   * Check if a null-terminated C- string is equal to this string.
   * \param iStr Other string.
   * \return True if they are equal; false if not.
   * \remarks The comparison is case-sensitive.
   */
  bool Compare (const char* iStr) const
  { return (strcmp (Data ? Data : "", iStr) == 0); }

  /**
   * Check if another string is equal to this one.
   * \param iStr Other string.
   * \return True if they are equal; false if not.
   * \remarks The comparison is case-insensitive.
   */
  bool CompareNoCase (const csStringBase& iStr) const
  {
    if (&iStr == this)
      return true;
    size_t const n = iStr.Length();
    if (Size != n)
      return false;
    if (Size == 0 && n == 0)
      return true;
    return (csStrNCaseCmp (Data, iStr.GetData (), Size) == 0);
  }

  /**
   * Check if a null-terminated C- string is equal to this string.
   * \param iStr Other string.
   * \return True if they are equal; false if not.
   * \remarks The comparison is case-insensitive.
   */
  bool CompareNoCase (const char* iStr) const
  { return (csStrNCaseCmp (Data ? Data : "", iStr, Size) == 0); }

  /**
   * Check if this string starts with another one.
   * \param iStr Other string.
   * \param ignore_case Causes the comparison to be case insensitive if true.
   * \return True if they are equal up to the length of iStr; false if not.
   */
  bool StartsWith (const csStringBase& iStr, bool ignore_case = false) const
  {
    if (&iStr == this)
      return true;
    size_t const n = iStr.Length();
    if (n == 0)
      return true;
    if (n > Size)
      return false;
    CS_ASSERT(Data != 0);
    if (ignore_case)
      return (csStrNCaseCmp (Data, iStr.GetData (), n) == 0);
    else
      return (strncmp (Data, iStr.GetData (), n) == 0);
  }

  /**
   * Check if this string starts with a null-terminated C- string.
   * \param iStr Other string.
   * \param ignore_case Causes the comparison to be case insensitive if true.
   * \return True if they are equal up to the length of iStr; false if not.
   */
  bool StartsWith (const char* iStr, bool ignore_case = false) const
  {
    if (iStr == 0)
      return false;
    size_t const n = strlen (iStr);
    if (n == 0)
      return true;
    if (n > Size)
      return false;
    CS_ASSERT(Data != 0);
    if (ignore_case)
      return (csStrNCaseCmp (Data, iStr, n) == 0);
    else
      return (strncmp (Data, iStr, n) == 0);
  }

  /**
   * Create an empty csStringBase object.
   * \remarks The newly constructed string represents a null-pointer.
   */
  csStringBase () : Data (0), Size (0), MaxSize (0), GrowBy (DEFAULT_GROW_BY),
    GrowExponentially (false) {}

  /**
   * Create a csStringBase object and reserve space for at least Length characters.
   * \remarks The newly constructed string represents a non-null zero-length
   *   string.
   */
  csStringBase (size_t Length) : Data (0), Size (0), MaxSize (0),
    GrowBy (DEFAULT_GROW_BY), GrowExponentially(false)
  { SetCapacity (Length); }

  /**
   * Copy constructor.
   * \remarks The newly constructed string will represent a null-pointer if and
   *   only if the template string represented a null-pointer.
   */
  csStringBase (const csStringBase& copy) : Data (0), Size (0), MaxSize (0),
    GrowBy (DEFAULT_GROW_BY), GrowExponentially(false)
  { Append (copy); }

  /**
   * Create a csStringBase object from a null-terminated C string.
   * \remarks The newly constructed string will represent a null-pointer if and
   *   only if the input argument is a null-pointer.
   */
  csStringBase (const char* src) : Data (0), Size (0), MaxSize (0),
    GrowBy (DEFAULT_GROW_BY), GrowExponentially(false)
  { Append (src); }

  /// Create a csStringBase object from a single signed character.
  csStringBase (char c) : Data (0), Size (0), MaxSize (0),
    GrowBy (DEFAULT_GROW_BY), GrowExponentially(false)
  { Append (c); }

  /// Create a csStringBase object from a single unsigned character.
  csStringBase (unsigned char c) : Data(0), Size (0), MaxSize (0),
    GrowBy (DEFAULT_GROW_BY), GrowExponentially(false)
  { Append ((char) c); }

  /// Destroy the csStringBase.
  virtual ~csStringBase ();

  /**
   * Get a copy of this string.
   * \remarks The newly constructed string will represent a null-pointer if and
   *   only if this string represents a null-pointer.
   */
  csStringBase Clone () const
  { return csStringBase (*this); }

  /**
   * Trim leading whitespace.
   * \return Reference to itself.
   * \remarks This is equivalent to Truncate(n) where 'n' is the last
   *   non-whitespace character, or zero if the string is composed entirely of
   *   whitespace.
   */
  csStringBase& LTrim();

  /**
   * Trim trailing whitespace.
   * \return Reference to itself.
   * \remarks This is equivalent to DeleteAt(0,n) where 'n' is the first
   *   non-whitespace character, or Lenght() if the string is composed entirely
   *   of whitespace.
   */
  csStringBase& RTrim();

  /**
   * Trim leading and trailing whitespace.
   * \return Reference to itself.
   * \remarks This is equivalent to LTrim() followed by RTrim().
   */
  csStringBase& Trim();

  /**
   * Trim leading and trailing whitespace, and collapse all internal
   * whitespace to a single space.
   * \return Reference to itself.
   */
  csStringBase& Collapse();

  /**
   * Format this string using sprintf()-style formatting directives.
   * \return Reference to itself.
   * \remarks Automatically allocates sufficient memory to hold result.  Newly
   *   formatted string replaces previous string value.
   */
  csStringBase& Format(const char* format, ...) CS_GNUC_PRINTF (2, 3);

  /**
   * Format this string using sprintf() formatting directives in a va_list.
   * \return Reference to itself.
   * \remarks Automatically allocates sufficient memory to hold result.  Newly
   *   formatted string replaces previous string value.
   */
  csStringBase& FormatV(const char* format, va_list args);

#define STR_FORMAT(TYPE) \
  static csStringBase Format (TYPE v);
  /** @{ */
  /**
   * Format this value using a sprintf() formatting directive.
   */
  STR_FORMAT(short)
  STR_FORMAT(unsigned short)
  STR_FORMAT(int)
  STR_FORMAT(unsigned int)
  STR_FORMAT(long)
  STR_FORMAT(unsigned long)
  STR_FORMAT(float)
  STR_FORMAT(double)
#undef STR_FORMAT

#define STR_FORMAT_INT(TYPE) \
  static csStringBase Format (TYPE v, int width, int prec=0);
  STR_FORMAT_INT(short)
  STR_FORMAT_INT(unsigned short)
  STR_FORMAT_INT(int)
  STR_FORMAT_INT(unsigned int)
  STR_FORMAT_INT(long)
  STR_FORMAT_INT(unsigned long)
#undef STR_FORMAT_INT

#define STR_FORMAT_FLOAT(TYPE) \
  static csStringBase Format (TYPE v, int width, int prec=6);
  STR_FORMAT_FLOAT(float)
  STR_FORMAT_FLOAT(double)
#undef STR_FORMAT_FLOAT
  /** @} */

  /**
   * Pad to a specified size with leading characters.
   * \param NewSize Size to which the string should grow.
   * \param PadChar Character with which to pad the string (default is space).
   * \return Reference to itself.
   * \remarks Never shortens the string.  If NewSize is less than or equal to
   *   Length(), nothing happens.
   */
  csStringBase& PadLeft (size_t NewSize, char PadChar = ' ');

  /// Return a copy of this string formatted with PadLeft().
  csStringBase AsPadLeft (size_t NewSize, char PadChar = ' ') const;

#define STR_PADLEFT(TYPE) \
  static csStringBase PadLeft (TYPE v, size_t iNewSize, char iChar=' ');
  /** @{ */
  /** 
   * Return a new left-padded string representation of a basic type.
   */
  STR_PADLEFT(const csStringBase&)
  STR_PADLEFT(const char*)
  STR_PADLEFT(char)
  STR_PADLEFT(unsigned char)
  STR_PADLEFT(short)
  STR_PADLEFT(unsigned short)
  STR_PADLEFT(int)
  STR_PADLEFT(unsigned int)
  STR_PADLEFT(long)
  STR_PADLEFT(unsigned long)
  STR_PADLEFT(float)
  STR_PADLEFT(double)
  STR_PADLEFT(bool)
#undef STR_PADLEFT
  /** @} */

  /**
   * Pad to a specified size with trailing characters.
   * \param NewSize Size to which the string should grow.
   * \param PadChar Character with which to pad the string (default is space).
   * \return Reference to itself.
   * \remarks Never shortens the string.  If NewSize is less than or equal to
   *   Length(), nothing happens.
   */
  csStringBase& PadRight (size_t NewSize, char PadChar = ' ');

  /// Return a copy of this string formatted with PadRight().
  csStringBase AsPadRight (size_t NewSize, char PadChar = ' ') const;

#define STR_PADRIGHT(TYPE) \
  static csStringBase PadRight (TYPE v, size_t iNewSize, char iChar=' ');
  /** @{ */
  /**
   * Return a new right-padded string representation of a basic type.
   */
  STR_PADRIGHT(const csStringBase&)
  STR_PADRIGHT(const char*)
  STR_PADRIGHT(char)
  STR_PADRIGHT(unsigned char)
  STR_PADRIGHT(short)
  STR_PADRIGHT(unsigned short)
  STR_PADRIGHT(int)
  STR_PADRIGHT(unsigned int)
  STR_PADRIGHT(long)
  STR_PADRIGHT(unsigned long)
  STR_PADRIGHT(float)
  STR_PADRIGHT(double)
  STR_PADRIGHT(bool)
#undef STR_PADRIGHT
  /** @} */

  /**
   * Pad to a specified size with leading and trailing characters so as to
   * center the string.
   * \param NewSize Size to which the string should grow.
   * \param PadChar Character with which to pad the string (default is space).
   * \return Reference to itself.
   * \remarks Never shortens the string.  If NewSize is less than or equal to
   *   Length(), nothing happens.  If the left and right sides can not be
   *   padded equally, then the right side will gain the extra one-character
   *   padding.
   */
  csStringBase& PadCenter (size_t NewSize, char PadChar = ' ');

  /// Return a copy of this string formatted with PadCenter().
  csStringBase AsPadCenter (size_t NewSize, char PadChar = ' ') const;

#define STR_PADCENTER(TYPE) \
  static csStringBase PadCenter (TYPE v, size_t iNewSize, char iChar=' ');
  /** @{ */
  /**
   * Return a new left+right padded string representation of a basic type.
   */
  STR_PADCENTER(const csStringBase&)
  STR_PADCENTER(const char*)
  STR_PADCENTER(char)
  STR_PADCENTER(unsigned char)
  STR_PADCENTER(short)
  STR_PADCENTER(unsigned short)
  STR_PADCENTER(int)
  STR_PADCENTER(unsigned int)
  STR_PADCENTER(long)
  STR_PADCENTER(unsigned long)
  STR_PADCENTER(float)
  STR_PADCENTER(double)
  STR_PADCENTER(bool)
#undef STR_PADCENTER
  /** @} */

#define STR_ASSIGN(TYPE) \
const csStringBase& operator = (TYPE s) { return Replace (s); }
  /** @{ */
  /**
   * Assign a formatted value to this string.
   */
  STR_ASSIGN(const csStringBase&)
  STR_ASSIGN(const char*)
  STR_ASSIGN(char)
  STR_ASSIGN(unsigned char)
  STR_ASSIGN(short)
  STR_ASSIGN(unsigned short)
  STR_ASSIGN(int)
  STR_ASSIGN(unsigned int)
  STR_ASSIGN(long)
  STR_ASSIGN(unsigned long)
  STR_ASSIGN(float)
  STR_ASSIGN(double)
  STR_ASSIGN(bool)
#undef STR_ASSIGN
  /** @} */

#define STR_OP_APPEND(TYPE) \
  csStringBase &operator += (TYPE s) { return Append (s); }
  /** @{ */
  /**
   * Append a formatted value to this string.
   */
  STR_OP_APPEND(const csStringBase&)
  STR_OP_APPEND(const char*)
  STR_OP_APPEND(char)
  STR_OP_APPEND(unsigned char)
  STR_OP_APPEND(short)
  STR_OP_APPEND(unsigned short)
  STR_OP_APPEND(int)
  STR_OP_APPEND(unsigned int)
  STR_OP_APPEND(long)
  STR_OP_APPEND(unsigned long)
  STR_OP_APPEND(float)
  STR_OP_APPEND(double)
  STR_OP_APPEND(bool)
#undef STR_OP_APPEND
  /** @} */

  /// Add another string to this one and return the result as a new string.
  csStringBase operator + (const csStringBase &iStr) const
  { return Clone ().Append (iStr); }

  /**
   * Get a pointer to the null-terminated character array.
   * \return A C-string pointer to the null-terminated character array; or zero
   *   if the string represents a null-pointer.
   * \remarks See the class description for a discussion about how and when the
   *   string will represent a null-pointer.
   */
  operator const char* () const
  { return Data; }

  /**
   * Check if another string is equal to this one.
   * \param Str Other string.
   * \return True if they are equal; false if not.
   * \remarks The comparison is case-sensitive.
   */
  bool operator == (const csStringBase& Str) const
  { return Compare (Str); }
  /**
   * Check if another string is equal to this one.
   * \param Str Other string.
   * \return True if they are equal; false if not.
   * \remarks The comparison is case-sensitive.
   */
  bool operator == (const char* Str) const
  { return Compare (Str); }
  /**
   * Check if another string is less than this one.
   * \param Str Other string.
   * \return True if the string is 'lesser' than \a Str, false
   *   otherwise.
   */
  bool operator < (const csStringBase& Str) const
  {
    return strcmp (GetDataSafe (), Str.GetDataSafe ()) < 0;
  }
  /**
   * Check if another string is less than this one.
   * \param Str Other string.
   * \return True if the string is 'lesser' than \a Str, false
   *   otherwise.
   */
  bool operator < (const char* Str) const
  {
    return strcmp (GetDataSafe (), Str) < 0;
  }
  /**
   * Check to see if a string is greater than this one.
   * \param Str Other string.
   * \return True if the string is 'greater' than \a Str, false
   *   otherwise.
   */
  bool operator > (const csStringBase& Str) const
  {
    return strcmp (GetDataSafe (), Str.GetDataSafe ()) > 0;
  }
  /**
   * Check to see if a string is greater than this one.
   * \param Str Other string.
   * \return True if the string is 'greater' than \a Str, false
   *   otherwise.
   */
  bool operator > (const char* Str) const
  {
    return strcmp (GetDataSafe (), Str) > 0;
  }
  /**
   * Check if another string is not equal to this one.
   * \param Str Other string.
   * \return False if they are equal; true if not.
   * \remarks The comparison is case-sensitive.
   */
  bool operator != (const csStringBase& Str) const
  { return !Compare (Str); }
  /**
   * Check if another string is not equal to this one.
   * \param Str Other string.
   * \return False if they are equal; true if not.
   * \remarks The comparison is case-sensitive.
   */
  bool operator != (const char* Str) const
  { return !Compare (Str); }

  /**
   * Convert this string to lower-case.
   * \return Reference to itself.
   */
  csStringBase& Downcase();
  /**
   * Convert this string to upper-case.
   * \return Reference to itself.
   */
  csStringBase& Upcase();

  /**
   * Detach the low-level null-terminated C-string buffer from the csString
   * object.
   * \return The low-level null-terminated C-string buffer, or zero if this
   *   string represents a null-pointer.  See the class description for a
   *   discussion about how and when the string will represent a null-pointer.
   * \remarks The caller of this function becomes the owner of the returned
   *   string buffer and is responsible for destroying it via `delete[]' when
   *   no longer needed.
   */
  virtual char* Detach ()
  { char* d = Data; Data = 0; Size = 0; MaxSize = 0; return d; }
};

/// Concatenate a null-terminated C-string with a csStringBase.
inline csStringBase operator + (const char* iStr1, const csStringBase &iStr2)
{
  return csStringBase (iStr1).Append (iStr2);
}

/// Concatenate a csStringBase with a null-terminated C-string.
inline csStringBase operator + (const csStringBase& iStr1, const char* iStr2)
{
  return iStr1.Clone ().Append (iStr2);
}

#define STR_SHIFT(TYPE) \
  inline csStringBase &operator << (csStringBase &s, TYPE v) { return s.Append (v); }
/** @{ */
/** 
 * Shift operator.  
 * For example: 
 * \code
 * s << "Hi " << name << "; see " << foo;
 * \endcode
 */
STR_SHIFT(const csStringBase&)
STR_SHIFT(const char*)
STR_SHIFT(char)
STR_SHIFT(unsigned char)
STR_SHIFT(short)
STR_SHIFT(unsigned short)
STR_SHIFT(int)
STR_SHIFT(unsigned int)
STR_SHIFT(long)
STR_SHIFT(unsigned long)
STR_SHIFT(float)
STR_SHIFT(double)
STR_SHIFT(bool)
#undef STR_SHIFT
/** @} */

/**
 * Subclass of csStringBase that contains an internal buffer which is faster
 * than teh always dynamically allocated buffer of csStringBase.
 */
template<int LEN = 36>
class csStringFast : public csStringBase
{
protected:
  char internalBuffer[LEN];
  virtual void SetCapacityInternal (size_t NewSize, bool extraSpace)
  {
    char* oldBuf = (MaxSize > LEN) ? Data : 0;
    char* newBuf;
    if (NewSize < LEN)
    {
      newBuf = internalBuffer;
      /* No need to honor extraSpace; later "reallocations" are free when
       * using the internal buffer, so no need to "allocate" more than
       * necessary. */
    }
    else
    {
      if (extraSpace)
	NewSize = ComputeNewSize (NewSize) + 1;
      newBuf = new char[NewSize];
    }
    if (newBuf != Data)
    {
      if (Data == 0 || Size == 0)
	newBuf[0] = '\0';
      else
	memcpy(newBuf, Data, Size + 1);
    }

    MaxSize = NewSize;
    Data = newBuf;
    delete[] oldBuf;
  }
public:
  /**
   * Create an empty csStringFast object.
   */
  csStringFast () : csStringBase() { }
  /**
   * Create a csStringFast object and reserve space for at least Length 
   * characters.
   */
  csStringFast (size_t Length) : csStringBase() { SetCapacity (Length); }
  /**
   * Copy constructor.
   */
  csStringFast (const csStringBase& copy) : csStringBase () 
  { Append (copy); }
  /**
   * Copy constructor.
   */
  csStringFast (const csStringFast& copy) : csStringBase () 
  { Append (copy); }
  /**
   * Create a csStringFast object from a null-terminated C string.
   */
  csStringFast (const char* src) : csStringBase() 
  { Append (src); }
  /// Create a csStringFast object from a single signed character.
  csStringFast (char c) : csStringBase()
  { Append (c); }
  /// Create a csStringFast object from a single unsigned character.
  csStringFast (unsigned char c) : csStringBase()
  { Append ((char)c); }
  /// Destroy the csStringBase.
  virtual ~csStringFast () { if (MaxSize <= LEN) Data = 0; }
  virtual void Free ()
  { if (MaxSize <= LEN) Data = 0; csStringBase::Free(); }

  virtual char* Detach ()
  { 
    char* d = (MaxSize <= LEN) ? csStrNew (Data) : Data; 
    Data = 0; Size = 0; MaxSize = 0; return d; }
};

CS_SPECIALIZE_TEMPLATE
class csStringFast<0> : public csStringBase
{
public:
  csStringFast () : csStringBase() { }
  csStringFast (size_t Length) : csStringBase(Length) { }
  csStringFast (const csStringBase& copy) : csStringBase (copy) { }
  csStringFast (const char* src) : csStringBase(src) { }
  csStringFast (char c) : csStringBase(c) { }
  csStringFast (unsigned char c) : csStringBase(c) { }
};

/**
 * Thin wrapper around csStringFast<> with its default buffer size.
 */
class csString : public csStringFast<>
{
public:
  /**
   * Create an empty csString object.
   */
  csString () : csStringFast<> () { }
  /**
   * Create a csString object and reserve space for at least Length characters.
   */
  csString (size_t Length) : csStringFast<> (Length) { }
  /**
   * Copy constructor.
   */
  csString (const csStringBase& copy) : csStringFast<> (copy) { }
  /**
   * Create a csString object from a null-terminated C string.
   */
  csString (const char* src) : csStringFast<> (src) { }
  /// Create a csString object from a single signed character.
  csString (char c) : csStringFast<> (c) { }
  /// Create a csString object from a single unsigned character.
  csString (unsigned char c) : csStringFast<> (c) { }
};


#endif // __CS_CSSTRING_H__
