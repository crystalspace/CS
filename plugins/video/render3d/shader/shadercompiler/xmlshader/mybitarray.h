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
#include "csutil/blockallocator.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{
  class MyBitArrayAllocator
  {
    typedef csBitArrayStorageType Bits2[2];
    CS_DECLARE_STATIC_CLASSVAR_REF (bitsAlloc2,
      BitsAlloc2, csBlockAllocator<Bits2>);
    typedef csBitArrayStorageType Bits4[4];
    CS_DECLARE_STATIC_CLASSVAR_REF (bitsAlloc4,
      BitsAlloc4, csBlockAllocator<Bits4>);
  public:
    static void* Alloc (const size_t n)
    {
      if (n <= sizeof (Bits2))
        return BitsAlloc2().AllocUninit();
      else if (n <= sizeof (Bits4))
        return BitsAlloc4().AllocUninit();
      else
      {
#ifdef CS_MEMORY_TRACKER
        static const char mtiInfo[] = "MyBitArrayAllocator";
        uintptr_t* ptr = (uintptr_t*)malloc (n + sizeof (uintptr_t)*2);
        *ptr++ = (uintptr_t)mtiRegisterAlloc (n, (void*)&mtiInfo);
        *ptr++ = n;
        return ptr;
#else
        return malloc (n);
#endif
      }
    }
    static void Free (void* p)
    {
      if (BitsAlloc4().TryFree ((Bits4*)p)) return;
      if (BitsAlloc2().TryFree ((Bits2*)p)) return;
#ifdef CS_MEMORY_TRACKER
      uintptr_t* ptr = ((uintptr_t*)p)-2;
      mtiRegisterFree ((csMemTrackerInfo*)*ptr, (size_t)ptr[1]);
      free (ptr);
#else
      free (p);
#endif
    }

    static void CompactAllocators()
    {
      BitsAlloc2().Compact();
      BitsAlloc4().Compact();
    }
  };

  typedef csBitArrayTweakable<64, MyBitArrayAllocator> MyBitArray;
}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#define XMLSNS CS_PLUGIN_NAMESPACE_NAME(XMLShader)

CS_SPECIALIZE_TEMPLATE
class csComparator<XMLSNS::MyBitArray, XMLSNS::MyBitArray> : 
  public csComparatorBitArray<XMLSNS::MyBitArray> { };

CS_SPECIALIZE_TEMPLATE
class csHashComputer<XMLSNS::MyBitArray> : 
  public csBitArrayHashComputer<XMLSNS::MyBitArray> { };

#undef XMLSNS

#endif // __CS_MYBITARRAY_H__
