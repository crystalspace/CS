//=============================================================================
//
//	Copyright (C)2002 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// OSXUsername.m
//
//	Cocoa/OpenStep-specific function to return the name of the logged
//	in user.
//
//-----------------------------------------------------------------------------
#include "OSXGetUsername.h"
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSString.h>

//-----------------------------------------------------------------------------
// OSXGetUsername
//-----------------------------------------------------------------------------
int OSXGetUsername(char* buff, size_t sz)
{
  int ok = 0;
  if (buff != 0 && sz > 0)
  {
    NSString* username = NSUserName();
    if (username != 0 &&
      ![username isEqualToString:@""] &&
      [username cStringLength] < sz)
    {
      [username getCString:buff maxLength:sz];
      buff[sz - 1] = '\0';
      ok = 1;
    }
  }
  return ok;
}
