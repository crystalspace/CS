/*
    Crystal Space Weak Reference Counting Implementation
    Copyright (C) 2006 by Jorrit Tyberghein
	      (C) 2006 by Amir Taaki

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

#ifndef __CS_CSUTIL_WEAKREFERENCED_H__
#define __CS_CSUTIL_WEAKREFERENCED_H__

/**\file
 * Weak Reference Counting Implementation
 */

#include "array.h"

namespace CS
{
namespace Utility
{

/**
 * This is a class which provides basic weak reference-counting semantics.
 * It can be used in conjunction with the smart pointer template class
 * csWeakRef (see <weakref.h>).  This class itself provides no functionality
 * beyond pointer ownership tracking. It is intended to be used to add
 * weak referencing support to other classes by inheriting from it.
 */
class WeakReferenced
{
public:
  WeakReferenced () : weakref_owners (0) {}
  ~WeakReferenced ()
  {
    if (weakref_owners)
    {
      for (size_t i = 0; i < weakref_owners->GetSize(); i++)
      {
        *((*weakref_owners)[i]) = 0;
      }
      delete weakref_owners;
    }
  }

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

private:
  typedef csArray<void**,
    csArrayElementHandler<void**>,
    CS::Memory::AllocatorMalloc,
    csArrayCapacityLinear<csArrayThresholdFixed<4> > > WeakRefOwnerArray;
  WeakRefOwnerArray* weakref_owners;
};

} // namespace Utility
} // namespace CS

#endif // __CS_CSUTIL_WEAKREFERENCED_H__
