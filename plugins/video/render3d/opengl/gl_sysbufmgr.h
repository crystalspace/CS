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

class csVector3;
class csVector2;
class csColor;

struct iLightingInfo;
struct iTextureHandle;


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
public:
  SCF_DECLARE_IBASE;

  
  csSysRenderBuffer (void *buffer, int size, CS_RENDERBUFFER_TYPE type)
  {
    SCF_CONSTRUCT_IBASE (NULL)

    csSysRenderBuffer::buffer = buffer;
    csSysRenderBuffer::size = size;
    csSysRenderBuffer::type = type;
  }
  ~csSysRenderBuffer ()
  {
    if (buffer != NULL)
      delete[] buffer;
  }
  
  /// Get a raw pointer to the bufferdata as different datatypes
  float* GetFloatBuffer() { return (float*)buffer; }
  unsigned char* GetUCharBuffer() { return (unsigned char*)buffer; }
  unsigned int* GetUIntBuffer() { return (unsigned int*)buffer; }
  csVector3* GetVector3Buffer() { return (csVector3*)buffer; }
  csVector2* GetVector2Buffer() { return (csVector2*)buffer; }
  csColor* GetColorBuffer() { return (csColor*)buffer; }

  /// Get type of buffer (where it's located)
  virtual CS_RENDERBUFFER_TYPE GetBufferType() { return type; }

  /// Get number of indices in the data (as different datatypes)
  int GetFloatLength() { return size/sizeof(float); }
  int GetUCharLength() { return size/sizeof(unsigned char); }
  int GetUIntLength() { return size/sizeof(unsigned int); }
  int GetVec3Length() { return size/sizeof(csVector3); }
  int GetVec2Length() { return size/sizeof(csVector2); }
  int GetColorLength() { return size/sizeof(csColor); }
};

class csSysRenderBufferManager: public iRenderBufferManager
{
public:
  SCF_DECLARE_IBASE;

  /// Allocate a buffer of the specified type and return it
  csPtr<iRenderBuffer> GetBuffer(int buffersize, CS_RENDERBUFFER_TYPE location);
  
  /// Lock a specified buffer. Return true if successful
  bool LockBuffer(iRenderBuffer* buffer) { return true; }

  /// Unlock buffer
  void UnlockBuffer(iRenderBuffer* buffer) {}
};

#endif //  __GL_SYSRBUFMGR_H__