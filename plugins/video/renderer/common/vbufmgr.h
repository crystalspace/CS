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

#ifndef __CS_VBUFMGR_H__
#define __CS_VBUFMGR_H__

#include "csutil/array.h"
#include "ivideo/vbufmgr.h"
#include "csgeom/box.h"

struct iObjectRegistry;
class csVector3;
class csVector2;
class csColor;

#define CS_VBUF_TOTAL_USERA 16

/**
 * A general vertex buffer.
 */
class csVertexBuffer : public iVertexBuffer
{
protected:
  /// The pointer to the vertices.
  csVector3* verts;
  csVector2* texels;
  csColor* colors;
	int usercmp[CS_VBUF_TOTAL_USERA];
	float* user[CS_VBUF_TOTAL_USERA];
  /// The number of vertices.
  int num_verts;
  /// Priority.
  int priority;
  /// The vertex buffer manager.
  iVertexBufferManager* mgr;
  /// Locked status.
  bool locked;
  /// Bounding box.
  csBox3 bbox;

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
  /// Get the current array of texels.
  virtual csVector2* GetTexels () const { return texels; }
  /// Get the current array of colors.
  virtual csColor* GetColors () const { return colors; }
  /// Get one of the current user arrays.
	virtual float* GetUserArray (int index) const { return user[index]; }
  /// Get the number of components of one of the current user arrays.
  virtual int GetUserArrayComponentCount (int index) const
  {
    return usercmp[index];
  }
  /// Get number of vertices.
  virtual int GetVertexCount () const { return num_verts; }
  /// Get bounding box.
  virtual const csBox3& GetBoundingBox () const { return bbox; }
  /// Set buffer.
  void SetBuffers (csVector3* verts, csVector2* texels,
  	csColor* colors, int num_verts, const csBox3& b)
  {
    csVertexBuffer::verts = verts;
    csVertexBuffer::texels = texels;
    csVertexBuffer::colors = colors;
    csVertexBuffer::num_verts = num_verts;
    for( int i=0; i<CS_VBUF_TOTAL_USERA; i++ )
      user[i] = 0;
    bbox = b;
  }

  void SetUserArray (int index, float* user, int num_components)
  {
    if (index>CS_VBUF_TOTAL_USERA) return;
    csVertexBuffer::user[index] = user;
    csVertexBuffer::usercmp[index] = num_components;
  }

  SCF_DECLARE_IBASE;
};

/**
 * A general polygon buffer.
 */
class csPolygonBuffer : public iPolygonBuffer
{
protected:
  /// The vertex buffer manager.
  iVertexBufferManager* mgr;

public:
  ///
  csPolygonBuffer (iVertexBufferManager* mgr);
  ///
  virtual ~csPolygonBuffer ();

  // AddPolygon and ClearPolygons are not implemented here. That should
  // go in subclass.

  SCF_DECLARE_IBASE;

  virtual void Prepare () { }
};

/**
 * General version of the vertex buffer manager.
 * Each 3D driver should derive a vertex buffer manager class from this one
 * and implement the missing functionality.
 */
class csVertexBufferManager : public iVertexBufferManager
{
protected:
  typedef csArray<csVertexBuffer*> csVBufVector;

  /// List of vertex buffers.
  csVBufVector buffers;

  ///
  iObjectRegistry *object_reg;

  /// list of registered clients
  csArray<iVertexBufferManagerClient*> vClients;

public:
  /// Initialize the vertex buffer manager
  csVertexBufferManager (iObjectRegistry* object_reg);
  /// Destroy the vertex buffer manager
  virtual ~csVertexBufferManager ();

  /// Remove a vertex buffer from the list of vertex buffers.
  void RemoveVBuf (iVertexBuffer* buf);

  SCF_DECLARE_IBASE;

  virtual csPtr<iVertexBuffer> CreateBuffer (int priority);
  virtual void ChangePriority (iVertexBuffer* buf, int new_priority);
  virtual bool LockBuffer (iVertexBuffer* buf,
  	csVector3* verts,
	csVector2* texels,
	csColor* colors,
	int num_verts, int buf_number,
	const csBox3& bbox);
  virtual bool LockUserArray (iVertexBuffer* buf,
		int index, float* user,
		int num_components, int buf_number);
  virtual void UnlockBuffer (iVertexBuffer* buf);

  virtual void AddClient (iVertexBufferManagerClient *client);
  virtual void RemoveClient (iVertexBufferManagerClient *client);

  // CreatePolygonBuffer() is not implemented here. Must go to
  // subclass!
};

#endif // __CS_VBUFMGR_H__

