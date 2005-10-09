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
 * support.
 */

/**
 * \addtogroup scf
 * @{ */
 
/**\page SCFExamples Some examples
 * \code
 * struct iFoo : virtual public iBase {
 *   SCF_INTERFACE(iFoo,0,0,1);
 * };
 * 
 * struct iBar : virtual public iBase {
 *   SCF_INTERFACE(iFoo,0,0,1);
 * };
 * 
 * class Foo : public scfImplementation1<Foo,iFoo>
 * {
 * public:
 *   Foo() : scfImplementationType(this) {}
 *   Foo(int, int) : scfImplementationType(0,0) {}
 * };
 * 
 * class Bar : public scfImplementationExt1<Bar,Foo,iBar>
 * {
 * public:
 *   Bar() : scfImplementationType(0) {}
 *   Bar(int x, int y) : scfImplementationType(0,x,y) {}
 * };
 * \endcode
*/


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
    typedef If InterfaceType;
    CS_FORCEINLINE static scfInterfaceVersion GetVersion()
    { return If::InterfaceTraits::GetVersion(); }
    CS_FORCEINLINE static char const * GetName() 
    { return If::InterfaceTraits::GetName(); }
  };
};

/**
 * Baseclass for the SCF implementation templates.
 * Provides common methods such as reference counting and handling of
 * weak references.
 */
template<class Class>
class CS_CRYSTALSPACE_EXPORT scfImplementation : public virtual iBase
{
public:
  /**
  * Constructor. Initialize the SCF-implementation in class named Class.
  * Will be called from scfImplementation(Ext)N constructor
  */
  scfImplementation (Class *object, iBase *parent = 0) :
      scfObject (object), scfRefCount (1), scfParent (parent), 
        scfWeakRefOwners (0)
  {
    if (scfParent) scfParent->IncRef ();
  }

  // Cleanup
  virtual ~scfImplementation()
  {
    scfRemoveRefOwners ();
  }

  virtual void DecRef ()
  {
    CS_ASSERT_MSG("Refcount decremented for destroyed object", 
      scfRefCount != 0);
    scfRefCount--;
    if (scfRefCount == 0)
    {
      scfRemoveRefOwners ();
      if (scfParent) scfParent->DecRef();
      delete scfObject;
    }
  }

  virtual void IncRef ()
  {
    CS_ASSERT_MSG("Refcount incremented from inside dtor", 
      scfRefCount != 0);
    scfRefCount++;
  }

  virtual int GetRefCount ()
  {
    return scfRefCount;
  }

  virtual void AddRefOwner (iBase** ref_owner)
  {
    if (!this->scfWeakRefOwners)
      scfWeakRefOwners = new csArray<iBase**> (0, 4);
    scfWeakRefOwners->InsertSorted (ref_owner);
  }

  virtual void RemoveRefOwner (iBase** ref_owner)
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
      *p = 0;
    }
    delete scfWeakRefOwners;
    scfWeakRefOwners = 0;
  }

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

    return 0;
  }
};


/* Here the magic happens: generate scfImplementationN and 
 * scfImplementationExtN classed */
#define SCF_IN_IMPLEMENTATION_H 1
// Generation is in separate file mostly for documentation generation purposes.
#include "scf_implgen.h"
#undef SCF_IN_IMPLEMENTATION_H

/** @} */

#endif
