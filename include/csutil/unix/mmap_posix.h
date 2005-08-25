/*
    Copyright (C) 2005 by Jorrit Tyberghein
              (C) 2005 Frank Richter

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

#ifndef __CS_CSUTIL_UNIX_MMAP_POSIX_H__
#define __CS_CSUTIL_UNIX_MMAP_POSIX_H__

#include "csextern.h"

/**\file
 * Memory mapping for platforms with POSIX mmap() support.
 */

/**
 * Memory mapping for platforms with POSIX mmap() support.
 * \remark This class serves as the platform-dependent part of 
 *  csMemoryMappedIO, use that for memory mapping support in your
 *  application.
 */
class CS_CRYSTALSPACE_EXPORT csPlatformMemoryMapping
{
protected:
  struct PlatformMemoryMapping
  {
    void* realPtr;
    size_t realSize;
  };

  int hMappedFile;
  size_t granularity;

  /// Create a new mapping.
  csPlatformMemoryMapping ();
  /// Destroy file mapping.
  ~csPlatformMemoryMapping ();
  
  bool OpenNative (const char* filename);
  bool Ok() { return hMappedFile != -1; }
  
  /**
   * Map a part of the file into memory and return a pointer to mapped data.
   * \a offset and \a len are the offset and length of the part of the file to
   * map. Both should be multiples of the granularity returned by 
   * GetPageGranularity(); otherwise, the function may fail. Returns 0 in case
   * of failure.
   */
  void MapWindow (PlatformMemoryMapping& mapping, size_t offset, size_t len);
  /// Unmap a mapping of the file.
  void UnmapWindow (PlatformMemoryMapping& mapping);
};


#endif // __CS_CSUTIL_UNIX_MMAP_POSIX_H__
