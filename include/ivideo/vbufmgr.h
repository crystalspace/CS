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

class csMatrix3;
class csPlane3;
class csVector3;
class csVector2;
class csColor;
struct iPolygonTexture;
struct iMaterialHandle;

SCF_VERSION (iVertexBuffer, 0, 1, 0);

/**
 * This interface represents a black-box vertex buffer.
 * Using the vertex buffer manager (see below) you can create objects
 * that implement this interface. These objects are managed by the
 * respective 3D renderer that provided the vertex buffer manager and
 * are supposed to store the vertices in the most efficient way possible.
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
   * Get the current array of texels.
   */
  virtual csVector2* GetTexels () const = 0;
  /**
   * Get the current array of colors.
   */
  virtual csColor* GetColors () const = 0;
  /**
   * Get the number of vertices.
   */
  virtual int GetVertexCount () const = 0;
};

SCF_VERSION (iPolygonBuffer, 0, 0, 3);

/**
 * This interface represents a black-box polygon buffer.
 * It is used to draw a mesh of polygons. The vertex buffer manager
 * will create instances of iPolygonBuffer. Internally it will hold
 * the most efficient representation for the 3D renderer to actually
 * render the polygons. On hardware this may means that the polygons
 * are converted to triangle meshes or triangle fans/strips. With the
 * software renderer it will probably keep the polygons as such.
 * Polygons in this buffer used indexed coordinates (with indices
 * that are usually relative to a vertex buffer).
 */
struct iPolygonBuffer : public iBase
{
  /**
   * Add a polygon to this buffer. The data pointed to by 'verts'
   * is copied so it can be discarded after calling AddPolygon.
   * 'mat_index' is an index in the material table (initialized
   * with AddMaterial()). It is best to add the polygons sorted by
   * material as that will generate the most efficient representation
   * on hardware.
   */
  virtual void AddPolygon (int* verts, int num_verts,
	const csPlane3& poly_normal,
	int mat_index,
	const csMatrix3& m_obj2tex, const csVector3& v_obj2tex,
	iPolygonTexture* poly_texture) = 0;

  /**
   * Set vertices to use for the polygons.
   * The given array is copied.
   */
  virtual void SetVertexArray (csVector3* verts, int num_verts) = 0;

  /**
   * Add a material.
   */
  virtual void AddMaterial (iMaterialHandle* mat_handle) = 0;
  /**
   * Get the number of materials.
   */
  virtual int GetMaterialCount () const = 0;
  /**
   * Get a material.
   */
  virtual iMaterialHandle* GetMaterial (int idx) const = 0;
  /**
   * Set a previously added material (this can be used to change
   * a material handle).
   */
  virtual void SetMaterial (int idx, iMaterialHandle* mat_handle) = 0;

  /// Clear all polygons, materials, and vertex array.
  virtual void Clear () = 0;
};

SCF_VERSION (iVertexBufferManagerClient, 0, 0, 1);

/**
 * Objects using services provided by the vertexbuffermanager can register
 * with the manager to receive information about it current state, e.g. the
 * manager tells its clients if he is going down, closes etc.
 */

struct iVertexBufferManagerClient : public iBase
{
  /**
   * This method is called whenever the the buffers managed become invalid,
   * that is, the clients have to request a new buffer from the manager.
   */
  virtual void ManagerClosing () = 0;
};

SCF_VERSION (iVertexBufferManager, 0, 0, 2);

/**
 * This interface represents the vertex buffer manager. You can use this
 * to create vertex buffers which can be used by the 3D renderer.
 */
struct iVertexBufferManager : public iBase
{
  //---------- Vertex Buffers -----------------------------------------------

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
   * use the vertex buffer. While the buffer is locked the arrays
   * that are given here may not be modified or altered in any way! The
   * buf_number indicates if the buffer has modified or not. You MUST
   * call UnlockBuffer() when you are ready with the buffer. Deleting a
   * vertex buffer with DecRef() automatically implies an UnlockBuffer().
   *<p>
   * This function will return false if the buffer could not be locked
   * for some reason. This may happen if too many buffers are locked at
   * the same time or if memory is low.
   */
  virtual bool LockBuffer (iVertexBuffer* buf,
  	csVector3* verts,
	csVector2* texels,
	csColor* colors,
	int num_verts, int buf_number) = 0;

  /**
   * Unlock a vertex buffer.
   */
  virtual void UnlockBuffer (iVertexBuffer* buf) = 0;

  //---------- Polygon Buffers -----------------------------------------------

  /**
   * Create an empty polygon buffer. The ref count of this polygon buffer
   * will be one. To remove it use DecRef().
   */
  virtual iPolygonBuffer* CreatePolygonBuffer () = 0;

  //---------- client handling -----------------------------------------------

  /**
   * A client using the services of the manager can register with it to receive
   * information about the state of the manager.
   */
  virtual void AddClient (iVertexBufferManagerClient *client) = 0;
  virtual void RemoveClient (iVertexBufferManagerClient *client) = 0;
};

#endif // __IVIDEO_VBUFMGR_H__

