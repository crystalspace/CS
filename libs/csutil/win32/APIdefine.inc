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

#include "csutil/sysfunc.h"
#include "cachedll.h"

cswinCacheDLL* CS_API_NAME::dll = 0;
bool CS_API_NAME::dllLoaded = false;
int CS_API_NAME::refCount = 0;
#define FUNC_GROUP_BEGIN(name)	    \
  bool CS_API_NAME::name ## _avail = false;
#define FUNC_GROUP_END
#define FUNC(ret, name, args)		    \
  CS_API_NAME::PFN ## name CS_API_NAME::name = 0;
#include CS_API_FUNCTIONS

void CS_API_NAME::IncRef()
{
  refCount++;
}

void CS_API_NAME::DecRef()
{
  refCount--;
  if (refCount == 0)
  {
    delete dll; 
    dll = 0;
    dllLoaded = false;
  }
}

int CS_API_NAME::GetRefCount ()
{
  return refCount;
}

bool CS_API_NAME::Available ()
{
  if (!dllLoaded)
  {
    dllLoaded = true;
    dll = new cswinCacheDLL (CS_API_DLL);
    if ((dll != 0) && (*dll != 0))
    {
#define STRINGIZE2(x)   #x
#define STRINGIZE(x)    STRINGIZE2(x)
      
#ifdef CS_DEBUG
      bool* flag;
      #define FUNC_GROUP_BEGIN(name)					    \
        name ## _avail = true;						    \
	flag = &name ## _avail;
      #define FUNC_GROUP_END
      #define FUNC(ret, name, args)					    \
        *flag &= ((name = (PFN ## name)GetProcAddress (*dll, #name)) != 0); \
        if (!name) csPrintf ("%s: Could not obtain %s\n", 		    \
	  STRINGIZE (CS_API_NAME), #name);
      #include CS_API_FUNCTIONS
#else      
      #define FUNC_GROUP_BEGIN(name)	    \
        name ## _avail = true
      #define FUNC_GROUP_END		    ;
      #define FUNC(ret, name, args)		    \
        && (name = (PFN ## name)GetProcAddress (*dll, #name))
      #include CS_API_FUNCTIONS
#endif
    }
#ifdef CS_DEBUG
    if (!dll || !*dll)
      csPrintf ("%s: Could not load %s\n", STRINGIZE (CS_API_NAME), 
	CS_API_DLL);
#endif
  }
  return dll != 0;
}
