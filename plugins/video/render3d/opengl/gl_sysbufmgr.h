/*
    Copyright (C) 2002 by Mårten Svanfeldt
                          Anders Stenberg

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

#ifndef __GL_SYSRBUFMGR_H__
#define __GL_SYSRBUFMGR_H__

#include "csutil/strhash.h"
#include "ivideo/rndbuf.h"

struct iLightingInfo;
struct iTextureHandle;


SCF_VERSION(csSysRenderBuffer, 0,0,2);
/**
 * This is a general buffer to be used by the renderer. It can ONLY be
 * created by the VB manager
 */
class csSysRenderBuffer : public iRenderBuffer
{
private:
  void *buffer;
  int size;
  CS_RENDERBUFFER_TYPE type;
  bool locked;
public:
  SCF_DECLARE_IBASE;

  
  csSysRenderBuffer (void *buffer, int size, CS_RENDERBUFFER_TYPE type)
  {
    SCF_CONSTRUCT_IBASE (NULL)

    csSysRenderBuffer::buffer = buffer;
    csSysRenderBuffer::size = size;
    csSysRenderBuffer::type = type;
    locked = false;
  }

  ~csSysRenderBuffer ()
  {
    if (buffer != NULL)
    {
      char* p = (char*)buffer;
      delete[] p;
    }
  }
  
  /**
   * Lock the buffer to allow writing and give us a pointer to the data
   * The pointer will be NULL if there was some error
   */
  virtual void* Lock(CS_BUFFER_LOCK_TYPE lockType)
  {
    locked = true;
    return buffer;
  }

  /// Releases the buffer. After this all writing to the buffer is illegal
  virtual void Release() { locked = false; }

  /// Get type of buffer (where it's located)
  virtual CS_RENDERBUFFER_TYPE GetBufferType() { return type; }

  /// Get the size of the buffer (in bytes)
  virtual int GetSize() {return size; }
};


class csSysRenderBufferManager: public iRenderBufferManager
{
public:
  SCF_DECLARE_IBASE;

  /// Allocate a buffer of the specified type and return it
  csPtr<iRenderBuffer> GetBuffer(int buffersize, CS_RENDERBUFFER_TYPE location);

};

#endif //  __GL_SYSRBUFMGR_H__
