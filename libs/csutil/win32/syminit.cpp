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
#include "csutil/sysfunc.h"
#include "csutil/win32/wintools.h"

#include <windows.h>
#include <tlhelp32.h>

#include "syminit.h"
#include "csutil/win32/DbgHelpAPI.h"

static void PrintError (const char* format, ...)
{
  csPrintfErr ("syminit: ");
  va_list args;
  va_start (args, format);
  csPrintfErrV (format, args);
  va_end (args);
  csPrintfErr ("\n");
}

namespace CrystalSpace
{
  namespace Debug
  {
    
    SymInitializer::SymInitializer ()
    {
      inited = false;
      hProc = INVALID_HANDLE_VALUE;
    }
    
    SymInitializer::~SymInitializer ()
    {
      if (inited)
      {
	DbgHelp::SymCleanup (GetSymProcessHandle ());
	DbgHelp::DecRef();
      }
      if (hProc != INVALID_HANDLE_VALUE)
	CloseHandle (hProc);
    }
    
    void SymInitializer::Init ()
    {
      if (inited) return;
      inited = true;
  
      DbgHelp::IncRef();
      if (!DbgHelp::SymInitialize (GetSymProcessHandle (), 0, true))
	PrintError ("SymInitialize : %s", 
	cswinGetErrorMessage (GetLastError ()));
      DbgHelp::SymSetOptions (SYMOPT_DEFERRED_LOADS |
	SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
    }
    
    HANDLE SymInitializer::GetSymProcessHandle ()
    {
      if (hProc == INVALID_HANDLE_VALUE)
      {
	if (!DuplicateHandle (GetCurrentProcess (), 
	  GetCurrentProcess (), GetCurrentProcess (), &hProc,
	  0, false, DUPLICATE_SAME_ACCESS))
	  hProc = GetCurrentProcess ();
      }
      return hProc;
    }

    void SymInitializer::RescanModules ()
    {
      /*
       If "deferred symbol loads" are enabled, for some reason the first 
       attempt to get symbol info after a rescan fails. So turn it off.
       */
      DbgHelp::SymSetOptions (SYMOPT_FAIL_CRITICAL_ERRORS |
	SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
    
      HANDLE hSnap = CreateToolhelp32Snapshot (TH32CS_SNAPMODULE,
	GetCurrentProcessId ());
      MODULEENTRY32 me;
      memset (&me, 0, sizeof (me));
      me.dwSize = sizeof (me);
      bool res = Module32First (hSnap, &me);
      while (res)
      {
	SetLastError (ERROR_SUCCESS);
	if (DbgHelp::SymLoadModule64 (symInit.GetSymProcessHandle (), 0,
	  me.szExePath, /*me.szExePath*/0, (LONG_PTR)me.modBaseAddr, 0) == 0)
	{
	  DWORD err = GetLastError ();
	  if (err != ERROR_SUCCESS)
	    PrintError ("SymLoadModule64 %s: %s", me.szModule, 
	      cswinGetErrorMessage (err));
	}
	res = Module32Next (hSnap, &me);
      }
      CloseHandle (hSnap);
    }
    
    SymInitializer symInit;
  } // namespace Debug
} // namespace CrystalSpace
