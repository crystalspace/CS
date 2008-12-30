/*
  Copyright (C) 2007 by Frank Richter

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_CSUTIL_CUSTOMALLOCATED_H__
#define __CS_CSUTIL_CUSTOMALLOCATED_H__

#include "csutil/custom_new_disable.h"

/**\file
 * Base class to allocate subclasses with cs_malloc().
 */

namespace CS
{
  namespace Memory
  {
    /**
     * Class that overrides operator new/operator delete/etc. 
     * with implementations using cs_malloc()/cs_free().
     * \remarks To outfit a class that also derives from another class with
     *   custom allocation don't use multiple inheritance, use 
     *   CustomAllocatedDerived<> instead.
     *
     *   The reason is that the CustomAllocated instance contained in the 
     *   derived class may take up some memory (in order to have a distinct 
     *   address in memory), memory which is otherwise unused and wasted.
     *   CustomAllocatedDerived<> works around that as it is a base class
     *   and can thus be empty; derivation is supported through templating.
     *   (For details see http://www.cantrip.org/emptyopt.html .)
     */
    class CustomAllocated
    {
    public:
      // Potentially throwing versions
    #ifndef CS_NO_EXCEPTIONS
      CS_FORCEINLINE void* operator new (size_t s) throw (std::bad_alloc)
      { 
	void* p = cs_malloc (s);
	if (!p) throw std::bad_alloc();
	return p;
      }
      CS_FORCEINLINE void* operator new[] (size_t s) throw (std::bad_alloc)
      { 
	void* p = cs_malloc (s);
	if (!p) throw std::bad_alloc();
	return p;
      }
    #else
      CS_FORCEINLINE void* operator new (size_t s) throw ()
      { return cs_malloc (s); }
      CS_FORCEINLINE void* operator new[] (size_t s) throw ()
      { return cs_malloc (s); }
    #endif
      
      CS_FORCEINLINE void operator delete (void* p) throw()
      { cs_free (p); }
      CS_FORCEINLINE void operator delete[] (void* p) throw()
      { cs_free (p); }
      
      // Nothrow versions
      CS_FORCEINLINE void* operator new (size_t s, const std::nothrow_t&) throw()
      { return cs_malloc (s); }
      CS_FORCEINLINE void* operator new[] (size_t s, const std::nothrow_t&) throw()
      { return cs_malloc (s); }
      CS_FORCEINLINE void operator delete (void* p, const std::nothrow_t&) throw()
      { cs_free (p); }
      CS_FORCEINLINE void operator delete[] (void* p, const std::nothrow_t&) throw()
      { cs_free (p); }
      
      // Placement versions
      CS_FORCEINLINE void* operator new(size_t /*s*/, void* p) throw() { return p; }
      CS_FORCEINLINE void* operator new[](size_t /*s*/, void* p) throw() { return p; }

      CS_FORCEINLINE void operator delete(void*, void*) throw() { }
      CS_FORCEINLINE void operator delete[](void*, void*) throw() { }
    
    #if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
      CS_FORCEINLINE void* operator new (size_t s, void*, int)
      { return cs_malloc (s); }
      CS_FORCEINLINE void operator delete (void* p, void*, int)
      { cs_free (p); }
      CS_FORCEINLINE void* operator new[] (size_t s,  void*, int)
      { return cs_malloc (s); }
      CS_FORCEINLINE void operator delete[] (void* p, void*, int)
      { cs_free (p); }
    #endif
    };
    
    /**
     * Class that overrides operator new/operator delete/etc. 
     * with implementations using cs_malloc()/cs_free().
     * \remarks Use this class when you want to add custom allocation to a
     *   a class that derives from some other class(es). See the 
     *   CustomAllocator remarks section for the explanation.
     */
    template<typename T>
    class CustomAllocatedDerived : public T
    {
    public:
      CustomAllocatedDerived () {}
      template<typename A>
      CustomAllocatedDerived (const A& a) : T (a) {}

      // Potentially throwing versions
    #ifndef CS_NO_EXCEPTIONS
      CS_FORCEINLINE_TEMPLATEMETHOD void* operator new (size_t s) throw (std::bad_alloc)
      { 
	void* p = cs_malloc (s);
	if (!p) throw std::bad_alloc();
	return p;
      }
      CS_FORCEINLINE_TEMPLATEMETHOD void* operator new[] (size_t s) throw (std::bad_alloc)
      { 
	void* p = cs_malloc (s);
	if (!p) throw std::bad_alloc();
	return p;
      }
    #else
      CS_FORCEINLINE_TEMPLATEMETHOD void* operator new (size_t s) throw ()
      { return cs_malloc (s); }
      CS_FORCEINLINE_TEMPLATEMETHOD void* operator new[] (size_t s) throw ()
      { return cs_malloc (s); }
    #endif
      
      CS_FORCEINLINE_TEMPLATEMETHOD void operator delete (void* p) throw()
      { cs_free (p); }
      CS_FORCEINLINE_TEMPLATEMETHOD void operator delete[] (void* p) throw()
      { cs_free (p); }
      
      // Nothrow versions
      CS_FORCEINLINE_TEMPLATEMETHOD void* operator new (size_t s, const std::nothrow_t&) throw()
      { return cs_malloc (s); }
      CS_FORCEINLINE_TEMPLATEMETHOD void* operator new[] (size_t s, const std::nothrow_t&) throw()
      { return cs_malloc (s); }
      CS_FORCEINLINE_TEMPLATEMETHOD void operator delete (void* p, const std::nothrow_t&) throw()
      { cs_free (p); }
      CS_FORCEINLINE_TEMPLATEMETHOD void operator delete[] (void* p, const std::nothrow_t&) throw()
      { cs_free (p); }
      
      // Placement versions
      CS_FORCEINLINE_TEMPLATEMETHOD void* operator new(size_t /*s*/, void* p) throw() { return p; }
      CS_FORCEINLINE_TEMPLATEMETHOD void* operator new[](size_t /*s*/, void* p) throw() { return p; }

      CS_FORCEINLINE_TEMPLATEMETHOD void operator delete(void*, void*) throw() { }
      CS_FORCEINLINE_TEMPLATEMETHOD void operator delete[](void*, void*) throw() { }
    
    #if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
      CS_FORCEINLINE_TEMPLATEMETHOD void* operator new (size_t s, void*, int)
      { return cs_malloc (s); }
      CS_FORCEINLINE_TEMPLATEMETHOD void operator delete (void* p, void*, int)
      { cs_free (p); }
      CS_FORCEINLINE_TEMPLATEMETHOD void* operator new[] (size_t s,  void*, int)
      { return cs_malloc (s); }
      CS_FORCEINLINE_TEMPLATEMETHOD void operator delete[] (void* p, void*, int)
      { cs_free (p); }
    #endif
    };
  } // namespace Memory
} // namespace CS

#include "csutil/custom_new_enable.h"

#endif // __CS_CSUTIL_CUSTOMALLOCATED_H__
