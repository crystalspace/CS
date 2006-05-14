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

#ifndef __CS_CSUTIL_ALIGNED_ALLOC_H__
#define __CS_CSUTIL_ALIGNED_ALLOC_H__

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
    static inline void* AlignedMalloc (size_t size, size_t align)
    {
    #ifdef CS_HAVE__ALIGNED_MALLOC
      return _aligned_malloc (size, align);
    #else
      void *mallocPtr = malloc (size + align + sizeof(void*));
      uintptr_t ptrInt = (intptr_t)mallocPtr;
    
      ptrInt = (ptrInt + align + sizeof(void*)) / align * align;
      *(((void**)ptrInt) - 1) = mallocPtr;
    
      return (void*)ptrInt;
    #endif
    }
    
    /// Free a block of memory allocated with AlignedMalloc.
    static inline void AlignedFree (void* ptr)
    {
    #ifdef CS_HAVE__ALIGNED_MALLOC
      _aligned_free (ptr);
    #else
      free (*(((void**)ptr) - 1));
    #endif
    }
    
    /**
     * Reallocate a block of memory with a new size, with the start address 
     * being aligned to a multiple of \p align bytes.
     * \remarks The returned block of memory must be freed with AlignedFree.
     * \warning The alignment must be the same as initially passed to 
     *   AlignedMalloc.
     */
    static inline void* AlignedRealloc (void* ptr, size_t size, 
      size_t align)
    {
    #ifdef CS_HAVE__ALIGNED_MALLOC
      return _aligned_realloc (ptr, size, align);
    #else
      void* orgPtr = *(((void**)ptr) - 1);
      uintptr_t offsetToData = (uintptr_t)ptr - (uintptr_t)orgPtr;
      void* newPtr = realloc (orgPtr, size + align + sizeof(void*));
      
      uintptr_t ptrInt = (intptr_t)newPtr;
      ptrInt = (ptrInt + align + sizeof(void*)) / align * align;
      uintptr_t newOffsetToData = ptrInt - (uintptr_t)newPtr;
      if (newOffsetToData != offsetToData)
      {
	// Ensure realloced data is aligned again
	memmove ((void*)(ptrInt), (void*)((uintptr_t)newPtr + offsetToData),
	  size);
      }
      
      *(((void**)ptrInt) - 1) = newPtr;
      return (void*)ptrInt;
    #endif
    }
  } // namespace Memory
} // namespace CS

/** @} */

#endif // __CS_CSUTIL_ALIGNED_ALLOC_H__
