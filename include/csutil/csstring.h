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

#define STR_FATAL(s) \
{ \
  printf s; \
  abort (); \
}

/**
 * This is a "string" class with a range of useful operators and
 * typesafe overloads.
 */
class csString
{
  char *Data;
  size_t Size, MaxSize;

public:
  /// Set string capacity to NewSize characters (plus one for ending NULL)
  void SetCapacity (size_t NewSize);

  /// Free the memory allocated for the string
  void Free ()
  {
    if (Data)
    {
      delete[] Data;
      Data = NULL;
      Size = MaxSize = 0;
    }
  }

  /// Truncate the string
  csString &Truncate (size_t iPos);

  /// Set string maximal capacity to current string length
  csString &Reclaim ()
  { SetCapacity (Size); return *this; }

  /// Clear the string (so that it contains only ending NULL character)
  csString& Clear ()
  { return Truncate (0); }

  /// Get a pointer to ASCIIZ string
  char *GetData () const
  { return Data; }

  /// Query string length
  size_t Length () const
  { return Size; }

  /// Check if string is empty
  bool IsEmpty () const
  { return !Size; }

  /// Get a reference to iPos'th character
  inline char &operator [] (size_t iPos)
  {
#ifdef CS_DEBUG
    if (iPos > Size)
      STR_FATAL(("Trying to access string `%s' at position %lu\n",
        Data, (unsigned long)iPos))
#endif
    return Data [iPos];
  }

  /// Set characetr number iPos to iChar
  void SetAt (size_t iPos, const char iChar)
  {
#ifdef CS_DEBUG
    if (iPos > Size)
      STR_FATAL (("Trying to do `%s'.SetAt (%lu)\n",
        Data, (unsigned long)iPos))
#endif
    Data [iPos] = iChar;
  }

  /// Get character at position iPos
  char GetAt (size_t iPos) const
  {
#ifdef CS_DEBUG
    if (iPos > Size)
      STR_FATAL (("Trying to do `%s'.GetAt (%lu)\n",
        Data, (unsigned long)iPos));
#endif
    return Data [iPos];
  }

  /// Delete iCount characters at iPos
  csString &DeleteAt (size_t iPos, size_t iCount = 1);

  /// Insert another string into this one at position iPos
  csString &Insert (size_t iPos, const csString &iStr);

  /// Insert a char into this string at position iPos
  csString &Insert (size_t iPos, const char iChar);

  /// Overlay another string onto a part of this string
  csString &Overwrite (size_t iPos, const csString &iStr);

  /// Append an ASCIIZ string to this one
  /// (possibly iCount characters from the string)
  csString &Append (const char *iStr, size_t iCount = (size_t)-1);

  /// Append a string to this one (possibly iCount characters from the string)
  csString &Append (const csString &iStr, size_t iCount = (size_t)-1);

  /// Append a character to this string
  csString &Append (char c){ char s[2]; s[0] = c; s[1] = 0; return Append(s); }
  csString &Append (unsigned char c){ return Append(char(c)); }

#define STR_APPEND(TYPE,FMT,SZ) csString &Append(TYPE n) \
  { char s[SZ]; sprintf(s, #FMT, n); return Append(s); }
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
  csString &Append (bool b) { return Append (b ? "1" : "0"); }
#endif

  /// Replace contents of this string with the contents of another
  csString &Replace (const csString &iStr, size_t iCount = (size_t)-1)
  {
    Size = 0;
    return Append (iStr, iCount);
  }

  /// Check if two strings are equal
  bool Compare (const csString &iStr) const
  {
    if (Size != iStr.Length ())
      return false;

    return (memcmp (Data, iStr.GetData (), Size) == 0);
  }

  /// Same but compare with an ASCIIZ string
  bool Compare (const char *iStr) const
  { return (strcmp (Data, iStr) == 0); }

  /// Compare two strings ignoring case
  bool CompareNoCase (const csString &iStr) const
  {
    if (Size != iStr.Length ())
      return false;
    return (strncasecmp (Data, iStr.GetData (), Size) == 0);
  }

  /// Compare ignoring case with an ASCIIZ string
  bool CompareNoCase (const char *iStr) const
  { return (strncasecmp (Data, iStr, Size) == 0); }

  /// Create an empty csString object
  csString () : Data (NULL), Size (0), MaxSize (0) {}

  /// Create an csString object and reserve space for iLength characters
  csString (size_t iLength) : Data (NULL), Size (0), MaxSize (0)
  { SetCapacity (iLength); }

  /// Copy constructor from existing csString.
  csString (const csString &copy) : Data (NULL), Size (0), MaxSize (0)
  { Append (copy); }

  /// Copy constructor from ASCIIZ string
  csString (const char *copy) : Data (NULL), Size (0), MaxSize (0)
  { Append (copy); }

  /// Copy constructor from a character
  csString (char c) : Data (NULL), Size (0), MaxSize (0)
  { Append (c); }

  /// Destroy a csString object
  virtual ~csString ();

  /// Get a copy of this string
  csString Clone () const
  { return csString (*this); }

  /// Trim all of the white space off of the left side of the string
  csString &LTrim();

  /// Trim all of the white space off of the right side of the string
  csString &RTrim();

  /// Does both LTrim() and RTrim()
  csString &Trim();

  /// Calls Trim() and collapses internal whitespace to a single space.
  csString &Collapse();

  csString &Format(const char *format, ...) CS_GNUC_PRINTF (2, 3);

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

  /// Pad to specified size with leading chars
  csString& PadLeft (size_t iNewSize, char iChar=' ');

  /// Return a new string formatted with PadLeft
  csString AsPadLeft (size_t iNewSize, char iChar=' ');

  /// Return a new left-padded string representation of a basic type
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

  /// Pad to specified size with trailing chars
  csString& PadRight (size_t iNewSize, char iChar=' ');

  /// Return a new string formatted with PadRight
  csString AsPadRight (size_t iNewSize, char iChar=' ');

  /// Return a new right-padded string representation of a basic type
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

  /// Pad to specified size between chars (any remainder is prepended)
  csString& PadCenter (size_t iNewSize, char iChar=' ');

  /// Return a copy of this string formatted with PadCenter
  csString AsPadCenter (size_t iNewSize, char iChar=' ');

  /// Return a new left+right padded string representation of a basic type
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
  csString &operator = (const csString &iStr)
  { return Replace (iStr); }

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
#if !defined(CS_USE_FAKE_BOOL_TYPE)
  STR_APPEND(bool)
#endif
#undef STR_APPEND

  /// Concatenate two strings and return a third one
  csString operator + (const csString &iStr) const
  { return Clone ().Append (iStr); }

  /// Return a const reference to this string in ASCIIZ format
  operator const char * () const
  { return Data ? Data : ""; }

  /// Check if two strings are equal
  bool operator == (const csString &iStr) const
  { return Compare (iStr); }

  /**
   * Detach the low-level null-terminated string buffer from the csString
   * object.  The caller of this function becomes the owner of the returned
   * string buffer and is responsible for destroying it via `delete[]' when
   * no longer needed.
   */
  char *Detach ()
  { char *d = Data; Data = NULL; Size = MaxSize = 0; return d; }
};

/// Concatenate a csString with an ASCIIZ and return resulting csString
inline csString operator + (const char* iStr1, const csString &iStr2)
{
  return csString (iStr1).Append (iStr2);
}

/// Same but in reverse order (optimization)
inline csString operator + (const csString &iStr1, const char* iStr2)
{
  return iStr1.Clone ().Append (iStr2);
}

/// Handy shift operators.
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
