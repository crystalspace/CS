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

// Returns milliseconds since first invocation of csGetTicks()
//  or csGetMicroTicks().
// With a 32bit integer there will be 49 days before this
// counter overflows.
csTicks csGetTicks ()
{
  return (csGetMicroTicks() / 1000);
}

// This function should return microseconds since first invocation
// time. When called once in a single thread this function is MT safe.
// If there is no high-performance counter available, this will
// provide really bad timing.
//
// Based on code by Thomas Hieber,  03/20/1999
int64 csGetMicroTicks()
{
  // Storage for the initial Get*Ticks() invocation call
  static int64 FirstCount = 0;
  // Storage for the performance timer frequency
  static int64 Freq      = 0;

  //Freq was set to -1, if the current Hardware does not support
  //high resolution timers. We will use GetTickCount instead then.
  if (Freq < 0)
    return GetTickCount() * 1000;

  //Freq is 0, the first time this function is being called.
  if (Freq == 0)
  {
    //try to determine the frequency of the high resulution timer
    if (!QueryPerformanceFrequency((LARGE_INTEGER*)&Freq))
    {
      //There is no such timer....
      Freq=-1;
      return GetTickCount() * 1000;
    }
    // Start counting from first time this function is called.
    QueryPerformanceCounter((LARGE_INTEGER*)&FirstCount);
  }

  //retrieve current count
  int64 Count = 0;
  QueryPerformanceCounter((LARGE_INTEGER*)&Count);

  return 1000000*(Count-FirstCount)/Freq;
}

void csSleep (int SleepTime)
{
  Sleep (SleepTime);
}

