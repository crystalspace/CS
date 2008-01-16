/*
    Crystal Space 3D engine
    Copyright (C) 2000-2007 by Jorrit Tyberghein

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

#ifndef __CS_IGEOM_TRIMESH_H__
#define __CS_IGEOM_TRIMESH_H__

#include "csutil/scf_interface.h"

class csFlags;

/**\file
 * Triangle mesh interface
 */

/**
 * \addtogroup geom_utils
 * @{ */
 
/** \name Flags for iTriangleMesh
 * @{ */
enum
{
  /**
  * The object is closed, if set. 
  * With closed we mean that if you run a beam of light through the object 
  * (through any point outside the mesh to another point outside) it will
  * always hit an even amount of faces (one going in, and one going out).
  * If you don't set CLOSED or NOTCLOSED then the state is not known and
  * the engine may test it if it wants.
  */
  CS_TRIMESH_CLOSED = 1,

  /**
  * The object is not closed, if set.
  * This is the opposite of #CS_TRIMESH_CLOSED. Use this flag if you are
  * absolutely certain that the object is not closed. The engine will not
  * attempt to test if the object is really closed or not. If you don't
  * set CLOSED or NOTCLOSED then the state is not known and the engine
  * may test it if it wants.
  */
  CS_TRIMESH_NOTCLOSED = 2,

  /**
  * The object is convex, if set. 
  * With convex we mean that if you run a beam of light through the object 
  * (through any point outside the mesh to another point outside) it will
  * always hit exactly two faces (one going in, and one going out). If you
  * don't * set CONVEX or NOTCONVEX then the state is not known and the
  * engine may test it if it wants.
  */
  CS_TRIMESH_CONVEX = 4,

  /**
  * The object is not convex, if set.
  * This is the opposite of #CS_TRIMESH_CONVEX. Use this flag if you are
  * absolutely certain that the object is not convex. The engine will not
  * attempt to test if the object is really convex or not. If you don't
  * set CONVEX or NOTCONVEX then the state is not known and the engine
  * may test it if it wants.
  */
  CS_TRIMESH_NOTCONVEX = 8,

  /**
  * Set this flag if the triangle mesh is deformable.
  */
  CS_TRIMESH_DEFORMABLE = 16
};
/** @} */

class csVector3;
struct csTriangle;

/**
 * This interface reprents a mesh of triangles. It is useful to communicate
 * geometry information outside of the engine. One place where this will
 * be useful is for communicating geometry information to the collision
 * detection plugin.<br>
 * All Crystal Space mesh objects (things, sprites, ...)
 * should implement and/or embed an implementation of this interface.
 *
 * A triangle mesh has the concept of a vertex buffer and an array of
 * triangles.
 *
 * Main creators of instances implementing this interface:
 * - Almost all mesh objects have several implementations of this
 *   interface.
 *
 * Main ways to get pointers to this interface:
 * - iObjectModel::GetTriangleData()
 *
 * Main users of this interface:
 * - Collision detection plugins (iCollideSystem)
 * - Visibility culler plugins (iVisibilityCuller)
 * - Shadow stencil plugin
 */
struct iTriangleMesh : public virtual iBase
{
  SCF_INTERFACE(iTriangleMesh, 1, 0, 0);
  /// Get the number of vertices for this mesh.
  virtual size_t GetVertexCount () = 0;
  /// Get the pointer to the array of vertices.
  virtual csVector3* GetVertices () = 0;
  /// Get the number of triangles for this mesh.
  virtual size_t GetTriangleCount () = 0;
  /// Get the triangle table for this mesh.
  virtual csTriangle* GetTriangles () = 0;
  /**
   * Lock the triangle mesh. This prevents the triangle
   * data from being cleaned up.
   */
  virtual void Lock () = 0;
  /**
   * Unlock the triangle mesh. This allows clean up again.
   */
  virtual void Unlock () = 0;

  /**
   * Get flags for this triangle mesh. This is zero or a combination of the
   * following flags:
   * - #CS_TRIMESH_CLOSED: mesh is closed.
   * - #CS_TRIMESH_NOTCLOSED: mesh is not closed.
   * - #CS_TRIMESH_CONVEX: mesh is convex.
   * - #CS_TRIMESH_NOTCONVEX: mesh is not convex.
   * - #CS_TRIMESH_DEFORMABLE: mesh is deformable.
   *
   * Note that if neither #CS_TRIMESH_CLOSED nor #CS_TRIMESH_NOTCLOSED
   * are set then the closed state is not known. Setting both is illegal.
   * Note that if neither #CS_TRIMESH_CONVEX nor #CS_TRIMESH_NOTCONVEX
   * are set then the convex state is not known. Setting both is illegal.
   */
  virtual csFlags& GetFlags () = 0;

  /**
   * When this number changes you know the triangle mesh has changed
   * (deformation has occured) since the last time you got another
   * number from this function.
   */
  virtual uint32 GetChangeNumber () const = 0;
};

/** @} */

#endif // __CS_IGEOM_TRIMESH_H__

