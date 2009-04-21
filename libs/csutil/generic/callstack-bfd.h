/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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

#ifndef __CS_LIBS_UTIL_WIN32_CALLSTACK_BFD_H__
#define __CS_LIBS_UTIL_WIN32_CALLSTACK_BFD_H__

#include "csextern.h"
#include "csutil/array.h"
#include "csutil/blockallocator.h"
#include "csutil/callstack.h"
#include "csutil/customallocated.h"
#include "csutil/strset.h"

#include "../callstack.h"
#include "../bfdsymbols.h"

namespace CS
{
  namespace Debug
  {
    class CallStackNameResolverBfd :
      public CallstackAllocatedDerived<iCallStackNameResolver>
    {
      typedef csHash<BfdSymbols*, uint64,
        CS::Memory::AllocatorMallocPlatform> ModulesHash;
      ModulesHash moduleBfds;
      
      BfdSymbols* BfdForAddress (void* addr);
    public:
      virtual ~CallStackNameResolverBfd();
    
      virtual bool GetAddressSymbol (void* addr, char*& sym);
      virtual void* OpenParamSymbols (void* addr);
      virtual bool GetParamName (void* handle, size_t paramNum, char*& sym);
      virtual void FreeParamSymbols (void* handle);
      virtual bool GetLineNumber (void* addr, char*& lineAndFile);
    };
  
  } // namespace Debug
} // namespace CS

#endif // __CS_LIBS_UTIL_WIN32_CALLSTACK_BFD_H__
