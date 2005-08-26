/*
    Copyright (C) 2002-2005 by Christopher Nelson
		  2005 by Frank Richter

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

#include "csutil/win32/mmap.h"

csPlatformMemoryMappingWin32::csPlatformMemoryMappingWin32 () :
  hMappedFile (INVALID_HANDLE_VALUE), hFileMapping (0)
{
  SYSTEM_INFO si;
  GetSystemInfo (&si);
  granularity = si.dwPageSize;
}

csPlatformMemoryMappingWin32::~csPlatformMemoryMappingWin32 ()
{
  if (hFileMapping != 0)
    CloseHandle (hFileMapping);

  if (hMappedFile != INVALID_HANDLE_VALUE)
    CloseHandle (hMappedFile);
}
  
bool csPlatformMemoryMappingWin32::OpenNative (const char* filename)
{
  hMappedFile = CreateFile (
    filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
  if (hMappedFile != INVALID_HANDLE_VALUE)
  {
    hFileMapping = CreateFileMapping (hMappedFile, 0, 
      PAGE_READONLY, 0, 0, 0);
    if (hFileMapping == 0) CloseHandle (hMappedFile);
  }
  return Ok();
}

bool csPlatformMemoryMappingWin32::Ok() 
{ 
  return hFileMapping != 0; 
}
  
void csPlatformMemoryMappingWin32::MapWindow (PlatformMemoryMapping& mapping, 
					       size_t offset, size_t len)
{
  LARGE_INTEGER offs;
  offs.QuadPart = offset;
  void* p = MapViewOfFile (hFileMapping, FILE_MAP_READ, offs.HighPart, 
    offs.LowPart, len);
  mapping.realPtr = p;
}

void csPlatformMemoryMappingWin32::UnmapWindow (PlatformMemoryMapping& mapping)
{
  if (mapping.realPtr != 0)
    UnmapViewOfFile (mapping.realPtr);
}
