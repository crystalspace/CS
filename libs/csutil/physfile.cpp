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

#include "cssysdef.h"
#include "csgeom/math.h"
#include "csutil/parasiticdatabuffer.h"
#include "csutil/platformfile.h"
#include "csutil/physfile.h"
#include "csutil/databuf.h"
#include <stdlib.h>
#include <sys/stat.h>

class csPhysicalFile::PartialView : public scfImplementation1<PartialView, iFile>
{
  csRef<csPhysicalFile> parent;
  size_t pos;
  size_t offset;
  size_t size;

  int status;
public:
  PartialView (csPhysicalFile* parent, size_t offset, size_t size)
    : scfImplementationType (this), parent (parent), pos (0), offset (offset), size (size),
      status (VFS_STATUS_OK)
  {}

  char const* GetName();
  size_t GetSize();
  int GetStatus();

  size_t Read(char* buffer, size_t nbytes);
  size_t Write(char const* data, size_t nbytes);
  void Flush();
  bool AtEOF();
  size_t GetPos();
  bool SetPos(size_t);

  csPtr<iDataBuffer> GetAllData(bool nullterm = false);
  csPtr<iDataBuffer> GetAllData (CS::Memory::iAllocator* allocator);

  csPtr<iFile> GetPartialView (size_t offset, size_t size = (size_t)~0);
};

int csPhysicalFile::GetStatus() { return last_error; }

csPhysicalFile::csPhysicalFile(char const* apath, char const* mode) 
  : scfImplementationType (this), fp(0), path(apath), owner(true), 
  last_error(VFS_STATUS_OK)
{
  bool file_must_exist (*mode == 'r');
  if (file_must_exist)
  {
    struct stat buf;
    if (stat (apath, &buf))
    {
      last_error = VFS_STATUS_OTHER;
      return;
    }
    if (!(buf.st_mode & S_IFREG))
    {
      last_error = VFS_STATUS_OTHER;
      return;
    }
  }
  fp = CS::Platform::File::Open (apath, mode);
  if (fp == 0)
    last_error = VFS_STATUS_ACCESSDENIED;
}

csPhysicalFile::csPhysicalFile(FILE* f, bool take_ownership, char const* n ) 
  : scfImplementationType (this), fp(f), owner(take_ownership), 
  last_error(VFS_STATUS_OK)
{
  if (n != 0)
    path = n;
  if (fp == 0)
    last_error = VFS_STATUS_OTHER;
}

csPhysicalFile::~csPhysicalFile()
{
  if (owner && fp != 0)
    fclose(fp);
}

size_t csPhysicalFile::Read(char* buff, size_t nbytes)
{
  CS::Threading::ScopedLock<CS::Threading::Mutex> lock (mutex);

  size_t rc = 0;
  if (fp != 0)
  {
    errno = 0;
    rc = fread(buff, 1, nbytes, fp);
    last_error = (errno == 0 ? VFS_STATUS_OK : VFS_STATUS_IOERROR);
  }
  else
    last_error = VFS_STATUS_OTHER;
  return rc;
}

size_t csPhysicalFile::Write(char const* data, size_t nbytes)
{
  CS::Threading::ScopedLock<CS::Threading::Mutex> lock (mutex);

  size_t rc = 0;
  if (fp != 0)
  {
    errno = 0;
    rc = fwrite(data, 1, nbytes, fp);
    last_error = (errno == 0 ? VFS_STATUS_OK : VFS_STATUS_IOERROR);
  }
  else
    last_error = VFS_STATUS_OTHER;
  return rc;
}

char const* csPhysicalFile::GetName()
{
  if (!path.IsEmpty())
    return path.GetData();
  else
    return "#csPhysicalFile";
}

size_t csPhysicalFile::GetSize()
{
  CS::Threading::ScopedLock<CS::Threading::Mutex> lock (mutex);

  size_t len = (size_t)-1;
  if (fp != 0)
  {
    errno = 0;
    size_t pos = ftell(fp);
    if (errno == 0 && fseek(fp, 0, SEEK_END) == 0)
    {
      len = ftell(fp);
      if (errno == 0)
        fseek(fp, (long)pos, SEEK_SET);
    }
    last_error = (errno == 0 ? VFS_STATUS_OK : VFS_STATUS_IOERROR);
  }
  else
    last_error = VFS_STATUS_OTHER;
  return len;
}

void csPhysicalFile::Flush()
{
  CS::Threading::ScopedLock<CS::Threading::Mutex> lock (mutex);

  if (fp != 0)
  {
    int const rc = fflush(fp);
    last_error = (rc == 0 ? VFS_STATUS_OK : VFS_STATUS_IOERROR);
  }
  else
    last_error = VFS_STATUS_OTHER;
}

bool csPhysicalFile::AtEOF()
{
  CS::Threading::ScopedLock<CS::Threading::Mutex> lock (mutex);

  bool rc;
  if (fp != 0)
  {
    rc = (feof(fp) != 0);
    last_error = VFS_STATUS_OK;
  }
  else
  {
    rc = true;
    last_error = VFS_STATUS_OTHER;
  }
  return rc;
}

size_t csPhysicalFile::GetPos()
{
  CS::Threading::ScopedLock<CS::Threading::Mutex> lock (mutex);

  size_t pos = (size_t)-1;
  if (fp != 0)
  {
    errno = 0;
    pos = ftell(fp);
    last_error = (errno == 0 ? VFS_STATUS_OK : VFS_STATUS_IOERROR);
  }
  else
    last_error = VFS_STATUS_OTHER;
  return pos;
}

bool csPhysicalFile::SetPos(size_t p)
{
  CS::Threading::ScopedLock<CS::Threading::Mutex> lock (mutex);

  bool ok = false;
  if (fp != 0)
  {
    errno = 0;
    fseek(fp, (long)p, SEEK_SET);
    last_error = (errno == 0 ? VFS_STATUS_OK : VFS_STATUS_IOERROR);
  }
  else
    last_error = VFS_STATUS_OTHER;
  ok = last_error == VFS_STATUS_OK;
  return ok;
}

csPtr<iDataBuffer> csPhysicalFile::GetAllData(bool nullterm)
{
  csDataBuffer* data = 0;
  size_t const len = GetSize();
  if (GetStatus() == VFS_STATUS_OK)
  {
    size_t const pos = GetPos();
    if (GetStatus() == VFS_STATUS_OK)
    {
      SetPos (0);
      if (GetStatus() != VFS_STATUS_OK) return (iDataBuffer*)nullptr;
      size_t const nbytes = len + (nullterm ? 1 : 0);
      char* buff = new char[nbytes]; // csDataBuffer takes ownership.
      size_t const nread = Read(buff, len);
      if (GetStatus() == VFS_STATUS_OK)
        SetPos(pos);
      if (GetStatus() == VFS_STATUS_OK)
      {
        if (nullterm)
          buff[nread] = '\0';
        data = new csDataBuffer(buff, nread + (nullterm ? 1 : 0));
      }
      else
        delete[] buff;
    }
  }
  return csPtr<iDataBuffer>(data);
}

csPtr<iDataBuffer> csPhysicalFile::GetAllData(CS::Memory::iAllocator* allocator)
{
  csRef<CS::DataBuffer<CS::Memory::AllocatorInterface> > data;
  size_t const len = GetSize();
  if (GetStatus() == VFS_STATUS_OK)
  {
    size_t const pos = GetPos();
    if (GetStatus() == VFS_STATUS_OK)
    {
      data.AttachNew (new CS::DataBuffer<CS::Memory::AllocatorInterface> (len,
        CS::Memory::AllocatorInterface (allocator)));
      SetPos (0);
      if (GetStatus() != VFS_STATUS_OK) return (iDataBuffer*)nullptr;
      size_t const nread = Read(data->GetData(), len);
      if ((nread != len) || (GetStatus() != VFS_STATUS_OK))
        data.Invalidate();
      SetPos(pos);
    }
  }
  return csPtr<iDataBuffer>(data);
}

csPtr<iFile> csPhysicalFile::GetPartialView (size_t offset, size_t size)
{
  if (!fp) return (iFile*)0;

  size_t const len = csMin (size, GetSize() - offset);
  return csPtr<iFile> (new PartialView (this, offset, len));
}

//---------------------------------------------------------------------------

char const* csPhysicalFile::PartialView::GetName()
{
  return parent->GetName();
}

size_t csPhysicalFile::PartialView::GetSize()
{
  status = VFS_STATUS_OK;
  return size;
}

int csPhysicalFile::PartialView::GetStatus()
{
  return status;
}

size_t csPhysicalFile::PartialView::Read (char* buffer, size_t nbytes)
{
  CS::Threading::ScopedLock<CS::Threading::Mutex> lock (parent->mutex);

  size_t readSize (csMin (nbytes, size - pos));

  errno = 0;
  size_t oldPos (ftell (parent->fp));
  status = (errno == 0 ? VFS_STATUS_OK : VFS_STATUS_IOERROR);
  if (status != VFS_STATUS_OK) return 0;
  errno = 0;
  fseek(parent->fp, (long)pos, SEEK_SET);
  status = (errno == 0 ? VFS_STATUS_OK : VFS_STATUS_IOERROR);
  if (status != VFS_STATUS_OK)
  {
    fseek(parent->fp, (long)oldPos, SEEK_SET);
    return 0;
  }

  errno = 0;
  size_t rc = fread (buffer, 1, readSize, parent->fp);
  status = (errno == 0 ? VFS_STATUS_OK : VFS_STATUS_IOERROR);
  fseek(parent->fp, (long)oldPos, SEEK_SET);
  pos += rc;
  return rc;
}

size_t csPhysicalFile::PartialView::Write(char const* data, size_t nbytes)
{
  // Don't allow writing to views.
  status = VFS_STATUS_ACCESSDENIED;
  return 0;
}

void csPhysicalFile::PartialView::Flush() {}

bool csPhysicalFile::PartialView::AtEOF()
{
  return pos >= size;
}

size_t csPhysicalFile::PartialView::GetPos()
{
  return pos;
}

bool csPhysicalFile::PartialView::SetPos(size_t p)
{
  if (p > size)
    return false;
  pos = p;
  return true;
}

csPtr<iDataBuffer> csPhysicalFile::PartialView::GetAllData (bool nullterm)
{
  csDataBuffer* data = 0;
  size_t const len = GetSize();
  if (GetStatus() == VFS_STATUS_OK)
  {
    size_t const pos = GetPos();
    if (GetStatus() == VFS_STATUS_OK)
    {
      SetPos (0);
      if (GetStatus() != VFS_STATUS_OK) return (iDataBuffer*)nullptr;
      size_t const nbytes = len + (nullterm ? 1 : 0);
      char* buff = new char[nbytes]; // csDataBuffer takes ownership.
      size_t const nread = Read(buff, len);
      if (GetStatus() == VFS_STATUS_OK)
        SetPos(pos);
      if (GetStatus() == VFS_STATUS_OK)
      {
        if (nullterm)
          buff[nread] = '\0';
        data = new csDataBuffer(buff, nread + (nullterm ? 1 : 0));
      }
      else
        delete[] buff;
    }
  }
  return csPtr<iDataBuffer>(data);
}

csPtr<iDataBuffer> csPhysicalFile::PartialView::GetAllData (CS::Memory::iAllocator* allocator)
{
  csRef<CS::DataBuffer<CS::Memory::AllocatorInterface> > data;
  size_t const len = GetSize();
  if (GetStatus() == VFS_STATUS_OK)
  {
    size_t const pos = GetPos();
    if (GetStatus() == VFS_STATUS_OK)
    {
      data.AttachNew (new CS::DataBuffer<CS::Memory::AllocatorInterface> (len,
        CS::Memory::AllocatorInterface (allocator)));
      SetPos (0);
      if (GetStatus() != VFS_STATUS_OK) return (iDataBuffer*)nullptr;
      size_t const nread = Read(data->GetData(), len);
      if ((nread != len) || (GetStatus() != VFS_STATUS_OK))
        data.Invalidate();
      SetPos(pos);
    }
  }
  return csPtr<iDataBuffer>(data);
}

csPtr<iFile> csPhysicalFile::PartialView::GetPartialView (size_t offset, size_t size)
{
  size_t const len = csMin (size, GetSize() - offset);
  return parent->GetPartialView (this->offset + offset, len);
}
