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
#ifndef __CS_UTIL_CALLSTACK_H__
#define __CS_UTIL_CALLSTACK_H__

/**\file
 * Call stack creation helper
 */

/*
 @@@ Not implemented on all platforms yet!
 */
 
#include "csextern.h"
#include "csutil/csstring.h"

// @@@ Document me
class CS_CRYSTALSPACE_EXPORT csCallStack
{
protected:
  virtual ~csCallStack() {}
public:
  virtual void Free() = 0;
  
  virtual size_t GetEntryCount () = 0;
  virtual bool GetFunctionName (size_t num, csString& str) = 0;
  virtual bool GetLineNumber (size_t num, csString& str) = 0;
  virtual bool GetParameters (size_t num, csString& str) = 0;
  void Print (FILE* f = stdout, bool Short = false)
  {
    for (size_t i = 0; i < GetEntryCount(); i++)
    {
      csString s;
      bool hasFunc = GetFunctionName (i, s);
      fprintf (f, "%s", hasFunc ? (const char*)s : "<unknown>");
      if (!Short && (GetLineNumber (i, s)))
	fprintf (f, " @%s", (const char*)s);
      if (!Short && (GetParameters (i, s)))
	fprintf (f, " (%s)", (const char*)s);
      fprintf (f, "\n");
    }
    fflush (f);
  }
};

// @@@ Document me
class CS_CRYSTALSPACE_EXPORT csCallStackHelper
{
public:
  /**
   * Create a call stack.
   * \param skip The number of calls on the top of the stack to remove from
   *  the returned call stack. This can be used if e.g. the call stack is
   *  created from some helper function and the helper function itself should
   *  not appear in the stack.
   * \return A call stack object.
   */
  static csCallStack* CreateCallStack (int skip = 0);
};

#endif // __CS_UTIL_CALLSTACK_H__
