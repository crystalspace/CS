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
#include "csutil/customallocated.h"
#include "csutil/ref.h"
#include "csutil/reftrackeraccess.h"
#include "csutil/threading/atomicops.h"
#include "csutil/threading/mutex.h"
#include "csutil/weakreferenced.h"

// Needs to have iBase etc
#include "csutil/scf_interface.h"

// Control if we want to use preprocessed file or run generation each time
#define SCF_IMPLGEN_PREPROCESSED
// Track some simple SCF-related stats
//#define SCF_TRACK_STATS

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
    CS_FORCEINLINE_TEMPLATEMETHOD 
    static scfInterfaceVersion GetVersion()
    { return If::InterfaceTraits::GetVersion(); }
    CS_FORCEINLINE_TEMPLATEMETHOD static char const * GetName() 
    { return If::InterfaceTraits::GetName(); }
  };
};

/// Various helpers for scfImplementation
class CS_CRYSTALSPACE_EXPORT scfImplementationHelper
{
protected:
  enum
  {
    scfstatTotal,
    scfstatParented,
    scfstatWeakreffed,
    scfstatMetadata,
    scfstatIncRef,
    scfstatDecRef,

    scfstatsNum
  };
  static uint64 stats[scfstatsNum];
  static CS::Threading::Mutex statsLock;

  CS_FORCEINLINE void BumpStat (int stat)
  {
#ifdef SCF_TRACK_STATS
    CS::Threading::ScopedLock<CS::Threading::Mutex> l (statsLock);
    stats[stat]++;
#endif
  }

  struct ScfImplAuxData : public CS::Memory::CustomAllocated,
			  public CS::Utility::AtomicRefCount,
			  public CS::Utility::Implementation::WeakReferenced
  {
    CS::Threading::Mutex lock;
    iBase *scfParent;
    scfInterfaceMetadataList* metadataList;

    ScfImplAuxData () : scfParent (0), metadataList (0) {}
    
    bool HasWeakRefOwners() const { return weakref_owners != 0; }
  };
  ScfImplAuxData* scfAuxData;

  void EnsureAuxData();
  void FreeAuxData();

  //-- Metadata handling
  void AllocMetadata (size_t numEntries);
  void CleanupMetadata ();

  iBase* GetSCFParent();

  // Some virtual helpers for the metadata registry
  virtual size_t GetInterfaceMetadataCount () const;

  scfImplementationHelper() : scfAuxData (0) {}
  virtual ~scfImplementationHelper();
};

/**
 * Baseclass for the SCF implementation templates.
 * Provides common methods such as reference counting and handling of
 * weak references.
 */
template<class Class>
class scfImplementation : public virtual iBase,
  public scfImplementationHelper,
  public CS::Memory::CustomAllocated
{
  struct WeakRefOwnersEmptyingHelper
  {
    CS::Utility::Implementation::WeakReferenced& weakReffed;
    bool doEmpty;
    
    WeakRefOwnersEmptyingHelper (CS::Utility::Implementation::WeakReferenced& weakReffed)
      : weakReffed (weakReffed), doEmpty (false) {}
    ~WeakRefOwnersEmptyingHelper ()
    {
      if (doEmpty) weakReffed.DeleteAllOwners();
    }
  };
public:
  /**
   * Constructor. Initialize the SCF-implementation in class named Class.
   * Will be called from scfImplementation(Ext)N constructor
   */
  scfImplementation (Class *object, iBase *parent = 0) :
      scfRefCount (1)
  {
    BumpStat (scfstatTotal);
    if (parent) BumpStat (scfstatParented);
    csRefTrackerAccess::TrackConstruction (object);
    if (parent) 
    {
      EnsureAuxData();
      scfAuxData->scfParent = parent;
      parent->IncRef ();
    }
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
    csRefTrackerAccess::TrackDestruction (GetSCFObject(), scfRefCount);
    if (HasAuxData())
    {
      scfAuxData->ClearRefOwners();
      CleanupMetadata ();
      iBase *scfParent = scfAuxData->scfParent;
      if (scfParent) scfParent->DecRef();
    }
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
    csRefTrackerAccess::TrackDecRef (GetSCFObject(), scfRefCount);
    /* Keep a reference to the aux data so we can keep weak ref owner locks
     * past the destructor call */
    csRef<ScfImplAuxData> keepAuxData (scfAuxData);
    int32 refcount;
    if (keepAuxData)
    {
      CS::Threading::ScopedLock<CS::Threading::Mutex> lockAuxData (keepAuxData->lock);
      /* The weak refs may need to be emptied after clearing and after the weak ref
       * locks have been released, but before the aux data lock is released */
      WeakRefOwnersEmptyingHelper emptyWeakRefs (*keepAuxData);
      CS::Utility::Implementation::WeakReferenced::ScopedWeakRefOwnersLock lockWeakRefs (*keepAuxData);
      if ((refcount = CS::Threading::AtomicOperations::Decrement (&scfRefCount)) == 0)
      {
	/* Remove ref owners now, to avoid deadlock if the dtor triggers the
	 * destruction of a ref owner */
	scfAuxData->ClearRefOwners();
	emptyWeakRefs.doEmpty = true;
      }
    }
    else
    {
      refcount = CS::Threading::AtomicOperations::Decrement (&scfRefCount);
    }
    if (refcount == 0)
      delete GetSCFObject();
    BumpStat (scfstatDecRef);
  }

  virtual void IncRef ()
  {
    CS_ASSERT_MSG("Refcount incremented from inside dtor", 
      scfRefCount != 0);
    csRefTrackerAccess::TrackIncRef (GetSCFObject(), scfRefCount);
    CS::Threading::AtomicOperations::Increment (&scfRefCount);
    BumpStat (scfstatIncRef);
  }

  virtual int GetRefCount ()
  {
    return CS::Threading::AtomicOperations::Read (&scfRefCount);
  }

  virtual void AddRefOwner (void** ref_owner, CS::Threading::Mutex* mutex)
  {
    EnsureAuxData();
    CS::Threading::ScopedLock<CS::Threading::Mutex> l (scfAuxData->lock);
    if (GetRefCount() <= 0)
    {
      /* In the case we are already in destruction, don't allow any new weak
       * references, just destroy it at once
       */
      *ref_owner = 0;
      return;
    }
    scfAuxData->AddRefOwner (ref_owner, mutex);
    if (!scfAuxData->HasWeakRefOwners())
      BumpStat (scfstatWeakreffed);
  }

  virtual void RemoveRefOwner (void** ref_owner)
  {
    if (!HasAuxData()) return;

    CS::Threading::ScopedLock<CS::Threading::Mutex> l (scfAuxData->lock);
    scfAuxData->RemoveRefOwner (ref_owner);
  }

  virtual scfInterfaceMetadataList* GetInterfaceMetadata ()
  {
    EnsureAuxData();
    CS::Threading::ScopedLock<CS::Threading::Mutex> l (scfAuxData->lock);
    if (!scfAuxData->metadataList)
    {
      BumpStat (scfstatMetadata);
      // Need to set it up, do so
      AllocMetadata (GetInterfaceMetadataCount ());
      FillInterfaceMetadata (0);
    }

    return scfAuxData->metadataList;
  }

protected:
  Class* GetSCFObject() { return static_cast<Class*> (this); }
  const Class* GetSCFObject() const { return static_cast<const Class*> (this); }

  int32 scfRefCount;

  void *QueryInterface (scfInterfaceID iInterfaceID,
                        scfInterfaceVersion iVersion)
  {
    // Default, just check iBase.. all objects have iBase    
    if (iInterfaceID == scfInterfaceTraits<iBase>::GetID () &&
      scfCompatibleVersion (iVersion, scfInterfaceTraits<iBase>::GetVersion ()))
    {
      GetSCFObject()->IncRef ();
      return static_cast<iBase*> (GetSCFObject());
    }

    // For embedded interfaces
    if (HasAuxData() && scfAuxData->scfParent)
      return scfAuxData->scfParent->QueryInterface (iInterfaceID, iVersion);

    return 0;
  }


  // Fill in interface metadata in the metadata table, starting at offset N
  virtual void FillInterfaceMetadata (size_t n)
  {
    scfInterfaceMetadataList* metadataList = scfAuxData->metadataList;
    if (!metadataList)
      return;

    FillInterfaceMetadataIf<iBase> (metadataList->metadata, n);
  }

  template<typename IF>
  CS_FORCEINLINE_TEMPLATEMETHOD static void FillInterfaceMetadataIf (
    scfInterfaceMetadata* metadataArray, size_t pos)
  {
    metadataArray[pos].interfaceName = scfInterfaceTraits<IF>::GetName ();
    metadataArray[pos].interfaceID = scfInterfaceTraits<IF>::GetID ();
    metadataArray[pos].interfaceVersion = scfInterfaceTraits<IF>::GetVersion ();
  }

  /* Note: can't put this into scfImplementationHelper, breaks on MingW shared builds
   * (possibly a clash between the method being forced inlined and dllexported) */
  CS_FORCEINLINE bool HasAuxData()
  {
    // Double-cast to cheat strict-aliasing rules
    return CS::Threading::AtomicOperations::Read ((void**)(void*)&scfAuxData) != 0; 
  }
};


/* Here the magic happens: generate scfImplementationN and 
* scfImplementationExtN classed */
#define SCF_IN_IMPLEMENTATION_H 1
#if defined(DOXYGEN_RUN) || !defined(SCF_IMPLGEN_PREPROCESSED)
  // Generation is in separate file mostly for documentation generation purposes.
  #include "scf_implgen.h"
#else
  #include "scf_implgen_p.h"
#endif

#undef SCF_IN_IMPLEMENTATION_H
#undef SCF_IMPLGEN_PREPROCESSED

/** @} */

#endif
