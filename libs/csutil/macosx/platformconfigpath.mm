/*
    Copyright (C) 2004 by Eric Sunshine

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSString.h>
#import <Foundation/NSPathUtilities.h>

#include "cssysdef.h"
#include "csutil/csstring.h"

csString csGetPlatformConfigPath (const char* key, bool /*local*/)
{
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  NSArray* paths = NSSearchPathForDirectoriesInDomains(
    NSLibraryDirectory, NSUserDomainMask, YES);
  NSString* nsdir = [[paths objectAtIndex:0]
    stringByAppendingPathComponent:@"Application Support"];
  csString dir([nsdir fileSystemRepresentation]);
  [pool release];
  return dir;
} 
