/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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

#ifndef __CS_CSUTIL_FIFO_H__
#define __CS_CSUTIL_FIFO_H__

#include "array.h"

template <class T, class ElementHandler = csArrayElementHandler<T>,
	   class MemoryAllocator = csArrayMemoryAllocator<T> >
class csFIFO
{
  csArray<T, ElementHandler, MemoryAllocator> a1, a2;
public:
  csFIFO (size_t icapacity = 0, size_t ithreshold = 0) 
    :  a1 (icapacity, ithreshold), a2 (icapacity, ithreshold) { }
  
  T PopTop ()
  {
    CS_ASSERT ((a1.Length() > 0) || (a2.Length() > 0));
    if (a2.Length() == 0)
    {
      size_t n = a1.Length();
      while (n-- > 0)
      {
	a2.Push (a1[n]);
      }
      a1.Empty();
    }
    return a2.Pop();
  }

  void Push (T const& what)
  {
    a1.Push (what);
  }

  size_t Length()
  {
    return a1.Length() + a2.Length();
  }

  bool Delete (T const& what)
  {
    return (a1.Delete (what) || a2.Delete (what));
  }

  void DeleteAll ()
  {
    a1.DeleteAll();
    a2.DeleteAll();
  }
};

#endif // __CS_CSUTIL_FIFO_H__
