/*
    Crystal Space Windowing System: object vector class
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

#ifndef __CSOBJVEC_H__
#define __CSOBJVEC_H__

#include "csutil/csvector.h"

/**
 * csObjVector is a version of csVector which assumes its components
 * are iBase heirs. FreeItem () decref the object .
 */
class csObjVector : public csVector
{
public:
  /// Constructor just passes control to csVector's
  csObjVector (int ilimit = 8, int ithreshold = 16) :
    csVector (ilimit, ithreshold) {}

  /// Delete all inserted objects before deleting the object itself
  virtual ~csObjVector ();

  /// Free a item as a object derived from csBase
  virtual bool FreeItem (csSome Item);
};

#endif // __CSOBJVEC_H__
