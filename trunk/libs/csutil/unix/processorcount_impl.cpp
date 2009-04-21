/*
Copyright (C) 2008 by Scott Johnson <scottj@cs.umn.edu>

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
#include "csgeom/math.h"
#include "csutil/csstring.h"
#include "csutil/sysfunc.h"

#ifdef CS_HAVE_SYSCONF_PROCESSORS
#include <unistd.h>
#endif

static uint query_sysconf()
{
  uint n = 0;
#if defined(CS_HAVE_SYSCONF_PROCESSORS)
  n = (uint)sysconf(_SC_NPROCESSORS_ONLN);
#endif
  return n;
}


static uint query_proc_cpuinfo()
{
  uint n = 0;
  FILE* f = fopen("/proc/cpuinfo", "r");
  if (f != 0)
  {
    csString line, key, val;
    char buff[ 1024 ];
    while (fgets(buff, sizeof(buff) - 1, f) != 0)
    {
      line = buff;
      size_t pos = line.Find(":");
      if (pos > 0)
      {
	key = line.Slice(0, pos);
	key.Trim();
	if (key.CompareNoCase("processor") || key.CompareNoCase("hw.ncpu"))
	{
	  val = line.Slice(pos + 1);
	  val.Trim();
	  n = csMax (static_cast<uint>(atol(val.GetData())), n+1);
	}
      }
    }
    fclose(f);
  }
  return n;
}


namespace CS {
  namespace Platform {
    namespace Implementation {

      uint GetProcessorCount()
      {
	size_t s;
	s = query_sysconf();
	if (s == 0)
	  s = query_proc_cpuinfo();
        return s;
      }

    } // End namespace Implementation
  } // End namespace Platform
} // End namespace CS

