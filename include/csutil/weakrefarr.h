/*
  Crystal Space Weak Reference array
  Copyright (C) 2004 by Jorrit Tyberghein

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

#ifndef __CS_WEAKREFARR_H__
#define __CS_WEAKREFARR_H__

/**\file
 * Weak Reference array
 */

//-----------------------------------------------------------------------------
// Note *1*: The explicit "this->" is needed by modern compilers (such as gcc
// 3.4.x) which distinguish between dependent and non-dependent names in
// templates.  See: http://gcc.gnu.org/onlinedocs/gcc/Name-lookup.html
//-----------------------------------------------------------------------------

#include "csextern.h"
#include "csutil/array.h"
#include "csutil/weakref.h"

/**
 * An array of weak references.
 */
template <class T>
class csWeakRefArray : public csSafeCopyArray<csWeakRef<T> >
{
public:
  /**
   * Initialize object to hold initially 'ilimit' elements, and increase
   * storage by 'ithreshold' each time the upper bound is exceeded.
   */
  csWeakRefArray (int ilimit = 0, int ithreshold = 0)
  	: csSafeCopyArray<csWeakRef<T> > (ilimit, ithreshold)
  {
  }

  /**
   * Compact this array by removing all weak references that have
   * become 0.
   */
  void Compact ()
  {
    int i = this->Length ()-1; // see *1*
    while (i >= 0)
    {
      if (this->Get (i) == 0)  // see *1*
	this->DeleteIndex (i);
      i--;
    }
  }
};

#endif // __CS_WEAKREFARR_H__
