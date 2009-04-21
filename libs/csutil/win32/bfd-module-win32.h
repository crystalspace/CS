/*
    Copyright (C) 2007 by Frank Richter

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

#include <windows.h>

#include "csutil/win32/DbgHelpAPI.h"
#include "syminit.h"

namespace
{
  class BfdModuleHelperWin32
  {
    uint64 base;
    CHAR moduleFN[MAX_PATH];
  public:
    BfdModuleHelperWin32 (void* symAddr) : base (0)
    {
      if (DbgHelp::SymSupportAvailable())
      {
	CS::Debug::symInit.Init ();
	
	base = DbgHelp::SymGetModuleBase64 (
	  CS::Debug::symInit.GetSymProcessHandle(), (uintptr_t)symAddr);
	if (base == 0)
	{
	  CS::Debug::symInit.RescanModules();
	  base = DbgHelp::SymGetModuleBase64 (
	    CS::Debug::symInit.GetSymProcessHandle(), (uintptr_t)symAddr);
	}
      }
      moduleFN[0] = 0;
    }
    
    uint64 GetBaseAddress () { return base; }
    const char* GetFileName ()
    {
      if (moduleFN[0] == 0)
      {
        if (GetModuleFileNameA ((HMODULE)(uintptr_t)base, moduleFN, 
	   sizeof(moduleFN)/sizeof(moduleFN[0])) == 0)
	  moduleFN[0] = 0;
	else
	{
	  char* p = moduleFN;
	  // libbfd seems to like forward slashes ...
	  while (*p != 0)
	  {
	    if (*p == '\\') *p = '/';
	    p++;
	  }
	}
      }
      return moduleFN;
    }
    uintptr_t GetAddrOffset ()
    {
      /* Compute relocation offset */
      uintptr_t addrOffs = 0;
      PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)(uintptr_t)base;
      PIMAGE_NT_HEADERS NTheader = 
	(PIMAGE_NT_HEADERS)((uint8*)dosHeader + dosHeader->e_lfanew);
      if (NTheader->Signature == 0x00004550) // check for PE sig
      {
	uintptr_t imageBase = NTheader->OptionalHeader.ImageBase;
	addrOffs = imageBase - (uintptr_t)base;
      }
      return addrOffs;
    }
  };
}
