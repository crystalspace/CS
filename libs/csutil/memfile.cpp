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
#include "csutil/memfile.h"
#include "csutil/databuf.h"
#include <stdlib.h>

SCF_IMPLEMENT_IBASE (csMemFile)
  SCF_IMPLEMENTS_INTERFACE (iFile)
SCF_IMPLEMENT_IBASE_END

const char* csMemFile::GetName() { return "#csMemFile"; }
const char* csMemFile::GetData() const { return buffer; }
size_t csMemFile::GetSize() { return size; }
int csMemFile::GetStatus() { return VFS_STATUS_OK; }
void csMemFile::Flush() {}
bool csMemFile::AtEOF() { return (cursor >= size); }
size_t csMemFile::GetPos() { return cursor; }
bool csMemFile::SetPos(size_t p) { cursor = p < size ? p : size; return true; }

csMemFile::csMemFile() :
  disposition(DISPOSITION_FREE), buffer(0), capacity(0), size(0), cursor(0)
  { SCF_CONSTRUCT_IBASE(0); }

csMemFile::csMemFile(const char* p, size_t s) :
  disposition(DISPOSITION_IGNORE), buffer((char*)p), capacity(s), size(s),
  cursor(0) { SCF_CONSTRUCT_IBASE(0); }

csMemFile::csMemFile(char* p, size_t s, Disposition d) :
  disposition(d), buffer(p), capacity(s), size(s), cursor(0)
  { SCF_CONSTRUCT_IBASE(0); }

csMemFile::~csMemFile()
{
  FreeBuffer();
  SCF_DESTRUCT_IBASE ();
}

void csMemFile::FreeBuffer()
{
  if (buffer != 0)
  {
    if (disposition == DISPOSITION_DELETE)
      delete[] buffer;
    else if (disposition == DISPOSITION_FREE)
      free(buffer);
  }
}

size_t csMemFile::Read(char* Data, size_t DataSize)
{
  const size_t remaining = cursor < size ? size - cursor : 0;
  const size_t nbytes = DataSize < remaining ? DataSize : remaining;
  if (nbytes != 0)
    memcpy(Data, buffer + cursor, nbytes);
  cursor += nbytes;
  return nbytes;
}

size_t csMemFile::Write(const char* Data, size_t DataSize)
{
  size_t written = 0;
  if (DataSize != 0 && Data != 0)
  {
    size_t new_cursor = cursor + DataSize;
    if (capacity < new_cursor)
    {
      if (capacity == 0)
        capacity = 1024;
      while (capacity < new_cursor)
        capacity <<= 1;

      // make buffer bigger
      char * new_buffer;
      if (disposition == DISPOSITION_FREE)
      {
	//new_buffer = (char*) realloc((void*) buffer, capacity);
	/*
	  @@@ This lead to problems with relative large files (several MB)
	  on VC. Using malloc()/free() instead of realloc() solved the
	  problems.
	 */

	new_buffer = (char*) malloc(capacity);
	if (!new_buffer)
	    return 0;
        if (buffer != 0)
          memcpy(new_buffer, buffer, size);
	free (buffer);
      }
      else
      {
	new_buffer = (char*) malloc(capacity);
        if (buffer != 0)
          memcpy(new_buffer, buffer, size);
	else
	  return 0;
	FreeBuffer();
      }  
      buffer = new_buffer;
      disposition = DISPOSITION_FREE;
    }
    memcpy(buffer + cursor, Data, DataSize);
    cursor = new_cursor;
    if (new_cursor > size)
      size = new_cursor;
    written = DataSize;
  }
  return written;
}

csPtr<iDataBuffer> csMemFile::GetAllData(bool nullterm)
{
  char* data = new char [size + 1];
  memcpy (data, buffer, size);
  *(data + size) = 0;
  iDataBuffer *db = new csDataBuffer (data, size);
  return csPtr<iDataBuffer> (db);
}

