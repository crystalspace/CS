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

// Needed for Memory-Mapped IO functions below.
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static bool map_window(mmioInfo * info, int fd, unsigned int offset, unsigned int len, bool writable) 
{
  unsigned char * p = (unsigned char*)mmap(0, len, PROT_READ | (writable ? PROT_WRITE : 0), MAP_PRIVATE, fd, offset);

  if ((int)p != -1) {
    info->hMappedFile = fd;
    info->data = p;
    info->file_size = len;
    info->close = true;

    return true;
  }
  
  return false;
}

bool MemoryMapWindow(mmioInfo * info, mmioInfo * original, unsigned int offset, unsigned int len, bool writable) 
{
  if (map_window(info, original->hMappedFile, offset, len, writable)) {
    info->close = false;

    return true;
  }

  return false;
}

bool MemoryMapWindow(mmioInfo * info, char const * filename, unsigned int offset, unsigned int len, bool writable) 
{
  int fd = open(filename, (writable ? O_RDWR : O_RDONLY));

  if (fd != -1 && map_window(info, fd, offset, len, writable))
    return true;

  close(fd);

  return false;
}

bool MemoryMapFile(mmioInfo* info, char const* filename)
{   
  struct stat st;
  int const fd = open(filename, O_RDONLY);

  if (fd != -1 && fstat(fd, &st) != -1 && map_window(info, fd, 0, st.st_size, false))
    return true;

  close(fd);
  
  return false;
}

void UnMemoryMapFile(mmioInfo* info)
{
  if (info->data != 0)
    munmap((char*)info->data, info->file_size);

  if (info->hMappedFile != -1 && info->close)
    close(info->hMappedFile);
}

