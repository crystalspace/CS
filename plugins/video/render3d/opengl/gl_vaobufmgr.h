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
#include "video/canvas/openglcommon/glextmanager.h"

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
  virtual csPtr<iRenderBuffer> CreateBuffer(int buffersize, 
    csRenderBufferType type,
    csRenderBufferComponentType comptype,
    int compcount);

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
  int size, compcount;
  csRenderBufferType type;
  csRenderBufferComponentType comptype;
  bool locked;
  unsigned int VAObufferID;
  csRenderBufferLockType lastlock;
  bool discarded;
  csVaoRenderBufferManager* vaomgr;
  csGLExtensionManager* ext;
  
public:
  SCF_DECLARE_IBASE;

  
  csVaoRenderBuffer (int size, csRenderBufferType type, 
    csRenderBufferComponentType comptype, int compcount,
    csVaoRenderBufferManager* vaomgr);

  virtual ~csVaoRenderBuffer ();
  
  
  /**
   * Lock the buffer to allow writing and give us a pointer to the data
   * The pointer will be 0 if there was some error
   */
  virtual void* Lock(csRenderBufferLockType lockType)
  {
    if(locked) return 0;

    lastlock = lockType;
    
    if(type == CS_BUF_INDEX)
    {
      if(indexbuffer == 0)
        indexbuffer = new char[size];

      locked = true;
      return indexbuffer;
    }

    if(lockType == CS_BUF_LOCK_RENDER)
    {
      locked = true;
      return (void*)VAObufferID;
    }
    else
    {
      //alloc a new tempblock if needed
      if(tempbuffer == 0)
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
  virtual csRenderBufferType GetBufferType() { return type; }

  /// Get the size of the buffer (in bytes)
  virtual int GetSize() { return size; }

  /// Gets the number of components per element
  virtual int GetComponentCount () { return compcount; }

  /// Gets the component type
  virtual csRenderBufferComponentType GetComponentType () { return comptype; }
};




#endif //  __CS_GL_VAORBUFMGR_H__
