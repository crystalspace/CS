/*
    Profiling tools.
    Copyright (C) 2005 by Jorrit Tyberghein

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
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "csutil/scf.h"
#include "csutil/util.h"
#include "csutil/profile.h"
#include "iutil/objreg.h"

//-----------------------------------------------------------------------------

#ifdef CS_DO_PROFILING

SCF_IMPLEMENT_IBASE (csProfiler)
  SCF_IMPLEMENTS_INTERFACE (iProfiler)
SCF_IMPLEMENT_IBASE_END

csProfiler::csProfiler ()
{
  SCF_CONSTRUCT_IBASE (0);
}

csProfiler::~csProfiler ()
{
  SCF_DESTRUCT_IBASE ();
}

void csProfiler::RegisterProfilePoint (const char* token,
	const char* file, int line,
  	uint32* ptr_count, uint32* ptr_time,
	uint32* ptr_timemin, uint32* ptr_timemax)
{
  csProfileInfo info;
  info.token = token;
  info.file = file;
  info.line = line;
  info.ptr_count = ptr_count;
  info.ptr_time = ptr_time;
  info.ptr_timemin = ptr_timemin;
  info.ptr_timemax = ptr_timemax;
  profile_info.Push (info);
}

void csProfiler::Dump ()
{
  size_t i;
  printf ("count time avg file line\n");
  for (i = 0 ; i < profile_info.Length () ; i++)
  {
    const csProfileInfo& pi = profile_info[i];
    if (*pi.ptr_count > 0)
      printf ("%" PRIu32 " %" PRIu32 "(%" PRIu32 "/%" PRIu32 ") %g %s/%s %d\n",
        *pi.ptr_count, *pi.ptr_time,
        *pi.ptr_timemin, *pi.ptr_timemax,
    	float (*pi.ptr_time) / float (*pi.ptr_count),
    	pi.token, pi.file, pi.line);
  }
  fflush (stdout);
}

void csProfiler::Reset ()
{
  size_t i;
  for (i = 0 ; i < profile_info.Length () ; i++)
  {
    const csProfileInfo& pi = profile_info[i];
    *pi.ptr_count = 0;
    *pi.ptr_time = 0;
    *pi.ptr_timemin = 1000000000;
    *pi.ptr_timemax = 0;
  }
}


#endif

//-----------------------------------------------------------------------------

