//=============================================================================
//
//	Copyright (C)1999 by Eric Sunshine <sunshine@sunshineco.com>
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
//	Implement NeXT-specific time function on behalf of csSystemDriver.
//	Returns time in milliseconds since first invocation.
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "cssys/system.h"
#include <libc.h>
#include <sys/time.h>

csTime csGetClicks ()
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
