/*
    Copyright (C) 2001 by Jorrit Tyberghein
    Written by Jorrit Tyberghein.

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

#ifndef __IVIDEO_VBUFMGR_H__
#define __IVIDEO_VBUFMGR_H__

#include "csutil/scf.h"

class csVector3;

SCF_VERSION (iVertexBuffer, 0, 0, 1);

/**
 * This interface represents a black-box vertex buffer.
 */
struct iVertexBuffer : public iBase
{
  /// Get the priority.
  virtual int GetPriority () const = 0;
  /// Check if the buffer is locked.
  virtual bool IsLocked () const = 0;
  /**
   * Get the current array of vertices.
   */
  virtual csVector3* GetVertices () const = 0;
  /**
   * Get the number of vertices.
   */
  virtual int GetVertexCount () const = 0;
};

SCF_VERSION (iVertexBufferManager, 0, 0, 1);

/**
 * This interface represents the vertex buffer manager. You can use this
 * to create vertex buffers which can be used by the 3D renderer.
 */
struct iVertexBufferManager : public iBase
{
  /**
   * Create an empty vertex buffer. The ref count of this vertex buffer
   * will be one. To remove it use DecRef(). The priority number can be
   * anything. Higher numbers mean the vertex buffer is more important.
   * A high priority vertex buffer should be used for objects that are
   * visible often. Low priority vertex buffers should be used for objects
   * that are rarely visible.
   */
  virtual iVertexBuffer* CreateBuffer (int priority) = 0;

  /**
   * Change the priority of a vertex buffer. This can be used when some
   * low-priority object becomes more important for example.
   */
  virtual void ChangePriority (iVertexBuffer* buf, int new_priority) = 0;

  /**
   * Lock this vertex buffer for use. Only when the vertex buffer is locked
   * are you allowed to make calls to iGraphics3D functions that actually
   * use the vertex buffer. While the buffer is locked the vertex array
   * that is given here may not be modified or altered in any way! The
   * buf_number indicates if the buffer has modified or not. You MUST
   * call UnlockBuffer() when you are ready with the buffer. Deleting a
   * vertex buffer with DecRef() automatically implies an UnlockBuffer().
   *<p>
   * This function will return false if the buffer could not be locked
   * for some reason. This may happen if too many buffers are locked at
   * the same time or if memory is low.
   */
  virtual bool LockBuffer (iVertexBuffer* buf,
  	csVector3* verts, int num_verts, int buf_number) = 0;

  /**
   * Unlock a vertex buffer.
   */
  virtual void UnlockBuffer (iVertexBuffer* buf) = 0;
};

#endif // __IVIDEO_VBUFMGR_H__

