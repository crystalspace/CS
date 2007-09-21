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
    };
  } // namespace Memory
} // namespace CS

#endif // __CS_CSUTIL_CUSTOMALLOCATED_H__
