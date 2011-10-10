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
#include "refcount.h"

namespace CS
{
namespace Utility
{
  namespace Implementation
  {
    /**
     * Base implementation for objects that are weak referencable.
     * Only provides reference owner managerment.
     */
    class WeakReferenced
    {
    public:
      WeakReferenced() : weakref_owners (nullptr) {}
      ~WeakReferenced ()
      {
	ClearRefOwners();
	delete weakref_owners;
      }

      void AddRefOwner (void** ref_owner, CS::Threading::Mutex* mutex)
      {
	if (!weakref_owners)
	  weakref_owners = new WeakRefOwnerArray (0);
	weakref_owners->InsertSorted (WeakRefOwner (ref_owner, mutex));
      }
      void RemoveRefOwner (void** ref_owner)
      {
	if (!weakref_owners)
	  return;

	size_t index = weakref_owners->FindSortedKey (
	  csArrayCmp<WeakRefOwner, void**>(ref_owner));

	if (index != csArrayItemNotFound)
	  weakref_owners->DeleteIndex (index);
      }
      
      /// Set reference owner pointers to nullptr
      void ClearRefOwners ()
      {
	if (weakref_owners)
	{
	  for (size_t i = 0; i < weakref_owners->GetSize(); i++)
	  {
	    const WeakRefOwner& wro ((*weakref_owners)[i]);
	    void** p = wro.ownerObj;
	    *p = nullptr;
	  }
	}
      }
      
      /// Empty ref owners array
      void DeleteAllOwners ()
      {
	if (weakref_owners)
	{
	  weakref_owners->DeleteAll ();
	}
      }
      
      /// Helper to lock the pointers of all weak reference owners
      class ScopedWeakRefOwnersLock
      {
	WeakReferenced& object;
      public:
	ScopedWeakRefOwnersLock (WeakReferenced& object) : object (object)
	{
	  if (object.weakref_owners)
	  {
	    for (size_t i = 0; i < object.weakref_owners->GetSize (); i++)
	    {
	      WeakRefOwner& wro ((*object.weakref_owners)[i]);
	      if (wro.ownerObjMutex) wro.ownerObjMutex->Lock();
	    }
	  }
	}
	~ScopedWeakRefOwnersLock ()
	{
	  if (object.weakref_owners)
	  {
	    for (size_t i = 0; i < object.weakref_owners->GetSize (); i++)
	    {
	      WeakRefOwner& wro ((*object.weakref_owners)[i]);
	      if (wro.ownerObjMutex) wro.ownerObjMutex->Unlock();
	    }
	  }
	}
      };

    protected:
      struct WeakRefOwner
      {
	void** ownerObj;
	CS::Threading::Mutex* ownerObjMutex;
	
	WeakRefOwner (void** p, CS::Threading::Mutex* mutex)
	  : ownerObj (p), ownerObjMutex (mutex) {}
	  
	bool operator<(const WeakRefOwner& other) const
	{ return ownerObj < other.ownerObj; }
	bool operator<(void** other) const
	{ return ownerObj < other; }
	friend bool operator<(void** o1, const WeakRefOwner& o2)
	{ return o1 < o2.ownerObj; }
      };
      typedef csArray<WeakRefOwner,
	csArrayElementHandler<WeakRefOwner>,
	CS::Memory::AllocatorMalloc,
	csArrayCapacityLinear<csArrayThresholdFixed<4> > > WeakRefOwnerArray;
      WeakRefOwnerArray* weakref_owners;
    };
  }

/**
 * This is a class which provides basic weak reference-counting semantics.
 * It can be used in conjunction with the smart pointer template class
 * csWeakRef (see <weakref.h>).  This class itself provides no functionality
 * beyond pointer ownership tracking. It is intended to be used to add
 * weak referencing support to other classes by inheriting from it.
 * \warning This class is <b>not</b> thread-safe! Manipulating reference owners
 * on different threads will not work correctly.
 */
class WeakReferenced
{
public:
  void AddRefOwner (void** ref_owner, CS::Threading::Mutex* romutex)
  {
    impl.AddRefOwner (ref_owner, romutex);
  }
  void RemoveRefOwner (void** ref_owner)
  {
    impl.RemoveRefOwner (ref_owner);
  }
  
  typedef WeakReferenced* WeakReferencedKeepAlive;
private:
  Implementation::WeakReferenced impl;
};

} // namespace Utility
} // namespace CS

#endif // __CS_CSUTIL_WEAKREFERENCED_H__
