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
#include "csutil/util.h"
#include "csutil/sysfunc.h"
#include "csutil/csstring.h"
#include "callstack.h"
#include "csutil/win32/callstack.h"
#include "csutil/win32/wintools.h"

#include <windows.h>
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

struct SymCallbackInfo
{
  csDirtyAccessArray<cswinCallStack::StackEntry::Param>* params;
  uint8* paramOffset;
};

csStringSet cswinCallStack::strings;

BOOL CALLBACK cswinCallStack::EnumSymCallback (SYMBOL_INFO* pSymInfo,
  ULONG SymbolSize, PVOID UserContext)
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

void cswinCallStack::AddFrame (const STACKFRAME64& frame, 
			       csDirtyAccessArray<StackEntry>& entries)
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

  SetLastError (ERROR_SUCCESS);
  bool result = DbgHelp::SymSetContext (symInit.GetSymProcessHandle (), 
    &stackFrame, 0);
  if (!result)
  {
    // Bit hackish: if SymSetContext() failed, scan the loaded DLLs
    // and try to load their debug info.
    RescanModules ();
    SetLastError (ERROR_SUCCESS);
    result = DbgHelp::SymSetContext (symInit.GetSymProcessHandle (), 
      &stackFrame, 0);
  }
  if (result)
  {
    str.Clear ();
    SymCallbackInfo callbackInfo;
    csDirtyAccessArray<StackEntry::Param> params;
    callbackInfo.params = &params;
    callbackInfo.paramOffset = (uint8*)(LONG_PTR)(frame.AddrStack.Offset - 8);
    if (DbgHelp::SymEnumSymbols (symInit.GetSymProcessHandle (), 
      0
      /*DbgHelp::SymGetModuleBase64(GetCurrentProcess(),frame.AddrPC.Offset)*/,
      "*", &EnumSymCallback, &callbackInfo))
    {
      const size_t n = params.Length();
      entry.params = new StackEntry::Param[n + 1];
      memcpy (entry.params, params.GetArray(), n * sizeof (StackEntry::Param));
      entry.params[n].name = 0;
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
  if (entries == 0) return 0;
  size_t n = 0;
  while (entries[n].instrPtr != (uintptr_t)~0) n++;
  return n;
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
  if (!DbgHelp::SymFromAddr (symInit.GetSymProcessHandle (),
    entries[num].instrPtr, &displace, symbolInfo))
  {
    // Bit hackish: if SymFromAddr() failed, scan the loaded DLLs
    // and try to load their debug info.
    RescanModules ();
    DbgHelp::SymFromAddr (symInit.GetSymProcessHandle (),
      entries[num].instrPtr, &displace, symbolInfo);
  }

  IMAGEHLP_MODULE64 module;
  memset (&module, 0, sizeof (IMAGEHLP_MODULE64));
  module.SizeOfStruct = sizeof (IMAGEHLP_MODULE64);
  DbgHelp::SymGetModuleInfo64 (symInit.GetSymProcessHandle (),
    entries[num].instrPtr, &module);

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
  if (entries[num].params == 0) return false;

  str.Clear();
  StackEntry::Param* param = entries[num].params;
  bool first = true;
  while (param->name != 0)
  {
    if (!first) { str << ", "; } else { first = false; }
    str << strings.Request (param->name);
    str << " = ";
    char tmp[23];
    uint32 data = param->value;
    sprintf (tmp, "%d(0x%.8x)", data, data);
    str << tmp;
    param++;
  }
  return true;
}

class CurrentThreadContextHelper
{
public:
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
  if (mutex != 0)
    CloseHandle (mutex);
}

/*
  When this #define is enabled, the current thread context will not have the
  instruction pointer somewhere in the kernel. It seems that with WinXP SP2
  the shipped dbghelp.dll can't grok the kernel stack, so we work around this
  by avoiding the kernel stack :P However, the approach uses some dubious
  home-grown thread synchronization, so a safe version is supplied (and 
  activate by not #defining AVOID_CONTEXT_IN_KERNEL). 
 */
#define AVOID_CONTEXT_IN_KERNEL

#ifdef AVOID_CONTEXT_IN_KERNEL
int contextGettingState;
#endif

DWORD CurrentThreadContextHelper::ContextThread (LPVOID lpParameter)
{
  ContextThreadParams& params = *((ContextThreadParams*)lpParameter);

  while (true)
  {
    WaitForSingleObject (params.evStartWork, INFINITE);
    if (params.context == 0)
      return 0;

#ifdef AVOID_CONTEXT_IN_KERNEL
    do
    {
      Sleep (0);
    }
    while (contextGettingState < 1);
#endif
    SuspendThread (params.CallingThread);
    GetThreadContext (params.CallingThread, params.context);
#ifdef AVOID_CONTEXT_IN_KERNEL
    contextGettingState = 2;
#endif
    ResumeThread (params.CallingThread);

    SetEvent (params.evFinishedWork);
  }
}

static CurrentThreadContextHelper contextHelper;

csCallStack* cswinCallStackHelper::CreateCallStackThreaded (int skip)
{
  const int currentContextSkip = 
#ifdef AVOID_CONTEXT_IN_KERNEL
    1;	// Skip CreateCallStack()
#else
    2;	// Skip one more to also hide SuspendThread()
#endif
  HANDLE hProc = symInit.GetSymProcessHandle ();
  HANDLE hThread = GetCurrentThread ();

  CONTEXT context;
  memset (&context, 0, sizeof (context));
  context.ContextFlags = CONTEXT_FULL;

  // This 'snippet' gets the context for the current thread
  {
    WaitForSingleObject (contextHelper.mutex, INFINITE);

    /* GetThreadContext() doesn't work reliably for the current thread, so do
     * it from another thread while the real current one is suspended. */
    if (contextHelper.hThread == 0)
    {
      contextHelper.evStartWork = CreateEvent (0, false, false, 0);
      if (contextHelper.evStartWork == 0)
      {
	ReleaseMutex (contextHelper.mutex);
	return 0;
      }
      contextHelper.evFinishedWork = CreateEvent (0, false, false, 0);
      if (contextHelper.evFinishedWork == 0)
      {
	ReleaseMutex (contextHelper.mutex);
	return 0;
      }

      contextHelper.params.evStartWork = contextHelper.evStartWork;
      contextHelper.params.evFinishedWork = contextHelper.evFinishedWork;

      DWORD ThreadID;
      contextHelper.hThread = CreateThread (0, 0, contextHelper.ContextThread, 
	&contextHelper.params, 0, &ThreadID);
      if (contextHelper.hThread == 0)
      {
	ReleaseMutex (contextHelper.mutex);
	return 0;
      }
    }

    HANDLE ThisThread;
    if (!DuplicateHandle (GetCurrentProcess (), 
      GetCurrentThread (), GetCurrentProcess (), &ThisThread,
      0, false, DUPLICATE_SAME_ACCESS))
    {
      ReleaseMutex (contextHelper.mutex);
      return 0;
    }

    contextHelper.params.CallingThread = ThisThread;
    contextHelper.params.context = &context;

  #ifdef AVOID_CONTEXT_IN_KERNEL
    contextGettingState = 0;
  #endif
    if (!SetEvent (contextHelper.evStartWork))
    {
      ReleaseMutex (contextHelper.mutex);
      return 0;
    }

  #ifdef AVOID_CONTEXT_IN_KERNEL
    contextGettingState = 1;

    do
    {
    }
    while (contextGettingState < 2);
  #else
    WaitForSingleObject (contextHelper.evFinishedWork, INFINITE);
  #endif

    CloseHandle (ThisThread);

    ReleaseMutex (contextHelper.mutex);
  }

  return CreateCallStack (hProc, hThread, context, skip + currentContextSkip); 
}

class CriticalSectionWrapper
{
  CRITICAL_SECTION cs;
public:
  CriticalSectionWrapper()
  { InitializeCriticalSection (&cs); }
  ~CriticalSectionWrapper()
  { DeleteCriticalSection (&cs); }
  void Enter()  
  { EnterCriticalSection (&cs); }
  void Leave()  
  { LeaveCriticalSection (&cs); }
};

static CriticalSectionWrapper ExceptStackSection;
static CONTEXT* currentContextPtr;

static LONG WINAPI ExceptionFilter (struct _EXCEPTION_POINTERS* ExceptionInfo)
{
  *((CONTEXT*)ExceptionInfo->ExceptionRecord->ExceptionInformation[0]) = 
    *(ExceptionInfo->ContextRecord);
  return EXCEPTION_CONTINUE_EXECUTION;
}

// Work around lack of ULONG_PTR on older platform SDKs
#if !defined(__int3264) || (_MSC_VER < 1300)
#undef ULONG_PTR
#define ULONG_PTR ULONG
#endif

/* A slightly odd method to get a thread context, deliberately raising an
 * exception to get to the exception handler... but a bit quicker than the "use
 * 2nd thread" method. */
csCallStack* cswinCallStackHelper::CreateCallStackExcept (int skip)
{
  ExceptStackSection.Enter();
    
  CONTEXT context;
  memset (&context, 0, sizeof (context));
  currentContextPtr = &context;

  LPTOP_LEVEL_EXCEPTION_FILTER oldFilter = 
    SetUnhandledExceptionFilter (&ExceptionFilter);
  ULONG_PTR contextPtr = (ULONG_PTR)&context;

  RaiseException (0, 0, 1, &contextPtr);
  SetUnhandledExceptionFilter (oldFilter);

  ExceptStackSection.Leave();

  HANDLE hProc = symInit.GetSymProcessHandle ();
  HANDLE hThread = GetCurrentThread ();
  return CreateCallStack (hProc, hThread, context, skip + 3); 
};

typedef BOOL (WINAPI* PFNISDEBUGGERPRESENT)();

csCallStack* csCallStackHelper::CreateCallStack (int skip)
{
  static PFNISDEBUGGERPRESENT IsDebuggerPresent = 0;
  static HMODULE hKernel32 = 0;

  if (hKernel32 == 0)
  {
    hKernel32 = GetModuleHandle ("kernel32.dll");
    IsDebuggerPresent = (PFNISDEBUGGERPRESENT)GetProcAddress (hKernel32,
      "IsDebuggerPresent");
  }

  if (!IsDebuggerPresent || IsDebuggerPresent())
    return cswinCallStackHelper::CreateCallStackThreaded (skip);
  else
    return cswinCallStackHelper::CreateCallStackExcept (skip);
}

csCallStack* cswinCallStackHelper::CreateCallStack (HANDLE hProc, 
						    HANDLE hThread,
						    CONTEXT& context, 
						    int skip)
{
  if (!DbgHelp::SymSupportAvailable()) return 0;

  symInit.Init ();

  DWORD machineType;
  STACKFRAME64 frame;
  memset (&frame, 0, sizeof (frame));
#if defined(CS_PROCESSOR_X86)
#if (CS_PROCESSOR_SIZE == 32)
  machineType = IMAGE_FILE_MACHINE_I386;
  frame.AddrPC.Offset = context.Eip;
  frame.AddrPC.Mode = AddrModeFlat;
  frame.AddrStack.Offset = context.Esp;
  frame.AddrStack.Mode = AddrModeFlat;
  frame.AddrFrame.Offset = context.Ebp;
  frame.AddrFrame.Mode = AddrModeFlat;
#elif (CS_PROCESSOR_SIZE == 64)
  machineType = IMAGE_FILE_MACHINE_AMD64;
  frame.AddrPC.Offset = context.Rip;
  frame.AddrPC.Mode = AddrModeFlat;
  frame.AddrStack.Offset = context.Rsp;
  frame.AddrStack.Mode = AddrModeFlat;
  frame.AddrFrame.Offset = context.Rbp;
  frame.AddrFrame.Mode = AddrModeFlat;
  /* @@@ Code below is for IA64.
  // Reference: http://www.mcse.ms/archive108-2003-11-97141.html
  machineType = IMAGE_FILE_MACHINE_IA64;
  frame.AddrPC.Offset = context.StIIP;
  frame.AddrPC.Mode = AddrModeFlat;
  frame.AddrStack.Offset = context.IntSp;
  frame.AddrStack.Mode = AddrModeFlat;
  frame.AddrBStore.Offset = context.RsBSP;
  frame.AddrBStore.Mode = AddrModeFlat;
  */
#else // CS_PROCESSOR_SIZE
#error Do not know how to stack walk for your processor word size.
#endif // CS_PROCESSOR_SIZE
#else // CS_PROCESSOR_FOO
#error Do not know how to stack walk for your platform.
#endif // CS_PROCESSOR_FOO

  int count = 0;
  csDirtyAccessArray<cswinCallStack::StackEntry> entries;
  cswinCallStack* stack = new cswinCallStack;
  while (DbgHelp::StackWalk64 (machineType, hProc,
    hThread, &frame, &context, 0, DbgHelp::SymFunctionTableAccess64, 
    DbgHelp::SymGetModuleBase64, 0))
  {
    count++;
    if (count > skip) stack->AddFrame (frame, entries);
  }
  const size_t n = entries.Length();
  if (n > 0)
  {
    stack->entries = new cswinCallStack::StackEntry[n + 1];
    memcpy (stack->entries, entries.GetArray(), 
      sizeof (cswinCallStack::StackEntry) * n);
  }

  return stack;
}
