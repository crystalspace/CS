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

/** This file implements a platform sepcific user configration which tries to
 * create the config files in $HOME/.crystal which is apporpriate for most
 * posixish systems.
 */
#include <sys/stat.h>

#include "cssysdef.h"
#include "csutil/cfgfile.h"
#include "csutil/csstring.h"
#include "cssys/sysfunc.h"

csPtr<iConfigFile> csGetPlatformConfig(const char* key)
{
  // Is $HOME set? otherwise fallback to standard mode
  const char* home = getenv("HOME");
  if (home == 0)
    return 0;
  
  // Construct directory and filename of the config file
  csString dir, fname;
  dir << home << PATH_SEPARATOR << ".crystal";
  fname << dir << PATH_SEPARATOR << key << ".cfg";

  // Try to create the directory (we assume that $HOME is already created)
  struct stat stats;
  if (stat(dir, &stats) != 0)
  {
    mode_t const m =
      S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRPS_IXGRP|S_IROTH|S_IWOTH|S_IXOTH;
    if (mkdir(dir, m) != 0)
    {
      fprintf(stderr,
  	  "Failed to create `%s' for configuration files (errno %d).\n",
	  dir, errno);
      return 0;
    }
  }

  return new csConfigFile(fname);
}
