/*
    Copyright (C) 2001 by Wouter Wijngaards

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

#if 0 // disabled none of the ports is using this at the moment
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cssysdef.h"

#include "csutil/util.h"

#include "cssys/sysfunc.h"

// these defines should be set by the configure script

#ifndef CS_CONFIGDIR
#define CS_CONFIGDIR "/usr/local/crystal"
#endif
#ifndef CS_PLUGINDIR
#define CS_PLUGINDIR "/usr/local/crystal/lib"
#endif

char* csGetConfigPath ()
{
  const char* crystal = getenv ("CRYSTAL");
  char* path;

  if (!crystal)
  {
    // no setting, use the default.
    // is the current dir a possible install? try opening scf.cfg,
    if (!access ("scf.cfg", F_OK))                                    
    {
      printf ("ConfigPath: .\n");
      return csStrNew (".");
    }

    // XXX: This shouldn't be here and can be removed as soon as the new
    // system works...
#if 1
    // debian install place
    if (!access ("/usr/lib/crystalspace/scf.cfg", F_OK))
    {
      return csStrNew ("/usr/lib/crystalspace/");
    }
#endif

    // default install place
    return csStrNew(CS_CONFIGDIR);
  }

  // check for $CRYSTAL/etc
  path = new char[320];
  strncpy (path, crystal, 300);
  strcat (path, "/etc/scf.cfg");
  if (!access (path, F_OK))
  {
    strncpy (path, crystal, 300);
    strcat (path, "/etc");
    printf ("ConfigPath: %s\n", path);
    return path;
  }
  
  // check for $CRYSTAL
  strncpy (path, crystal, 300);
  strcat (path, "/scf.cfg");
  if (!access (path, F_OK))
  {
    strncpy (path, crystal, 320);
    printf ("ConfigPath: %s\n", path);
    return path;
  }
    
  fprintf (stderr, 
      "Couldn't find scf.cfg in '%s' (defined by CRYSTAL var).\n", crystal);

  delete[] path;
  return NULL;
}

char** csGetPluginPaths ()
{
  const char* crystal = getenv ("CRYSTAL");

  if (!crystal)
  {
    char** paths = new char* [3];
    paths[0] = csStrNew(CS_PLUGINDIR);
    paths[1] = csStrNew(".");
    paths[2] = 0;
  
    return paths;
  }

  char** paths = new char* [3];
  paths[0] = csStrNew(crystal);
  paths[1] = csStrNew(".");
  paths[2] = 0;
  
  return paths;
}

#endif

