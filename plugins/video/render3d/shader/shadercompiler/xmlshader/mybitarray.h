/*
  Copyright (C) 2006 by Frank Richter
	    (C) 2006 by Jorrit Tyberghein

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

#ifndef __CS_MYBITARRAY_H__
#define __CS_MYBITARRAY_H__

#include "csutil/bitarray.h"
#include "csutil/fixedsizeallocator.h"

#include "tempheap.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{
  class MyBitArrayAllocatorMalloc : CS::Memory::AllocatorMalloc
  {
    typedef CS::Memory::AllocatorMalloc Allocator;
    
    typedef csFixedSizeAllocator<sizeof (csBitArrayStorageType) * 2, 
      Allocator> BitsAlloc2Type;
    CS_DECLARE_STATIC_CLASSVAR_REF (bitsAlloc2,
      BitsAlloc2, BitsAlloc2Type);
    
    typedef csFixedSizeAllocator<sizeof (csBitArrayStorageType) * 2, 
      Allocator> BitsAlloc4Type;
    CS_DECLARE_STATIC_CLASSVAR_REF (bitsAlloc4,
      BitsAlloc4, BitsAlloc4Type);
  public:
    void* Alloc (const size_t n)
    {
      if (n <= sizeof (csBitArrayStorageType) * 2)
        return BitsAlloc2().Alloc();
      else if (n <= sizeof (csBitArrayStorageType) * 4)
        return BitsAlloc4().Alloc();
      else
      {
        return Allocator::Alloc (n);
      }
    }
    void Free (void* p)
    {
      if (BitsAlloc4().TryFree (p)) return;
      if (BitsAlloc2().TryFree (p)) return;
      Allocator::Free (p);
    }

    static void CompactAllocators()
    {
      BitsAlloc2().Compact();
      BitsAlloc4().Compact();
    }
  };
  class MyBitArrayAllocatorTemp : TempHeapAlloc
  {
    typedef TempHeapAlloc Allocator;
    
    typedef csFixedSizeAllocator<sizeof (csBitArrayStorageType) * 2, 
      Allocator> BitsAlloc2Type;
    CS_DECLARE_STATIC_CLASSVAR_REF (bitsAlloc2,
      BitsAlloc2, BitsAlloc2Type);
    
    typedef csFixedSizeAllocator<sizeof (csBitArrayStorageType) * 2, 
      Allocator> BitsAlloc4Type;
    CS_DECLARE_STATIC_CLASSVAR_REF (bitsAlloc4,
      BitsAlloc4, BitsAlloc4Type);
  public:
    void* Alloc (const size_t n)
    {
      if (n <= sizeof (csBitArrayStorageType) * 2)
        return BitsAlloc2().Alloc();
      else if (n <= sizeof (csBitArrayStorageType) * 4)
        return BitsAlloc4().Alloc();
      else
      {
        return Allocator::Alloc (n);
      }
    }
    void Free (void* p)
    {
      if (BitsAlloc4().TryFree (p)) return;
      if (BitsAlloc2().TryFree (p)) return;
      Allocator::Free (p);
    }

    static void CompactAllocators()
    {
      BitsAlloc2().Compact();
      BitsAlloc4().Compact();
    }
  };

  //@{
  /**
   * Specialized bit array that uses block allocation for smaller
   * sizes.
   */
  typedef csBitArrayTweakable<64, MyBitArrayAllocatorMalloc> MyBitArrayMalloc;
  typedef csBitArrayTweakable<64, MyBitArrayAllocatorTemp> MyBitArrayTemp;
  //@}
}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#define XMLSNS CS_PLUGIN_NAMESPACE_NAME(XMLShader)

template<>
class csComparator<XMLSNS::MyBitArrayMalloc, XMLSNS::MyBitArrayMalloc> : 
  public csComparatorBitArray<XMLSNS::MyBitArrayMalloc> { };

template<>
class csHashComputer<XMLSNS::MyBitArrayMalloc> : 
  public csHashComputerBitArray<XMLSNS::MyBitArrayMalloc> { };

template<>
class csComparator<XMLSNS::MyBitArrayTemp, XMLSNS::MyBitArrayTemp> : 
  public csComparatorBitArray<XMLSNS::MyBitArrayTemp> { };

template<>
class csHashComputer<XMLSNS::MyBitArrayTemp> : 
  public csHashComputerBitArray<XMLSNS::MyBitArrayTemp> { };

#undef XMLSNS

#endif // __CS_MYBITARRAY_H__
