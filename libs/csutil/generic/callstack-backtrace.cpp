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

#include "../demangle.h"
#include "callstack-backtrace.h"

#include <execinfo.h>

namespace CrystalSpace
{
namespace Debug
{

bool CallStackCreatorBacktrace::CreateCallStack (
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

bool CallStackNameResolverBacktrace::GetAddressSymbol (void* addr, 
  csString& sym)
{
  char** s = backtrace_symbols (&addr, 1);
  if (!s) return false;
  sym = s[0];
  free(s);
  // Try demangling... for this, try to extract the symbol name from the line
  {
    size_t symStart = sym.FindFirst ('(');
    if (symStart != (size_t)-1)
    {
      symStart++;
      size_t symEnd = sym.FindFirst ("+)", symStart);
      if (symEnd != (size_t)-1)
      {
        csString tmp;
        sym.SubString (tmp, symStart, symEnd - symStart);
        // ...and replace with the demangled one
        CrystalSpace::Debug::Demangle (tmp, tmp);
        sym.DeleteAt (symStart, symEnd - symStart);
        sym.Insert (symStart, tmp);
      }
    }
  }
  return true;
}

void* CallStackNameResolverBacktrace::OpenParamSymbols (void* addr)
{
  return 0;
}

bool CallStackNameResolverBacktrace::GetParamName (void*, size_t, csString&)
{
  return false;
}

void CallStackNameResolverBacktrace::FreeParamSymbols (void* handle)
{
}

bool CallStackNameResolverBacktrace::GetLineNumber (void*, csString&)
{
  return false;
}

} // namespace Debug
} // namespace CrystalSpace

