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

#ifndef __IUTIL_STRING_H__
#define __IUTIL_STRING_H__

#include "csutil/scf.h"

SCF_VERSION (iString, 0, 0, 1);

/// This is a SCF-compatible interface for csString.
struct iString : public iBase
{
  /// Set string capacity to NewSize characters (plus one for ending NULL)
  virtual void SetCapacity (size_t NewSize) = 0;

  /// Truncate the string
  virtual void Truncate (size_t iPos) = 0;

  /// Set string maximal capacity to current string length
  virtual void Reclaim () = 0;

  /// Clear the string (so that it contains only ending NULL character)
  inline void Clear ()
  { Truncate (0); }

  /// Get a copy of this string
  virtual iString *Clone () const = 0;

  /// Get a pointer to ASCIIZ string
  virtual char *GetData () const = 0;

  /// Query string length
  virtual size_t Length () const = 0;

  /// Check if string is empty
  inline bool IsEmpty () const
  { return !Length (); }

  /// Get a reference to iPos'th character
  virtual inline char& operator [] (size_t iPos) = 0;

  /// Set characetr number iPos to iChar
  virtual void SetAt (size_t iPos, char iChar) = 0;

  /// Get character at position iPos
  virtual char GetAt (size_t iPos) const = 0;

  /// Insert another string into this one at position iPos
  virtual void Insert (size_t iPos, iString *iStr) = 0;

  /// Overlay another string onto a part of this string
  virtual void Overwrite (size_t iPos, iString *iStr) = 0;

  /// Append an ASCIIZ string to this one (up to iCount characters)
  virtual iString &Append (const char *iStr, size_t iCount = (size_t)-1) = 0;

  /// Append a string to this one (possibly iCount characters from the string)
  virtual iString &Append (const iString *iStr, size_t iCount = (size_t)-1)=0;

  /// Replace contents of this string with the contents of another
  virtual void Replace (const iString *iStr, size_t iCount = (size_t)-1) = 0;

  /// Check if two strings are equal
  virtual bool Compare (const iString *iStr) const = 0;

  /// Compare two strings ignoring case
  virtual bool CompareNoCase (const iString *iStr) const = 0;

  /// Append another string to this
  iString &operator += (const iString &iStr)
  { return Append (&iStr); }

  /// Append an ASCIIZ to this string
  iString &operator += (const char *iStr)
  { return Append (iStr); }

  /// Concatenate two strings and return a third one
  iString &operator + (const iString &iStr) const
  { return Clone ()->Append (&iStr); }

  /// Convert iString into ASCIIZ
  operator const char * () const
  { return GetData (); }

  /// Check if two strings are equal
  bool operator == (const iString &iStr) const
  { return Compare (&iStr); }
};

#endif // __IUTIL_STRING_H__
