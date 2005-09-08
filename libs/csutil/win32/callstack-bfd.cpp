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

#include <windows.h>

#include "csutil/win32/DbgHelpAPI.h"
#include "syminit.h"

namespace CrystalSpace
{
  namespace Debug
  {
    CallStackNameResolverBfd::~CallStackNameResolverBfd()
    {
      csHash<BfdSymbols*, uint64>::GlobalIterator it = 
	moduleBfds.GetIterator();
      while (it.HasNext())
      {
	BfdSymbols* bfd = it.Next();
	delete bfd;
      }
    }
    
    BfdSymbols* CallStackNameResolverBfd::BfdForAddress (void* addr)
    {
      if (!DbgHelp::SymSupportAvailable()) return false;
      symInit.Init ();
      
      uint64 base = DbgHelp::SymGetModuleBase64 (symInit.GetSymProcessHandle(),
	(uintptr_t)addr);
      if (base == 0)
      {
	symInit.RescanModules();
	base = DbgHelp::SymGetModuleBase64 (symInit.GetSymProcessHandle(),
	  (uintptr_t)addr);
      }
      if (base == 0) return 0;
      
      BfdSymbols* bfd = moduleBfds.Get (base, 0);
      if (bfd == 0)
      {
	CHAR moduleFN[MAX_PATH];
	if (GetModuleFileNameA ((HMODULE)(uintptr_t)base, moduleFN, 
	  sizeof(moduleFN)/sizeof(moduleFN[0])) == 0)
	  return false;
	
	bfd = new BfdSymbols (moduleFN, base);
	moduleBfds.Put (base, bfd);
      }
      if (!bfd->Ok()) return 0;
      
      return bfd;
    }
    
    bool CallStackNameResolverBfd::GetAddressSymbol (void* addr, 
      csString& sym)
    {
      BfdSymbols* bfd = BfdForAddress (addr);
      if (!bfd) return false;
	
      const char* filename;
      const char* function;
      uint line;
      
      if (bfd->FindSymbol ((uintptr_t)addr, filename, function, line))
      {
	if (!function) return false;
	csString tmp;
	CrystalSpace::Debug::Demangle (function, tmp);
	sym.Format ("[%p] (%s)%s", addr, bfd->GetFileName(), 
	  tmp.GetData());
	return true;
      }
      
      return false;
    }
    
    void* CallStackNameResolverBfd::OpenParamSymbols (void* addr)
    {
      return 0;
    }
    
    bool CallStackNameResolverBfd::GetParamName (void*, size_t, csString&)
    {
      return false;
    }
    
    void CallStackNameResolverBfd::FreeParamSymbols (void* handle)
    {
    }
    
    bool CallStackNameResolverBfd::GetLineNumber (void*, csString&)
    {
      return false;
    }

  } // namespace Debug
} // namespace CrystalSpace
