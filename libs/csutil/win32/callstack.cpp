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
#include "csutil/util.h"
#include "csutil/sysfunc.h"
#include "csutil/csstring.h"
#include "callstack.h"
#include "csutil/win32/callstack.h"
#include "csutil/win32/wintools.h"

#include <tlhelp32.h>

static void PrintError (const char* format, ...)
{
  csPrintfErr ("wincallstack: ");
  va_list args;
  va_start (args, format);
  csPrintfErrV (format, args);
  va_end (args);
  csPrintfErr ("\n");
}

static void RescanModules ();

class SymInitializer
{
  bool inited;
  HANDLE hProc;
public:
  SymInitializer ()
  {
    inited = false;
    hProc = INVALID_HANDLE_VALUE;
  }
  ~SymInitializer ()
  {
    if (inited)
    {
      DbgHelp::SymCleanup (GetSymProcessHandle ());
      DbgHelp::DecRef();
    }
    if (hProc != INVALID_HANDLE_VALUE)
      CloseHandle (hProc);
  }
  void Init ()
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
  HANDLE GetSymProcessHandle ()
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
};

static SymInitializer symInit;

static void RescanModules ()
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
    if (DbgHelp::SymLoadModule64 (symInit.GetSymProcessHandle (), 0, me.szExePath,
      /*me.szExePath*/0, (LONG_PTR)me.modBaseAddr, 0) == 0)
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

struct SymCallbackInfo
{
  csArray<cswinCallStack::StackEntry::Param>* params;
  uint8* paramOffset;
};

csStringSet cswinCallStack::strings;

BOOL cswinCallStack::EnumSymCallback (SYMBOL_INFO* pSymInfo, ULONG SymbolSize, 
				      PVOID UserContext)
{
  if ((pSymInfo->Flags & SYMFLAG_PARAMETER) != 0)
  {
    SymCallbackInfo* info = (SymCallbackInfo*)UserContext;

    uint32 data = *((uint32*)(info->paramOffset + pSymInfo->Address));
    StackEntry::Param& param =
      info->params->GetExtend(info->params->Length ());
    param.name = strings.Request (pSymInfo->Name);
    param.value = data;
  }

  return TRUE;
}

void cswinCallStack::AddFrame (const STACKFRAME64& frame)
{
  csString str;
  StackEntry& entry = entries.GetExtend (entries.Length ());

  entry.instrPtr = frame.AddrPC.Offset;

  IMAGEHLP_STACK_FRAME stackFrame;
  memset (&stackFrame, 0, sizeof (IMAGEHLP_STACK_FRAME));
  stackFrame.InstructionOffset = frame.AddrPC.Offset;
  stackFrame.ReturnOffset = frame.AddrReturn.Offset;
  stackFrame.FrameOffset = frame.AddrFrame.Offset;
  stackFrame.StackOffset = frame.AddrStack.Offset;
  stackFrame.BackingStoreOffset = frame.AddrBStore.Offset;
  stackFrame.FuncTableEntry = (LONG_PTR)frame.FuncTableEntry;
  stackFrame.Params[0] = frame.Params[0];
  stackFrame.Params[1] = frame.Params[1];
  stackFrame.Params[2] = frame.Params[2];
  stackFrame.Params[3] = frame.Params[3];
  stackFrame.Virtual = frame.Virtual;

  entry.hasParams = false;
  bool result = DbgHelp::SymSetContext (symInit.GetSymProcessHandle (), 
    &stackFrame, 0);
  if (!result)
  {
    // Bit hackish: if SymSetContext() failed, scan the loaded DLLs
    // and try to load their debug info.
    RescanModules ();
    result = DbgHelp::SymSetContext (symInit.GetSymProcessHandle (), 
      &stackFrame, 0);
  }
  if (result)
  {
    str.Clear ();
    SymCallbackInfo callbackInfo;
    callbackInfo.params = &entry.params;
    callbackInfo.paramOffset = (uint8*)(LONG_PTR)(frame.AddrStack.Offset - 8);
    if (DbgHelp::SymEnumSymbols (symInit.GetSymProcessHandle (), 
      0
      /*DbgHelp::SymGetModuleBase64(GetCurrentProcess(),frame.AddrPC.Offset)*/,
      "*", &EnumSymCallback, &callbackInfo))
    {
      entry.hasParams = true;
    }
  }
  else
  {
    DWORD err = GetLastError ();
    if (err != ERROR_SUCCESS)
      PrintError ("SymSetContext: %s", cswinGetErrorMessage (err));
  }
}

size_t cswinCallStack::GetEntryCount ()
{
  return entries.Length();
}

bool cswinCallStack::GetFunctionName (size_t num, csString& str)
{
  str.Clear();

  static const int MaxSymbolLen = 512;
  static const int symbolInfoSize = sizeof (SYMBOL_INFO) + MaxSymbolLen - 1;
  static uint8 _symbolInfo[symbolInfoSize];
  static PSYMBOL_INFO symbolInfo = (PSYMBOL_INFO)&_symbolInfo;

  memset (symbolInfo, 0, symbolInfoSize);
  symbolInfo->SizeOfStruct = sizeof (SYMBOL_INFO);
  symbolInfo->MaxNameLen = MaxSymbolLen;
  uint64 displace;
  if (!DbgHelp::SymFromAddr (symInit.GetSymProcessHandle (), entries[num].instrPtr,
    &displace, symbolInfo))
  {
    // Bit hackish: if SymFromAddr() failed, scan the loaded DLLs
    // and try to load their debug info.
    RescanModules ();
    DbgHelp::SymFromAddr (symInit.GetSymProcessHandle (), entries[num].instrPtr,
      &displace, symbolInfo);
  }

  IMAGEHLP_MODULE64 module;
  memset (&module, 0, sizeof (IMAGEHLP_MODULE64));
  module.SizeOfStruct = sizeof (IMAGEHLP_MODULE64);
  DbgHelp::SymGetModuleInfo64 (symInit.GetSymProcessHandle (), entries[num].instrPtr,
    &module);

  if (symbolInfo->Name[0] != 0)
  {
    str.Format ("[%.8x] (%s)%s+0x%x", (unsigned int)entries[num].instrPtr,
      (module.ImageName[0] != 0) ? module.ImageName : "<unknown>",
      symbolInfo->Name, (unsigned int)displace);
  }
  else
  {
    str.Format ("[%.8x] (%s)<unknown>", (uint32)entries[num].instrPtr,
      (module.ImageName[0] != 0) ? module.ImageName : "<unknown>");
  }

  return true;
}

bool cswinCallStack::GetLineNumber (size_t num, csString& str)
{
  str.Clear();

  IMAGEHLP_LINE64 line;
  memset (&line, 0, sizeof (IMAGEHLP_LINE64));
  line.SizeOfStruct = sizeof (IMAGEHLP_LINE64);
  DWORD displacement;
  if (DbgHelp::SymGetLineFromAddr64 (symInit.GetSymProcessHandle (), 
    entries[num].instrPtr, &displacement, &line))
  {
    str.Format ("%s:%u", line.FileName, (uint)line.LineNumber);
    return true;
  }
  return false;
}

bool cswinCallStack::GetParameters (size_t num, csString& str)
{
  if (!entries[num].hasParams) return false;

  str.Clear();
  for (size_t i = 0; i < entries[num].params.Length(); i++)
  {
    if (i > 0) str << ", ";
    str << strings.Request (entries[num].params[i].name);
    str << " = ";
    char tmp[23];
    uint32 data = entries[num].params[i].value;
    sprintf (tmp, "%d(0x%.8x)", data, data);
    str << tmp;
  }
  return true;
}

class CurrentThreadContextHelper
{
  HANDLE mutex;

  struct ContextThreadParams
  {
    HANDLE evStartWork;
    HANDLE evFinishedWork;

    HANDLE CallingThread;
    CONTEXT* context;
  } params;
  HANDLE hThread;
  HANDLE evStartWork;
  HANDLE evFinishedWork;

  static DWORD WINAPI ContextThread (LPVOID lpParameter);
public:
  CurrentThreadContextHelper ();
  ~CurrentThreadContextHelper ();

  bool GetCurrentThreadContext (CONTEXT* context);
};

CurrentThreadContextHelper::CurrentThreadContextHelper ()
{
  mutex = CreateMutex (0, false, 0);
  hThread = evStartWork = evFinishedWork = 0;
}

CurrentThreadContextHelper::~CurrentThreadContextHelper ()
{
  if (hThread != 0)
  {
    params.context = 0;
    SetEvent (evStartWork);
    WaitForSingleObject (hThread, INFINITE);

    hThread = 0;
  }
  if (evStartWork != 0)
    CloseHandle (evStartWork);
  if (evFinishedWork != 0)
    CloseHandle (evFinishedWork);
}

DWORD CurrentThreadContextHelper::ContextThread (LPVOID lpParameter)
{
  ContextThreadParams& params = *((ContextThreadParams*)lpParameter);

  while (true)
  {
    WaitForSingleObject (params.evStartWork, INFINITE);

    if (params.context == 0)
      return 0;

    SuspendThread (params.CallingThread);
    GetThreadContext (params.CallingThread, params.context);
    ResumeThread (params.CallingThread);

    SetEvent (params.evFinishedWork);
  }
}

bool CurrentThreadContextHelper::GetCurrentThreadContext (CONTEXT* context)
{
  WaitForSingleObject (mutex, INFINITE);

  /* GetThreadContext() doesn't work reliably for the current thread, so do it
   * from another thread while the real current one is suspended. */
  if (hThread == 0)
  {
    evStartWork = CreateEvent (0, false, false, 0);
    if (evStartWork == 0)
    {
      ReleaseMutex (mutex);
      return false;
    }
    evFinishedWork = CreateEvent (0, false, false, 0);
    if (evFinishedWork == 0)
    {
      ReleaseMutex (mutex);
      return false;
    }

    params.evStartWork = evStartWork;
    params.evFinishedWork = evFinishedWork;

    DWORD ThreadID;
    hThread = CreateThread (0, 0, ContextThread, &params, 
      0, &ThreadID);
    if (hThread == 0)
    {
      ReleaseMutex (mutex);
      return false;
    }
  }

  HANDLE ThisThread;
  if (!DuplicateHandle (GetCurrentProcess (), 
    GetCurrentThread (), GetCurrentProcess (), &ThisThread,
    0, false, DUPLICATE_SAME_ACCESS))
  {
    ReleaseMutex (mutex);
    return false;
  }

  params.CallingThread = ThisThread;
  params.context = context;

  if (!SetEvent (evStartWork))
  {
    ReleaseMutex (mutex);
    return false;
  }
  WaitForSingleObject (evFinishedWork, INFINITE);

  CloseHandle (ThisThread);

  ReleaseMutex (mutex);
  return true;
}

static CurrentThreadContextHelper contextHelper;

csCallStack* csCallStackHelper::CreateCallStack (int skip)
{
  HANDLE hProc = symInit.GetSymProcessHandle ();
  HANDLE hThread = GetCurrentThread ();

  CONTEXT context;
  memset (&context, 0, sizeof (context));
  context.ContextFlags = CONTEXT_FULL;
  if (!contextHelper.GetCurrentThreadContext (&context))
    return 0;

  return cswinCallStackHelper::CreateCallStack (hProc, hThread, context, 
    skip + 1); // Always skip one more to hide GetCurrentThreadContext()
}

csCallStack* cswinCallStackHelper::CreateCallStack (HANDLE hProc, 
						    HANDLE hThread,
						    CONTEXT& context, 
						    int skip)
{
  if (!DbgHelp::SymSupportAvailable()) return 0;

  symInit.Init ();

  STACKFRAME64 frame;
  memset (&frame, 0, sizeof (frame));
#ifdef CS_PROCESSOR_X86
  frame.AddrPC.Offset = context.Eip;
  frame.AddrPC.Mode = AddrModeFlat;
  frame.AddrStack.Offset = context.Esp;
  frame.AddrStack.Mode = AddrModeFlat;
  frame.AddrFrame.Offset = context.Ebp;
  frame.AddrFrame.Mode = AddrModeFlat;
#else
  #error Don't know how to stack walk for your platform.
#endif

  int count = 0;
  cswinCallStack* stack = new cswinCallStack;
  while (DbgHelp::StackWalk64 (IMAGE_FILE_MACHINE_I386, hProc,
    hThread, &frame, &context, 0, DbgHelp::SymFunctionTableAccess64, 
    DbgHelp::SymGetModuleBase64, 0))
  {
    count++;
    // Always skip the first entry (this func)
    if (count > (skip + 1)) stack->AddFrame (frame);
  }

  return stack;
}
