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

#include <stdarg.h>
#include "csutil/snprintf.h"

/**
 * This is a string class with a range of useful operators and
 * typesafe overloads.
 */
class csString
{
protected:
  /// String buffer.
  char *Data;
  /// Length of string; not including null terminator.
  size_t Size;
  /// Size in bytes of allocated string buffer.
  size_t MaxSize;

public:
  /**
   * Set string capacity to at least NewSize characters (plus one for null
   * terminator).  Never shrinks capacity.  If you need to actually reclaim
   * memory, then use Free() or Reclaim().
   */
  void SetCapacity (size_t NewSize);

  /// Free the memory allocated for the string
  void Free ();

  /// Truncate the string to length iLen.
  csString &Truncate (size_t iLen);

  /// Set string buffer capacity to exactly hold the current content.
  csString &Reclaim ();

  /// Clear the string (so that it contains only a null terminator)
  csString& Clear ()
  { return Truncate (0); }

  /**
   * Get a pointer to ASCIIZ string - warning: this is a non constpointer
   * ,so use this function with care!
   */
  char* GetData () const
  { return Data; }

  /// Query string length.  Length does not include null terminator.
  size_t Length () const
  { return Size; }

  /// Check if string is empty
  bool IsEmpty () const
  { return (Size == 0); }

  /// Get a reference to n'th character
  char& operator [] (size_t n)
  {
    CS_ASSERT (n < Size);
    return Data [n];
  }

  /// get n'th character
  char operator [] (size_t n) const
  {
    CS_ASSERT (n < Size);
    return Data[n];
  }

  /**
   * Set character at position `n'.  Does not expand string if `n' is greater
   * than length of string.
   */
  void SetAt (size_t n, const char c)
  {
    CS_ASSERT (n < Size);
    Data [n] = c;
  }

  /// Get character at n'th position.
  char GetAt (size_t n) const
  {
    CS_ASSERT (n < Size);
    return Data [n];
  }

  /// Delete iCount characters at starting iPos
  csString& DeleteAt (size_t iPos, size_t iCount = 1);

  /// Insert another string into this one at position iPos
  csString& Insert (size_t iPos, const csString&);

  /// Insert a char into this string at position iPos
  csString& Insert (size_t iPos, const char);

  /// Overlay another string onto a part of this string
  csString& Overwrite (size_t iPos, const csString&);

  /**
   * Append an ASCIIZ string to this one.  If iCount is -1, then the entire
   * string is appended.  Otherwise, only iCount characters from the string are
   * appended.
   */
  csString& Append (const char*, size_t iCount = (size_t)-1);

  /**
   * Append a string to this one.  If iCount is -1, then the entire string is
   * appended.  Otherwise, only iCount characters from the string are appended.
   */
  csString& Append (const csString &iStr, size_t iCount = (size_t)-1);

  /// Append a character to this string.
  csString& Append (char c)
  { char s[2]; s[0] = c; s[1] = '\0'; return Append(s); }
  /// Append an unsigned character to this string.
  csString &Append (unsigned char c)
  { return Append(char(c)); }

#define STR_APPEND(TYPE,FMT,SZ) csString& Append(TYPE n) \
  { char s[SZ]; cs_snprintf(s, SZ, #FMT, n); return Append(s); }
  STR_APPEND(short, %hd, 32)
  STR_APPEND(unsigned short, %hu, 32)
  STR_APPEND(int, %d, 32)
  STR_APPEND(unsigned int, %u, 32)
  STR_APPEND(long, %ld, 32)
  STR_APPEND(unsigned long, %lu, 32)
  STR_APPEND(float, %g, 64)
  STR_APPEND(double, %g, 64)
#undef STR_APPEND

#if !defined(CS_USE_FAKE_BOOL_TYPE)
  /// Append a boolean (as a number -- 1 or 0) to this string
  csString& Append (bool b) { return Append (b ? "1" : "0"); }
#endif

  /**
   * Replace contents of this string with the contents of another.  If iCount
   * is -1, then use the entire replacement string.  Otherwise, use iCount
   * characters from the replacement string.
   */
  csString& Replace (const csString& iStr, size_t iCount = (size_t)-1)
  {
    Size = 0;
    return Append (iStr, iCount);
  }

  /**
   * Replace contents of this string with the contents of another.  If iCount
   * is -1, then use the entire replacement string.  Otherwise, use iCount
   * characters from the replacement string.
   */
  csString& Replace (const char* iStr, size_t iCount = (size_t)-1)
  {
    Size = 0;
    return Append (iStr, iCount);
  }

#define STR_REPLACE(TYPE) csString& Replace (TYPE s) { Size = 0; return Append(s); }
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
#ifndef CS_USE_FAKE_BOOL_TYPE
  STR_REPLACE(bool)
#endif
#undef STR_REPLACE

  /// Check if two strings are equal
  bool Compare (const csString& iStr) const
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

  /// Check if an ASCIIZ string is equal to this string.
  bool Compare (const char* iStr) const
  { return (strcmp (Data ? Data : "", iStr) == 0); }

  /// Compare two strings ignoring case
  bool CompareNoCase (const csString& iStr) const
  {
    if (&iStr == this)
      return true;
    size_t const n = iStr.Length();
    if (Size != n)
      return false;
    if (Size == 0 && n == 0)
      return true;
    return (strncasecmp (Data, iStr.GetData (), Size) == 0);
  }

  /// Compare ignoring case with an ASCIIZ string
  bool CompareNoCase (const char* iStr) const
  { return (strncasecmp (Data ? Data : "", iStr, Size) == 0); }

  /// Create an empty csString object
  csString () : Data (NULL), Size (0), MaxSize (0) {}

  /// Create an csString object and reserve space for iLength characters
  csString (size_t iLength) : Data (NULL), Size (0), MaxSize (0)
  { SetCapacity (iLength); }

  /// Copy constructor from existing csString.
  csString (const csString& copy) : Data (NULL), Size (0), MaxSize (0)
  { Append (copy); }

  /// Copy constructor from ASCIIZ string
  csString (const char* copy) : Data (NULL), Size (0), MaxSize (0)
  { Append (copy); }

  /// Copy constructor from a character
  csString (char c) : Data (NULL), Size (0), MaxSize (0)
  { Append (c); }

  /// Copy constructor from a character (unsigned)
  csString (unsigned char c) : Data(NULL), Size (0), MaxSize (0)
  { Append ((char) c); }

  /// Destroy a csString object
  virtual ~csString ();

  /// Get a copy of this string
  csString Clone () const
  { return csString (*this); }

  /// Trim leading whitespace.
  csString& LTrim();

  /// Trim trailing whitespace.
  csString& RTrim();

  /// Trim leading and trailing whitespace.
  csString& Trim();

  /**
   * Trims leading and trailing whitespace, and collapses all internal
   * whitespace to a single space.
   */
  csString& Collapse();

  /**
   * Format this string using sprintf() formatting directives.  Automatically
   * allocates sufficient memory to hold result.  Newly formatted string
   * overwrites previous string value.
   */
  csString& Format(const char *format, ...) CS_GNUC_PRINTF (2, 3);

#define STR_FORMAT(TYPE,FMT,SZ) \
  static csString Format (TYPE v);
  STR_FORMAT(short, %hd, 32)
  STR_FORMAT(unsigned short, %hu, 32)
  STR_FORMAT(int, %d, 32)
  STR_FORMAT(unsigned int, %u, 32)
  STR_FORMAT(long, %ld, 32)
  STR_FORMAT(unsigned long, %lu, 32)
  STR_FORMAT(float, %g, 64)
  STR_FORMAT(double, %g, 64)
#undef STR_FORMAT

#define STR_FORMAT_INT(TYPE,FMT) \
  static csString Format (TYPE v, int width, int prec=0);
  STR_FORMAT_INT(short, hd)
  STR_FORMAT_INT(unsigned short, hu)
  STR_FORMAT_INT(int, d)
  STR_FORMAT_INT(unsigned int, u)
  STR_FORMAT_INT(long, ld)
  STR_FORMAT_INT(unsigned long, lu)
#undef STR_FORMAT_INT

#define STR_FORMAT_FLOAT(TYPE) \
  static csString Format (TYPE v, int width, int prec=6);
  STR_FORMAT_FLOAT(float)
  STR_FORMAT_FLOAT(double)
#undef STR_FORMAT_FLOAT

  /// Pad to specified size with leading characters (default: space).
  csString& PadLeft (size_t iNewSize, char iChar=' ');

  /// Return a new string formatted with PadLeft().
  csString AsPadLeft (size_t iNewSize, char iChar=' ');

  // Return a new left-padded string representation of a basic type
#define STR_PADLEFT(TYPE) \
  static csString PadLeft (TYPE v, size_t iNewSize, char iChar=' ');
  STR_PADLEFT(const csString&)
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
#if !defined(CS_USE_FAKE_BOOL_TYPE)
  STR_PADLEFT(bool)
#endif
#undef STR_PADLEFT

  /// Pad to specified size with trailing characters (default: space).
  csString& PadRight (size_t iNewSize, char iChar=' ');

  /// Return a new string formatted with PadRight().
  csString AsPadRight (size_t iNewSize, char iChar=' ');

  // Return a new right-padded string representation of a basic type
#define STR_PADRIGHT(TYPE) \
  static csString PadRight (TYPE v, size_t iNewSize, char iChar=' ');
  STR_PADRIGHT(const csString&)
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
#if !defined(CS_USE_FAKE_BOOL_TYPE)
  STR_PADRIGHT(bool)
#endif
#undef STR_PADRIGHT

  /// Pad to specified size between characters (any remainder is appended).
  csString& PadCenter (size_t iNewSize, char iChar=' ');

  /// Return a copy of this string formatted with PadCenter().
  csString AsPadCenter (size_t iNewSize, char iChar=' ');

  // Return a new left+right padded string representation of a basic type
#define STR_PADCENTER(TYPE) \
  static csString PadCenter (TYPE v, size_t iNewSize, char iChar=' ');
  STR_PADCENTER(const csString&)
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
#if !defined(CS_USE_FAKE_BOOL_TYPE)
  STR_PADCENTER(bool)
#endif
#undef STR_PADCENTER

  /// Assign a string to another
#define STR_ASSIGN(TYPE) const csString& operator = (TYPE s) { return Replace (s); }
  STR_ASSIGN(const csString&)
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
#ifndef CS_USE_FAKE_BOOL_TYPE
  STR_ASSIGN(bool)
#endif
#undef STR_ASSIGN

#define STR_APPEND(TYPE) csString &operator += (TYPE s) { return Append (s); }
  STR_APPEND(const csString&)
  STR_APPEND(const char*)
  STR_APPEND(char)
  STR_APPEND(unsigned char)
  STR_APPEND(short)
  STR_APPEND(unsigned short)
  STR_APPEND(int)
  STR_APPEND(unsigned int)
  STR_APPEND(long);
  STR_APPEND(unsigned long)
  STR_APPEND(float)
  STR_APPEND(double)
#ifndef CS_USE_FAKE_BOOL_TYPE
  STR_APPEND(bool)
#endif
#undef STR_APPEND

  /// Add another string to this one and return the result as a new string.
  const csString& operator + (const csString &iStr) const
  { return Clone ().Append (iStr); }

  /// Return a const reference to this string in ASCIIZ format
  operator const char* () const
  { return Data ? Data : ""; }

  /// Check if two strings are equal
  bool operator == (const csString& iStr) const
  { return Compare (iStr); }
  bool operator == (const char* iStr) const
  { return Compare (iStr); }

  /**
   * Detach the low-level null-terminated string buffer from the csString
   * object.  The caller of this function becomes the owner of the returned
   * string buffer and is responsible for destroying it via `delete[]' when
   * no longer needed.  The returned value may be NULL if no buffer had been
   * allocated for this string.
   */
  char* Detach ()
  { char *d = Data; Data = 0; Size = 0; MaxSize = 0; return d; }
};

/// Concatenate a csString with an ASCIIZ and return resulting csString
inline csString operator + (const char* iStr1, const csString &iStr2)
{
  return csString (iStr1).Append (iStr2);
}

/// Concatenate a csString with an ASCIIZ and return resulting csString
inline csString operator + (const csString &iStr1, const char* iStr2)
{
  return iStr1.Clone ().Append (iStr2);
}

// Handy shift operators.
#define STR_SHIFT(TYPE) \
  inline csString &operator << (csString &s, TYPE v) { return s.Append (v); }
STR_SHIFT(const csString&)
STR_SHIFT(const char*)
STR_SHIFT(char)
STR_SHIFT(unsigned char)
STR_SHIFT(short)
STR_SHIFT(unsigned short)
STR_SHIFT(int)
STR_SHIFT(unsigned int)
STR_SHIFT(long);
STR_SHIFT(unsigned long)
STR_SHIFT(float)
STR_SHIFT(double)
#if !defined(CS_USE_FAKE_BOOL_TYPE)
STR_SHIFT(bool)
#endif
#undef STR_SHIFT

#endif // __CS_CSSTRING_H__
