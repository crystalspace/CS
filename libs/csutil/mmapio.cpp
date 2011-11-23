/*
    Copyright (C) 2002-2005 by Jorrit Tyberghein
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
#include "csgeom/math.h"
#include "csutil/mmapio.h"
#include "csutil/platformfile.h"
#include "csutil/ref.h"
#include "csutil/sysfunc.h"

#include "iutil/databuff.h"
#include "iutil/vfs.h"

#include <errno.h>

csMemoryMappedIO::csMemoryMappedIO(char const *filename, iVFS* vfs)
{
  const char* realpath = 0;
  if (vfs)
  {
    csRef<iDataBuffer> rpath = vfs->GetRealPath (filename);
    realpath = (char*)(rpath->GetData ());
  }
  else
  {
    realpath = filename;
  }
  valid_mmio_object = false;
  if (realpath)
  {
    valid_platform = OpenNative (realpath);
    if (!valid_platform)
    {
      hMappedFile = CS::Platform::File::Open (realpath, "rb");
    }
    valid_mmio_object = valid_platform || hMappedFile;
  }
}

csMemoryMappedIO::~csMemoryMappedIO()
{
  if (valid_mmio_object)
  {
    if (!valid_platform)
    {
      fclose (hMappedFile);
    }
  }
}

csRef<csMemoryMapping> csMemoryMappedIO::GetData (size_t offset, size_t length)
{
  if (valid_mmio_object)
  {
    uint8* p = 0;
    csRef<PlatformMapping> mapping;
    mapping.AttachNew (new PlatformMapping (this));
    if (!valid_platform)
    {
      if (fseek (hMappedFile, (long)offset, SEEK_SET) != 0)
      {
        csPrintfErr (
          "csMemoryMappedIO::GetData(): fseek error (errno = %d)!\n", errno);
        return 0;
      }
      p = new uint8[length];
      const size_t res = fread (p, 1, length, hMappedFile);
      if (res != length)
      {
        csPrintfErr (
          "csMemoryMappedIO::GetData(): fread error (errno = %d)!\n", errno);
        delete[] p;
        return 0;
      }
    }
    else
    {
      size_t maxSize = GetMaxSize ();
      if (offset + length > maxSize) return 0;
      // Granularity-aligned start and end positions
      size_t alignedStart = (offset / granularity) * granularity;
      size_t alignedEnd = 
        ((offset + length + granularity - 1) / granularity) * granularity;
      // Get mapping for aligned range
      MapWindow (*mapping, alignedStart, 
        csMin (alignedEnd - alignedStart, maxSize));
      p = (uint8*)mapping->realPtr;
      if (!p) return 0;
      p += (offset % granularity);
    }
    mapping->data = p;
    mapping->length = length;
    return mapping;
  }
  return 0;
}
  
void csMemoryMappedIO::FreeMapping (PlatformMapping* mapping)
{
  if (!valid_platform)
    delete[] mapping->data;
  else
    UnmapWindow (*mapping);
}

bool 
csMemoryMappedIO::IsValid()
{
  return valid_mmio_object;
}
