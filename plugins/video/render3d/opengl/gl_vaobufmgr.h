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

#ifndef __CS_GL_VAORBUFMGR_H__
#define __CS_GL_VAORBUFMGR_H__

#include "csutil/strhash.h"
#include "ivideo/rndbuf.h"

struct iLightingInfo;
struct iTextureHandle;
class csVaoRenderBuffer;
class csGLRender3D;

class csVaoRenderBufferManager: public iRenderBufferManager
{
  friend class csVaoRenderBuffer;
  csGLRender3D* render3d; /// Must keep this to be able to use extensions
public:
  SCF_DECLARE_IBASE;

  /// Allocate a buffer of the specified type and return it
  csPtr<iRenderBuffer> CreateBuffer(int buffersize, CS_RENDERBUFFER_TYPE location);

    /**
   * Nonstandard constructor, as we need access to the internal of opengl-renderer
   * If not successful return false.
   */
  bool Initialize(csGLRender3D* render3d);

};

SCF_VERSION(csVaoRenderBuffer, 0,0,1);
/**
 * This is a general buffer to be used by the renderer. It can ONLY be
 * created by the VB manager
 */
class csVaoRenderBuffer : public iRenderBuffer
{
private:
  void *tempbuffer; //used when getting data from caller, returned by lock etc
  void *indexbuffer;  //only used if this is a index-buffer
  int size;
  CS_RENDERBUFFER_TYPE type;
  bool locked;
  unsigned int VAObufferID;
  CS_BUFFER_LOCK_TYPE lastlock;
  bool discarded;
  csVaoRenderBufferManager* vaomgr;
  
public:
  SCF_DECLARE_IBASE;

  
  csVaoRenderBuffer (int size, CS_RENDERBUFFER_TYPE type, csVaoRenderBufferManager* vaomgr);

  virtual ~csVaoRenderBuffer ();
  
  
  /**
   * Lock the buffer to allow writing and give us a pointer to the data
   * The pointer will be NULL if there was some error
   */
  virtual void* Lock(CS_BUFFER_LOCK_TYPE lockType)
  {
    if(locked) return 0;

    lastlock = lockType;
    
    if(type == CS_BUF_INDEX)
    {
      if(indexbuffer == NULL)
        indexbuffer = new char[size];

      return indexbuffer;
    }

    if(lockType == iRenderBuffer::CS_BUF_LOCK_RENDER)
    {
      locked = true;
      return (void*)VAObufferID;
    }
    else
    {
      //alloc a new tempblock if needed
      if(tempbuffer == NULL)
        tempbuffer = new char[size];

      locked = true;
      return tempbuffer;
    }
  }

  /// Releases the buffer. After this all writing to the buffer is illegal
  virtual void Release() ;

  virtual bool IsDiscarded() { return discarded; }

  virtual void CanDiscard(bool value) {}

  /// Get type of buffer (where it's located)
  virtual CS_RENDERBUFFER_TYPE GetBufferType() { return type; }

  /// Get the size of the buffer (in bytes)
  virtual int GetSize() { return size; }
};




#endif //  __CS_GL_VAORBUFMGR_H__
