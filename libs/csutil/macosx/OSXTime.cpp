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
  struct timezone z = { 0, 0 };
  struct timeval r;
  gettimeofday(&r, &z);
  static long base = 0;
  if (base != 0)
    return (r.tv_sec - base) * 1000 + r.tv_usec / 1000;
  else
  {
    base = r.tv_sec;
    return 0;
  }
}


//-----------------------------------------------------------------------------
// csSleep
//-----------------------------------------------------------------------------
void csSleep(int msecs)
{
  usleep(msecs * 1000);
}
