/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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

#include "csutil/win32/minidump.h"

/*
struct ThreadContextFilterInfo
{
  HANDLE filterThread;
  CONTEXT newContext;
};

static BOOL CALLBACK ThreadFilterCB (PVOID CallbackParam,
  const PMINIDUMP_CALLBACK_INPUT CallbackInput, 
  PMINIDUMP_CALLBACK_OUTPUT CallbackOutput)
{
}
*/

const char* cswinMinidumpWriter::WriteMinidump (
  PMINIDUMP_EXCEPTION_INFORMATION except)
{
  DbgHelp::IncRef();
  if (!DbgHelp::MinidumpAvailable())
  {
    DbgHelp::DecRef();
    return 0;
  }

  char dumpFN[MAX_PATH];
  char tempPath[MAX_PATH - 13];

  GetTempPath (sizeof (tempPath), tempPath);
  GetTempFileName (tempPath, "dmp", 0, dumpFN);

  HANDLE dumpFile = CreateFile (dumpFN, GENERIC_READ | GENERIC_WRITE, 
    FILE_SHARE_READ, 0, CREATE_ALWAYS, 
    FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED,
    0);

  /*
    The callstack for the thread we were called from is kinda crappy,
    (IP somewhere in the kernel), so replace it with a stack that at
    least points to this function.
   */
  /*ThreadContextFilterInfo filter;
  filter.filterThread = GetCurrentThread ();
  memset (&filter.newContext, 0, sizeof (filter.newContext));
  filter.newContext.ContextFlags = CONTEXT_FULL;
  GetThreadContext (filter.filterThread, &filter.newContext);*/

  bool dumpSucceeded = DbgHelp::MiniDumpWriteDump (GetCurrentProcess(),
    GetCurrentProcessId(), dumpFile,
    MiniDumpWithDataSegs | MiniDumpScanMemory | 
    MiniDumpWithIndirectlyReferencedMemory,
    except, 0, 0);

  CloseHandle (dumpFile);

  DbgHelp::DecRef();

  if (dumpSucceeded)
  {
    static char finalDumpFN[MAX_PATH];

    strcpy (finalDumpFN, dumpFN);
#ifdef CS_DEBUG
    char* dot = strrchr (finalDumpFN, '.');
    strcpy (dot, ".dmp");

    if (MoveFile (dumpFN, finalDumpFN) == 0)
      return 0;
#endif

    return finalDumpFN;
  }
  else
    return 0;

}
