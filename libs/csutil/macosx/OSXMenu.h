#ifndef __MACOSX_OSXMenu_h
#define __MACOSX_OSXMenu_h
//=============================================================================
//
//	Copyright (C)1999-2003 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// OSXMenu.h
//
//	Generate a menu from a configuration file definition.
//
//-----------------------------------------------------------------------------
#include "OSXConfigFile.h"
#import <Foundation/NSObject.h>
@class NSMenu;

#if defined(__cplusplus)
extern "C" {
#endif

NSMenu* OSXMenuGenerate(id assistant, char const* menu_ident, OSXConfigHandle);

#if defined(__cplusplus)
}
#endif

#endif // __MACOSX_OSXMenu_h
