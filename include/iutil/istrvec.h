/*
    Crystal Space String Vector interface
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

#ifndef __ISTRVEC_H__
#define __ISTRVEC_H__

#include "csutil/scf.h"

SCF_VERSION (iStrVector, 0, 0, 1);

/// This is a SCF-compatible interface for csStrVector.
struct iStrVector : public iBase
{
  /// Query array length
  virtual int Length () const = 0;

  /// Push a string onto the stack
  virtual void Push (char *iValue) = 0;

  /// Pop a string from the top of stack
  virtual char *Pop () = 0;

  /// Get Nth string in vector
  virtual char *Get (int iIndex) const = 0;

  /// Find index of given string
  virtual int Find (const char *iValue) const = 0;

  /// Find index of a string in a pre-sorted string array
  virtual int FindSorted (const char *iValue) const = 0;

  /// Sort the string array
  virtual void QuickSort () = 0;

  /// Delete Nth string in the array
  virtual void Delete (int iIndex) = 0;

  /// Insert a string before Nth string in the array
  virtual void Insert (int iIndex, char *iValue) = 0;

  /// Delete all strings in array
  virtual void DeleteAll () = 0;
};

#endif // __ISTRVEC_H__
