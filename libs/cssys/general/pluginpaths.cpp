/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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
#include "cssys/sysfunc.h"
#include "cssys/syspath.h"
#include "csutil/util.h"

csPluginPaths* csGetPluginPaths (const char* argv0)
{
  csPluginPaths* paths = new csPluginPaths;

  char* appPath = csGetAppPath (argv0);
  char* configPath = csGetConfigPath ();
  
  /*
     Don't add "/".
     @@@ Won't work on Win32.
   */
  if (appPath && *appPath && (*appPath != PATH_SEPARATOR))
  {
    paths->AddOnce (appPath, true, "app");
  }

  if (configPath)
  {
    csString tmp;
    tmp << configPath << PATH_SEPARATOR << "lib";
    paths->AddOnce (tmp, true, "crystal");

    tmp.Clear();
    tmp << configPath << PATH_SEPARATOR << "crystal";
    paths->AddOnce (tmp, true, "crystal");

    paths->AddOnce (configPath, false, "crystal");
    
    delete[] configPath;
  }

  delete[] appPath;

  return paths;
}

