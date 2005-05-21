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

#include <execinfo.h>

class csBacktraceCallStack : public csCallStack
{
  char** traceData;
  int traceCount;
protected:
  virtual ~csBacktraceCallStack() 
  {
    if (traceData != 0) free (traceData);
  }
public:
  csBacktraceCallStack (void** data, int count)
  {
    traceData = backtrace_symbols (data, count);
    traceCount = count;
  }
  
  virtual void Free() { delete this; }
  
  virtual size_t GetEntryCount ()
  {
    return (traceData != 0) ? traceCount : 0;
  }
  virtual bool GetFunctionName (size_t num, csString& str)
  {
    str.Replace (traceData[num]);
    return true;
  }
  virtual bool GetLineNumber (size_t num, csString& str)
  {
    return false;
  }
  virtual bool GetParameters (size_t num, csString& str)
  {
    return false;
  }
};

csCallStack* csCallStackHelper::CreateCallStack (int skip)
{
  void* traceBuffer[200];
  int count = backtrace (traceBuffer, sizeof (traceBuffer) / sizeof (void*));
  return new csBacktraceCallStack (traceBuffer, count);
}
