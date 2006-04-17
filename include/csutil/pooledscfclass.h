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

// hack: work around problems caused by #defining 'new'
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# undef new
#endif
#include <new>

/**
 * Derive an SCF implementation from this class to have it pooled.
 * - The \a Super template argument is the scfImplementation...<> class
 *   you would normally use.
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
template<typename Super>
class scfImplementationPooled : public Super
{
  typedef typename Super::scfClassType scfClassType;
public:
  typedef scfImplementationPooled<Super> scfPooledImplementationType;

  class Pool
  {
    friend class scfImplementationPooled<Super>;
    struct Entry
    {
      Entry* next;
    };
    Entry* pool;
    size_t allocedEntries;
  #ifdef CS_MEMORY_TRACKER
    csMemTrackerInfo* mti;
  #endif
  public:
    Pool () : pool (0), allocedEntries (0) 
    {
  #ifdef CS_MEMORY_TRACKER
      mti = 0;
  #endif
    }
    ~Pool () 
    {
      while (pool != 0)
      {
	Entry* n = pool->next;
	free (pool);
      #ifdef CS_MEMORY_TRACKER
        mtiUpdateAmount (mti, -1, 
          -int(sizeof (scfClassType) + sizeof (Entry)));
      #endif
	pool = n;
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
    if (p.pool != 0)
    {
      newEntry = p.pool;
      p.pool = p.pool->next;
    }
    else
    {
      newEntry = (PoolEntry*)malloc (sizeof (PoolEntry) + n);
    #ifdef CS_MEMORY_TRACKER
      if (p.mti == 0)
      {
        p.mti = mtiRegisterAlloc (sizeof (PoolEntry) + n, 
          (void*)typeid (Pool).name());
      }
      else
        mtiUpdateAmount (p.mti, 1, int (sizeof (PoolEntry) + n));
    #endif
    }
    p.allocedEntries++;
    scfClassType* newInst = 
      (scfClassType*)((uint8*)newEntry + sizeof (PoolEntry));
    /* A bit nasty: set scfPool member of the (still unconstructed!) 
     * instance... */
    ((scfPooledImplementationType*)newInst)->scfPool = &p;
    return newInst;
  }

  //@{
  /// Recycle a new instance of a pooled SCF class.
  inline void operator delete (void* instance, Pool& p) 
  {
    typedef typename Pool::Entry PoolEntry;
    // Go from instance to pool entry pointer
    PoolEntry* entry = (PoolEntry*)((uint8*)instance - sizeof (PoolEntry));
    entry->next = p.pool;
    p.pool = entry;
    p.allocedEntries--;
  }
  inline void operator delete (void* instance) 
  {
    scfClassType* object = (scfClassType*)instance;
    Pool& p = *(((scfImplementationPooled*)object)->scfPool);
    scfImplementationPooled::operator delete (object, p);
  }
  //@}

  /// DecRef() implementation that returns the object to the pool.
  void DecRef ()
  {
    if (this->scfRefCount == 1)
    {
      //this->operator delete (this->scfObject, *scfPool);
      delete this->scfObject;
      return;
    }
    this->scfRefCount--;
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

#endif // __CS_UTIL_POOLEDSCFCLASS_H__
