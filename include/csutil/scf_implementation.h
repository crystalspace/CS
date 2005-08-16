/*
  Crystal Space Shared Class Facility (SCF)
  This header contains the parts of SCF that is needed when creating
  new classes which implements SCF interfaces.

  Copyright (C) 2005 by Marten Svanfeldt
            (C) 2005 by Michael Adams

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

#ifndef __CSUTIL_SCF_IMPLEMENTATION_H__
#define __CSUTIL_SCF_IMPLEMENTATION_H__

#include "csextern.h"

#include "csutil/array.h"

// Needs to have iBase etc
#include "csutil/scf_interface.h"

/**\file
 * Crystal Space Shared Class Facility (SCF) - implementation creation 
 * support
 */

/**
 * \addtogroup scf
 * @{ */

/**
 * Baseclass for the SCF implementation templates.
 * Provides common methods such as reference counting and handling of
 * weak references.
 */
template<class Class>
class scfImplementation : public virtual iBase
{
public:
  /**
  * Constructor. Initialize the SCF-implementation in class named Class.
  * Will be called from scfImplementation(Ext)N constructor
  */
  scfImplementation (Class *object, iBase *parent = 0) :
      scfObject (object), scfRefCount (1), scfParent (parent), 
        scfWeakRefOwners (NULL)
  {
    if (scfParent) scfParent->IncRef ();
  }

  // Cleanup
  virtual ~scfImplementation()
  {
    scfRemoveRefOwners ();
  }

  /**
   * Decrease reference count, and when it runs out delete the object 
   */
  void DecRef ()
  {
    scfRefCount--;
    if (scfRefCount == 0)
    {
      scfRemoveRefOwners ();
      if (scfParent) scfParent->DecRef();
      delete scfObject;
    }
  }

  /**
   * Increase reference count. 
   */
  void IncRef ()
  {
    scfRefCount++;
  }

  /**
   * Get the reference count. Just for debugging. 
   */
  int GetRefCount ()
  {
    return scfRefCount;
  }

  void AddRefOwner (iBase** ref_owner)
  {
    if (!this->scfWeakRefOwners)
      scfWeakRefOwners = new csArray<iBase**> (0, 4);
    scfWeakRefOwners->InsertSorted (ref_owner);
  }

  void RemoveRefOwner (iBase** ref_owner)
  {
    if (!scfWeakRefOwners)
      return;

    size_t index = scfWeakRefOwners->FindSortedKey (
      csArrayCmp<iBase**, iBase**>(ref_owner));

    if (index != csArrayItemNotFound)
      scfWeakRefOwners->DeleteIndex (index);
  }

protected:
  Class *scfObject;

  int scfRefCount;
  iBase *scfParent;
  csArray<iBase**>* scfWeakRefOwners;

  void scfRemoveRefOwners ()
  {
    if (!scfWeakRefOwners)
      return;

    for (size_t i = 0; i < scfWeakRefOwners->Length (); i++)
    {
      iBase** p = (*scfWeakRefOwners)[i];
      *p = NULL;
    }
    delete scfWeakRefOwners;
    scfWeakRefOwners = NULL;
  }

  /**
   * Query this implementation for a specific interface 
   */
  void *QueryInterface (scfInterfaceID iInterfaceID,
                        scfInterfaceVersion iVersion)
  {
    // Default, just check iBase.. all objects have iBase
    if (iInterfaceID == scfInterfaceTraits<iBase>::GetID () &&
      scfCompatibleVersion (iVersion, scfInterfaceTraits<iBase>::GetVersion ()))
    {
      scfObject->IncRef ();
      return CS_STATIC_CAST(iBase*,scfObject);
    }

    // For embedded interfaces
    if (scfParent)
      return scfParent->QueryInterface (iInterfaceID, iVersion);

    return NULL;
  }

  /**
   * Helper function to get a given interface. Used in the implementation
   * of child-classes
   */
  template<class I>
  CS_FORCEINLINE void* GetInterface(scfInterfaceID iInterfaceID, 
                                    scfInterfaceVersion iVersion)
  {
    if (iInterfaceID == scfInterfaceTraits<I>::GetID () &&
      scfCompatibleVersion (iVersion, scfInterfaceTraits<I>::GetVersion ()))
    {
      scfObject->IncRef ();
      return CS_STATIC_CAST(I*, scfObject);
    }
    else
    {
      return NULL;
    }
  }
};

/**
 * Fugly helper to resolve some bad situations. ;)
 * Basically, it adds a new entry to QueryInterface() without adding another 
 * class to inheritance.
 *
 * Consider the following case:
 *
 * \code
 * struct iA : public virtual iBase {};
 * struct iB : public iA {};
 *
 * class myB : public scfImplementation1<myB, iB> {..}.
 * \endcode
 *
 * Querying iA from myB will then fail even though myB inherits from iA
 * (through iB). By changing the declaration to
 *
 * \code
 * class myB : public scfImplementation2<myB, iB, scfFakeInterface<iA> > {..}
 * \endcode
 *
 * you make sure you can query iA from myB.
 * \warning This is potentially dangerous as you can add whatever interface
 * to another. USE WITH CARE!
 */
template<class If>
class scfFakeInterface
{
public:
  struct InterfaceTraits 
  {
    static CS_FORCEINLINE scfInterfaceVersion GetVersion()
    { 
      return If::InterfaceTraits::GetVersion ();
    }
    static CS_FORCEINLINE char const * GetName() 
    { 
      return If::InterfaceTraits::GetName ();
    }
  };
};

#define SCF_IN_IMPLEMENTATION_H 1
// Instead of duplicating the code for every scfImplementationN and
// scfImplementationExtN, the code is factored out into an include file
// that we include multiple times.
#define SCF_IMPL_N 1
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 2
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 3
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 4
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 5
#include "scf_impl.h"
#undef SCF_IMPL_N
/*
#define SCF_IMPL_N 6
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 7
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 8
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 9
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 10
#include "scf_impl.h"
#undef SCF_IMPL_N
*/
// Now all the scfImplementationExt are defined
#define SCF_IMPL_EXT

#define SCF_IMPL_N 1
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 2
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 3
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 4
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 5
#include "scf_impl.h"
#undef SCF_IMPL_N
/*
#define SCF_IMPL_N 6
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 7
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 8
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 9
#include "scf_impl.h"
#undef SCF_IMPL_N

#define SCF_IMPL_N 10
#include "scf_impl.h"
#undef SCF_IMPL_N
*/
#undef SCF_IMPL_EXT
#undef SCF_IN_IMPLEMENTATION_H

/** @} */



/* Some examples:
struct iFoo : virtual public iBase {
  SCF_INTERFACE(iFoo,0,0,1);
};

struct iBar : virtual public iBase {
  SCF_INTERFACE(iFoo,0,0,1);
};

class Foo : public scfImplementation1<Foo,iFoo>
{
public:
  Foo() : scfImplementationType(this) {}
  Foo(int, int) : scfImplementationType(0,0) {}
};

class Bar : public scfImplementationExt1<Bar,Foo,iBar>
{
public:
  Bar() : scfImplementationType(0) {}
  Bar(int x, int y) : scfImplementationType(0,x,y) {}
};
*/

#endif

