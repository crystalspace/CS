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

#ifndef __CS_OBJPOOL_H__
#define __CS_OBJPOOL_H__

#include "parray.h"

/**
 * This class defines a 'pool' class for the given type.
 * This class can be used to create objects of the given 
 * type, but it will re-use older objects if possible to save time. For this 
 * reason, unused objects of the given type should not be deleted but given 
 * to the pool.
 */
template <class T>
class csObjectPool
{
protected:
  csPDelArray<T> Objects;

public:
  /// Get an object from the pool.
  T *Alloc ()
  {
    if (Objects.Length () > 0)
      return Objects.Pop ();
    else
      return new T ();
  }

  /// Give an object back to the pool
  void Free (T* o)
  {
    Objects.Push (o);
  }
};

#endif // __CS_OBJPOOL_H__

