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
#include "csutil/csstring.h"
#include "cachedll.h"
#include "callstack.h"

#include <tlhelp32.h>

/*
  DbgHelp API stuff.
  Unfortunately, dbghelp.h isn't included in MinGW. So all that's needed goes here.
 */

enum ADDRESS_MODE
{
  AddrMode1616,
  AddrMode1632,
  AddrModeReal,
  AddrModeFlat
};

struct ADDRESS64 
{
  uint64 Offset;
  WORD Segment;
  ADDRESS_MODE Mode;
};
typedef ADDRESS64* LPADDRESS64;

struct KDHELP64 
{  
  uint64 Thread;  
  DWORD ThCallbackStack;  
  DWORD ThCallbackBStore;  
  DWORD NextCallback;  
  DWORD FramePointer;  
  uint64 KiCallUserMode;  
  uint64 KeUserCallbackDispatcher;  
  uint64 SystemRangeStart;  
  uint64 Reserved[8];
};
typedef KDHELP64* PKDHELP64;

struct STACKFRAME64 
{  
  ADDRESS64 AddrPC;  
  ADDRESS64 AddrReturn;  
  ADDRESS64 AddrFrame;  
  ADDRESS64 AddrStack;  
  ADDRESS64 AddrBStore;  
  PVOID FuncTableEntry;  
  uint64 Params[4];  
  BOOL Far;  
  BOOL Virtual;  
  uint64 Reserved[3];  
  KDHELP64 KdHelp;
};
typedef STACKFRAME64* LPSTACKFRAME64;

struct SYMBOL_INFO 
{  
  ULONG SizeOfStruct;  
  ULONG TypeIndex;  
  uint64 Reserved[2];  
  ULONG Reserved2;  
  ULONG Size;  
  uint64 ModBase;  
  ULONG Flags;  
  uint64 Value;  
  uint64 Address;  
  ULONG Register;  
  ULONG Scope;  
  ULONG Tag;  
  ULONG NameLen;  
  ULONG MaxNameLen;  
  CHAR Name[1];
};
typedef SYMBOL_INFO* PSYMBOL_INFO;

#define SYMFLAG_PARAMETER        0x00000040

#define SYMOPT_UNDNAME                  0x00000002
#define SYMOPT_DEFERRED_LOADS           0x00000004
#define SYMOPT_LOAD_LINES               0x00000010
#define SYMOPT_FAIL_CRITICAL_ERRORS     0x00000200

enum SYM_TYPE
{
    SymNone = 0,
    SymCoff,
    SymCv,
    SymPdb,
    SymExport,
    SymDeferred,
    SymSym,
    SymDia,
    SymVirtual,
    NumSymTypes
};

struct IMAGEHLP_MODULE64 
{  
  DWORD SizeOfStruct;  
  uint64 BaseOfImage;  
  DWORD ImageSize;  
  DWORD TimeDateStamp;  
  DWORD CheckSum;  
  DWORD NumSyms;  
  SYM_TYPE SymType;  
  CHAR ModuleName[32];  
  CHAR ImageName[256];  
  CHAR LoadedImageName[256];  
  /*
    The following fields are only supported on newer versions of dbghelp.dll,
    but the versions shipped with W2k resp. WXP lack them.
   */
  /*CHAR LoadedPdbName[256];  
  DWORD CVSig;  
  CHAR CVData[MAX_PATH*3];  
  DWORD PdbSig;  
  GUID PdbSig70;  
  DWORD PdbAge;  
  BOOL PdbUnmatched;  
  BOOL DbgUnmatched;  
  BOOL LineNumbers;  
  BOOL GlobalSymbols;  
  BOOL TypeInfo;*/
};
typedef IMAGEHLP_MODULE64* PIMAGEHLP_MODULE64;

struct IMAGEHLP_LINE64
{  
  DWORD SizeOfStruct;  
  PVOID Key;  
  DWORD LineNumber;  
  PCHAR FileName;  
  uint64 Address;
};
typedef IMAGEHLP_LINE64* PIMAGEHLP_LINE64;

typedef BOOL (CALLBACK* PSYM_ENUMERATESYMBOLS_CALLBACK) (PSYMBOL_INFO pSymInfo,
  ULONG SymbolSize, PVOID UserContext);
typedef BOOL (CALLBACK* PSYM_ENUMMODULES_CALLBACK64) (PSTR ModuleName,
  uint64 BaseOfDll, PVOID UserContext);

struct IMAGEHLP_STACK_FRAME 
{  
  uint64 InstructionOffset;  
  uint64 ReturnOffset;  
  uint64 FrameOffset;  
  uint64 StackOffset;  
  uint64 BackingStoreOffset;  
  uint64 FuncTableEntry;  
  uint64 Params[4];  
  uint64 Reserved[5];  
  BOOL Virtual;  
  ULONG Reserved2;
};
typedef IMAGEHLP_STACK_FRAME* PIMAGEHLP_STACK_FRAME;
typedef void* PIMAGEHLP_CONTEXT;

/** 
 * Wrapper to dynamically load DbgHelp.dll.
 */
class DbgHelp
{
private:
  static cswinCacheDLL dll;
  static bool dllLoaded;
  static bool dllAvailable;
public:
  typedef BOOL (WINAPI* PREAD_PROCESS_MEMORY_ROUTINE64) (HANDLE hProcess,
    uint64 qwBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead);
  typedef PVOID (WINAPI* PFUNCTION_TABLE_ACCESS_ROUTINE64) (HANDLE hProcess,
    uint64 AddrBase);
  typedef uint64 (WINAPI* PGET_MODULE_BASE_ROUTINE64) (HANDLE  hProcess, 
    uint64 Address);
  typedef uint64 (WINAPI* PTRANSLATE_ADDRESS_ROUTINE64) (HANDLE hProcess,
    HANDLE hThread, LPADDRESS64 lpaddr);
  typedef BOOL (WINAPI* PFNStackWalk64) (DWORD MachineType, HANDLE hProcess, 
    HANDLE hThread, LPSTACKFRAME64 StackFrame, PVOID ContextRecord,  
    PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,  
    PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,  
    PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,  
    PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress);
  typedef BOOL (WINAPI* PFNSymInitialize) (HANDLE hProcess, PSTR UserSearchPath,
    BOOL fInvadeProcess);
  typedef BOOL (WINAPI* PFNSymCleanup) (HANDLE hProcess);
  typedef DWORD (WINAPI* PFNSymSetOptions) (DWORD SymOptions);
  typedef BOOL (WINAPI* PFNSymFromAddr) (HANDLE hProcess, uint64 Address, 
    uint64* Displacement, PSYMBOL_INFO Symbol);
  typedef PVOID (WINAPI* PFNSymFunctionTableAccess64) (HANDLE hProcess, 
    uint64 AddrBase);
  typedef uint64 (WINAPI* PFNSymGetModuleBase64) (HANDLE hProcess, uint64 dwAddr);
  typedef BOOL (WINAPI* PFNSymGetModuleInfo64) (HANDLE hProcess, uint64 dwAddr,
    PIMAGEHLP_MODULE64 ModuleInfo);
  typedef BOOL (WINAPI* PFNSymGetLineFromAddr64) (HANDLE hProcess, uint64 dwAddr,
    PDWORD pdwDisplacement, PIMAGEHLP_LINE64 Line);
  typedef BOOL (WINAPI* PFNSymEnumSymbols) (HANDLE hProcess, uint64 BaseOfDll,  
    PCSTR Mask, PSYM_ENUMERATESYMBOLS_CALLBACK EnumSymbolsCallback, 
    PVOID UserContext);
  typedef BOOL (WINAPI* PFNSymSetContext) (HANDLE hProcess, PIMAGEHLP_STACK_FRAME StackFrame,
    PIMAGEHLP_CONTEXT Context);
  typedef uint64 (WINAPI* PFNSymLoadModule64) (HANDLE hProcess, HANDLE hFile, 
    PSTR ImageName, PSTR ModuleName, uint64 BaseOfDll, DWORD SizeOfDll);

  static PFNStackWalk64 StackWalk64;
  static PFNSymInitialize SymInitialize;
  static PFNSymCleanup SymCleanup;
  static PFNSymSetOptions SymSetOptions;
  static PFNSymFromAddr SymFromAddr;
  static PFNSymFunctionTableAccess64 SymFunctionTableAccess64;
  static PFNSymGetModuleBase64 SymGetModuleBase64;
  static PFNSymGetModuleInfo64 SymGetModuleInfo64;
  static PFNSymGetLineFromAddr64 SymGetLineFromAddr64;
  static PFNSymEnumSymbols SymEnumSymbols;
  static PFNSymSetContext SymSetContext;
  static PFNSymLoadModule64 SymLoadModule64;
  static bool Available ();
};

cswinCacheDLL DbgHelp::dll ("dbghelp.dll");
bool DbgHelp::dllLoaded = false;
bool DbgHelp::dllAvailable = false;
DbgHelp::PFNStackWalk64 DbgHelp::StackWalk64 = 0;
DbgHelp::PFNSymInitialize DbgHelp::SymInitialize = 0;
DbgHelp::PFNSymCleanup DbgHelp::SymCleanup = 0;
DbgHelp::PFNSymSetOptions DbgHelp::SymSetOptions = 0;
DbgHelp::PFNSymFromAddr DbgHelp::SymFromAddr = 0;
DbgHelp::PFNSymFunctionTableAccess64 DbgHelp::SymFunctionTableAccess64 = 0;
DbgHelp::PFNSymGetModuleBase64 DbgHelp::SymGetModuleBase64 = 0;
DbgHelp::PFNSymGetModuleInfo64 DbgHelp::SymGetModuleInfo64 = 0;
DbgHelp::PFNSymGetLineFromAddr64 DbgHelp::SymGetLineFromAddr64 = 0;
DbgHelp::PFNSymEnumSymbols DbgHelp::SymEnumSymbols = 0;
DbgHelp::PFNSymSetContext DbgHelp::SymSetContext = 0;
DbgHelp::PFNSymLoadModule64 DbgHelp::SymLoadModule64 = 0;

bool DbgHelp::Available ()
{
  if (!dllLoaded)
  {
    dllLoaded = true;
    if (dll != 0)
    {
      StackWalk64 = (PFNStackWalk64)GetProcAddress (dll, "StackWalk64");
      SymInitialize = (PFNSymInitialize)GetProcAddress (dll, "SymInitialize");
      SymCleanup = (PFNSymCleanup)GetProcAddress (dll, "SymCleanup");
      SymSetOptions = (PFNSymSetOptions)GetProcAddress (dll, "SymSetOptions");
      SymFromAddr = (PFNSymFromAddr)GetProcAddress (dll, "SymFromAddr");
      SymFunctionTableAccess64 = 
	(PFNSymFunctionTableAccess64)GetProcAddress (dll, 
	"SymFunctionTableAccess64");
      SymGetModuleBase64 = 
	(PFNSymGetModuleBase64)GetProcAddress (dll, "SymGetModuleBase64");
      SymGetModuleInfo64 = 
	(PFNSymGetModuleInfo64)GetProcAddress (dll, "SymGetModuleInfo64");
      SymGetLineFromAddr64 = 
	(PFNSymGetLineFromAddr64)GetProcAddress (dll, "SymGetLineFromAddr64");
      SymEnumSymbols = 
	(PFNSymEnumSymbols)GetProcAddress (dll, "SymEnumSymbols");
      SymSetContext = (PFNSymSetContext)GetProcAddress (dll, "SymSetContext");
      SymLoadModule64 = 
	(PFNSymLoadModule64)GetProcAddress (dll, "SymLoadModule64");
      dllAvailable = (StackWalk64 != 0) && (SymInitialize != 0) &&
	(SymCleanup != 0) && (SymSetOptions != 0) && 
	(SymFunctionTableAccess64 != 0) && (SymGetModuleBase64 != 0) &&
	(SymGetModuleInfo64 != 0) && (SymGetLineFromAddr64 != 0) &&
	(SymEnumSymbols != 0) && (SymSetContext != 0) && 
	(SymLoadModule64 != 0);
    }
  }
  return dllAvailable;
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
    StackEntry::Param& param = info->params->GetExtend (info->params->Length ());
    param.name = strings.Request (pSymInfo->Name);//csStrNew (pSymInfo->Name);
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
  if (DbgHelp::SymSetContext (GetCurrentProcess (), &stackFrame, 0))
  {
    str.Clear ();
    SymCallbackInfo callbackInfo;
    callbackInfo.params = &entry.params;
    callbackInfo.paramOffset = (uint8*)(LONG_PTR)(frame.AddrStack.Offset - 8);
    if (DbgHelp::SymEnumSymbols (GetCurrentProcess (), 0,
      "*", &EnumSymCallback, &callbackInfo))
    {
      entry.hasParams = true;
    }
  }
}

int cswinCallStack::GetEntryCount ()
{
  return entries.Length();
}

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
    DbgHelp::SymLoadModule64 (GetCurrentProcess (), 0, me.szModule,
      me.szExePath, (LONG_PTR)me.modBaseAddr, 0);
    res = Module32Next (hSnap, &me);
  }
  CloseHandle (hSnap);
}

bool cswinCallStack::GetFunctionName (int num, csString& str)
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
  if (!DbgHelp::SymFromAddr (GetCurrentProcess (), entries[num].instrPtr,
    &displace, symbolInfo))
  {
    // Bit hackish: if SymFromAddr() failed, scan the loaded DLLs
    // and try to load their debug info.
    RescanModules ();
    DbgHelp::SymFromAddr (GetCurrentProcess (), entries[num].instrPtr,
      &displace, symbolInfo);
  }

  IMAGEHLP_MODULE64 module;
  memset (&module, 0, sizeof (IMAGEHLP_MODULE64));
  module.SizeOfStruct = sizeof (IMAGEHLP_MODULE64);
  DbgHelp::SymGetModuleInfo64 (GetCurrentProcess (), entries[num].instrPtr,
    &module);

  if (symbolInfo->Name[0] != 0)
  {
    str.Format ("[%.8x] (%s)%s+0x%x", (uint32)entries[num].instrPtr,
      (module.ImageName[0] != 0) ? module.ImageName : "<unknown>",
      symbolInfo->Name, (uint32)displace);
  }
  else
  {
    str.Format ("[%.8x] (%s)<unknown>", (uint32)entries[num].instrPtr,
      (module.ImageName[0] != 0) ? module.ImageName : "<unknown>");
  }

  return true;
}

bool cswinCallStack::GetLineNumber (int num, csString& str)
{
  str.Clear();

  IMAGEHLP_LINE64 line;
  memset (&line, 0, sizeof (IMAGEHLP_LINE64));
  line.SizeOfStruct = sizeof (IMAGEHLP_LINE64);
  DWORD displacement;
  if (DbgHelp::SymGetLineFromAddr64 (GetCurrentProcess (), 
    entries[num].instrPtr, &displacement, &line))
  {
    str.Format ("%s:%ud", line.FileName, (uint)line.LineNumber);
    return true;
  }
  return false;
}

bool cswinCallStack::GetParameters (int num, csString& str)
{
  if (!entries[num].hasParams) return false;

  str.Clear();
  for (int i = 0; i < entries[num].params.Length(); i++)
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

class SymInitializer
{
  bool inited;
public:
  SymInitializer ()
  {
    inited = false;
  }
  ~SymInitializer ()
  {
    if (inited)
    {
      DbgHelp::SymCleanup (GetCurrentProcess ());
    }
  }
  void Init ()
  {
    if (inited) return;
    inited = true;

    DbgHelp::SymInitialize (GetCurrentProcess (), 0, true);
    DbgHelp::SymSetOptions (SYMOPT_DEFERRED_LOADS | SYMOPT_FAIL_CRITICAL_ERRORS |
      SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
  }
};

static SymInitializer symInit;

csCallStack* csCallStackHelper::CreateCallStack (uint skip)
{
  if (!DbgHelp::Available()) return 0;

  symInit.Init ();

  HANDLE hProc = GetCurrentProcess ();
  HANDLE hThread = GetCurrentThread ();

  CONTEXT context;
  memset (&context, 0, sizeof (context));
  context.ContextFlags = CONTEXT_FULL;
  GetThreadContext (hThread, &context);
  STACKFRAME64 frame;
  memset (&frame, 0, sizeof (frame));
  frame.AddrPC.Offset = context.Eip;
  frame.AddrPC.Mode = AddrModeFlat;
  frame.AddrFrame.Offset = context.Ebp;
  frame.AddrFrame.Mode = AddrModeFlat;

  uint count = 0;
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
