/*
    Crystal Space Shared Class Facility (SCF)
    Copyright (C) 2005 by Marten Svanfeldt and Michael D. Adams

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

#ifndef __CSSCF_NEW_H__
#define __CSSCF_NEW_H__

/**\file
 * Crystal Space Shared Class Facility (SCF)
 */

/**
 * \addtogroup scf
 * @{ */

// scfInterfaceID, scfInterfaceVersion, and SCF_INTERFACE are declared in scf.h

template<class Interface>
class scfInterfaceTraits
{
public:
  static scfInterfaceVersion GetVersion()
  {
    return Interface::InterfaceTraits::GetVersion();
  }

  static scfInterfaceID GetID()
  {
    scfInterfaceID& ID = GetMyID();
    if (ID == (scfInterfaceID)(-1))
    {
      ID = iSCF::SCF->GetInterfaceID(GetName());
      csStaticVarCleanup (CleanupID);
    }
    return ID;
  }

  static char const* GetName()
  {
    return Interface::InterfaceTraits::GetName();
  }
private:
  // This idiom is a Meyers singleton
  static scfInterfaceID& GetMyID()
  {
    static scfInterfaceID ID = (scfInterfaceID)-1;
    return ID;
  }
  static void CleanupID()
  {
    GetMyID() = (scfInterfaceID)-1;
  }
};

#ifdef SCF_TODO
// For some reason this breaks at runtime.
#undef SCF_VERSION
#define SCF_VERSION(Name,Major,Minor,Micro)                \
struct Name;                                               \
CS_SPECIALIZE_TEMPLATE                                     \
struct scfInterface<Name> : public scfInterfaceTraits<Name>\
{                                                          \
  static scfInterfaceVersion GetVersion()                  \
  { return SCF_CONSTRUCT_VERSION(Major, Minor, Micro); }   \
  static char const* GetName()                             \
  { return #Name; }                                        \
}
#endif

template<class Class>
class scfImplementation : public virtual iBase
{
public:
  scfImplementation (Class *object, iBase *parent) :
    scfObject(object), scfRefCount (1),
    scfParent (parent), scfWeakRefOwners (NULL)
  {
    if (scfParent) scfParent->IncRef();
  }
  
  virtual ~scfImplementation()
  {
    scfRemoveRefOwners();
  }

  void DecRef()
  {
    scfRefCount--;
    if (scfRefCount == 0)
    {
      if (scfParent) scfParent->DecRef();
      delete scfObject;
    }
  }

  void IncRef()
  {
    scfRefCount++;
  }

  int GetRefCount()
  {
    return scfRefCount;
  }

  void AddRefOwner(iBase** ref_owner)
  {
    if (!this->scfWeakRefOwners)
      scfWeakRefOwners = new csArray<iBase**> (0, 4);
    scfWeakRefOwners->InsertSorted (ref_owner);
  }

  void RemoveRefOwner(iBase** ref_owner)
  {
    if (!scfWeakRefOwners)
      return;
    
    size_t index =
      scfWeakRefOwners->FindSortedKey(
        csArrayCmp<iBase**, iBase**>(ref_owner));
                     
    if (index != csArrayItemNotFound)
      scfWeakRefOwners->DeleteIndex (index);
  }

protected:
  Class *scfObject;

  int scfRefCount;
  iBase *scfParent;
  csArray<iBase**>* scfWeakRefOwners;
    
  void scfRemoveRefOwners()
  {
    if (!scfWeakRefOwners)
      return;
    
    for (size_t i = 0; i < scfWeakRefOwners->Length(); i++)
    {
      iBase** p = (*scfWeakRefOwners)[i];
      *p = NULL;
    }
    delete scfWeakRefOwners;
    scfWeakRefOwners = NULL;
  }
  
  void *QueryInterface(
    scfInterfaceID iInterfaceID,
    scfInterfaceVersion iVersion)
  {
    if (iInterfaceID == scfInterfaceTraits<iBase>::GetID () &&
        scfCompatibleVersion(iVersion, scfInterfaceTraits<iBase>::GetVersion()))
    {
      scfObject->IncRef();
      return CS_STATIC_CAST(iBase*,scfObject);
    }
    
    if (scfParent) 
      return scfParent->QueryInterface(iInterfaceID, iVersion);
    
    return NULL;
  }

  template<class I>
  inline void*
  GetInterface(scfInterfaceID iInterfaceID, scfInterfaceVersion iVersion)
  {
    if (iInterfaceID == scfInterfaceTraits<I>::GetID() &&
        scfCompatibleVersion(iVersion, scfInterfaceTraits<I>::GetVersion()))
    {
      scfObject->IncRef();
      return CS_STATIC_CAST(I*, scfObject);
    }
    else
    {
      return NULL;
    }
  }
};

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

#undef SCF_IMPL_EXT

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
  Foo() : scfImplementationType(0,0) {}
  Foo(int, int) : scfImplementationType(0,0) {}
};

class Bar : public scfImplementationExt1<Bar,Foo,iBar>
{
public:
  Bar() : scfImplementationType(0) {}
  Bar(int x, int y) : scfImplementationType(0,x,y) {}
};
*/

/** @} */

#endif // __CSSCF_NEW_H__

