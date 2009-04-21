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
#include "csutil/weakref.h"
#include "iutil/objreg.h"

#include "csutil/win32/DbgHelpAPI.h"
 
/**
 * Helper to write minidumps on Win32.
 * \remarks This class provides functionality specific to the Win32 
 *  platform. To ensure that code using this functionality compiles properly 
 *  on all other platforms, the use of the class and inclusion of the 
 *  header file should be surrounded by appropriate 
 *  '\#if defined(CS_PLATFORM_WIN32) ... \#endif' statements.
 */
class CS_CRYSTALSPACE_EXPORT cswinMinidumpWriter
{
public:
  /**
   * Callback that can be provided by the application to further 
   * deal with the crash dump file.
   */
  typedef void (*FnCrashMinidumpHandler) (const char* dumpFile);
protected:
  static csWeakRef<iObjectRegistry> object_reg;
  static FnCrashMinidumpHandler customHandler;

  static LPTOP_LEVEL_EXCEPTION_FILTER oldFilter;
  static LONG WINAPI ExceptionFilter (
    struct _EXCEPTION_POINTERS* ExceptionInfo);
public:
  /**
   * Write a dump of the current state of the process.
   * \return The filename where the dump was written to. Is created in a
   *  location for temp files.
   */
  static const char* WriteMinidump (
    PMINIDUMP_EXCEPTION_INFORMATION except = 0, bool dumpHeap = false);

  /**
   * Write a mini dump that is wrapped inside a zip and also contains a
   * textual stack trace and the reporter log file.
   * \return The filename where the zip was written to. Is created in a
   *  location for temp files.
   */
  static const char* WriteWrappedMinidump (iObjectRegistry* object_reg = 0,
    PMINIDUMP_EXCEPTION_INFORMATION except = 0, bool dumpHeap = false);

  /**
   * Enable the built-in crash handler.
   * Sets up an exception handler that creates a dump using 
   * WriteWrappedMinidump(). In case a custom \a handler is provided, it is
   * called. Otherwise, a message box containing the dump file is displayed.
   */
  static void EnableCrashMinidumps (FnCrashMinidumpHandler handler = 0);
  /**
   * Set the object registry used by the built-in crash handler.
   * It is needed to collect some extra information, notable the reporter
   * log. 
   * \remark Not setting this value will not result in failure later.
   */
  static void SetCrashMinidumpObjectReg (iObjectRegistry* object_reg);
  /**
   * Disable the built-in crash handler.
   */
  static void DisableCrashMinidumps ();
};

