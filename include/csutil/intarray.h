/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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

#ifndef __SARRAY_H__
#define __SARRAY_H__

#include "csutil/garray.h"

CS_TYPEDEF_GROWING_ARRAY (csIntArrayHelper, int);

class csIntArray : public csIntArrayHelper
{
public:
  /// Find a contained number and return its index
  int Find (int val) const;

  /// Push the given value n times on the array
  void PushFill (int val, int n);

  /// Push n numbers, ranging 'offset' to 'offset+n-1' in increasing order
  void PushIdentityMapping (int n, int offset = 0);

  /// Create an array that is filled n times with the given value
  static csIntArray *CreateUniformMapping (int val, int n);

  /// Create an array that is filled with the values 'offset' to 'offset+n-1'
  static csIntArray *CreateIdentityMapping (int n, int offset = 0);
};

#endif // __SARRAY_H__
