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
#include "csutil/physfile.h"
#include "csutil/databuf.h"
#include <stdlib.h>

SCF_IMPLEMENT_IBASE(csPhysicalFile)
  SCF_IMPLEMENTS_INTERFACE(iFile)
SCF_IMPLEMENT_IBASE_END

int csPhysicalFile::GetStatus() { return last_error; }

csPhysicalFile::csPhysicalFile(char const* apath, char const* mode) :
  fp(0), path(apath), owner(true), last_error(VFS_STATUS_OK)
  {
  SCF_CONSTRUCT_IBASE(0);
  fp = fopen(apath, mode);
  if (fp == 0)
    last_error = VFS_STATUS_ACCESSDENIED;
  }

csPhysicalFile::csPhysicalFile(FILE* f, bool take_ownership, char const* n ) :
  fp(f), owner(take_ownership), last_error(VFS_STATUS_OK)
  {
  SCF_CONSTRUCT_IBASE(0);
  if (n != 0)
    path = n;
  if (fp == 0)
    last_error = VFS_STATUS_OTHER;
  }

csPhysicalFile::~csPhysicalFile()
{
  if (owner && fp != 0)
    fclose(fp);
  SCF_DESTRUCT_IBASE ();
}

size_t csPhysicalFile::Read(char* buff, size_t nbytes)
{
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
  bool ok = false;
  if (fp != 0)
  {
    errno = 0;
    fseek(fp, (long)p, SEEK_SET);
    last_error = (errno == 0 ? VFS_STATUS_OK : VFS_STATUS_IOERROR);
  }
  else
    last_error = VFS_STATUS_OTHER;
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
