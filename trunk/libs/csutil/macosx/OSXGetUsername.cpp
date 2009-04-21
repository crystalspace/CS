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
// OSXGetUsername.cpp
//
//	Platform-specific function to return the name of the logged in user.
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "csutil/csstring.h"
#include "csutil/sysfunc.h"
#include "OSXGetUsername.h"

//-----------------------------------------------------------------------------
// csGetUsername
//-----------------------------------------------------------------------------
csString csGetUsername()
{
  csString username;
  char buff[512];
  if (OSXGetUsername(buff, sizeof(buff)))
    username = buff;
  username.Trim();
  return username;
}
