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

typedef long long csLongTicks;

// Static initializer, for the csGetTicks function. The csGetTicks
// need to be called once before there are any chance that it will
// be called from multiple threads to initialize static variables.
class csInitGetTicks 
{
public:
  csInitGetTicks()
  {
    csGetTicks();
  }    
};

// Constructor called before main is invoke
csInitGetTicks initGetTicks;

// This function should return milliseconds since first invocation
// time. With a 32bit integer there will be 49 days before this
// counter overflow. When called once in a single thread this
// function is MT safe. 
csTicks csGetTicks ()
{
  static struct timeval now, FirstCount;

  // Start counting from first time this function is called. 
  if (FirstCount.tv_sec == 0)
    gettimeofday (&FirstCount, 0);    

  gettimeofday (&now, 0);
  return (now.tv_sec  - FirstCount.tv_sec ) * 1000 + 
         (now.tv_usec - FirstCount.tv_usec) / 1000;
}
