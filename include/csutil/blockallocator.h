/*
  Crystal Space Generic Object Block Allocator
  Copyright (C) 2005 by Eric Sunshine <sunshine@sunshineco.com>
	    (C) 2006 by Frank Richter

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
#ifndef __CSUTIL_BLOCKALLOCATOR_H__
#define __CSUTIL_BLOCKALLOCATOR_H__

/**\file
 * Generic Memory Block Allocator
 */

#include "csutil/fixedsizeallocator.h"

#include "csutil/custom_new_disable.h"

/**\addtogroup util_memory
 * @{ */

/**
 * Block allocator disposal mixin that just destructs an instance.
 */
template<typename T>
class csBlockAllocatorDisposeDelete
{
public:
  /**
   * Construct with a given block allocator.
   */
  template<typename BA>
  csBlockAllocatorDisposeDelete (const BA&, bool legit)
  { (void)legit; }
  /// Destructs the object.
  void Dispose (void* p) 
  {
    ((T*)p)->~T();
  }
};

/**
 * Block allocator disposal mixin that destructs, unless upon final cleanup,
 * where a warning is emitted stating that objects leaked.
 */
template<typename T>
class csBlockAllocatorDisposeLeaky
{
  bool doWarn;
#ifdef CS_DEBUG
  const char* parentClass;
  const void* parent;
  size_t count;
#endif
public:
#ifdef CS_DEBUG
  /**
   * Construct with a given block allocator.
   * \a legit is a flag that indicates whether a disposal is done on user 
   * request(true) or due an automatic cleanup(false). In the latter case,
   * objects are not destructed and a warning is emitted.
   * \a ba is used for some informational output.
   */
  template<typename BA>
  csBlockAllocatorDisposeLeaky (const BA& ba, bool legit) :
    doWarn (!legit), parentClass (typeid(BA).name()), parent (&ba),
    count (0)
  { 
  }
#else
  template<typename BA>
  csBlockAllocatorDisposeLeaky (const BA&, bool legit) : doWarn (!legit)
  { }
#endif
  ~csBlockAllocatorDisposeLeaky()
  {
#ifdef CS_DEBUG
    if ((count > 0) && doWarn)
    {
      csPrintfErr("%s %p leaked %zu objects.\n", parentClass, (void*)this, 
        count);
    }
#endif
  }
  /// Destructs the object, unless in auto-cleanup.
  void Dispose (void* p) 
  {
    if (!doWarn) ((T*)p)->~T();
#ifdef CS_DEBUG
    count++;
#endif
  }
};

/**
 * This class implements a memory allocator which can efficiently allocate
 * objects that all have the same size. It has no memory overhead per
 * allocation (unless the objects are smaller than sizeof(void*) bytes) and is
 * extremely fast, both for Alloc() and Free(). The only restriction is that
 * any specific allocator can be used for just one type of object (the type for
 * which the template is instantiated).
 * \remarks The objects are properly constructed and destructed.
 * <p>
 * \remarks Assumes that the class \c T with which the template is instantiated
 *   has a default (zero-argument) constructor. Alloc() uses this constructor
 *   to initialize each vended object.
 * <p>
 * \remarks Defining the macro CS_BLOCKALLOC_DEBUG will cause freed objects to
 *   be overwritten with '0xfb' bytes. This can be useful to track use of
 *   already freed objects, as they can be more easily recognized (as some
 *   members will be likely bogus.)
 * \sa csArray
 * \sa csMemoryPool
 */
template <class T,
  class Allocator = CS::Memory::AllocatorMalloc, 
  class ObjectDispose = csBlockAllocatorDisposeDelete<T> >
class csBlockAllocator : public csFixedSizeAllocator<sizeof (T), Allocator>
{
public:
  typedef csBlockAllocator<T, Allocator> ThisType;
  typedef T ValueType;
  typedef Allocator AllocatorType;

protected:
  typedef csFixedSizeAllocator<sizeof (T), Allocator> superclass;
private:
  void* Alloc (size_t /*n*/) { return 0; }                       // Illegal
  void* Alloc (void* /*p*/, size_t /*newSize*/) { return 0; }   // Illegal
  void SetMemTrackerInfo (const char* /*info*/) { }             // Illegal
public:
  /**
   * Construct a new block allocator.
   * \param nelem Number of elements to store in each allocation unit.
   * \param warn_unfreed If true, in debug mode warn about objects not
   *   explicitly released via Free().
   * \remarks Bigger values for \c nelem will improve allocation performance,
   *   but at the cost of having some potential waste if you do not add
   *   \c nelem elements to each pool.  For instance, if \c nelem is 50 but you
   *   only add 3 elements to the pool, then the space for the remaining 47
   *   elements, though allocated, will remain unused (until you add more
   *   elements).
   *
   * \remarks If use use csBlockAllocator as a convenient and lightweight
   *   garbage collection facility (for which it is well-suited), and expect it
   *   to dispose of allocated objects when the pool itself is destroyed, then
   *   set \c warn_unfreed to false. On the other hand, if you use
   *   csBlockAllocator only as a fast allocator but intend to manage each
   *   object's life time manually, then you may want to set \c warn_unfreed to
   *   true in order to receive diagnostics about objects which you have
   *   forgotten to release explicitly via manual invocation of Free().
   */
  csBlockAllocator(size_t nelem = 32) : superclass (nelem)
  {
  }

  /**
   * Destroy all allocated objects and release memory.
   */
  ~csBlockAllocator()
  {
    ObjectDispose dispose (*this, false);
    DisposeAll (dispose);
  }

  /**
   * Destroy all objects allocated by the pool.
   * \remarks All pointers returned by Alloc() are invalidated. It is safe to
   *   perform new allocations from the pool after invoking Empty().
   */
  void Empty()
  {
    ObjectDispose dispose (*this, true);
    DisposeAll (dispose);
  }

  /**
   * Allocate a new object. 
   * The default (no-argument) constructor of \a T is invoked. 
   */
  T* Alloc ()
  {
    return new (superclass::Alloc()) T;
  }


  /// @@@ Document me!
  T* AllocUninit() 
  {
    return superclass::Alloc();
  }

  /// @@@ Document me!
  void FreeUninit (void* p)
  {
    superclass::Free(p);
  }

  /**
   * Deallocate an object. It is safe to provide a null pointer.
   * \param p Pointer to deallocate.
   * \param invokeDtor If true, the destructor of \a T is invoked. If false, 
   *   the memory is only freed. This is mostly useful for custom 
   *   \c delete implementations.
   */
  void Free (T* p)
  {
    ObjectDispose dispose (*this, true);
    superclass::Free (dispose, p);
  }
  /**
   * Try to delete an object. Usage is the same as Free(), the difference
   * being that \c false is returned if the deallocation failed (the reason
   * is most likely that the memory was not allocated by the allocator).
   */
  bool TryFree (T* p)
  {
    ObjectDispose dispose (*this, true);
    return superclass::TryFree (dispose, p);
  }
};

/** @} */

#include "csutil/custom_new_enable.h"

#endif // __CSUTIL_BLOCKALLOCATOR_H__
