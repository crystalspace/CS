/*
    Map2cs: a convertor to convert the frequently used MAP format, into
    something, that can be directly understood by Crystal Space.

    Copyright (C) 1999 Thomas Hieber (thieber@gmx.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "mapstd.h"
#include "zipfile.h"
#include "bindata.h"

bool CZipFile::AddFile(const char* originalname, const char* zipname)
{
  FILE* fd = fopen(originalname, "rb");
  if (!fd)
  {
    csPrintf("Can't find file '%s'!\n", originalname);
    return false;
  }

  //Check the size of the file.
  fseek(fd, 0, SEEK_END); //Seek to end of file
  size_t size = ftell(fd);   //Check current position
  fseek(fd, 0, SEEK_SET); //Seek to beginning of file

  void* file = NewFile(zipname, size);
  if (!file)
  {
    csPrintf("Can't add new file '%s' to zip archive\n", originalname);
    return false;
  }

  char Buffer[1000];

  size_t totalread = 0;
  size_t numread   = 0;
  do
  {
    numread = fread (Buffer, 1, 1000, fd);
    if (!Write(file, Buffer, numread))
    {
      csPrintf("Can't add data of file '%s' to zip archive\n", originalname);
      return false;
    }
    totalread += numread;
  }
  while(numread>0);

  assert(totalread==size);

  return true;
}

bool CZipFile::AddData(const char* Data, int Size, const char* zipname)
{
  assert(Data);
  assert(zipname);

  void* file = NewFile(zipname, Size);
  if (!file)
  {
    csPrintf("Can't add new file '%s' to zip archive\n", zipname);
    return false;
  }

  if (!Write(file, Data, Size))
  {
    csPrintf("Can't add data (%s) to zip archive\n", zipname);
    return false;
  }

  return true;
}

bool CZipFile::AddData(CBinaryData* pData, const char* zipname)
{
  assert(pData);
  assert(zipname);

  return AddData(pData->GetData(), pData->GetSize(), zipname);
}

bool CZipFile::ExtractData(CBinaryData* pData, const char* zipname)
{
  assert(pData);
  assert(zipname);

  size_t filesize = 0;
  if (FileExists (zipname, &filesize))
  {
    char* pZipData = Read (zipname);
    assert(pZipData);

    pData->SetData (pZipData, (int)filesize);
    delete[] pZipData;

    return true;
  }

  return false;
}

