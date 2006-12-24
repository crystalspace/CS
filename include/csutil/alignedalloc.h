/*
    Copyright (C) 2006 by Frank Richter
	      (C) 2006 by Marten Svanfeldt

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

#ifndef __CS_CSUTIL_ALIGNEDALLOC_H__
#define __CS_CSUTIL_ALIGNEDALLOC_H__

/**\file
 * Aligned memory allocations.
 */

/**\addtogroup util_memory
 * @{ */

namespace CS
{
  namespace Memory
  {
    /**
     * Allocate a block of memory, with the start address being aligned
     * to a multiple of \p align bytes.
     * \remarks The returned block of memory must be freed with AlignedFree.
     */
    CS_CRYSTALSPACE_EXPORT CS_ATTRIBUTE_MALLOC void* AlignedMalloc (
      size_t size, size_t align);
    /// Free a block of memory allocated with AlignedMalloc.
    CS_CRYSTALSPACE_EXPORT void AlignedFree (void* ptr);
    /**
     * Reallocate a block of memory with a new size, with the start address 
     * being aligned to a multiple of \p align bytes.
     * \remarks The returned block of memory must be freed with AlignedFree.
     * \warning The alignment must be the same as initially passed to 
     *   AlignedMalloc.
     */
    CS_CRYSTALSPACE_EXPORT void* AlignedRealloc (void* ptr, size_t size, 
      size_t align);
  } // namespace Memory
} // namespace CS

/** @} */

#endif // __CS_CSUTIL_ALIGNEDALLOC_H__
