/*
    Copyright (C) 2002 by Christopher Nelson

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

#define CS_SYSDEF_PROVIDE_HARDWARE_MMIO
#include "cssysdef.h"
#include "csutil/csmmap.h"

static bool map_window (mmioInfo* info, unsigned int offset, 
  unsigned int len, bool writable) 
{
  unsigned char* p =
    (unsigned char*)MapViewOfFile (info->hFileMapping, 
    writable ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ, 0, offset, len);

  if (p != 0) 
  {
    info->data = p;
    info->file_size = len;

    return true;
  }
  
  return false;
}

bool MemoryMapWindow(mmioInfo* info, mmioInfo* original, unsigned int offset, 
  unsigned int len, bool writable) 
{
  memcpy (info, original, sizeof (mmioInfo));
  if (map_window (info, offset, len, writable)) 
  {
    info->close = false;

    return true;
  }

  return false;
}

bool MemoryMapWindow(mmioInfo* info, char const * filename, 
  unsigned int offset, unsigned int len, bool writable) 
{
  bool ok = false;
  memset (info, 0, sizeof (mmioInfo));
  HANDLE file, mapping = INVALID_HANDLE_VALUE;
  file = CreateFile (
    filename, GENERIC_READ | (writable ? GENERIC_WRITE : 0), 
    FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
  if (file != INVALID_HANDLE_VALUE)
  {
    unsigned int const sz = GetFileSize (file, 0);
    if (sz != 0xFFFFFFFF)
    {
      mapping = CreateFileMapping (file, 0, 
        writable ? PAGE_READWRITE : PAGE_READONLY, 0, 0, 0);
      if (mapping != 0)
      {
	info->hMappedFile = file;
	info->hFileMapping = mapping;
	info->close = true;
        ok = map_window (info, 0, sz, writable);
      }
    }
  }
  if (!ok)
  {
    if (mapping != 0)
      CloseHandle (mapping);
    if (file != INVALID_HANDLE_VALUE)
      CloseHandle (file);
  }
  return ok;
}

// Fills in the mmioInfo struct by mapping in filename.
// Returns true on success, false otherwise.
bool MemoryMapFile (mmioInfo* info, char const* filename)
{  
  bool ok = false;
  memset (info, 0, sizeof (mmioInfo));
  HANDLE file, mapping = INVALID_HANDLE_VALUE;
  file = CreateFile (
    filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
  if (file != INVALID_HANDLE_VALUE)
  {
    unsigned int const sz = GetFileSize(file, 0);
    if (sz != 0xFFFFFFFF)
    {
      mapping = CreateFileMapping (file, 0, PAGE_READONLY, 0, 0, 0);
      if (mapping != 0)
      {
	info->hMappedFile = file;
	info->hFileMapping = mapping;
	info->close = true;
        ok = map_window (info, 0, sz, false);
      }
    }
  }
  if (!ok)
  {
    if (mapping != 0)
      CloseHandle (mapping);
    if (file != INVALID_HANDLE_VALUE)
      CloseHandle (file);
  }
  return ok;
}

void UnMemoryMapFile(mmioInfo* info)
{
  if (info->data != 0)
    UnmapViewOfFile(info->data);

  if (info->close)
  {
    if (info->hFileMapping != INVALID_HANDLE_VALUE)
      CloseHandle (info->hFileMapping);
  
    if (info->hMappedFile != INVALID_HANDLE_VALUE)
      CloseHandle (info->hMappedFile);
  }
}
