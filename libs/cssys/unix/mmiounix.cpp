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

// Needed for Memory-Mapped IO functions below.
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

bool MemoryMapFile(mmioInfo* info, char const* filename)
{   
  bool ok = false;
  struct stat st;
  int const fd = open(filename, O_RDONLY);
  if (fd != -1 && fstat(fd, &st) != -1)
  {
    unsigned char* p=(unsigned char*)mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if ((int)p != -1)
    {
      info->hMappedFile = fd;
      info->data = p;
      info->file_size = st.st_size;
      ok = true;
    }
  }
  if (!ok && fd != -1)
    close(fd);
  return ok;
}

void UnMemoryMapFile(mmioInfo* info)
{
  if (info->data != 0)
#ifdef OS_SOLARIS  
    munmap((char *)info->data, info->file_size);
#else
    munmap(info->data, info->file_size);
#endif
  if (info->hMappedFile != -1)
    close(info->hMappedFile);
}
