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

#ifndef __CS_IVIDEO_VBUFMGR_H__
#define __CS_IVIDEO_VBUFMGR_H__

/**\file
 * Vertex buffer manager interface
 */
 
/**
 * \addtogroup gfx3d
 * @{ */
 
#include "csutil/scf.h"
#include "ivideo/txtmgr.h"

class csBox3;
class csMatrix3;
class csPlane3;
class csVector3;
class csVector2;
class csColor;
class csTransform;
struct iMaterialHandle;
struct csPolyTextureMapping;

SCF_VERSION (iVertexBuffer, 0, 1, 1);

/**
 * @@@OR@@@
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
   * Get all of the current user arrays.
   */
  virtual float* GetUserArray (int index) const = 0;
  /**
   * Get the number of components of one of the current user arrays.
   */
  virtual int GetUserArrayComponentCount (int index) const = 0;
  /**
   * Get the number of vertices.
   */
  virtual int GetVertexCount () const = 0;
  /**
   * Get a bounding box for all the vertices.
   */
  virtual const csBox3& GetBoundingBox () const = 0;
};

SCF_VERSION (iPolygonBuffer, 0, 3, 0);

/**
 * @@@OR@@@
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
   * Set vertices to use for the polygons.
   * The given array is copied. 
   */
  virtual void SetVertexArray (csVector3* verts, int num_verts) = 0;
  /**
   * Gets the number of vertices.
   */
  virtual int GetVertexCount () const = 0;
  /**
   * Gets the array of vertices.  	
   */
  virtual csVector3* GetVertices () const = 0;
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

  /**
   * After adding everything and before using this polygon buffer you
   * should call Prepare().
   */
  virtual void Prepare () = 0;

  /**
   * Sets the polygon buffer as dirty.
   * This means that the mesh is affected by some light.
   */
  virtual void MarkLightmapsDirty () = 0;

  /**
   * Get a bounding box for all the vertices.
   */
  virtual const csBox3& GetBoundingBox () const = 0;

  /**
   * Add a polygon to this buffer. The data pointed to by 'verts'
   * is copied so it can be discarded after calling AddPolygon.
   * 'mat_index' is an index in the material table (initialized
   * with AddMaterial()). It is best to add the polygons sorted by
   * material as that will generate the most efficient representation
   * on hardware.
   */
  virtual void AddPolygon (int num_verts,
	int* verts,
	csPolyTextureMapping* texmap,
	const csPlane3& poly_normal,
	int mat_index,
	iRendererLightmap* lm) = 0;
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
 * @@@OR@@@
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
  virtual csPtr<iVertexBuffer> CreateBuffer (int priority) = 0;

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
	int num_verts, int buf_number,
	const csBox3& bbox) = 0;

  virtual bool LockUserArray (iVertexBuffer* buf,
	int index, float* user, 
	int num_components, int buf_number) = 0;

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

/** @} */

#endif // __CS_IVIDEO_VBUFMGR_H__

