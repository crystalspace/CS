/*
  Crystal Space Smart Pointers
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

#include "csutil/weakref.h"
#include "csutil/array.h"

template <class T>
class csWeakRefArrayMemoryAllocator
{
public:
  static csWeakRef<T>* Alloc (int count)
  {
    return (csWeakRef<T>*)malloc (count * sizeof(csWeakRef<T>));
  }

  static void Free (csWeakRef<T>* mem)
  {
    free (mem);
  }

  static csWeakRef<T>* Realloc (csWeakRef<T>* mem, int relevantcount, int newcount)
  {
    int i;
    // First unlink all weak references from their object.
    for (i = 0 ; i < relevantcount ; i++) mem[i].Unlink ();
    mem = (csWeakRef<T>*)realloc (mem, newcount * sizeof(csWeakRef<T>));
    for (i = 0 ; i < relevantcount ; i++) mem[i].Link ();
    return mem;
  }
};

/**
 * An array of weak references.
 */
template <class T>
class csWeakRefArray
	: public csArray<csWeakRef<T>,
		csArrayElementHandler<csWeakRef<T> >,
		csWeakRefArrayMemoryAllocator<T> >
{
public:
  /**
   * Initialize object to hold initially 'ilimit' elements, and increase
   * storage by 'ithreshold' each time the upper bound is exceeded.
   */
  csWeakRefArray (int ilimit = 0, int ithreshold = 0)
  	: csArray<csWeakRef<T>, csArrayElementHandler<csWeakRef<T> >,
			csWeakRefArrayMemoryAllocator<T> > (ilimit, ithreshold)
  {
  }

  /**
   * Compact this array by removing all weak references that have
   * become 0.
   */
  void Compact ()
  {
    int i = Length ()-1;
    while (i >= 0)
    {
      if (Get (i) == 0)
        DeleteIndex (i);
      i--;
    }
  }
};

#endif // __CS_WEAKREFARR_H__

