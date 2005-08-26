/*
    Copyright (C) 2002-2005 by Christopher Nelson
	      (C) 2005 by Frank Richter

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

#include "cssysdef.h"

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "csutil/mmap_posix.h"

csPlatformMemoryMappingPosix::csPlatformMemoryMappingPosix ()
{
  hMappedFile = -1;
  granularity = getpagesize();
}

csPlatformMemoryMappingPosix::~csPlatformMemoryMappingPosix ()
{
  if (hMappedFile != -1)
    close (hMappedFile);
}

bool csPlatformMemoryMappingPosix::OpenNative (const char* filename)
{
  hMappedFile = open(filename, O_RDONLY);
  return Ok();
}

void csPlatformMemoryMappingPosix::MapWindow (PlatformMemoryMapping& mapping, 
                                         size_t offset, size_t len)
{
  if (hMappedFile == -1) return;
  
  mapping.realPtr = mmap(0, len, PROT_READ, MAP_PRIVATE, hMappedFile, offset);
  if (mapping.realPtr == (void*)-1) mapping.realPtr = 0;
  mapping.realSize = len;
}

void csPlatformMemoryMappingPosix::UnmapWindow (PlatformMemoryMapping& mapping)
{
  if (mapping.realPtr != 0)
    munmap (mapping.realPtr, mapping.realSize);
}

