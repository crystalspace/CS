/*
    Crystal Space utility library: string class
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
  void SetSize (size_t NewSize);

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

  /// Set string maximal capacity to current string length
  void Reclaim ()
  { SetSize (Size); }

  /// Clear the string (so that it contains only ending NULL character)
  csString& Clear ()
  {
    SetSize (0);
    return *this;
  }

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
#ifdef DEBUG
    if (iPos > Size)
      STR_FATAL(("Trying to access string `%s' at position %lu\n",
        Data, (unsigned long)iPos))
#endif
    return Data [iPos];
  }

  /// Set characetr number iPos to iChar
  void SetAt (size_t iPos, const char iChar)
  {
#ifdef DEBUG
    if (iPos > Size)
      STR_FATAL (("Trying to do `%s'.SetAt (%lu)\n",
        Data, (unsigned long)iPos))
#endif
    Data [iPos] = iChar;
  }

  /// Get character at position iPos
  char GetAt (size_t iPos) const
  {
#ifdef DEBUG
    if (iPos > Size)
      STR_FATAL (("Trying to do `%s'.GetAt (%lu)\n",
        Data, (unsigned long)iPos));
#endif
    return Data [iPos];
  }

  /// Insert another string into this one at position iPos
  csString &Insert (size_t iPos, const csString &iStr);

  /// Overlay another string onto a part of this string
  csString &Overwrite (size_t iPos, const csString &iStr);

  /// Append an ASCIIZ string to this one
  /// (possibly iCount characters from the string)
  csString &Append (const char *iStr, size_t iCount = (size_t)-1);

  /// Append a string to this one (possibly iCount characters from the string)
  csString &Append (const csString &iStr, size_t iCount = (size_t)-1);

  /// Append a character to this string
  csString &Append (char c){ char s[2]; s[0] = c; s[1] = 0; return Append(s); }

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
  { SetSize (iLength); }

  /// Copy constructor
  csString (const csString &copy) : Data (NULL), Size (0), MaxSize (0)
  { Append (copy); }

  /// Yet another copy constructor
  csString (const char *copy) : Data (NULL), Size (0), MaxSize (0)
  { Append (copy); }

  /// Destroy a csString object
  virtual ~csString ();

  /// Get a copy of this string
  csString Clone () const
  { return csString (*this); }

  /// Assign a string to another
  csString &operator = (const csString &iStr)
  { return Replace (iStr); }

  /// Append another string to this
  csString &operator += (const csString &iStr)
  { return Append (iStr); }

  /// Append an ASCIIZ to this string
  csString &operator += (const char* iStr)
  { return Append (iStr); }

  /// Concatenate two strings and return a third one
  csString operator + (const csString &iStr) const
  { return Clone ().Append (iStr); }

  /// Convert csString into ASCIIZ
  operator char * ()
  { return Data; }

  /// Return a const reference to this string in ASCIIZ format
  operator const char * () const
  { return Data; }

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

#endif // __CS_CSSTRING_H__
