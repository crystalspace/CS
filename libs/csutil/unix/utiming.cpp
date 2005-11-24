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
#include <sys/time.h>
#include "cssysdef.h"
#include "csutil/sysfunc.h"

csTicks csGetTicks ()
{
  struct timeval now;
  gettimeofday (&now, 0);
  // To avoid early wrap of the timer we subtract a high number
  // from seconds. We know that gettimeofday is relative to 1-1-1970.
  return (now.tv_sec  - 1100000000 ) * 1000 + 
         (now.tv_usec) / 1000;
}
