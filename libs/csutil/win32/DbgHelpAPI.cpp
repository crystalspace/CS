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
#include "cachedll.h"
#include "csutil/win32/DbgHelpAPI.h"

cswinCacheDLL* DbgHelp::dll = 0;
bool DbgHelp::dllLoaded = false;
int DbgHelp::refCount = false;
#define FUNC_GROUP_BEGIN(name)	    \
  bool DbgHelp::name ## _avail = false;
#define FUNC_GROUP_END
#define FUNC(ret, name, args)		    \
  DbgHelp::PFN ## name DbgHelp::name = 0;
#include "csutil/win32/DbgHelpAPI.fun"

void DbgHelp::IncRef()
{
  refCount++;
}

void DbgHelp::DecRef()
{
  refCount--;
  if (refCount == 0)
  {
    delete dll; 
    dll = 0;
    dllLoaded = false;
  }
}

bool DbgHelp::Available ()
{
  if (!dllLoaded)
  {
    dllLoaded = true;
    dll = new cswinCacheDLL ("dbghelp.dll");
    if ((dll != 0) && (*dll != 0))
    {
      #define FUNC_GROUP_BEGIN(name)	    \
	name ## _avail = true
      #define FUNC_GROUP_END		    ;
      #define FUNC(ret, name, args)		    \
	&& (name = (PFN ## name)GetProcAddress (*dll, #name))
      #include "csutil/win32/DbgHelpAPI.fun"
    }
  }
  return dll != 0;
}
