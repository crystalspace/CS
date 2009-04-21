/*
    Copyright (C) 2003 by Jorrit Tyberghein

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

#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include "cssysdef.h"
#include "csutil/sysfunc.h"

// Undefine this if your system does not support locale.  In a long-run, if we
// find such a platform, we should add the detection of locale to the configure
// script.
#define CS_USE_I18N

#ifdef CS_USE_I18N
#include <locale.h>
#endif

bool csPlatformStartup (iObjectRegistry*)
{
#ifdef CS_USE_I18N
  // Never do "LC_ALL" because it would break things such as numeric format.
  setlocale (LC_COLLATE, "");
  setlocale (LC_CTYPE, "");
  setlocale (LC_MESSAGES, "");
  setlocale (LC_TIME, "");
#endif
  return true;
}

bool csPlatformShutdown(iObjectRegistry*)
{
  return true;
}

void csSleep (int SleepTime)
{
  usleep (SleepTime * 1000);
}
