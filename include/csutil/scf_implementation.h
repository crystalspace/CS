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
#include "csutil/reftrackeraccess.h"

// Needs to have iBase etc
#include "csutil/scf_interface.h"

#ifndef CS_TYPENAME
  #ifdef CS_REF_TRACKER
   #include <typeinfo>
   #define CS_TYPENAME(x)		    typeid(x).name()
  #else
   #define CS_TYPENAME(x)		    0
  #endif
#endif

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
    csRefTrackerAccess::TrackConstruction (object);
    if (scfParent) scfParent->IncRef ();
  }

  /**
   * Copy constructor.
   * Use of the default copy constructor is nor desired since the information
   * is insufficient to fully initialize scfImplementation; hence, an
   * explicit copy constructor must be created in the derived class that
   * initializes scfImplementation like in the normal constructor, i.e.
   * "scfImplementation (this)".
   */
  scfImplementation (const scfImplementation& /*other*/) : iBase()
  {
    CS_ASSERT_MSG ("To allow copying SCF classes, create a copy "
      "constructor in the derived class, and initialize scfImplementation "
      "like in the normal constructor, i.e. use "
      "\"scfImplementation (this)\".", false);
  }

  // Cleanup
  virtual ~scfImplementation()
  {
    csRefTrackerAccess::TrackDestruction (scfObject, scfRefCount);
    scfRemoveRefOwners ();
  }

  /**
   * Assign to another instance.
   * When assigning an SCF object to another, anything contained
   * in this class should not be copied, since it's all instance-
   * specific. Hence the assignment operator does nothing. */
  scfImplementation& operator= (const scfImplementation& /*other*/)
  {
    return *this;
  }

  virtual void DecRef ()
  {
    CS_ASSERT_MSG("Refcount decremented for destroyed object", 
      scfRefCount != 0);
    csRefTrackerAccess::TrackDecRef (scfObject, scfRefCount);
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
    csRefTrackerAccess::TrackIncRef (scfObject, scfRefCount);
    scfRefCount++;
  }

  virtual int GetRefCount ()
  {
    return scfRefCount;
  }

  virtual void AddRefOwner (void** ref_owner)
  {
    if (!this->scfWeakRefOwners)
      scfWeakRefOwners = new WeakRefOwnerArray (0);
    scfWeakRefOwners->InsertSorted (ref_owner);
  }

  virtual void RemoveRefOwner (void** ref_owner)
  {
    if (!scfWeakRefOwners)
      return;

    size_t index = scfWeakRefOwners->FindSortedKey (
      csArrayCmp<void**, void**>(ref_owner));

    if (index != csArrayItemNotFound)
      scfWeakRefOwners->DeleteIndex (index);
  }

protected:
  Class *scfObject;

  int scfRefCount;
  iBase *scfParent;
  typedef csArray<void**,
    csArrayElementHandler<void**>,
    csArrayMemoryAllocator<void**>,
    csArrayCapacityLinear<csArrayThresholdFixed<4> > > WeakRefOwnerArray;
  WeakRefOwnerArray* scfWeakRefOwners;

  void scfRemoveRefOwners ()
  {
    if (!scfWeakRefOwners)
      return;

    for (size_t i = 0; i < scfWeakRefOwners->Length (); i++)
    {
      void** p = (*scfWeakRefOwners)[i];
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
      return static_cast<iBase*> (scfObject);
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
