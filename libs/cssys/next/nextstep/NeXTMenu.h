#ifndef __NeXT_NeXTMenu_h
#define __NeXT_NeXTMenu_h
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
// NeXTMenu.h
//
//	Generate a menu from the INI style configuration file NeXTMenu.cfg.
//
//-----------------------------------------------------------------------------
#import "util/inifile.h"
@class Menu;

Menu* NeXTMenuGenerate();

#endif // __NeXT_NeXTMenu_h
