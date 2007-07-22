//=============================================================================
//
//	Copyright (C)1999-2001,2007 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// OSXLoadLibrary.m
//
//	Mac OS X dynamic library loading and symbol lookup.
//
//-----------------------------------------------------------------------------
#include "OSXLoadLibrary.h"
#include <stdlib.h>
#include <unistd.h> // access()
#include <mach-o/dyld.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSString.h>

static char* CS_ERROR_MESSAGE = 0;

//-----------------------------------------------------------------------------
// clear_error_message
//-----------------------------------------------------------------------------
static void clear_error_message()
{
  if (CS_ERROR_MESSAGE != 0)
  {
    free(CS_ERROR_MESSAGE);
    CS_ERROR_MESSAGE = 0;
  }
}


//-----------------------------------------------------------------------------
// set_error_message
//-----------------------------------------------------------------------------
static void set_error_message(NSString* s)
{
  clear_error_message();
  CS_ERROR_MESSAGE = (char*)malloc([s cStringLength] + 1);
  [s getCString:CS_ERROR_MESSAGE];
}


//-----------------------------------------------------------------------------
// report_dyld_error()
//-----------------------------------------------------------------------------
static void report_dyld_error()
{
  NSLinkEditErrors err;
  int code;
  char const* path;
  char const* msg;
  NSLinkEditError(&err, &code, &path, &msg);
  set_error_message([NSString stringWithFormat:
    @"Dynamic linker error (err=%d, code=%d): %s (%s)", err, code, msg, path]);
}


//-----------------------------------------------------------------------------
// OSXLoadLibrary
//-----------------------------------------------------------------------------
OSXLibraryHandle OSXLoadLibrary(char const* path)
{
  OSXLibraryHandle handle = 0;
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  if (access(path, F_OK) == 0)
  {
    NSObjectFileImage image = 0;
    NSObjectFileImageReturnCode rc =
      NSCreateObjectFileImageFromFile(path, &image);
    if (rc == NSObjectFileImageSuccess)
      {
	NSModule const module =
	  NSLinkModule(image, path, NSLINKMODULE_OPTION_RETURN_ON_ERROR);
	if (module != 0)
	  handle = (OSXLibraryHandle)module;
	else
	  report_dyld_error();
      }
    else
      set_error_message([NSString stringWithFormat:
        @"NSCreateObjectFileImageFromFile(%s) failed (error %d)", path, rc]);
  }
  else
    set_error_message(
      [NSString stringWithFormat:@"File does not exist (%s)", path]);
  [pool release];
  return handle;
}


//-----------------------------------------------------------------------------
// OSXGetLibrarySymbol
//-----------------------------------------------------------------------------
void* OSXGetLibrarySymbol(OSXLibraryHandle handle, char const* s)
{
  void* address = 0;
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  NSString* symname = [NSString stringWithFormat:@"_%s", s];
  NSSymbol symbol =
    NSLookupSymbolInModule((NSModule)handle, [symname cString]);
  if (symbol != 0)
  {
    address = NSAddressOfSymbol(symbol);
    if (address == 0)
      NSLog(@"Symbol undefined: %@", symbol);
  }
  else
    report_dyld_error();
  [pool release];
  return address;
}


//-----------------------------------------------------------------------------
// OSXUnloadLibrary
//-----------------------------------------------------------------------------
int OSXUnloadLibrary(OSXLibraryHandle handle)
{
  (void)handle;
  return 0; // Unimplemented (0=failure).
}


//-----------------------------------------------------------------------------
// OSXGetLibraryError
//-----------------------------------------------------------------------------
char const* OSXGetLibraryError()
{
  return CS_ERROR_MESSAGE;
}
