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
#include "cssys/sysdriv.h"

// Define this if we have GCC3.
// @@@ We need to change the makefile to detect this automatically!
#ifndef COMP_GCC3
#define COMP_GCC3 0
#endif

#if COMP_GCC3
extern "C"
{
  extern long int __sysconf (int);
}
#endif

// This function should return milliseconds since some specific time
csTicks csGetTicks ()
{
  struct tms buf;
#if COMP_GCC3
  return times (&buf) * 1000 / ((__clock_t) __sysconf(2));
#else
  return times (&buf) * 1000 / CLK_TCK;
#endif
}
