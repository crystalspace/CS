/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#include <unistd.h>
#include <sys/times.h>
#include "cssysdef.h"
#include "cssys/sysfunc.h"


// This function should return milliseconds since some specific time
csTicks csGetTicks ()
{
  // We shouldn't cache 1000/CLK_TCK, because we loose accuracy
  // on platforms where CLK_TCK is 60. (16 != 16.66666...)
#ifdef _SC_CLK_TCK
  static clock_t clktck = 0;
  if (clktck == 0)
    clktck = sysconf(_SC_CLK_TCK);  
#else
#define clktck CLK_TCK
#endif
  // NOTE: times() can return -1 on errors.  Yikes!
  struct tms buf;
  return (1000 * times (&buf)) / clktck;
}
