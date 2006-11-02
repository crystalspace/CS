/*
    Crystal Space Weak Reference Counting Interface
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_WEAKREFERENCED_H__
#define __CS_WEAKREFERENCED_H__

#include "refcount.h"
#include "array.h"

namespace CS
{

/**
 * This is a class which provides basic weak reference-counting semantics.
 * It can be used in conjunction with the smart pointer template class
 * csWeakRef (see <weakref.h>).  This class itself provides no functionality
 * beyond reference counting (csRefCount) and pointer ownership tracking.
 * It is intended that you should subclass CS::WeakReferenced and add needed
 * functionality.
 */
class WeakReferenced : public csRefCount
{
private:
  typedef csArray<void**,
    csArrayElementHandler<void**>,
    CS::Memory::AllocatorMalloc,
    csArrayCapacityLinear<csArrayThresholdFixed<4> > > WeakRefOwnerArray;
  WeakRefOwnerArray* weakref_owners;
public:
  WeakReferenced () : weakref_owners (0) {}

  void AddRefOwner (void** ref_owner)
  {
    if (!weakref_owners)
      weakref_owners = new WeakRefOwnerArray (0);
    weakref_owners->InsertSorted (ref_owner);
  }
  void RemoveRefOwner (void** ref_owner)
  {
    if (!weakref_owners)
      return;

    size_t index = weakref_owners->FindSortedKey (
      csArrayCmp<void**, void**>(ref_owner));

    if (index != csArrayItemNotFound)
      weakref_owners->DeleteIndex (index);
  }
};

}

#endif // __CS_WEAKREFERENCED_H__
