/*
    Copyright (C) 2002 by Marten Svanfeldt
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

#ifndef __CS_IVIDEO_RNDBUF_H__
#define __CS_IVIDEO_RNDBUF_H__

/** \file 
 * Render buffer interface
 */
 
/**
 * \addtogroup gfx3d
 * @{ */

#include "csutil/strset.h"

class csVector3;
class csVector2;
class csColor;
class csReversibleTransform;
struct iTextureHandle;
struct iMaterialWrapper;


/**
 * Buffer usage type.
 * Drivers may do some optimizations based on this value. Use a type that most
 * closely matches the intended use.
 */
enum csRenderBufferType
{
  /// Data that changes every couple of frames and is drawn repeatedly.
  CS_BUF_DYNAMIC,
  /// Data that is seldom modified and often drawn.
  CS_BUF_STATIC,
  /// Data that changes every time it is drawn.
  CS_BUF_STREAM
};

/// Type of components
enum csRenderBufferComponentType
{
  CS_BUFCOMP_BYTE = 0,
  CS_BUFCOMP_UNSIGNED_BYTE,
  CS_BUFCOMP_SHORT,
  CS_BUFCOMP_UNSIGNED_SHORT,
  CS_BUFCOMP_INT,
  CS_BUFCOMP_UNSIGNED_INT,
  CS_BUFCOMP_FLOAT,
  CS_BUFCOMP_DOUBLE
};

/**
 * Type of lock
 * CS_BUF_LOCK_NORMAL: Just get a point to the buffer, nothing special
 * CS_BUF_LOCK_RENDER: Special lock only to be used by renderer
 */
enum csRenderBufferLockType
{
  CS_BUF_LOCK_NOLOCK,
  CS_BUF_LOCK_NORMAL,
  //CS_BUF_LOCK_RENDER
};

SCF_VERSION (iRenderBuffer, 0, 0, 3);

/**
 * This is a general buffer to be used by the renderer. It can ONLY be
 * created by the VB manager
 */
struct iRenderBuffer : public iBase
{
  /**
   * Lock the buffer to allow writing and give us a pointer to the data.
   * The pointer will be (void*)-1 if there was some error.
   * \param discardPrevious Specifies whether the same pointer as last time
   *  should be returned (all the data will be still there, useful if only
   *  a part of the data is changed). However, setting this to 'true' may 
   *  cause a performance penalty - specifically, if the data is currently in
   *  use, the driver may have to wait until the buffer is available again.
   */
  virtual void* Lock(csRenderBufferLockType lockType, 
    bool samePointer = false) = 0;

  /// Releases the buffer. After this all writing to the buffer is illegal
  virtual void Release() = 0;

  virtual void CopyToBuffer(const void *data, int length) = 0;

  /// Sets the number of components per element
  virtual void SetComponentCount (int count) = 0;

  /// Gets the number of components per element
  virtual int GetComponentCount () const = 0;

  /// Sets the component type (float, int, etc)
  virtual void SetComponentType (csRenderBufferComponentType type) = 0;

  /// Gets the component type (float, int, etc)
  virtual csRenderBufferComponentType GetComponentType () const = 0;

  /// Get type of buffer (static/dynamic)
  virtual csRenderBufferType GetBufferType() const = 0;

  /// Get the size of the buffer (in bytes)
  virtual int GetSize() const = 0;

  virtual void SetStride(int stride) = 0;

  virtual int GetStride() const = 0;

  virtual void SetOffset(int offset) = 0;
};

/** @} */

#endif // __CS_IVIDEO_RNDBUF_H__
