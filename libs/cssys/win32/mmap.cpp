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

#define CS_SYSDEF_PROVIDE_HARDWARE_MMIO
#include "cssysdef.h"

// Fills in the mmioInfo struct by mapping in filename.
// Returns true on success, false otherwise.
bool MemoryMapFile(mmioInfo* info, char const* filename)
{  
  bool ok = false;
  HANDLE file, mapping = INVALID_HANDLE_VALUE;
  file = CreateFile(
    filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
  if (file != INVALID_HANDLE_VALUE)
  {
    unsigned int const sz = GetFileSize(file, NULL);
    if (sz != 0xFFFFFFFF)
    {
      mapping = CreateFileMapping(file, NULL, PAGE_READONLY, 0, 0, NULL);
      if (mapping != NULL)
      {
	unsigned char* p =
	  (unsigned char*)MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, sz);
	if (p != NULL)
	{
	  info->hMappedFile = file;
	  info->hFileMapping = mapping;
	  info->data = p;
	  info->file_size = sz;
	  ok = true;
	}
      }
    }
  }
  if (!ok)
  {
    if (mapping != NULL)
      CloseHandle(mapping);
    if (file != INVALID_HANDLE_VALUE)
      CloseHandle(file);
  }
  return ok;
}

void UnMemoryMapFile(mmioInfo* info)
{
  if (info->data != NULL)
    UnmapViewOfFile(info->data);

  if (info->hMappedFile != INVALID_HANDLE_VALUE)
    CloseHandle(info->hMappedFile);

  if (info->hFileMapping != INVALID_HANDLE_VALUE)
    CloseHandle(info->hFileMapping);
}
