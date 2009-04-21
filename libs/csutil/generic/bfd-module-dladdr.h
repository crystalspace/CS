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

#include <dlfcn.h>

namespace
{
  class BfdModuleHelperDladdr
  {
    bool ok;
    Dl_info dlinfo;
  public:
    BfdModuleHelperDladdr (void* symAddr)
    {
      ok = dladdr (symAddr, &dlinfo);
    }
    
    uint64 GetBaseAddress ()
    { 
      if (!ok) return 0;
      return (uintptr_t)dlinfo.dli_fbase;
    }
    const char* GetFileName ()
    {
      return dlinfo.dli_fname;
    }
    uintptr_t GetAddrOffset ()
    {
      void* mod = dlopen (dlinfo.dli_fname, RTLD_NOW);
      /* Observation 1: shared libraries start at virtual address 0,
         executables somewhere else. Thus, for shared libs, correct all
         addresses by the given base address. Executables don't need such
         correction. 
         @@@ Could there be shared libs that start at something other than 0?
             How to detect the start address there?

	 Observation 2: For executables, dlopen() returns 0.
       */
      uintptr_t offs = (mod == 0) ? 0 : ((uintptr_t)-GetBaseAddress());
      if (mod != 0)
      {
        dlclose (mod);
      }
      return offs;
    }
  };
}
