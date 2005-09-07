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
#include "csutil/callstack.h"

#include "callstack-backtrace.h"

#include <execinfo.h>

bool csCallStackCreatorBacktrace::CreateCallStack (
  csDirtyAccessArray<CallStackEntry>& entries, 
  csDirtyAccessArray<uintptr_t>& params, bool fast)
{
  void* traceBuffer[200];
  int count = backtrace (traceBuffer, sizeof (traceBuffer) / sizeof (void*));
  for (int i = 1; i < count; i++)
  {
    CallStackEntry entry;
    entry.address = traceBuffer[i];
    entry.paramNum = csParamUnknown;
    entries.Push (entry);
  }
  return true;
}

bool csCallStackNameResolverBacktrace::GetAddressSymbol (void* addr, 
  csString& sym)
{
  char** s = backtrace_symbols (&addr, 1);
  if (!s) return false;
  sym.Replace (s[0]);
  free(s);
  return true;
}

void* csCallStackNameResolverBacktrace::OpenParamSymbols (void* addr)
{
  return 0;
}

bool csCallStackNameResolverBacktrace::GetParamName (void*, size_t, csString&)
{
  return false;
}

void csCallStackNameResolverBacktrace::FreeParamSymbols (void* handle)
{
}

bool csCallStackNameResolverBacktrace::GetLineNumber (void*, csString&)
{
  return false;
}
