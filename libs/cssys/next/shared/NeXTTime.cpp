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
// NeXTTime.cpp
//
//	Clock related functionality for MacOS/X, MacOS/X Server 1.0 (Rhapsody),
//	OpenStep, and NextStep.
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include <libc.h>
#include <unistd.h>
#include <sys/time.h>

//-----------------------------------------------------------------------------
// csGetTicks
//	Implement NeXT-specific clock function.  Returns milliseconds since
//	first invocation.
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
void csSleep( int msecs )
{
  usleep(msecs * 1000);
}
