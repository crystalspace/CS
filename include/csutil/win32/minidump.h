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

/**\file
 * Helper to write minidumps on Win32.
 */
 
#include "csutil/ref.h"
#include "iutil/vfs.h"

#include "csutil/win32/DbgHelpAPI.h"
 
/**
 * Helper to write minidumps on Win32.
 */
class CS_CSUTIL_EXPORT cswinMinidumpWriter
{
public:
  /**
   * Write a dump of the current state of the process.
   * \return The filename where the dump was written to. Is created in a
   *  location for temp files.
   */
  static const char* WriteMinidump (
    PMINIDUMP_EXCEPTION_INFORMATION except = 0, bool dumpHeap = false);
};
