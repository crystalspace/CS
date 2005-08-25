/*
    Copyright (C) 2002 by Jorrit Tyberghein

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
#include "csutil/mmapio.h"
#include "csutil/ref.h"

#include "iutil/databuff.h"
#include "iutil/vfs.h"

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
      hMappedFile = fopen (realpath, "rb");
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
      p = new uint8[length];
      fseek (hMappedFile, (long)offset, SEEK_SET);
      fread (p, 1, length, hMappedFile);
    }
    else
    {
      size_t pageStart = offset / granularity;
      size_t pageEnd = (offset + length + granularity - 1) / granularity;
      MapWindow (*mapping, pageStart * granularity, 
          (pageEnd - pageStart) * granularity);
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
