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

#ifndef __CS_SCFSTRV_H__
#define __CS_SCFSTRV_H__

#include "iutil/strvec.h"
#include "csutil/csstrvec.h"

/// This class is a thin wrapper around csStrVector with SCF capability
class scfStrVector : public iStrVector
{
  csStrVector v;

public:
  DECLARE_IBASE;

  /// Create a iStrVector from scratch
  scfStrVector (int iLimit = 16, int iDelta = 16) : v (iLimit, iDelta)
  { CONSTRUCT_IBASE (NULL); }

  /// Destructor - nothing to do
  virtual ~scfStrVector ()
  { }

  /// Query array length
  virtual int Length () const;

  /// Push a string onto the stack
  virtual void Push (char *iValue);

  /// Pop a string from the top of stack
  virtual char *Pop ();

  /// Get Nth string in vector
  virtual char *Get (int iIndex) const;

  /// Find index of given string
  virtual int Find (const char *iValue) const;

  /// Find index of a string in a pre-sorted string array
  virtual int FindSorted (const char *iValue) const;

  /// Sort the string array
  virtual void QuickSort ();

  /// Delete Nth string in the array
  virtual void Delete (int iIndex);

  /// Insert a string before Nth string in the array
  virtual void Insert (int iIndex, char *iValue);

  /// Delete all strings in array
  virtual void DeleteAll ();
};

#endif // __CS_SCFSTRV_H__
