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

#include "scf.h"

// hack: work around problems caused by #defining 'new'
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# undef new
#endif
#include <new>

#define SCF_DECLARE_IBASE_POOLED_DECL(Class, parentClass)\
  Class Pool                                            \
  {                                                     \
    struct PoolEntry;                                   \
    PoolEntry* pool;                                    \
    PoolEntry* allocedEntries;                          \
  public:                                               \
    Pool ();                                            \
    ~Pool ();                                           \
    parentClass* Alloc ();                              \
    void Recycle (parentClass* instance);               \
  };                                                    \
  friend class Pool;                                    \
  Pool* scfPool;                                        \
  SCF_DECLARE_IBASE

/**
 * Embed this macro instead of #SCF_DECLARE_IBASE into an SCF class that is to
 * be pooled.
 * \remarks To obtain an instance of \a Class, create an instance of
 *  \a Class::Pool and use it's Alloc() method, e.g.:
 *  \code
 *  csFoo::Pool fooPool;
 *  ...
 *  csRef<csFoo> foo; foo.AttachNew (fooPool.Alloc ());
 *  \endcode
 * \remarks A pooled class \a Class must implement a constructor with the
 *  following prototype:
 *  \code
 *   Class (Pool* pool);
 *  \endcode
 * \remarks Inside the constructor, #SCF_CONSTRUCT_IBASE_POOLED needs to be
 *  called.
 * \remarks If you need to set instance-specific data, you need to provide a
 *  custom method for that.
 */
#define SCF_DECLARE_IBASE_POOLED(parentClass) \
  SCF_DECLARE_IBASE_POOLED_DECL(class, parentClass)

/**
 * Same as SCF_DECLARE_IBASE_POOLED, use for external classes.
 */
#define SCF_DECLARE_IBASE_POOLED_EXTERN(Extern, parentClass) \
  SCF_DECLARE_IBASE_POOLED_DECL(class Extern, parentClass)

/**
 * Has to be invoked inside the class constructor instead of
 * #SCF_CONSTRUCT_IBASE.
 */
#define SCF_CONSTRUCT_IBASE_POOLED(Pool)                \
  SCF_CONSTRUCT_IBASE(0);                               \
  scfPool = (Pool)

#define SCF_IMPLEMENT_IBASE_POOL_HELPERS(parentClass)   \
  struct parentClass::Pool::PoolEntry                   \
  {                                                     \
    parentClass instance;                               \
    PoolEntry* next;                                    \
  };

/**
 * Implement the constructor for the pool manager of \a Class.
 */
#define SCF_IMPLEMENT_IBASE_POOL_CTOR(Class)            \
Class::Pool::Pool ()                                    \
{                                                       \
  pool = 0;                                             \
  allocedEntries = 0;                                   \
}

/**
 * Implement the destructor for the pool manager of \a Class.
 */
#define SCF_IMPLEMENT_IBASE_POOL_DTOR(Class)            \
Class::Pool::~Pool ()                                   \
{                                                       \
  while (pool != 0)                                     \
  {                                                     \
    PoolEntry* n = pool->next;                          \
    free (pool);                                        \
    pool = n;                                           \
  }                                                     \
  CS_ASSERT_MSG ("not all SCF-pooled instances released",\
    allocedEntries == 0);                               \
}

/**
 * Implement \a Alloc() for the pool manager of \a Class.
 */
#define SCF_IMPLEMENT_IBASE_POOL_ALLOC(Class)           \
Class* Class::Pool::Alloc ()                            \
{                                                       \
  PoolEntry* newEntry;                                  \
  if (pool != 0)                                        \
  {                                                     \
    newEntry = pool;                                    \
    pool = pool->next;                                  \
  }                                                     \
  else                                                  \
  {                                                     \
    newEntry = (PoolEntry*)malloc (sizeof (PoolEntry)); \
  }                                                     \
  Class* newInst = &newEntry->instance;                 \
  new (newInst) Class (this);                           \
  newEntry->next = allocedEntries;                      \
  allocedEntries = newEntry;                            \
  return newInst;                                       \
}

/**
 * Implement \a Recycle() for the pool manager of \a Class.
 */
#define SCF_IMPLEMENT_IBASE_POOL_RECYCLE(Class)         \
void Class::Pool::Recycle (Class* instance)             \
{                                                       \
  PoolEntry* prev = 0;                                  \
  PoolEntry* entry = allocedEntries;                    \
  while (&entry->instance != instance)                  \
  {                                                     \
    prev = entry;                                       \
    entry = entry->next;                                \
  }                                                     \
  if (prev != 0)                                        \
    prev->next = entry->next;                           \
  else                                                  \
    allocedEntries = entry->next;                       \
  instance->~Class();                                   \
  entry->next = pool;                                   \
  pool = entry;                                         \
}

/**
 * Implement pool manager for \a Class.
 */
#define SCF_IMPLEMENT_IBASE_POOL(Class)                 \
  SCF_IMPLEMENT_IBASE_POOL_CTOR(Class)                  \
  SCF_IMPLEMENT_IBASE_POOL_DTOR(Class)                  \
  SCF_IMPLEMENT_IBASE_POOL_ALLOC(Class)                 \
  SCF_IMPLEMENT_IBASE_POOL_RECYCLE(Class)

/**
 * Implement IncRef() for a pooled class.
 */
#define SCF_IMPLEMENT_IBASE_INCREF_POOLED(Class)        \
void Class::IncRef ()                                   \
{                                                       \
  scfRefCount++;                                        \
}

/**
 * Implement DecRef() for a pooled class.
 */
#define SCF_IMPLEMENT_IBASE_DECREF_POOLED(Class)        \
void Class::DecRef ()                                   \
{                                                       \
  if (scfRefCount == 1)                                 \
  {                                                     \
    scfPool->Recycle (this);                            \
    return;                                             \
  }                                                     \
  scfRefCount--;                                        \
}

/**
 * Use this in the source module instead of #SCF_IMPLEMENT_IBASE.
 */
#define SCF_IMPLEMENT_IBASE_POOLED(Class)               \
  SCF_IMPLEMENT_IBASE_POOL_HELPERS(Class)               \
  SCF_IMPLEMENT_IBASE_POOL(Class)                       \
  SCF_IMPLEMENT_IBASE_INCREF_POOLED(Class)              \
  SCF_IMPLEMENT_IBASE_DECREF_POOLED(Class)              \
  SCF_IMPLEMENT_IBASE_GETREFCOUNT(Class)                \
  SCF_IMPLEMENT_IBASE_REFOWNER(Class)                   \
  SCF_IMPLEMENT_IBASE_REMOVE_REF_OWNERS(Class)          \
  SCF_IMPLEMENT_IBASE_QUERY(Class)

/** @} */

#endif // __CS_UTIL_POOLEDSCFCLASS_H__
