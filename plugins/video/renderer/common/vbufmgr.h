/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __VBUFMGR_H__
#define __VBUFMGR_H__

#include "csutil/csvector.h"
#include "csutil/typedvec.h"
#include "ivideo/vbufmgr.h"

struct iObjectRegistry;

/**
 * A general vertex buffer.
 */
class csVertexBuffer : public iVertexBuffer
{
protected:
  /// The pointer to the vertices.
  csVector3* verts;
  /// The number of vertices.
  int num_verts;
  /// Priority.
  int priority;
  /// The vertex buffer manager.
  iVertexBufferManager* mgr;
  /// Locked status.
  bool locked;

public:
  ///
  csVertexBuffer (iVertexBufferManager* mgr);
  ///
  virtual ~csVertexBuffer ();

  /// Lock/unlock buffer.
  void SetLock (bool l) { locked = l; }
  /// Is locked?
  virtual bool IsLocked () const { return locked; }

  /// Get priority.
  virtual int GetPriority () const { return priority; }
  /// Set priority.
  void SetPriority (int pri) { priority = pri; }

  /// Get buffer.
  virtual csVector3* GetVertices () const { return verts; }
  /// Get number of vertices.
  virtual int GetVertexCount () const { return num_verts; }
  /// Set buffer.
  void SetVertices (csVector3* verts, int num_verts)
  {
    csVertexBuffer::verts = verts;
    csVertexBuffer::num_verts = num_verts;
  }

  SCF_DECLARE_IBASE;
};

/**
 * General version of the vertex buffer manager.
 * Each 3D driver should derive a vertex buffer manager class from this one
 * and implement the missing functionality.
 */
class csVertexBufferManager : public iVertexBufferManager
{
protected:
  CS_DECLARE_TYPED_VECTOR_NODELETE (csVBufVector, csVertexBuffer);

  /// List of vertex buffers.
  csVBufVector buffers;

  ///
  iObjectRegistry *object_reg;

public:
  /// Initialize the vertex buffer manager
  csVertexBufferManager (iObjectRegistry* object_reg);
  /// Destroy the vertex buffer manager
  virtual ~csVertexBufferManager ();

  /// Remove a vertex buffer from the list of vertex buffers.
  void RemoveVBuf (iVertexBuffer* buf);

  SCF_DECLARE_IBASE;

  virtual iVertexBuffer* CreateBuffer (int priority);
  virtual void ChangePriority (iVertexBuffer* buf, int new_priority);
  virtual bool LockBuffer (iVertexBuffer* buf,
  	csVector3* verts, int num_verts, int buf_number);
  virtual void UnlockBuffer (iVertexBuffer* buf);
};

#endif // __VBUFMGR_H__

