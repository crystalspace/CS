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
#include "callstack.h"
#include "csutil/win32/callstack.h"

#include <tlhelp32.h>

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
    if (DbgHelp::SymEnumSymbols (GetCurrentProcess (), 
      0
      /*DbgHelp::SymGetModuleBase64(GetCurrentProcess(),frame.AddrPC.Offset)*/,
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
    str.Format ("%s:%u", line.FileName, (uint)line.LineNumber);
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
      DbgHelp::DecRef();
    }
  }
  void Init ()
  {
    if (inited) return;
    inited = true;

    DbgHelp::IncRef();
    DbgHelp::SymInitialize (GetCurrentProcess (), 0, true);
    DbgHelp::SymSetOptions (SYMOPT_DEFERRED_LOADS |
      SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
  }
};

static SymInitializer symInit;

csCallStack* csCallStackHelper::CreateCallStack (int skip)
{
  HANDLE hProc = GetCurrentProcess ();
  HANDLE hThread = GetCurrentThread ();

  CONTEXT context;
  memset (&context, 0, sizeof (context));
  context.ContextFlags = CONTEXT_FULL;
  GetThreadContext (hThread, &context);

  return cswinCallStackHelper::CreateCallStack (hProc, hThread, context, 
    skip);
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
  frame.AddrPC.Offset = context.Eip;
  frame.AddrPC.Mode = AddrModeFlat;
  frame.AddrFrame.Offset = context.Ebp;
  frame.AddrFrame.Mode = AddrModeFlat;

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
