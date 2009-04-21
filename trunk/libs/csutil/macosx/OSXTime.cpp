//=============================================================================
//
//	Copyright (C)1999-2001 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// OSXTime.cpp
//
//	Clock related functionality for MacOS/X.
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include <libc.h>
#include <unistd.h>
#include <sys/time.h>

//-----------------------------------------------------------------------------
// Static initializer, for the csGetTicks function. The csGetTicks
// need to be called once before there are any chance that it will
// be called from multiple threads to initialize static variables.
//-----------------------------------------------------------------------------
class csInitGetTicks 
{
public:
  csInitGetTicks() { csGetTicks(); }    
};

// Constructor called before main() is invoke
csInitGetTicks initGetTicks;

//-----------------------------------------------------------------------------
// csGetTicks
//	Implement OSX-specific clock function.  Returns milliseconds since
//	first invocation time.With a 32bit integer there will be 49 days before
//	this // counter overflow. When called once in a single thread this //
//	function is MT safe.
//-----------------------------------------------------------------------------
csTicks csGetTicks()
{
  return (csGetMicroTicks() / 1000);
}

// This function should return microseconds since first invocation
// time. When this, or csGetTicks() is called once in a single thread
// this function is MT safe. 
int64 csGetMicroTicks ()
{
  // Storage for the initial invocation call
  static struct timeval FirstCount;

  // Flag indicating wether timing has been initialized
  static int TimingInitialized = 0;

  // Start counting from first time this function is called. 
  if (!TimingInitialized)
  {
    gettimeofday (&FirstCount, 0);    
    TimingInitialized=1;
  }

  struct timeval now;
  gettimeofday (&now, 0);
  return ((int64)(now.tv_sec  - FirstCount.tv_sec )) * 1000000 + 
    (now.tv_usec - FirstCount.tv_usec);
}

//-----------------------------------------------------------------------------
// csSleep
//-----------------------------------------------------------------------------
void csSleep(int msecs)
{
  usleep(msecs * 1000);
}
