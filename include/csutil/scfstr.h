/*
    Crystal Space Shared String Vector class
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __SCFSTR_H__
#define __SCFSTR_H__

#include "iutil/string.h"
#include "csutil/csstring.h"

/// This is a thin SCF wrapper around csString
class scfString : public iString
{
  csString s;

public:
  DECLARE_IBASE;

  /// Create an empty scfString object
  scfString ()
  { CONSTRUCT_IBASE (NULL); }

  /// Create an scfString object and reserve space for iLength characters
  scfString (int iLength) : s(iLength)
  { CONSTRUCT_IBASE (NULL); }

  /// Copy constructor
  scfString (const iString &copy) : s(copy.GetData())
  { CONSTRUCT_IBASE (NULL); }

  /// Yet another copy constructor
  scfString (const char *copy) : s(copy)
  { CONSTRUCT_IBASE (NULL); }

  /// Destroy a scfString object
  virtual ~scfString () {}

  /// Set string capacity to NewSize characters (plus one for ending NULL)
  virtual void SetCapacity (size_t NewSize);

  /// Truncate the string
  virtual void Truncate (size_t iPos);

  /// Set string maximal capacity to current string length
  virtual void Reclaim ();

  /// Get a copy of this string
  virtual iString *Clone () const;

  /// Get a pointer to ASCIIZ string
  virtual char *GetData () const;

  /// Query string length
  virtual size_t Length () const;

  /// Get a reference to iPos'th character
  virtual inline char& operator [] (size_t iPos);

  /// Set characetr number iPos to iChar
  virtual void SetAt (size_t iPos, char iChar);

  /// Get character at position iPos
  virtual char GetAt (size_t iPos) const;

  /// Insert another string into this one at position iPos
  virtual void Insert (size_t iPos, iString *iStr);

  /// Overlay another string onto a part of this string
  virtual void Overwrite (size_t iPos, iString *iStr);

  /// Append an ASCIIZ string to this one (up to iCount characters)
  virtual iString &Append (const char *iStr, size_t iCount = (size_t)-1);

  /// Append a string to this one (possibly iCount characters from the string)
  virtual iString &Append (const iString *iStr, size_t iCount = (size_t)-1);

  /// Replace contents of this string with the contents of another
  virtual void Replace (const iString *iStr, size_t iCount = (size_t)-1);

  /// Check if two strings are equal
  virtual bool Compare (const iString *iStr) const;

  /// Compare two strings ignoring case
  virtual bool CompareNoCase (const iString *iStr) const;
};

#endif // __SCFSTR_H__
