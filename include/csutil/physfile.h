/*
    Copyright (C) 2003 by Eric Sunshine <sunshine@sunshineco.com>

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

#ifndef __CS_PHYSFILE_H__
#define __CS_PHYSFILE_H__

#include "csextern.h"
#include "iutil/vfs.h"
#include "csstring.h"
#include <stdio.h>

/**
 * An implementation of the abstract iFile interface for real files within
 * the physical filesystem.
 */
class CS_CRYSTALSPACE_EXPORT csPhysicalFile : public iFile
{
public:
  /**
   * Construct from a filename using fopen() access flags (i.e. "r", "rb",
   * "w+", etc.).  It is usually a good idea to open files in binary mode (i.e.
   * "rb, "wb", etc.).  This ensures that operations on "number of bytes"
   * operate as expected.  For instance, requesting 10 bytes with Read() will
   * return 10 bytes in binary mode (assuming end-of-file has not been
   * reached), whereas in text mode, fewer bytes might be returned (if line
   * terminators of the form CFLF have been collapsed to LF at read time).
   */
  csPhysicalFile(char const* path, char const* mode);
  /**
   * Construct from an existing FILE*.  If take_ownership is true, the FILE*
   * will be closed via fclose() when the csPhysicalFile is destroyed.  The
   * optional path argument is used only to seed the stored name for use by the
   * GetName() method.  If not supplied, then an opaque, essentially
   * meaningless name is returned by GetName().  It is usually a good idea to
   * open files in binary mode (i.e. "rb, "wb", etc.).  This ensures that
   * operations on "number of bytes" operate as expected.  For instance,
   * requesting 10 bytes with Read() will return 10 bytes in binary mode
   * (assuming end-of-file has not been reached), whereas in text mode, fewer
   * bytes might be returned (if line terminators of the form CFLF have been
   * collapsed to LF at read time).
   */
  csPhysicalFile(FILE*, bool take_ownership, char const* path = 0);
  /// Destructor
  virtual ~csPhysicalFile();

  /**
   * Returns the path used to construct the object, or "#csPhysicalFile" if no
   * path was given when constructed from an existing FILE*.
   */
  virtual char const* GetName();
  /// Query file size.
  virtual size_t GetSize();
  /// Check (and clear) file last error status.
  virtual int GetStatus();

  /// Read data from file.
  virtual size_t Read(char* buffer, size_t nbytes);
  /// Write data to buffer.
  virtual size_t Write(char const* data, size_t nbytes);
  /// Flush the stream.
  virtual void Flush();
  /// Return true if at end of buffer
  virtual bool AtEOF();
  /// Query current cursor position,
  virtual size_t GetPos();
  /// Set current cursor position.
  virtual bool SetPos(size_t);

  /**
   * Get entire file data in one go.  Creates a copy of the data, so changing
   * the file won't affect any buffers previously returned by this function.
   * Be aware that, for large files, this can allocate a significant amount of
   * memory.  If nullterm is true, then a null terminator is appended to the
   * returned data.
   */
  virtual csPtr<iDataBuffer> GetAllData(bool nullterm = false);

  SCF_DECLARE_IBASE;

protected:
  FILE* fp;
  csString path;
  bool owner;
  int last_error;
};

#endif // __CS_PHYSFILE_H__
