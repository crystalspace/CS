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
#include "cssys/sysfunc.h"

// This function should return milliseconds since some specific
// time. If you don't have milliseconds easily just rewrite by
// using 'time (NULL)*1000' or something like that.

csTicks csGetTicks ()
{
  //03/20/1999 Thomas Hieber: completely redone to get true Millisecond 
  //accuracy instead of very rough ticks. This routine will also provide
  //correct wrap around at the end of "long"
#if defined(__CYGWIN32__)
#	define __int64 long long
#endif

  static __int64 Freq      = 0;
  static __int64 LastCount = 0;
  static __int64 LastRest  = 0;
  static long    LastTime  = 0;

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
  }

  //retrieve current count
  __int64 Count = 0;
  QueryPerformanceCounter((LARGE_INTEGER*)&Count);
  
  //calculate the time passed since last call, and add the rest of
  //those tics that didn't make it into the last reported time.
  __int64 Delta = 1000*(Count-LastCount)+LastRest;

  LastTime += (long)(Delta/Freq); //save the new value
  LastRest  = Delta%Freq;         //save those ticks not being counted
  LastCount = Count;              //save last count

  return LastTime; //return a high quality measurement of time. 
}

void csSleep (int SleepTime)
{
  Sleep (SleepTime);
}

