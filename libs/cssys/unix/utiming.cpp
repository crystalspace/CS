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
  struct tms buf;

  // We shouldn't cache 1000/CLK_TCK, because we loose accuracy
  // on platforms where CLK_TCK is 60. (16 != 16.66666...)
  static csLongTicks Freq      = 0;
  static csLongTicks FirstCount = 0;

  if (Freq == 0)
  {
#ifdef _SC_CLK_TCK
    Freq = sysconf(_SC_CLK_TCK);
#else
    Freq = CLK_TCK;
#endif
    // Start counting from first time this function is called. 
    FirstCount = (csLongTicks)times (&buf);
  } 

  csLongTicks Count = (csLongTicks)times (&buf);
  
  return 1000*(Count-FirstCount)/Freq;
}
