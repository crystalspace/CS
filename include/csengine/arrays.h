/*
    Dynamic arrays of engine objects
    Copyright (C) 1999 by Andrew Zabolotny

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

#ifndef __CS_ARRAYS_H__
#define __CS_ARRAYS_H__

#include "csutil/typedvec.h"

class csLightHalo;

CS_DECLARE_TYPED_VECTOR_NODELETE (csHaloArrayHelper, csLightHalo);

/// A dynamic array of csLightHalo objects
class csHaloArray : public csHaloArrayHelper {
public:
  virtual bool FreeItem (csSome item);
  virtual int CompareKey (csSome Item, csConstSome Key, int Mode) const;
  csHaloArray (int l=8, int t=8) : csHaloArrayHelper (l, t) {}
};

#endif // __CS_ARRAYS_H__
