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

#ifndef __CS_LIBS_UTIL_WIN32_CALLSTACK_H__
#define __CS_LIBS_UTIL_WIN32_CALLSTACK_H__

#include "csextern.h"
#include "csutil/array.h"
#include "csutil/blockallocator.h"
#include "csutil/callstack.h"
#include "csutil/strset.h"

#include "csutil/win32/DbgHelpAPI.h"

class CS_CSUTIL_EXPORT cswinCallStack : public csCallStack
{
public:
  static csStringSet strings;

  struct CS_CSUTIL_EXPORT StackEntry
  {
    uintptr_t instrPtr;

    struct Param
    {
      csStringID name;
      uintptr_t value;
    };
    Param* params;

    StackEntry() : instrPtr (~0), params(0) {}
    ~StackEntry() { delete[] params; }
  };

  StackEntry* entries;
  static BOOL CALLBACK EnumSymCallback (SYMBOL_INFO* pSymInfo, 
    ULONG SymbolSize, PVOID UserContext);

  void AddFrame (const STACKFRAME64& frame, 
    csDirtyAccessArray<StackEntry>& entries);

  cswinCallStack() : csCallStack(), entries(0) {}
  virtual ~cswinCallStack() { delete[] entries; }

  virtual size_t GetEntryCount ();
  virtual bool GetFunctionName (size_t num, csString& str);
  virtual bool GetLineNumber (size_t num, csString& str);
  virtual bool GetParameters (size_t num, csString& str);
};

#endif // __CS_LIBS_UTIL_WIN32_CALLSTACK_H__
