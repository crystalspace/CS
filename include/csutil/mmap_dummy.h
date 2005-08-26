/*
    Copyright (C) 2005 by Jorrit Tyberghein
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

#ifndef __CS_CSUTIL_MMAP_DUMMY_H__
#define __CS_CSUTIL_MMAP_DUMMY_H__

#include "csextern.h"

/**\file
 * Memory mapping for platforms without memory mapping support.
 */

/**
 * Memory mapping for platforms without memory mapping support - it does
 * nothing, so the software emulation kicks in when csMemoryMappedIO is used.
 */
class csPlatformMemoryMappingDummy
{
protected:
  struct PlatformMemoryMapping
  {
    void* realPtr;
  };

  /// Create a new mapping.
  csPlatformMemoryMappingDummy () {}
  /// Destroy file mapping.
  ~csPlatformMemoryMappingDummy () {}
  
  bool OpenNative (const char* filename) { return false; }
  bool Ok() { return false; }
  
  void MapWindow (PlatformMemoryMapping& mapping, size_t offset, size_t len)
  { mapping.realPtr = 0; }
  void UnmapWindow (PlatformMemoryMapping& mapping) {}
};


#endif // __CS_CSUTIL_MMAP_DUMMY_H__
