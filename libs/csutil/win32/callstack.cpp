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
#include "csutil/csstring.h"
#include "csutil/csuctransform.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/sysfunc.h"
#include "csutil/timemeasure.h"
#include "csutil/util.h"
#include "callstack.h"
#include "csutil/win32/callstack.h"
#include "csutil/win32/wintools.h"

#include <windows.h>
#include <tlhelp32.h>

//#define CALLSTACK_PROFILE

#ifdef CALLSTACK_PROFILE
#define MEASURE_FUNCTION csMeasureTime measureLocal ("%s", CS_FUNCTION_NAME)
#define MEASURE_INTERMEDIATE measureLocal.PrintIntermediate (" %d", __LINE__)
#else
#define MEASURE_FUNCTION
#define MEASURE_INTERMEDIATE
#endif

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

struct SymCallbackCountInfo
{
  csDirtyAccessArray<uintptr_t>* params;
  uint8* paramOffset;
  size_t count;
};

static BOOL CALLBACK EnumSymCallbackCount (SYMBOL_INFO* pSymInfo,
  ULONG SymbolSize, PVOID UserContext)
{
  if ((pSymInfo->Flags & SYMFLAG_PARAMETER) != 0)
  {
    SymCallbackCountInfo* info = (SymCallbackCountInfo*)UserContext;

    uintptr_t data = *((uintptr_t*)(info->paramOffset + pSymInfo->Address));
    info->params->Push (data);
    info->count++;
  }

  return TRUE;
}

static size_t GetParams (const STACKFRAME64& frame, 
                         csDirtyAccessArray<uintptr_t>& params)
{
  MEASURE_FUNCTION;

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
    SymCallbackCountInfo callbackInfo;
    callbackInfo.params = &params;
    callbackInfo.paramOffset = (uint8*)(LONG_PTR)(frame.AddrStack.Offset - 8);
    callbackInfo.count = 0;
    if (DbgHelp::SymEnumSymbols (symInit.GetSymProcessHandle (), 
      0
      /*DbgHelp::SymGetModuleBase64(GetCurrentProcess(),frame.AddrPC.Offset)*/,
      "*", &EnumSymCallbackCount, &callbackInfo))
      return callbackInfo.count;
  }
  else
  {
    DWORD err = GetLastError ();
    if (err != ERROR_SUCCESS)
      PrintError ("SymSetContext: %s", cswinGetErrorMessage (err));
  }
  return csParamUnknown;
}

static bool CreateCallStack (HANDLE hProc, HANDLE hThread, CONTEXT& context,
                             int skip, bool fast,
                             csDirtyAccessArray<CallStackEntry>& entries, 
                             csDirtyAccessArray<uintptr_t>& params)
{
  MEASURE_FUNCTION;
  if (!DbgHelp::SymSupportAvailable()) return false;

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
  while (DbgHelp::StackWalk64 (machineType, hProc,
    hThread, &frame, &context, 0, DbgHelp::SymFunctionTableAccess64, 
    DbgHelp::SymGetModuleBase64, 0))
  {
    count++;
    if (count > skip)
    {
      CallStackEntry entry;
      entry.address = (void*)(uintptr_t)frame.AddrPC.Offset;
      entry.paramOffs = params.Length();
      entry.paramNum = fast ? csParamUnknown : GetParams (frame, params) ;
      entries.Push (entry);
    }
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

static bool CreateCallStackThreaded (int skip, bool fast,
                                     csDirtyAccessArray<CallStackEntry>& entries, 
                                     csDirtyAccessArray<uintptr_t>& params)
{
  const int currentContextSkip = 
#ifdef AVOID_CONTEXT_IN_KERNEL
    1;  // Skip CreateCallStack()
#else
    2;  // Skip one more to also hide SuspendThread()
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

  return CreateCallStack (hProc, hThread, context, skip + currentContextSkip,
    fast, entries, params); 
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
static bool CreateCallStackExcept (int skip, bool fast,
                                   csDirtyAccessArray<CallStackEntry>& entries,
                                   csDirtyAccessArray<uintptr_t>& params)
{
  ExceptStackSection.Enter();
    
  CONTEXT context;
  memset (&context, 0, sizeof (context));
  currentContextPtr = &context;

  LPTOP_LEVEL_EXCEPTION_FILTER oldFilter = 
    SetUnhandledExceptionFilter (&ExceptionFilter);
  ULONG_PTR contextPtr = (ULONG_PTR)&context;

  RaiseException (0, 0, 1, &contextPtr);
  /* HACK: At this point we have the CONTEXT of the backtrace, however, we
   * subsequently call a couple of functions, with the risk that those
   * clobber the stack space where precious return addresses from
   * RaiseException() lie. To avoid losing those addresses, "protect" that
   * space on the stack by allocating a small block of memory that hopefully
   * covers the area in question.
   */
  alloca (sizeof(ULONG_PTR) * 16);
  SetUnhandledExceptionFilter (oldFilter);

  ExceptStackSection.Leave();

  HANDLE hProc = symInit.GetSymProcessHandle ();
  HANDLE hThread = GetCurrentThread ();
  return CreateCallStack (hProc, hThread, context, skip + 3, fast, entries, params); 
}

csCallStack* cswinCallStackHelper::CreateCallStack (HANDLE hProc, 
                                                    HANDLE hThread, 
                                                    CONTEXT& context,
                                                    int skip, bool fast)
{
  skip += 1; /* Adjust for this function */
  csCallStackImpl* stack = new csCallStackImpl();
  if (::CreateCallStack (hProc, hThread, context, skip, fast, stack->entries,
    stack->params))
    return stack;
  delete stack;
  return 0;
}

typedef BOOL (WINAPI* PFNISDEBUGGERPRESENT)();

bool csCallStackCreatorDbgHelp::CreateCallStack (
  csDirtyAccessArray<CallStackEntry>& entries, 
  csDirtyAccessArray<uintptr_t>& params, bool fast)
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
    return ::CreateCallStackThreaded (0, fast, entries, params);
  else
    return ::CreateCallStackExcept (0, fast, entries, params);
}

bool csCallStackNameResolverDbgHelp::GetAddressSymbol (void* addr, 
  csString& str)
{
  static const int MaxSymbolLen = 512;
  static const int symbolInfoSize = 
    sizeof (SYMBOL_INFO) + (MaxSymbolLen - 1) * sizeof(char);
  static uint8 _symbolInfo[symbolInfoSize];
  static PSYMBOL_INFO symbolInfo = (PSYMBOL_INFO)&_symbolInfo;

  memset (symbolInfo, 0, symbolInfoSize);
  symbolInfo->SizeOfStruct = sizeof (SYMBOL_INFO);
  symbolInfo->MaxNameLen = MaxSymbolLen;
  uint64 displace;
  if (!DbgHelp::SymFromAddr (symInit.GetSymProcessHandle (),
    (uintptr_t)addr, &displace, symbolInfo))
  {
    // Bit hackish: if SymFromAddr() failed, scan the loaded DLLs
    // and try to load their debug info.
    RescanModules ();
    DbgHelp::SymFromAddr (symInit.GetSymProcessHandle (),
      (uintptr_t)addr, &displace, symbolInfo);
  }

  IMAGEHLP_MODULEW64 module;
  memset (&module, 0, sizeof (IMAGEHLP_MODULEW64));
  /* Oddity/bug?: newer dbghelp versions seem to always fill the 
   * IMAGEHLP_MODULE64 structure completely, even if a smaller size
   * was specified. However, we still set the size to that of an older,
   * smaller IMAGEHLP_MODULE64 with the hope for some backwards compatibility.
   */
  module.SizeOfStruct = ((uint8*)module.LoadedImageName) - ((uint8*)&module);
  DbgHelp::SymGetModuleInfoW64 (symInit.GetSymProcessHandle (),
    (uintptr_t)addr, &module);

  if (symbolInfo->Name[0] != 0)
  {
    str.Format ("[%p] (%ls)%s+0x%" CS_PRIx64,
      addr,
      (module.ImageName[0] != 0) ? module.ImageName : L"<unknown>",
      symbolInfo->Name, displace);
  }
  else
  {
    str.Format ("[%p] (%ls)<unknown>", addr,
      (module.ImageName[0] != 0) ? module.ImageName : L"<unknown>");
  }

  return true;
}

static BOOL CALLBACK EnumSymCallbackNames (SYMBOL_INFO* pSymInfo,
  ULONG SymbolSize, PVOID UserContext)
{
  if ((pSymInfo->Flags & SYMFLAG_PARAMETER) != 0)
  {
    csArray<csString>* info = (csArray<csString>*)UserContext;

    info->Push (pSymInfo->Name);
  }

  return TRUE;
}

void* csCallStackNameResolverDbgHelp::OpenParamSymbols (void* addr)
{
  IMAGEHLP_STACK_FRAME stackFrame;
  memset (&stackFrame, 0, sizeof (IMAGEHLP_STACK_FRAME));
  stackFrame.InstructionOffset = (uintptr_t)addr;

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
    csArray<csString>* names = new csArray<csString>;
    if (DbgHelp::SymEnumSymbols (symInit.GetSymProcessHandle (), 
      0
      /*DbgHelp::SymGetModuleBase64(GetCurrentProcess(),frame.AddrPC.Offset)*/,
      "*", &EnumSymCallbackNames, names))
      return names;
    else
      delete names;
  }
  else
  {
    DWORD err = GetLastError ();
    if (err != ERROR_SUCCESS)
      PrintError ("SymSetContext: %s", cswinGetErrorMessage (err));
  }
  return 0;
}

bool csCallStackNameResolverDbgHelp::GetParamName (void* h, size_t n, csString& str)
{
  csArray<csString>* names = (csArray<csString>*)h;
  if (n >=  names->Length()) return false;
  str = names->Get (n);
  return true;
}

void csCallStackNameResolverDbgHelp::FreeParamSymbols (void* h)
{
  csArray<csString>* names = (csArray<csString>*)h;
  delete names;
}

bool csCallStackNameResolverDbgHelp::GetLineNumber (void* addr, csString& str)
{
  IMAGEHLP_LINE64 line;
  memset (&line, 0, sizeof (IMAGEHLP_LINE64));
  line.SizeOfStruct = sizeof (IMAGEHLP_LINE64);
  DWORD displacement;
  if (DbgHelp::SymGetLineFromAddr64 (symInit.GetSymProcessHandle (), 
    (uintptr_t)addr, &displacement, &line))
  {
    str.Format ("%s:%" PRIu32, line.FileName, (uint32)line.LineNumber);
    return true;
  }
  return false;
}
