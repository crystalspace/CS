/*
    Copyright (C) 2000 by Eric Sunshine <sunshine@sunshineco.com>

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

#ifndef __CS_MEMFILE_H__
#define __CS_MEMFILE_H__

/**\file
 * File interface to memory buffer
 */

#include "csextern.h"
#include "iutil/vfs.h"

/**
 * Essentially a raw memory buffer which implements the abstract iFile
 * interface.
 */
class CS_CRYSTALSPACE_EXPORT csMemFile : public iFile
{
public:
  /// Disposition of memory buffer at destruction time.
  enum Disposition
  {
    /// Deallocate with delete[].
    DISPOSITION_DELETE,
    /// Deallocate with free().
    DISPOSITION_FREE,
    /// Ignore; assume that outside agent owns buffer.
    DISPOSITION_IGNORE
  };

public:
  /// Construct an empty memory file.
  csMemFile();
  /// Construct a memory file from an existing memory buffer but do not free.
  csMemFile(const char*, size_t);
  /**
   * Construct a memory file from an existing memory buffer and free later.
   * Note that when writing to the buffer, the original buffer may be 
   * discarded and a new one created due required resizing.
   */
  csMemFile(char*, size_t, Disposition = DISPOSITION_DELETE);
  /**
   * Construct a memory file from an existing data buffer.
   * \param buf The data buffer to use.
   * \param readOnly Whether to treat the buffer as read-only. If \p true,
   *  writing to the memory file will create a copy of the buffer. If 
   *  \p false, changes will affect the buffer. Note that when writing to the
   *  buffer, the original buffer may be discarded and a new one created due
   *  required resizing.
   */
  csMemFile(iDataBuffer* buf, bool readOnly);
  /// Destructor
  virtual ~csMemFile();

  /// Returns "#csMemFile";
  virtual const char* GetName();
  /// Query buffer size
  virtual size_t GetSize();
  /// Check (and clear) file last error status
  virtual int GetStatus();

  /// Read data from buffer
  virtual size_t Read(char* Data, size_t DataSize);
  /// Write data to buffer
  virtual size_t Write(const char* Data, size_t DataSize);
  /// Flush the stream.
  virtual void Flush();
  /// Return true if at end of buffer
  virtual bool AtEOF();
  /// Query current cursor position
  virtual size_t GetPos();
  /// Set current cursor position
  virtual bool SetPos(size_t);

  /**
   * Get entire file data in one go.
   * Changing the file won't affect buffers returned by this function. 
   */
  virtual csPtr<iDataBuffer> GetAllData (bool nullterm = false);
  /**
   * Returns a pointer to the memory buffer.  May return 0 if memory file
   * is empty.  Use GetSize() for size info.
   */
  virtual const char* GetData() const;

  SCF_DECLARE_IBASE;

protected:
  csRef<iDataBuffer> buffer;
  size_t size;
  size_t cursor;
  bool copyOnWrite;
};

#endif // __CS_MEMFILE_H__
