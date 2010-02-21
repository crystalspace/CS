/*
  Copyright (C) 2004-2006 by Frank Richter
	    (C) 2004-2006 by Jorrit Tyberghein

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

#ifndef __CS_CONDEVAL_SLICEALLOC_H__
#define __CS_CONDEVAL_SLICEALLOC_H__

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{
  struct SliceAllocator
  {
    static const size_t valueSetsPerSlice = 32;
    static const size_t sliceSize = valueSetsPerSlice * sizeof (ValueSet);

    typedef CS::Memory::FixedSizeAllocatorSafe<sliceSize, TempHeapAlloc>
      BlockAlloc;

    CS_DECLARE_STATIC_CLASSVAR_REF (sliceAlloc, SliceAlloc, 
      BlockAlloc);

    static inline uint8* Alloc (size_t blocksize) 
    {
      return (uint8*)SliceAlloc().Alloc (blocksize);
    }
    static inline void Free (uint8* p)
    {
      SliceAlloc().Free (p);
    }
    static void CompactAllocator()
    {
      SliceAlloc().Compact();
    }
    static void SetMemTrackerInfo (const char*) {}
  };

  struct SliceAllocatorBool
  {
    static const size_t valueSetsPerSlice = 32;
    static const size_t sliceSize = valueSetsPerSlice * sizeof (ValueSetBool);

    typedef CS::Memory::FixedSizeAllocatorSafe<sliceSize, TempHeapAlloc>
      BlockAlloc;

    CS_DECLARE_STATIC_CLASSVAR_REF (sliceAlloc, SliceAllocBool, 
      BlockAlloc);

    static inline uint8* Alloc (size_t blocksize) 
    {
      return (uint8*)SliceAllocBool().Alloc (blocksize);
    }
    static inline void Free (uint8* p)
    {
      SliceAllocBool().Free (p);
    }
    static void CompactAllocator()
    {
      SliceAllocBool().Compact();
    }
    static void SetMemTrackerInfo (const char*) {}
  };
}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#endif // __CS_CONDEVAL_SLICEALLOC_H__
