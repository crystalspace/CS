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

#ifndef __CS_LIBS_UTIL_WIN32_SYMINIT_H__
#define __CS_LIBS_UTIL_WIN32_SYMINIT_H__

namespace CrystalSpace
{
  namespace Debug
  {
    class SymInitializer
    {
      bool inited;
      HANDLE hProc;
    public:
      SymInitializer ();
      ~SymInitializer ();
      void Init ();
      HANDLE GetSymProcessHandle ();
    
      void RescanModules ();
    };
    
    extern SymInitializer symInit;
  } // namespace Debug
} // namespace CrystalSpace

#endif // __CS_LIBS_UTIL_WIN32_SYMINIT_H__
