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

#include "csextern.h"
#include "csutil/csstring.h"

class CS_CSUTIL_EXPORT csCallStack
{
protected:
  virtual ~csCallStack() {}
public:
  virtual void Free() { delete this; }
  
  virtual int GetEntryCount () = 0;
  virtual bool GetFunctionName (int num, csString& str) = 0;
  virtual bool GetLineNumber (int num, csString& str) = 0;
  virtual bool GetParameters (int num, csString& str) = 0;
  void Print (FILE* f = stdout, bool Short = false)
  {
    for (int i = 0; i < GetEntryCount(); i++)
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

class CS_CSUTIL_EXPORT csCallStackHelper
{
public:
  static csCallStack* CreateCallStack (uint skip = 0);
};

#endif // __CS_UTIL_CALLSTACK_H__
