#ifndef __OSX_csosdefs_h
#define __OSX_csosdefs_h
//=============================================================================
//
//	Copyright (C)1999-2002 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// csosdefs.h
//
//	MacOS/X-specific interface to common functionality.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// The 2D graphics driver used by the software renderer on this platform.
//-----------------------------------------------------------------------------
#undef  CS_SOFTWARE_2D_DRIVER
#define CS_SOFTWARE_2D_DRIVER "crystalspace.graphics2d.coregraphics"
#define CS_SOFTWARE_2D_DRIVER_COCOA "crystalspace.graphics2d.cocoa"

#undef  CS_OPENGL_2D_DRIVER
#define CS_OPENGL_2D_DRIVER "crystalspace.graphics2d.glosx"

#undef  CS_SOUND_DRIVER
#define CS_SOUND_DRIVER "crystalspace.sound.driver.coreaudio"


//-----------------------------------------------------------------------------
// Provide CS_MAXPATHLEN, PATH_SEPARATOR, PATH_DELIMITER with proper values.
//-----------------------------------------------------------------------------
#include <sys/param.h>
#define CS_MAXPATHLEN MAXPATHLEN
#define PATH_SEPARATOR '/'
#define PATH_DELIMITER ':'


//-----------------------------------------------------------------------------
// Pull in definitions for getwd(), ntohl(), htonl(), select(), etc.
// NOTE: On MacOS/X, libc.h pulls in sys/mount.h which pulls in net/radix.h
// which defines a macro named Free().  This macro interferes with several
// Crystal Space classes which have methods named Free(), so we must
// #undef it.
//-----------------------------------------------------------------------------
#if defined(CS_SYSDEF_PROVIDE_GETCWD)  || \
    defined(CS_SYSDEF_PROVIDE_SOCKETS) || \
    defined(CS_SYSDEF_PROVIDE_SELECT)  || \
    defined(CS_SYSDEF_PROVIDE_ACCESS)
#include <libc.h>
#undef Free
#endif

#if defined(CS_SYSDEF_PROVIDE_SELECT)
#include <string.h> // For memset()
#define bzero(b,len) memset(b,0,len) /* bzero used by FD_ZERO */
#undef CS_SYSDEF_PROVIDE_SELECT
#endif


//-----------------------------------------------------------------------------
// NeXT does not properly support Posix 'dirent', so fake it with 'direct'.
//-----------------------------------------------------------------------------
#ifdef CS_SYSDEF_PROVIDE_DIR

#include <sys/dir.h>
#include <sys/dirent.h>
#define __NEED_GENERIC_ISDIR

#endif // CS_SYSDEF_PROVIDE_DIR


//-----------------------------------------------------------------------------
// Note by Matt Reda: I did some rough testing of csQint() and friends on the
// PowerPC.  It appears to work ok, but is actually slower.  Some simple
// tests show that csQint() is roughly twice as slow as a cast from double
// to long
//-----------------------------------------------------------------------------
#define CS_NO_IEEE_OPTIMIZATIONS


//-----------------------------------------------------------------------------
// MacOS/X mmap() functionality for memory-mapped I/O.
//-----------------------------------------------------------------------------
#if defined(CS_SYSDEF_PROVIDE_HARDWARE_MMIO)

#define CS_HAS_MEMORY_MAPPED_IO 1

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Unix specific memory mapped I/O platform dependent stuff
struct mmioInfo
{          
  int file;               // Handle to the mapped file.
  unsigned int file_size; // File size.
  unsigned char* data;    // Base pointer to the data.
};

// Fill in the mmioInfo struct by mapping in filename.
// Returns true on success, false otherwise.
inline bool MemoryMapFile(mmioInfo* info, char const* filename)
{   
  bool ok = false;
  struct stat st;
  int const fd = open(filename, O_RDONLY);
  if (fd != -1 && fstat(fd, &st) != -1)
  {
    unsigned char* p=(unsigned char*)mmap(0, st.st_size, PROT_READ, 0, fd, 0);
    if ((int)p != -1)
    {
      info->file = fd;
      info->data = p;
      info->file_size = st.st_size;
      ok = true;
    }
  }
  if (!ok && fd != -1)
    close(fd);
  return ok;
}

inline void UnMemoryMapFile(mmioInfo* info)
{
  if (info->data != 0)
    munmap(info->data, info->file_size);
  if (info->file != -1)
    close(info->file);
}

#endif // CS_SYSDEF_PROVIDE_HARDWARE_MMIO

#endif // __OSX_csosdefs_h
