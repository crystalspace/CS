/*
  Copyright (C) 2005 by Jorrit Tyberghein
            (C) 2005 by Frank Richter

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

#ifndef __CS_LIBS_CSUTIL_CALLSTACK_H__
#define __CS_LIBS_CSUTIL_CALLSTACK_H__

#include "csutil/dirtyaccessarray.h"

/**
 * Parameter count signaling that the number and values of the parameters
 * for a callstack entry are unknown.
 */
const size_t csParamUnknown = (size_t)~0;

/// Callstack entryoffset
struct CallStackEntry
{
  /// Instruction pointer
  void* address;
  /// Number of parameters
  size_t paramNum;
  /// Index of first parameter in callstack params
  size_t paramOffs;
};

/// Interface for a call stack creator
struct iCallStackCreator
{
  /// Fill the arrays with call stack information
  virtual bool CreateCallStack (csDirtyAccessArray<CallStackEntry>& entries,
    csDirtyAccessArray<uintptr_t>& params, bool fast) = 0;
};

/// Call stack symbol name resolver
struct iCallStackNameResolver
{
  /**
   * Get textual description of an instruction pointer. The returned string
   * should contain address itself, symbol name, module if possible.
   */
  virtual bool GetAddressSymbol (void* addr, csString& sym) = 0;
  /**
   * Start retrieving names for parameters of the function in which \a addr 
   * lies.
   */
  virtual void* OpenParamSymbols (void* addr) = 0;
  /// Get name of a parameter of a function
  virtual bool GetParamName (void* handle, size_t paramNum, csString& sym) = 0;
  /// End retrieving names for parameters.
  virtual void FreeParamSymbols (void* handle) = 0;
  /// Get the file and line number for an instruction pointer.
  virtual bool GetLineNumber (void* addr, csString& lineAndFile) = 0;
};

class csCallStackImpl : public csCallStack
{
public:
  csDirtyAccessArray<CallStackEntry> entries;
  csDirtyAccessArray<uintptr_t> params;
  
  virtual ~csCallStackImpl();
  csCallStackImpl ();
  virtual void Free();
  
  virtual size_t GetEntryCount ();
  virtual bool GetFunctionName (size_t num, csString& str);
  virtual bool GetLineNumber (size_t num, csString& str);
  virtual bool GetParameters (size_t num, csString& str);
};

#endif // __CS_LIBS_CSUTIL_CALLSTACK_H__
