/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein

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
#include <stdlib.h>
#include <time.h>
#include <sys/timeb.h>
#include "csutil/sysfunc.h"

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
  //03/20/1999 Thomas Hieber: completely redone to get true Millisecond
  //accuracy instead of very rough ticks. This routine will also provide
  //correct wrap around at the end of "long"
#if defined(__CYGWIN32__)
#      define __int64 long long
#endif

  static __int64 Freq      = 0;
  static __int64 FirstCount = 0;

  //Freq was set to -1, if the current Hardware does not support
  //high resolution timers. We will use GetTickCount instead then.
  if (Freq < 0)
  {
    return GetTickCount();
  }

  //Freq is 0, the first time this function is being called.
  if (Freq == 0)
  {
    //try to determine the frequency of the high resulution timer
    if (!QueryPerformanceFrequency((LARGE_INTEGER*)&Freq))
    {
      //There is no such timer....
      Freq=-1;
      return csGetTicks();
    }
    // Start counting from first time this function is called.
    QueryPerformanceCounter((LARGE_INTEGER*)&FirstCount);
  }

  //retrieve current count
  __int64 Count = 0;
  QueryPerformanceCounter((LARGE_INTEGER*)&Count);

  return 1000*(Count-FirstCount)/Freq;
}

void csSleep (int SleepTime)
{
  Sleep (SleepTime);
}

