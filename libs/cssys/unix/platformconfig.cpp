/*
    Copyright (C) 2003 by Matthias Braun

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

#include <sys/stat.h>

#include "csutil/snprintf.h"
#include "csutil/cfgfile.h"
#include "cssys/sysfunc.h"

csPtr<iConfigFile> csGetPlatformConfig(const char* key)
{
  // is $HOME set? otherwise fallback to standard mode
  const char* home = getenv("HOME");
  if (!home)
    return 0;
  
  // construct directory and filename of the config file
  char fname[1000];
  char dir[1000];
  cs_snprintf(dir, 1000, "%s/.crystal", getenv("HOME"));
  cs_snprintf(fname, 1000, "%s/%s.cfg", dir, key);

  // try to create the directory (we assume that $HOME is already created)
  struct stat stats;
  if (stat(dir, &stats) != 0)
  {
    if (mkdir(dir, S_IWUSR | S_IXUSR | S_IRUSR) != 0)
    {
      fprintf(stderr,
  	  "couldn't create directory '%s' for configuration files.\n", dir);
      return 0;
    }
  }

  // create/read a config file
  csRef<csConfigFile> configfile = csPtr<csConfigFile> (new csConfigFile);
  if (!configfile->Load(fname)) {
    fprintf(stderr, "couldn't create configfile '%s'.\n", fname);
    return 0;
  }
    
  return csPtr<iConfigFile> (configfile);
}

