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

#ifndef __CS_GL_SYSRBUFMGR_H__
#define __CS_GL_SYSRBUFMGR_H__

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
  int size, compcount;
  csRenderBufferType type;
  csRenderBufferComponentType comptype;
  bool locked;
  bool discarded;
public:
  SCF_DECLARE_IBASE;

  csSysRenderBuffer (void *buffer, int size, csRenderBufferType type,
    csRenderBufferComponentType comptype, int compcount)
  {
    SCF_CONSTRUCT_IBASE (NULL)

    csSysRenderBuffer::buffer = buffer;
    csSysRenderBuffer::size = size;
    csSysRenderBuffer::type = type;
    csSysRenderBuffer::comptype = comptype;
    csSysRenderBuffer::compcount = compcount;
    locked = false;
    discarded = true;
  }

  virtual ~csSysRenderBuffer ()
  {
    if (buffer != NULL)
      delete[] (char *)buffer;
  }
  
  /**
   * Lock the buffer to allow writing and give us a pointer to the data
   * The pointer will be NULL if there was some error
   */
  virtual void* Lock(csRenderBufferLockType lockType)
  {
    locked = true;
    return buffer;
  }

  /// Releases the buffer. After this all writing to the buffer is illegal
  virtual void Release() { locked = false; discarded = false; }

  virtual bool IsDiscarded() { return discarded; }

  virtual void CanDiscard(bool value) {}

  /// Get type of buffer (where it's located)
  virtual csRenderBufferType GetBufferType() { return type; }

  /// Get the size of the buffer (in bytes)
  virtual int GetSize() { return size; }

  /// Gets the number of components per element
  virtual int GetComponentCount () { return compcount; }

  /// Gets the component type
  virtual csRenderBufferComponentType GetComponentType () { return comptype; }
};


class csSysRenderBufferManager: public iRenderBufferManager
{
public:
  SCF_DECLARE_IBASE;

  csSysRenderBufferManager::csSysRenderBufferManager ()
  {
    SCF_CONSTRUCT_IBASE (NULL);
  }

  /// Allocate a buffer of the specified type and return it
  csPtr<iRenderBuffer> CreateBuffer(int buffersize, 
    csRenderBufferType type,
    csRenderBufferComponentType comptype,
    int compcount);
};

#endif //  __CS_GL_SYSRBUFMGR_H__
