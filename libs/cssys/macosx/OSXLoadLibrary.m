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
// OSXLoadLibrary.m
//
//	OpenStep-specific dynamic library loading and symbol lookup.
//
//	MacOS/X Server and OpenStep handle symbolic references within
//	dynamically loaded libraries in a different manner than most other
//	platforms.  On most platforms, all symbols linked into the plug-in
//	module are considered private unless explicitly exported.  On MacOS/X
//	Server and OpenStep, on the other hand, all symbols linked into the
//	plug-in module are public by default.  This can easily lead to
//	symbolic clashes when two or more plug-in modules have been linked
//	with the same libraries or define like-named symbols.  To work around
//	this problem, this dynamic module loader takes advantage the
//	NSLinkEditErrorHandlers facility to instruct DYLD to ignore duplicate
//	symbolic reference at module load time.
//
//	One alternative approach for dealing with this problem would be to
//	ensure that the none of the plug-in modules export like-named symbols,
//	but this is difficult to control in an environment where plug-in
//	modules may come from a variety of sources.  A second alternative
//	would be to strip unneeded symbols from the plug-in module at build
//	time.  This is a fairly decent insurance against symbolic collisions
//	and, in fact, this scheme has been used successfully on NextStep.
//	Unfortunately, it is not so easily employed on OpenStep since it also
//	strips away references to Objective-C classes which are defined within
//	the plug-in itself, and the OpenStep run-time complains bitterly about
//	these missing symbols even though the NextStep run-time has no problem
//	with their absence.  (It is possible to instruct the "strip" utility
//	to preserve these symbols, but doing so requires manual intervention
//	and potential future maintenance costs as the list of exported symbols
//	changes.)
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
// handle_collision
//	When a symbolic collision occurs, choose to publish one of the
//	symbols; the choice of which is rather arbitrary.
//-----------------------------------------------------------------------------
static NSModule handle_collision(NSSymbol sym, NSModule m_old, NSModule m_new)
{
  return m_old;
}


//-----------------------------------------------------------------------------
// initialize_loader
//-----------------------------------------------------------------------------
static void initialize_loader()
{
  static BOOL installed = NO;
  if (!installed)
  {
    static NSLinkEditErrorHandlers const handlers = { 0, handle_collision, 0 };
    NSInstallLinkEditErrorHandlers((NSLinkEditErrorHandlers*)&handlers);
    installed = YES;
  }
}


//-----------------------------------------------------------------------------
// OSXLoadLibrary
//-----------------------------------------------------------------------------
OSXLibraryHandle OSXLoadLibrary(char const* path)
{
  OSXLibraryHandle handle = 0;
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  initialize_loader();
  if (access(path, F_OK) == 0)
  {
    NSObjectFileImage image = 0;
    NSObjectFileImageReturnCode rc =
      NSCreateObjectFileImageFromFile(path, &image);
    if (rc == NSObjectFileImageSuccess)
    {
      NSModule const module = NSLinkModule(image, path, TRUE);
      if (module != 0)
	handle = (OSXLibraryHandle)module;
      else
	set_error_message(
	  [NSString stringWithFormat:@"NSLinkModule(%s) failed", path]);
    }
    else
      set_error_message([NSString stringWithFormat:
	@"NSCreateObjectFileImageFromFile(%s) failed (error %d)",
	path, rc]);
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
  NSString* symbol = [NSString stringWithFormat:@"_%s", s];
  address = NSAddressOfSymbol(NSLookupAndBindSymbol([symbol cString]));
  if (address == 0)
    NSLog(@"Symbol undefined: %@", symbol);
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
