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

#ifndef __CS_MEMORY_MAPPED_IO__
#define __CS_MEMORY_MAPPED_IO__

/**\file 
 * Platform-independent Memory-Mapped IO
 */

#include "csextern.h"
#include "bitarray.h"
#include "ref.h"
#include "refcount.h"

struct iVFS;

/* Note: the "#define csPlatformMemoryMapping ..." is done to appease doxygen,
 * which produces funny results when all the memory mapping base classes
 * would be in unison named "csPlatformMemoryMapping".
 */
#if defined(CS_PLATFORM_WIN32)
  #include "win32/mmap.h"
  #define csPlatformMemoryMapping csPlatformMemoryMappingWin32
#elif defined(CS_HAVE_POSIX_MMAP)
  #include "unix/mmap_posix.h"
  #define csPlatformMemoryMapping csPlatformMemoryMappingPosix
#else
  /* @@@ FIXME: dummy mmap */
#endif

/**
 * Memory mapping, as returned by csMemoryMappedIO::GetData().
 */
class csMemoryMapping : public csRefCount
{
public:
  /// Get size of mapped data
  virtual size_t GetLength() = 0;
  /// Get pointer to mapped data
  virtual void* GetData() = 0;
};

/**
 * Defines a simple memory-mapped IO class that is portable.
 */  
class CS_CRYSTALSPACE_EXPORT csMemoryMappedIO : public csPlatformMemoryMapping,
                                                 public csRefCount
{
private:
  /// Set to true if this object is valid
  bool valid_mmio_object;

  /// Handle to the mapped file 
  FILE *hMappedFile;
  
  /// true if \c platform contains valid data.
  bool valid_platform;
public:
  /** 
   * Block size is the size of blocks that you want to get from the file,
   * filename is the name of the file to map. If you supply a VFS,
   * \c filename is tried to be resolved to a native path. Otherwise,
   * \c filename is used as is, hence it must already be a native path.
   */
  csMemoryMappedIO(char const *filename, iVFS* vfs = 0);

  /** 
   * Destroys the mmapio object, closes open files, and releases memory.
   */
  virtual ~csMemoryMappedIO();

  /**
   * Obtain a piece of the mapped file.
   */
  csRef<csMemoryMapping> GetData (size_t offset, size_t length);
  
  /**
   * Returns true the memory was mapped successfully.
   */
  bool IsValid();

private:

  struct PlatformMapping : public PlatformMemoryMapping,
                           public csMemoryMapping
  {
    csRef<csMemoryMappedIO> parent;
    size_t length;
    uint8* data;
    
    PlatformMapping (csMemoryMappedIO* parent) : parent(parent) {}
    virtual ~PlatformMapping() { parent->FreeMapping (this); }
    virtual size_t GetLength() { return length; }
    virtual void* GetData() { return data; }
  };
  friend class PlatformMapping;
  
  void FreeMapping (PlatformMapping* mapping);
};

#endif // __CS_MEMORY_MAPPED_IO__

