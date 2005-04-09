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

#include "csutil/dirtyaccessarray.h"
#include "csutil/sysfunc.h"
#include "csutil/archive.h"
#include "csutil/callstack.h"
#include "csutil/physfile.h"
#include "csutil/ref.h"
#include "csutil/util.h"
#include "csutil/win32/callstack.h"
#include "csutil/win32/win32.h"

#include "iutil/objreg.h"
#include "ivaria/stdrep.h"

#include "cachedll.h"
#include "csutil/win32/minidump.h"
#include "csutil/memfile.h"

#include <windows.h>
#include <tlhelp32.h>

static void WriteRangeInfo (HANDLE tempFile, HANDLE rangeFile, 
			    DWORD rangeStart, DWORD rangeSize,
			    DWORD rangeOffset, int& rangeCount)
{
  if (rangeStart != 0)
  {
    MINIDUMP_MEMORY_DESCRIPTOR md;
    md.StartOfMemoryRange = rangeStart;
    md.Memory.DataSize = rangeSize;
    md.Memory.Rva = rangeOffset;

    DWORD bytesWritten;
    WriteFile (rangeFile, &md, sizeof (md), &bytesWritten, 0);
    rangeCount++;
  }
}

static void CollectHeapInfo (HANDLE snap, HANDLE tempFile,
  HANDLE rangeFile, int& rangeCount)
{
  HEAPLIST32 heapList;
  memset (&heapList, 0, sizeof (heapList));
  heapList.dwSize = sizeof (heapList);

  HEAPENTRY32 heapEntry;
  memset (&heapEntry, 0, sizeof (heapEntry));
  heapEntry.dwSize = sizeof (heapEntry);

  DWORD rangeStart = 0;
  DWORD rangeSize = 0;
  DWORD rangeOffset = 0;

  bool hasHeapList = Heap32ListFirst (snap, &heapList);
  while (hasHeapList)
  {
    bool hasHeapEntry = Heap32First (&heapEntry, 
      GetCurrentProcessId(), heapList.th32HeapID);
    while (hasHeapEntry)
    {
      DWORD bytesWritten;
      WriteFile (tempFile, (void*)heapEntry.dwAddress, 
	heapEntry.dwBlockSize, &bytesWritten, 0);

      if (bytesWritten == heapEntry.dwBlockSize)
      {
	if (heapEntry.dwAddress == rangeStart + rangeSize)
	{
	  rangeSize += heapEntry.dwBlockSize;
	}
	else
	{
	  WriteRangeInfo (tempFile, rangeFile, rangeStart, rangeSize, 
	    rangeOffset, rangeCount);

	  rangeStart = heapEntry.dwAddress;
	  rangeSize = heapEntry.dwBlockSize;
	  rangeOffset = SetFilePointer (tempFile, 0, 0, FILE_CURRENT) -
	    heapEntry.dwBlockSize;
	}
      }
      else
	SetFilePointer (tempFile, -bytesWritten, 0, FILE_CURRENT);

      hasHeapEntry = Heap32Next (&heapEntry);
    }

    hasHeapList = Heap32ListNext (snap, &heapList);
  }

  WriteRangeInfo (tempFile, rangeFile, rangeStart, rangeSize, rangeOffset, 
    rangeCount);
}

static bool PostprocessMiniDump (HANDLE dumpFile, HANDLE rangeFile,
				 int& rangeCount)
{
  bool didProcess = false;
  DWORD dumpSize = GetFileSize (dumpFile, 0);

  HANDLE mapping = CreateFileMapping (dumpFile, 0, PAGE_READWRITE, 0, 0, 0);
  if (mapping != 0)
  {
    void* file = MapViewOfFile (mapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);

    if (file != 0)
    {
      PMINIDUMP_HEADER minidump = (PMINIDUMP_HEADER)file;

      PMINIDUMP_DIRECTORY streamDir = 
	(PMINIDUMP_DIRECTORY)((uint8*)minidump + minidump->StreamDirectoryRva);

      /*
	Scan through the dump file, look for the memory list
      */
      uint streamNum = minidump->NumberOfStreams;
      while (streamNum-- > 0)
      {
	if (streamDir->StreamType == MemoryListStream)
	{
	  PMINIDUMP_MEMORY_LIST myMemList = 
	    (PMINIDUMP_MEMORY_LIST)((uint8*)minidump + 
	      streamDir->Location.Rva);

	  HANDLE rangeMapping = CreateFileMapping (rangeFile, 0, PAGE_READWRITE, 
	    0, 0, 0);
	  if (rangeMapping != 0)
	  {
	    MINIDUMP_MEMORY_DESCRIPTOR* ranges = (MINIDUMP_MEMORY_DESCRIPTOR*)
	      MapViewOfFile (rangeMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	    if (ranges != 0)
	    {
	      DWORD mySize = sizeof (MINIDUMP_MEMORY_LIST) +
		sizeof (MINIDUMP_MEMORY_DESCRIPTOR) * 
		(myMemList->NumberOfMemoryRanges + rangeCount);
	      DWORD myOffs = dumpSize + mySize;

	      /*
		Ours & the mem list already in the dump will be concatenated
		and appended to the end.
	      */
	      streamDir->Location.Rva = dumpSize;
	      streamDir->Location.DataSize = mySize;

	      // Set up correct offsets in our mem list
	      for (int j = 0; j < rangeCount; j++)
	      {
		ranges[j].Memory.Rva += myOffs;
	      }
	    }
	    UnmapViewOfFile (ranges);
	  }
	  CloseHandle (rangeMapping);

	  SetFilePointer (rangeFile, 0, 0, FILE_END);

	  MINIDUMP_MEMORY_DESCRIPTOR* MemoryRanges = 
	    (MINIDUMP_MEMORY_DESCRIPTOR*)((ULONG32*)myMemList + 1);
	  for (uint i = 0; i < myMemList->NumberOfMemoryRanges; i++)
	  {
	    DWORD bytesWritten;
	    WriteFile (rangeFile, MemoryRanges + i, 
	      sizeof (MINIDUMP_MEMORY_DESCRIPTOR), &bytesWritten, 0);
	    rangeCount++;
	  }

	  didProcess = true;
	  break;
	}
	streamDir++;
      }
      UnmapViewOfFile (file);
    }

    CloseHandle (mapping);
  }

  return didProcess;
}

csWeakRef<iObjectRegistry> cswinMinidumpWriter::object_reg;
cswinMinidumpWriter::FnCrashMinidumpHandler cswinMinidumpWriter::customHandler = 0;
LPTOP_LEVEL_EXCEPTION_FILTER cswinMinidumpWriter::oldFilter = 0;

const char* cswinMinidumpWriter::WriteMinidump (
  PMINIDUMP_EXCEPTION_INFORMATION except, bool dumpHeap)
{
  DbgHelp::IncRef();
  if (!DbgHelp::MinidumpAvailable())
  {
    DbgHelp::DecRef();
    return 0;
  }

  char tempPath[MAX_PATH - 13];
  GetTempPath (sizeof (tempPath), tempPath);

  char heapFN[MAX_PATH];
  char rangeFN[MAX_PATH];
  HANDLE hHeapDump = INVALID_HANDLE_VALUE;
  HANDLE hHeapRanges = INVALID_HANDLE_VALUE;
  int rangeCount = 0;

  if (dumpHeap)
  {
    GetTempFileName (tempPath, "dhp", 0, heapFN);

    hHeapDump = CreateFile (heapFN, GENERIC_READ | GENERIC_WRITE, 
      FILE_SHARE_READ, 0, CREATE_ALWAYS, 
      FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED,
      0);

    GetTempFileName (tempPath, "dhr", 0, rangeFN);

    hHeapRanges = CreateFile (rangeFN, GENERIC_READ | GENERIC_WRITE, 
      FILE_SHARE_READ, 0, CREATE_ALWAYS, 
      FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED,
      0);

    HANDLE snap = CreateToolhelp32Snapshot (TH32CS_SNAPHEAPLIST, 0);

    if ((hHeapRanges != INVALID_HANDLE_VALUE) && 
      (hHeapDump != INVALID_HANDLE_VALUE) &&
      (snap != INVALID_HANDLE_VALUE))
    {
      CollectHeapInfo (snap, hHeapDump, hHeapRanges, rangeCount);

      CloseHandle (snap);
    }
    else
    {
      if (hHeapRanges != INVALID_HANDLE_VALUE) 
      {
	CloseHandle (hHeapRanges);
	DeleteFileA (rangeFN);
      }
      if (hHeapDump != INVALID_HANDLE_VALUE) 
      {
	CloseHandle (hHeapDump);
	DeleteFileA (heapFN);
      }
      if (snap != INVALID_HANDLE_VALUE) 
	CloseHandle (snap);

      dumpHeap = false;
    }
  }

  char dumpFN[MAX_PATH];
  GetTempFileName (tempPath, "dmp", 0, dumpFN);

  HANDLE dumpFile = CreateFile (dumpFN, GENERIC_READ | GENERIC_WRITE, 
    FILE_SHARE_READ, 0, CREATE_ALWAYS, 
    FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED,
    0);

  /*
    @@@ The callstack for the thread we were called from is kinda crappy,
    (IP somewhere in the kernel), maybe replace it with a stack that at
    least points to this function at some point.

    Stack is fine when exception information is passed in.
   */

  bool dumpSucceeded = DbgHelp::MiniDumpWriteDump (GetCurrentProcess(),
    GetCurrentProcessId(), dumpFile,
    MiniDumpWithDataSegs | MiniDumpScanMemory/* | 
    MiniDumpWithIndirectlyReferencedMemory*/,
    except, 0, 0);

  if (dumpHeap && PostprocessMiniDump (dumpFile, hHeapRanges, rangeCount))
  {
    SetFilePointer (dumpFile, 0, 0, FILE_END);
    DWORD bytesWritten;

    char buf[0x40000];
    DWORD bytesRead;

    // Write the mem list to the dump end
    SetFilePointer (hHeapRanges, 0, 0, FILE_BEGIN);
    ULONG32 rangeNum = rangeCount;
    WriteFile (dumpFile, &rangeNum, sizeof (ULONG32), &bytesWritten, 0);
    while (1)
    {
      ReadFile (hHeapRanges, buf, sizeof (buf), &bytesRead, 0);
      if (bytesRead == 0) break;
    
      WriteFile (dumpFile, buf, bytesRead, &bytesWritten, 0);
      if (bytesWritten == 0) break;
    }

    // Copy over the actual memory data
    SetFilePointer (hHeapDump, 0, 0, FILE_BEGIN);
    while (1)
    {
      ReadFile (hHeapDump, buf, sizeof (buf), &bytesRead, 0);
      if (bytesRead == 0) break;
    
      WriteFile (dumpFile, buf, bytesRead, &bytesWritten, 0);
      if (bytesWritten == 0) break;
    }
  }

  CloseHandle (dumpFile);

  if (dumpHeap)
  {
    CloseHandle (hHeapDump);
    DeleteFileA (heapFN);
    CloseHandle (hHeapRanges);
    DeleteFileA (rangeFN);
  }

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

static void CopyPhysicalToArchive (const char* fn, csArchive* archive, 
				   const char* entryName)
{
  HANDLE srcFile = CreateFile (fn, GENERIC_READ, 
    FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 
    FILE_FLAG_SEQUENTIAL_SCAN, 0);

  if (srcFile != INVALID_HANDLE_VALUE)
  {
    void* entry = archive->NewFile (entryName, GetFileSize (srcFile, 0));

    char buf[0x40000];
    DWORD bytesRead;

    while (1)
    {
      ReadFile (srcFile, buf, sizeof (buf), &bytesRead, 0);
      if (bytesRead == 0) break;
    
      archive->Write (entry, buf, bytesRead);
    }

    CloseHandle (srcFile);
  }
}

const char* cswinMinidumpWriter::WriteWrappedMinidump (
  iObjectRegistry* object_reg, PMINIDUMP_EXCEPTION_INFORMATION except, 
  bool dumpHeap)
{
  CONTEXT context (*except->ExceptionPointers->ContextRecord);
  csCallStack* stack = cswinCallStackHelper::CreateCallStack (
    GetCurrentProcess(), GetCurrentThread(), context,
    -1);
  const char* dumpFileName = cswinMinidumpWriter::WriteMinidump (except);

  if (dumpFileName != 0)
  {
    char tempPath[MAX_PATH];
    GetTempPath (sizeof (tempPath), tempPath);
    static csStringFast<MAX_PATH+32> reportName;
    reportName.Format ("%s\\cscrash%" PRIu32 ".zip", tempPath, 
      (uint32)GetCurrentProcessId ());

    csArchive* reportZip = new csArchive (reportName);
    CopyPhysicalToArchive (dumpFileName, reportZip, "crash.dmp");
    reportZip->Flush ();

    DeleteFileA (dumpFileName);

    if (object_reg)  
    {
      csRef<iVFS> vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
      csRef<iStandardReporterListener> stdrep = CS_QUERY_REGISTRY (object_reg,
	iStandardReporterListener);
      if (vfs && stdrep)
      {
	csRef<iDataBuffer> realConPath = 
	  vfs->GetRealPath (stdrep->GetDebugFile ());
	CopyPhysicalToArchive ((char*)realConPath->GetData(), reportZip, 
	  "console.txt");
      }
    }
    reportZip->Flush ();

    if (stack)
    {
      void* callstackEntry = reportZip->NewFile ("callstack.txt");
      csString s;
      csString line;
      for (size_t i = 0; i < stack->GetEntryCount(); i++)
      {
	line.Clear();
	bool hasFunc = stack->GetFunctionName (i, s);
	line = hasFunc ? s : "<unknown>";
	if (stack->GetLineNumber (i, s))
	  line << " @" << s;
	if (stack->GetParameters (i, s))
	  line << " (" << s << ")";
	line << "\n";

	reportZip->Write (callstackEntry, line.GetData(), 
	  (size_t)line.Length());
      }
      reportZip->Flush ();
      stack->Free();
    }

    delete reportZip;

    return reportName;
  }
  return 0;
}

//#define TEST_EXCEPTION_HANDLER

LONG WINAPI cswinMinidumpWriter::ExceptionFilter (
  struct _EXCEPTION_POINTERS* ExceptionInfo)
{
#ifdef TEST_EXCEPTION_HANDLER
  MessageBoxA (0, "Attach debugger now", "Exception handler test", 
    MB_OK | MB_ICONWARNING);
#endif

  LONG ret = EXCEPTION_EXECUTE_HANDLER;

  static bool nest = false;
  if (nest) return ret;

  nest = true;
 
  MINIDUMP_EXCEPTION_INFORMATION mei;
  mei.ClientPointers = true;
  mei.ExceptionPointers = ExceptionInfo;
  mei.ThreadId = GetCurrentThreadId();
  const char* dumpFileName = WriteWrappedMinidump (object_reg, &mei);

  if (dumpFileName)
  {
    if (customHandler)
    {
      customHandler (dumpFileName);
    }
    else
    {
      csString buf;
      buf.Format ("The application crashed. Dump has been written to %s", 
	dumpFileName);
      MessageBoxA (0, buf, 0, MB_OK | MB_ICONERROR);
    }
  }

#ifdef CS_DEBUG
  if (oldFilter != 0)
    ret = oldFilter (ExceptionInfo);
  nest = false;
  return ret;
#else
  ExitProcess (0xb4dc0de);
  nest = false;
  return ret;
#endif
}

void cswinMinidumpWriter::EnableCrashMinidumps (FnCrashMinidumpHandler handler)
{
  if (oldFilter == 0)
  {
    oldFilter = SetUnhandledExceptionFilter (&ExceptionFilter);
  }
  customHandler = handler;
}

void cswinMinidumpWriter::SetCrashMinidumpObjectReg (iObjectRegistry* object_reg)
{
  cswinMinidumpWriter::object_reg = object_reg;
}

void cswinMinidumpWriter::DisableCrashMinidumps ()
{
  if (oldFilter != 0)
    SetUnhandledExceptionFilter (oldFilter);
  oldFilter = 0;
}
