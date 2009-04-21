/*
  Call stack creation helper
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
#ifndef __CS_UTIL_WIN32_CALLSTACK_H__
#define __CS_UTIL_WIN32_CALLSTACK_H__

/**\file
 * Call stack creation helper (Win32-specific)
 */

#include "csextern.h"
#include "csutil/callstack.h"

/**
 * Call stack creation helper (Win32-specific)
 * \remarks This class provides functionality specific to the Win32 
 *  platform. To ensure that code using this functionality compiles properly 
 *  on all other platforms, the use of the class and inclusion of the 
 *  header file should be surrounded by appropriate 
 *  '\#if defined(CS_PLATFORM_WIN32) ... \#endif' statements.
 */
class CS_CRYSTALSPACE_EXPORT cswinCallStackHelper
{
public:
  /**
   * Create a call stack.
   * Works similar to csCallStackHelper::CreateCallStack(), with the 
   * difference that you can provide some Win32-specific arguments here.
   * \param hProc The process for which the call stack is created.
   * \param hThread The thread for which the call stack is created.
   * \param context Context information.   
   * \param skip The number of calls on the top of the stack to remove from
   *  the returned call stack. This can be used if e.g. the call stack is
   *  created from some helper function and the helper function itself should
   *  not appear in the stack.
   * \param fast Flag whether a fast call stack creation should be preferred 
   *  (usually at the expense of retrieved information).
   * \return A call stack object.
   */
  static csCallStack* CreateCallStack (HANDLE hProc, HANDLE hThread,
    CONTEXT& context, int skip = 0, bool fast = false);
};

#endif // __CS_UTIL_WIN32_CALLSTACK_H__
