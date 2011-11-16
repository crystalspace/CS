/*
    Copyright (C) 2006-2007 by Frank Richter

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

#ifndef __CS_CSUTIL_ALLOCATORIMPL_H__
#define __CS_CSUTIL_ALLOCATORIMPL_H__

/**\file
 * Basic allocator classes.
 */

#include "csutil/scf_implementation.h"
#include "iutil/allocator.h"

/**\addtogroup util_memory
 * @{ */

namespace CS
{
  namespace Memory
  {
    /**
     * iAllocator implementation, wrapping arbitrary allocator.
     * Allocator must be copy-constructible.
     */
    template<typename Allocator>
    class AllocatorImplementation :
        public scfImplementation1<AllocatorImplementation<Allocator>,
                                  iAllocator>
    {
      Allocator alloc;

      typedef scfImplementation1<AllocatorImplementation<Allocator>,
                                 iAllocator> scfImplementationType;
    public:
      AllocatorImplementation (const Allocator& alloc)
        : scfImplementationType (this), alloc (alloc) {}

      void Free (void* p)
      {
        return alloc.Free (p);
      }

      CS_ATTRIBUTE_MALLOC void* Alloc (const size_t n)
      {
        return alloc.Alloc (n);
      }

      void* Realloc (void* p, size_t newSize)
      {
        return alloc.Realloc (p, newSize);
      }

      void SetMemTrackerInfo (const char* info)
      {
        alloc.SetMemTrackerInfo (info);
      }
    };
  } // namespace Memory
} // namespace CS

/** @} */

#endif // __CS_CSUTIL_ALLOCATORIMPL_H__
