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

#define STR_FATAL(s) \
{ \
  printf s; \
  abort (); \
}

/**
 * This is a "string" class with a primitive set of operators -
 * assign, concatenate, delete, compare. This is way less effective
 * than regular char[] so use it only if you're extremely lazy and
 * don't care about performance and code size.
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
      free (Data);
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
  csString (int iLength) : Data (NULL), Size (0), MaxSize (0)
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

  /// Format like sprintf. Very fast.
  /// Warning! No bounds checking or resizing is performed!
  csString &Format (const char *format, ...)
  {
    va_list args;
    va_start (args, format);
    vsprintf (Data, format, args);
    va_end (args);
    return *this;
  }

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
