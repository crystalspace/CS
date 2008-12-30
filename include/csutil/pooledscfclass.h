/*
    Copyright (C) 2004 by Jorrit Tyberghein
              (C) 2004 by Frank Richter

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

#ifndef __CS_UTIL_POOLEDSCFCLASS_H__
#define __CS_UTIL_POOLEDSCFCLASS_H__

/**\file
 * Macros to create SCF classes that support pooling.
 */

/**
 * \addtogroup scf
 * @{ */

#include "csutil/scf.h"
#include "csutil/threading/atomicops.h"
#include "csutil/threading/mutex.h"

#include "csutil/custom_new_disable.h"

/**
 * Derive an SCF implementation from this class to have it pooled.
 * - The \a Super template argument is the scfImplementation...<> class
 *   you would normally use.
 * - \a Locked specifies whether the pool should be locked for allocations.
 *   Use it if allocations may be made from different threads.
 * \code
 * class csFoo : 
 *   public scfImplementationPooled<scfImplementation1<csFoo,
 *                                                     iFoo> >
 * {
 *   ...
 * };
 * \endcode
 * - A pooled class must needs to have the scfPooledImplementationType
 *   class in its initializer list:
 * \code
 *   csFoo () : scfPooledImplementationType (this) { ... }
 * \endcode
 * - To obtain an instance of \a Class, use new with the pool object as
 *   the placement argument:
 * \code
 *   csFoo::Pool fooPool;
 *   ...
 *   csRef<csFoo> foo; foo.AttachNew (new (fooPool) csFoo);
 * \endcode
 */
template<typename Super, typename Allocator = CS::Memory::AllocatorMalloc,
  bool Locked = false>
class scfImplementationPooled : public Super
{
  typedef typename Super::scfClassType scfClassType;
public:
  typedef scfImplementationPooled<Super, Allocator, Locked>
    scfPooledImplementationType;

  class Pool : public CS::Threading::OptionalMutex<Locked>
  {
    friend class scfImplementationPooled<Super, Allocator, Locked>;
    struct Entry
    {
      Entry* next;
    };
    CS::Memory::AllocatorPointerWrapper<Entry, Allocator> pool;
    size_t allocedEntries;
  public:
    Pool () : pool ((Entry*)0), allocedEntries (0) 
    {
    }
    Pool (const Allocator& alloc) : pool (alloc, 0), allocedEntries (0) 
    {
    }
    ~Pool () 
    {
      while (pool.p != 0)
      {
	Entry* n = pool.p->next;
	pool.Free (pool.p);
	pool.p = n;
      }
      CS_ASSERT_MSG ("not all SCF-pooled instances released",
	allocedEntries == 0);
    }
  };
protected:
  /// Pointer to the pool this instance is from.
  Pool* scfPool;
public:
  /// Allocate a new instance of a pooled SCF class.
  inline void* operator new (size_t n, Pool& p)
  { 
    typedef typename Pool::Entry PoolEntry;
    CS_ASSERT_MSG ("Alloc size mismatches class size expected for pooled "
      "allocation", n == sizeof (scfClassType));
    PoolEntry* newEntry;
    {
      CS::Threading::ScopedLock<Pool> lock (p);
      if (p.pool.p != 0)
      {
	newEntry = p.pool.p;
	p.pool.p = p.pool.p->next;
      }
      else
      {
	newEntry = static_cast<PoolEntry*> (p.pool.Alloc (n));
      }
      p.allocedEntries++;
    }
    scfClassType* newInst = reinterpret_cast<scfClassType*> (newEntry);
    /* A bit nasty: set scfPool member of the (still unconstructed!) 
     * instance... */
    static_cast<scfPooledImplementationType*> (newInst)->scfPool = &p;
    return newInst;
  }

  //@{
  /// Recycle a new instance of a pooled SCF class.
  inline void operator delete (void* instance, Pool& p) 
  {
    typedef typename Pool::Entry PoolEntry;
    PoolEntry* entry = static_cast<PoolEntry*> (instance);
    {
      CS::Threading::ScopedLock<Pool> lock (p);
      entry->next = p.pool.p;
      p.pool.p = entry;
      p.allocedEntries--;
    }
  }
  inline void operator delete (void* instance) 
  {
    scfClassType* object = static_cast<scfClassType*> (instance);
    Pool& p = *(static_cast<scfImplementationPooled*> (object)->scfPool);
    scfImplementationPooled::operator delete (object, p);
  }
  //@}

  /// DecRef() implementation that returns the object to the pool.
  void DecRef ()
  {
    csRefTrackerAccess::TrackDecRef (this->GetSCFObject(), this->scfRefCount);
    if (CS::Threading::AtomicOperations::Decrement (&this->scfRefCount) == 0)
    {
      delete this->GetSCFObject();
    }
  }

  //@{
  /**
   * Constructor. Call from the derived class with 'this' as first argument.
   */
  scfImplementationPooled (scfClassType* object) : 
    Super (object) {}
  template<typename A>
  scfImplementationPooled (scfClassType* object, A a) : 
    Super (object, a) {}
  template<typename A, typename B>
  scfImplementationPooled (scfClassType* object, A a, B b) : 
    Super (object, a, b) {}
  template<typename A, typename B, typename C>
  scfImplementationPooled (scfClassType* object, A a, B b, C c) : 
    Super (object, a, b, c) {}
  template<typename A, typename B, typename C, typename D>
  scfImplementationPooled (scfClassType* object, A a, B b, C c, D d) : 
    Super (object, a, b, c, d) {}
  template<typename A, typename B, typename C, typename D, typename E>
  scfImplementationPooled (scfClassType* object, A a, B b, C c, D d, E e) : 
    Super (object, a, b, c, d, e) {}
  //@}
};

/** @} */

#include "csutil/custom_new_enable.h"

#endif // __CS_UTIL_POOLEDSCFCLASS_H__
