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

#include "csutil/verbosity.h"

bool csCheckVerbosity (int argc, const char* const argv[], 
  const char* msgClass, const char* msgSubclass)
{
  bool verbose = false;
  const char* verboseFlags = 0;

  for (int i = 1; i < argc; ++i)
  {
    char const* s = argv[i];
    if (*s == '-')
    {
      do { ++s; } while (*s == '-');
      const char* eqSign = strchr (s, '=');
      if (strncmp(s, "verbose", eqSign ? (eqSign - s) : strlen (s)) == 0)
      {
	verbose = true;
	if (eqSign != 0)
	  verboseFlags = eqSign + 1;
	break;
      }
    }
  }
  
  if (verbose)
  {
    csVerbosityParser vp (verboseFlags ? verboseFlags : "");
    return vp.CheckFlag (msgClass, msgSubclass);
  }

  return false;
}

int csVerbosityParser::VfKeyCompare (const VerbosityFlag& vf, 
				      const char* const& K)
{
  return strcmp (vf.msgClass, K);
}

int csVerbosityParser::VfCompare (const VerbosityFlag& vf1, 
				   const VerbosityFlag& vf2)
{
  return strcmp (vf1.msgClass, vf2.msgClass);
}

csVerbosityParser::csVerbosityParser (const char* flags)
{
  if (flags == 0)
  {
    all = ForceResult | ForceFalse;
  }
  else if (*flags == 0)
  {
    all = ForceResult | ForceTrue;
  }
  else
  {
    all = 0;
    
    CS_ALLOC_STACK_ARRAY (char, msgClassStr, strlen (flags) + 1);
    strcpy (msgClassStr, flags);
    char* msgClass = msgClassStr;
    
    while ((msgClass != 0) && (*msgClass != 0))
    {
      char* nextClass = strchr (msgClass, ',');
      if (nextClass != 0)
      {
	*nextClass = 0;
	nextClass++;
      }
      size_t vfIndex = verbosityFlags.FindSortedKey (
	csArrayCmp<VerbosityFlag, const char*> (msgClass, &VfKeyCompare));
      if (vfIndex == csArrayItemNotFound)
	vfIndex = verbosityFlags.InsertSorted (VerbosityFlag(), 
	  &VfCompare);
      
      VerbosityFlag& vf = verbosityFlags[vfIndex];
      vf.msgClass = msgClass;
      char* subclass = strchr (msgClass, ':');
      if (subclass != 0)
      {
	*subclass = 0;
	subclass++;
	
	while ((subclass != 0) && (*subclass != 0))
	{
	  char* nextSubclass = strchr (subclass, ',');
	  if (nextSubclass != 0)
	  {
	    *nextSubclass = 0;
	    nextSubclass++;
	  }
	  
	  bool subFlag = true;
	  if (subclass[0] == '+')
	  {
	    subFlag = true;
	    subclass++;
	  }
	  else if (subclass[0] == '-')
	  {
	    subFlag = false;
	    subclass++;
	  }
	  if (strcmp (subclass, "*") == 0)
	    vf.defaultFlag = subFlag;
	  else
	  {
	    vf.msgSubclasses.PutUnique (subclass, subFlag);
	  }
	  
	  subclass = nextSubclass;
	}
      }
      msgClass = nextClass;
    }
  }
}

bool csVerbosityParser::CheckFlag (const char* msgClass, 
				    const char* msgSubclass)
{
  if (all & ForceResult) return all & ForceTrue;
    
  size_t vfIndex = verbosityFlags.FindSortedKey (
    csArrayCmp<VerbosityFlag, const char*> (msgClass, &VfKeyCompare));
  if (vfIndex != csArrayItemNotFound)
  {
    const VerbosityFlag& vf = verbosityFlags[vfIndex];
    return (msgSubclass != 0) ? vf.msgSubclasses.Get (
      msgSubclass, vf.defaultFlag) : vf.defaultFlag;
  }
  
  return false;
}

SCF_IMPLEMENT_IBASE(csVerbosityManager)
  SCF_IMPLEMENTS_INTERFACE(iVerbosityManager)
SCF_IMPLEMENT_IBASE_END
