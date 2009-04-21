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
  // Storage for the base count from which the delta is calculated.
  static int64 BaseCount = 0;
  // Storage for the performance timer frequency
  static int64 Freq      = 0;
  // Storage for the number of whole seconds that have elapsed since we've started
  static int64 WholeSecs = 0;

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
    QueryPerformanceCounter((LARGE_INTEGER*)&BaseCount);
  }

  //retrieve current count
  int64 Count = 0;
  QueryPerformanceCounter((LARGE_INTEGER*)&Count);

  // The number of elapsed counts from the base
  int64 ElapsedRaw=(Count-BaseCount);

  // 2006/04/30 - There are some AMD X2 processors out now that incorrectly halt the instruction counter when
  //              one of the cores is put to sleep.  This can cause time to appear to jump around (forward and back)
  //              as the process is run on one core or another.  Workarounds are documented on various web sites.
  // CS_ASSERT_MSG("Timing error! Your system has reported a backwards-moving time value!", ElapsedRaw>=0);

  // There's not much that can be done about this.  Let's presume time hasn't moved and hope we get scheduled
  //  on the same logical processor we started on sometime soon.
  if (ElapsedRaw < 0)
    ElapsedRaw = 0;

  // If we've passed a whole second, move it into the whole seconds accumulator
  if (ElapsedRaw >= Freq)
  {
    int64 ElapsedSecs = ElapsedRaw / Freq;
    // Add whole seconds to the appropriate accumulator
    WholeSecs += ElapsedSecs;
    // Add the elapsed whole seconds to the base count in the form of raw count
    BaseCount += (ElapsedSecs * Freq);
    // Remove from the elapsed count in the form of raw count
    ElapsedRaw -= (ElapsedSecs * Freq);
  }

  // Counters on fast systems may count at the clock rate of the CPU.  A CPU running at
  //  2.4 ghz will cause the calculation of (1000000 * ElapsedCycles) to overflow a 64
  //  bit signed integer at about an hour of runtime. By breaking the calculation up
  //  in this manner, we extend runtime before overflow to 1,000,000 hours on such a system
  return (1000000 * WholeSecs) + ((1000000 * ElapsedRaw) / Freq);
}

void csSleep (int SleepTime)
{
  Sleep (SleepTime);
}

