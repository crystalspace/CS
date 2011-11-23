/*
    Copyright (C) 2011 by Frank Richter

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

#ifndef __CS_IUTIL_ALLOCATOR_H__
#define __CS_IUTIL_ALLOCATOR_H__

/**\file
 * Memory allocator interface
 */

#include "csutil/scf_interface.h"

/**\addtogroup util_memory
 * @{ */

namespace CS
{
  namespace Memory
  {
    /**
     * Allocator interface.
     * Provides the general allocator interface methods described in \ref Allocators.
     */
    struct iAllocator : public virtual iBase
    {
      SCF_INTERFACE (CS::Memory::iAllocator, 0, 0, 1);

      /// Free the block \p p.
      virtual void Free (void* p) = 0;
      /// Allocate a block of memory of size \p n.
      virtual CS_ATTRIBUTE_MALLOC void* Alloc (const size_t n) = 0;
      /// Resize the allocated block \p p to size \p newSize.
      virtual void* Realloc (void* p, size_t newSize) = 0;
      /// Set the information used for memory tracking.
      virtual void SetMemTrackerInfo (const char* info) = 0;
    };
  } // namespace Memory
} // namespace CS

/** @} */

#endif // __CS_IUTIL_ALLOCATOR_H__
