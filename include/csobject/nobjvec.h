/*
    Crystal Space: Named Object Vector class
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

#ifndef __NOBJVEC_H__
#define __NOBJVEC_H__

#include "csutil/csobjvec.h"

class csObject;

/**
 * csNamedObjVector is a version of csObjVector that assumes all
 * its components are csObject's which can be examined by name etc.
 * All csVector methods should work with this class: Find, FindKey,
 * QuickSort, FindSortedKey and so on.
 */
class csNamedObjVector : public csObjVector
{
public:
  /// Constructor just passes control to csVector's
  csNamedObjVector (int ilimit = 8, int ithreshold = 16) :
    csObjVector (ilimit, ithreshold) {}

  /// Find an item in this vector by name and returns it (or NULL if not found)
  csObject *FindByName (const char* iName) const;

  /// Compare two objects by their names
  virtual int Compare (csSome Item1, csSome Item2, int Mode) const;

  /// Compare object's name with a string
  virtual int CompareKey (csSome Item, csConstSome Key, int Mode) const;

  /// Override Get() to avoid casting to csObject
  csObject *Get (int idx) const
  { return (csObject *)csVector::Get (idx); }
};

#endif // __NOBJVEC_H__
