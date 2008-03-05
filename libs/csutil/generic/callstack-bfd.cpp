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

#include "cssysdef.h"
#include "callstack-bfd.h"
#include "../demangle.h"

#include "csutil/custom_new_disable.h"
#include <string>
#include "csutil/custom_new_enable.h"

#if defined(CS_PLATFORM_WIN32)
#include "libs/csutil/win32/bfd-module-win32.h"
typedef BfdModuleHelperWin32 BfdModuleHelper;
#elif defined(CS_HAVE_DLADDR)
#include "libs/csutil/generic/bfd-module-dladdr.h"
typedef BfdModuleHelperDladdr BfdModuleHelper;
#else
#error BFD symbol resolution enabled but no platform-specific module support; \
  implement or fix configure
#endif

namespace CS
{
  namespace Debug
  {
    CallStackNameResolverBfd::~CallStackNameResolverBfd()
    {
      ModulesHash::GlobalIterator it = 
	moduleBfds.GetIterator();
      while (it.HasNext())
      {
	BfdSymbols* bfd = it.Next();
	delete bfd;
      }
    }
    
    BfdSymbols* CallStackNameResolverBfd::BfdForAddress (void* addr)
    {
      BfdModuleHelper modHelper (addr);
      uint64 base = modHelper.GetBaseAddress ();
      if (base == 0) return false;
      
      BfdSymbols* bfd = moduleBfds.Get (base, 0);
      if (bfd == 0)
      {
	const char* moduleFN = modHelper.GetFileName ();
	if (!moduleFN || !*moduleFN)
	  return false;
	
	uintptr_t addrOffs = modHelper.GetAddrOffset ();
	bfd = new BfdSymbols (moduleFN, addrOffs);
	moduleBfds.Put (base, bfd);
      }
      if (!bfd->Ok()) return 0;
      
      return bfd;
    }
    
    bool CallStackNameResolverBfd::GetAddressSymbol (void* addr, 
      char*& sym)
    {
      BfdSymbols* bfd = BfdForAddress (addr);
      if (!bfd) return false;
	
      const char* filename;
      const char* function;
      uint line;
      
      if (bfd->FindSymbol ((uintptr_t)addr, filename, function, line))
      {
	if (!function) return false;
	char* tmp = CS::Debug::Demangle (function);
        char buf[512];
        snprintf (buf, sizeof (buf), "[%p] (%s)%s", addr,
          bfd->GetFileName(), tmp);
        sym = strdup (buf);
        free (tmp);
	return true;
      }
      
      return false;
    }
    
    void* CallStackNameResolverBfd::OpenParamSymbols (void* addr)
    {
      return 0;
    }
    
    bool CallStackNameResolverBfd::GetParamName (void*, size_t, char*&)
    {
      return false;
    }
    
    void CallStackNameResolverBfd::FreeParamSymbols (void* handle)
    {
    }
    
    bool CallStackNameResolverBfd::GetLineNumber (void* addr, char*& str)
    {
      BfdSymbols* bfd = BfdForAddress (addr);
      if (!bfd) return false;
	
      const char* filename;
      const char* function;
      uint line;
      
      if (bfd->FindSymbol ((uintptr_t)addr, filename, function, line))
      {
	if (!function) return false;
        char buf[512];
        if (line > 0) 
	  snprintf (buf, sizeof (buf), "%s:%d", filename, line);
        else
	  snprintf (buf, sizeof (buf), "%s", filename);
	str = strdup (buf);
	return true;
      }
      
      return false;
    }

  } // namespace Debug
} // namespace CS
