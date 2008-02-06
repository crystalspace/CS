//=============================================================================
//
//	Copyright (C) 2002 by Matt Reda <mreda@mac.com>
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
// License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library; if not, write to the Free Software Foundation,
// Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//=============================================================================
//-----------------------------------------------------------------------------
// OSXInstPath.m
//
//	Cocoa-specific function to determine installation path.
//
//-----------------------------------------------------------------------------
#include "OSXInstallPath.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSString.h>

int OSXGetInstallPath(char* buff, size_t sz, char path_sep)
{
  int ok = 0;
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  NSUserDefaults* defs = [NSUserDefaults standardUserDefaults];
  NSString* s = [defs stringForKey:@"CrystalSpaceRootIgnore"];
        
  buff[0] = '\0';
  if (s == 0 || [s isEqualToString:@""] ||
     (![s hasPrefix:@"Y"] && ![s hasPrefix:@"y"] &&	// Yes
      ![s hasPrefix:@"T"] && ![s hasPrefix:@"t"] &&	// True
      ![s hasPrefix:@"O"] && ![s hasPrefix:@"o"] &&	// On
      ![s hasPrefix:@"1"]))				// 1
  {
    s = [defs stringForKey:@"CrystalSpaceRoot"];
    if (s != 0 && ![s isEqualToString:@""])
    {
      NSMutableString* path = [s mutableCopy];
      int const n = [path length];
      // >=2 to avoid stripping "/" from path if path is root directory.
      if (n >= 2 && [path characterAtIndex:n - 1] != path_sep)
        [path appendFormat:@"%c", path_sep];
      [path getFileSystemRepresentation:buff maxLength:sz];
      [path release];
      ok = 1;
    }
  }
  [pool release];
  return ok;
}
