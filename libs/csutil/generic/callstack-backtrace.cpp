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

#include "csutil/custom_new_disable.h"
#include <string>
#include "csutil/custom_new_enable.h"

namespace CS
{
namespace Debug
{

bool CallStackCreatorBacktrace::CreateCallStack (
  CallStackEntriesArray& entries, 
  CallStackParamsArray& /*params*/, bool /*fast*/)
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
  char*& sym)
{
  char** s = backtrace_symbols (&addr, 1);
  if (!s) return false;
  std::string symTmp;
  symTmp = s[0];
  free(s);
  // Try demangling... for this, try to extract the symbol name from the line
  {
    size_t symStart = symTmp.find ('(');
    if (symStart != (size_t)-1)
    {
      symStart++;
      size_t symEnd = symTmp.find ("+)", symStart);
      if (symEnd != (size_t)-1)
      {
        std::string tmp = symTmp.substr (symStart, symEnd - symStart);
        // ...and replace with the demangled one
        char* symDemangled = Demangle (tmp.c_str());
        symTmp.erase (symStart, symEnd - symStart);
        symTmp.insert (symStart, symDemangled);
        free (symDemangled);
      }
    }
  }
  sym = strdup (symTmp.c_str());
  return true;
}

void* CallStackNameResolverBacktrace::OpenParamSymbols (void* /*addr*/)
{
  return 0;
}

bool CallStackNameResolverBacktrace::GetParamName (void*, size_t, char*&)
{
  return false;
}

void CallStackNameResolverBacktrace::FreeParamSymbols (void* /*handle*/)
{
}

bool CallStackNameResolverBacktrace::GetLineNumber (void*, char*&)
{
  return false;
}

} // namespace Debug
} // namespace CS

