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

#include "cssysdef.h"
#include <stdlib.h>
#include "csutil/databuf.h"
#include "csutil/memfile.h"
#include "csutil/parasiticdatabuffer.h"
#include "csgeom/math.h"

namespace
{
  class DataBufferFreeCS : public csDataBuffer
  {
  public:
    DataBufferFreeCS (char* data, size_t size) : 
      csDataBuffer (data, size, false) { }
    virtual ~DataBufferFreeCS()
    {
      cs_free (csDataBuffer::GetData ());
    }
  };

  class DataBufferFreePlatform : public csDataBuffer
  {
  public:
    DataBufferFreePlatform (char* data, size_t size) : 
      csDataBuffer (data, size, false) { }
    virtual ~DataBufferFreePlatform()
    {
      free (csDataBuffer::GetData ());
    }
  };
}

const char* csMemFile::GetName() { return "#csMemFile"; }
const char* csMemFile::GetData() const 
{ return buffer ? buffer->GetData() : 0; }
size_t csMemFile::GetSize() { return size; }
int csMemFile::GetStatus() { return status; }
void csMemFile::Flush() {}
bool csMemFile::AtEOF() { return (cursor >= size); }
size_t csMemFile::GetPos() { return cursor; }
bool csMemFile::SetPos(size_t p) { cursor = p < size ? p : size; return true; }

csMemFile::csMemFile() 
  : scfImplementationType (this),
  status (VFS_STATUS_OK), size(0), cursor(0), copyOnWrite (true), readOnly (false)
{ 
}

csMemFile::csMemFile(const char* p, size_t s)
  : scfImplementationType (this),
  status (VFS_STATUS_OK), size(s), cursor(0), copyOnWrite (true), readOnly (false)
{ 
  buffer.AttachNew (new csDataBuffer ((char*)p, s, false));
}

csMemFile::csMemFile(char* p, size_t s, Disposition d) 
  : scfImplementationType (this),
  status (VFS_STATUS_OK), size(s), cursor(0), copyOnWrite (false), readOnly (false)
{ 
  if (d == DISPOSITION_CS_FREE)
    buffer.AttachNew (new DataBufferFreeCS (p, s));
  else if (d == DISPOSITION_PLATFORM_FREE)
    buffer.AttachNew (new DataBufferFreePlatform (p, s));
  else
    buffer.AttachNew (new csDataBuffer ((char*)p, s, 
      d == DISPOSITION_DELETE));
}

csMemFile::csMemFile(iDataBuffer* buf, bool readOnly) 
  : scfImplementationType (this),
  buffer(buf), status (VFS_STATUS_OK), size(buf ? buf->GetSize() : 0), cursor(0), 
  copyOnWrite (readOnly), readOnly (readOnly)
{
}

csMemFile::~csMemFile()
{
}

size_t csMemFile::Read(char* Data, size_t DataSize)
{
  if (cursor >= size)
  {
    status = VFS_STATUS_IOERROR;
    return 0;
  }
  const size_t remaining = cursor < size ? size - cursor : 0;
  const size_t nbytes = DataSize < remaining ? DataSize : remaining;
  status = (nbytes == DataSize) ? VFS_STATUS_OK : VFS_STATUS_IOERROR;
  if (nbytes != 0)
    memcpy (Data, buffer->GetData() + cursor, nbytes);
  cursor += nbytes;
  return nbytes;
}

size_t csMemFile::Write(const char* Data, size_t DataSize)
{
  if (readOnly)
  {
    status = VFS_STATUS_ACCESSDENIED;
    return 0;
  }

  size_t written = 0;
  if (DataSize != 0 && Data != 0)
  {
    size_t new_cursor = cursor + DataSize;
    size_t capacity = buffer ? buffer->GetSize() : 0;
    if (capacity < new_cursor)
    {
      const size_t maxCapInc = 1024*1024;

      if (capacity == 0)
        capacity = 1024;
      while (capacity < new_cursor)
        capacity += csMin (capacity, maxCapInc);

      copyOnWrite = true;
    }
    if (copyOnWrite)
    {
      csRef<iDataBuffer> newBuf;
      newBuf.AttachNew (new csDataBuffer (capacity));
      if (buffer)
	memcpy (newBuf->GetData(), buffer->GetData(), buffer->GetSize());
      buffer = newBuf;
    }

    memcpy(buffer->GetData() + cursor, Data, DataSize);
    cursor = new_cursor;
    if (new_cursor > size)
      size = new_cursor;
    written = DataSize;

    copyOnWrite = false;
  }
  status = VFS_STATUS_OK;
  return written;
}

csPtr<iDataBuffer> csMemFile::GetAllData (bool nullterm)
{
  status = VFS_STATUS_OK;
  if (nullterm)
  {
    char* data = new char [size + 1];
    if (buffer)
      memcpy (data, buffer->GetData(), size);
    *(data + size) = 0;
    iDataBuffer *db = new csDataBuffer (data, size);
    return csPtr<iDataBuffer> (db);
  }
  else
  {
    if (!buffer) return 0;
    copyOnWrite = true;
    if (buffer->GetSize() == size)
    {
      return csPtr<iDataBuffer> (buffer);
    }
    else
    {
      iDataBuffer *db = new csParasiticDataBuffer (buffer,
	0, size);
      return csPtr<iDataBuffer> (db);
    }
  }
}

csPtr<iDataBuffer> csMemFile::GetAllData (CS::Memory::iAllocator* /*allocator*/)
{
  return GetAllData ();
}

csPtr<iFile> csMemFile::GetPartialView (size_t offset, size_t size)
{
  if (!buffer) return 0;
  copyOnWrite = true;
  size_t bufSize (csMin (size, GetSize() - offset));
  csRef<iDataBuffer> viewBuffer;
  if ((offset == 0) && (buffer->GetSize() == bufSize))
  {
    viewBuffer = buffer;
  }
  else
  {
    viewBuffer.AttachNew (new csParasiticDataBuffer (buffer, offset, bufSize));
  }

  csMemFile* new_file (new csMemFile (viewBuffer, true));
  return csPtr<iFile> (new_file);
}

void csMemFile::Empty()
{
  buffer.Invalidate();
  size = 0;
  cursor = 0;
  copyOnWrite = true;
  readOnly = false;
}
