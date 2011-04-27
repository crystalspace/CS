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

#include "cssysdef.h"
#include "csutil/alignedalloc.h"

namespace CS
{
  // ptmalloc functions
  namespace ptmalloc_
  {
    extern "C"
    {
      extern void* ptmalloc (size_t n);
      extern void ptfree (void* p);
      extern void* ptrealloc (void* p, size_t n);
      extern void* ptmemalign (size_t a, size_t n);
    }
  }

  namespace Memory
  {
    /**
     * Allocate a block of memory, with the start address being aligned
     * to a multiple of \p align bytes.
     * \remarks The returned block of memory must be freed with AlignedFree.
     */
    void* AlignedMalloc (size_t size, size_t align)
    {
    #if !defined(CS_NO_PTMALLOC)
      return ptmalloc_::ptmemalign (align, size);
    #elif defined(CS_HAVE__ALIGNED_MALLOC)
      return _aligned_malloc (size, align);
    #else
      void *mallocPtr = cs_malloc (size + align + sizeof(void*));
      uintptr_t ptrInt = (intptr_t)mallocPtr;
    
      ptrInt = (ptrInt + align + sizeof(void*)) / align * align;
      *(((void**)ptrInt) - 1) = mallocPtr;
    
      return (void*)ptrInt;
    #endif
    }
    
    /// Free a block of memory allocated with AlignedMalloc.
    void AlignedFree (void* ptr)
    {
    #if !defined(CS_NO_PTMALLOC)
      ptmalloc_::ptfree (ptr);
    #elif defined(CS_HAVE__ALIGNED_MALLOC)
      _aligned_free (ptr);
    #else
      cs_free (*(((void**)ptr) - 1));
    #endif
    }
    
    /**
     * Reallocate a block of memory with a new size, with the start address 
     * being aligned to a multiple of \p align bytes.
     * \remarks The returned block of memory must be freed with AlignedFree.
     * \warning The alignment must be the same as initially passed to 
     *   AlignedMalloc.
     */
    void* AlignedRealloc (void* ptr, size_t size, 
      size_t align)
    {
    #if !defined(CS_NO_PTMALLOC)
      void* newPtr = ptmalloc_::ptrealloc (ptr, size);
      if ((newPtr != ptr)
        && ((((uintptr_t)newPtr) / align * align) != (uintptr_t)newPtr))
      {
        // Bah, alignment borked. Need to alloc again :(
        void* newPtrAligned = ptmalloc_::ptmemalign (align, size);
        memcpy (newPtrAligned, newPtr, size);
        ptfree (newPtr);
        newPtr = newPtrAligned;
      }
      return newPtr;
    #elif defined(CS_HAVE__ALIGNED_MALLOC)
      return _aligned_realloc (ptr, size, align);
    #else
      void* orgPtr = *(((void**)ptr) - 1);
      uintptr_t offsetToData = (uintptr_t)ptr - (uintptr_t)orgPtr;
      void* newPtr = cs_realloc (orgPtr, size + align + sizeof(void*));
      
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
