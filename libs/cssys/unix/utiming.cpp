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

#include <time.h>
#include <sys/times.h>
#include "cssysdef.h"
#include "cssys/sysfunc.h"

#if !defined(COMP_GCC3)
#define cs_clk_tck() (1000 / CLK_TCK)
#else
extern "C" long int __sysconf(int);
inline long int cs_clk_tck()
{
  static long int cache = 0;
  if (cache == 0)
    cache = 1000 / ((__clock_t)__sysconf(2));
  return cache;
}
#endif

// This function should return milliseconds since some specific time
csTicks csGetTicks ()
{
  // NOTE: times() can return -1 on errors.  Yikes!
  struct tms buf;
  return times (&buf) * cs_clk_tck();
}
