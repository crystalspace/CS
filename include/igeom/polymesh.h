/*
    Crystal Space 3D engine
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef __CS_IGEOM_POLYMESH_H__
#define __CS_IGEOM_POLYMESH_H__

#include "csutil/scf.h"
#include "csutil/flags.h"

/**
 * \addtogroup geom_utils
 * @{ */
 
/** \name flags for iPolygonMesh.
 * @{ */

/**
 * If this flag is set then the object is closed. With closed we mean
 * that if you run a beam of light through the object it will always
 * hit an even amount of faces (one going in, and one going out).
 */
#define CS_POLYMESH_CLOSED 1

/**
 * If this flag is set then the object is not closed.
 * This is the opposite of CS_POLYMESH_CLOSED. Use this flag if you are
 * absolutely certain that the object is not closed. The engine will not
 * attempt to test if the object is really closed or not. If you don't
 * set CLOSED or NOTCLOSED then the state is not known and the engine
 * may test it if it wants.
 */
#define CS_POLYMESH_NOTCLOSED 2

/**
 * If this flag is set then the object is convex. With convex we mean
 * that if you run a beam of light through the object it will always
 * hit an two faces (one going in, and one going out).
 */
#define CS_POLYMESH_CONVEX 4

/**
 * If this flag is set then the object is not convex.
 * This is the opposite of CS_POLYMESH_CONVEX. Use this flag if you are
 * absolutely certain that the object is not convex. The engine will not
 * attempt to test if the object is really convex or not. If you don't
 * set CONVEX or NOTCONVEX then the state is not known and the engine
 * may test it if it wants.
 */
#define CS_POLYMESH_NOTCONVEX 8

/**
 * Set this flag if the polygon mesh is deformable.
 */
#define CS_POLYMESH_DEFORMABLE 16

/** @} */

/**
 * A polygon. Note that this structure is only valid if used
 * in combination with a vertex table. The vertex array then
 * contains indices in that table.
 */
struct csMeshedPolygon
{
  int num_vertices;
  int* vertices;
};

class csVector3;
class csTriangle;

SCF_VERSION (iPolygonMesh, 0, 4, 0);

/**
 * This interface reprents a mesh of polygons. It is useful to communicate
 * geometry information outside of the engine. One place where this will
 * be useful is for communicating geometry information to the collision
 * detection plugin.<br>
 * All Crystal Space mesh objects (things, sprites, ...)
 * should implement and/or embed an implementation of this interface.
 * <p>
 * A polygon mesh has the concept of a vertex buffer and an array of polygons.
 * A triangle mesh is also supported. A mesh object typically only implements
 * either a polygon mesh or a triangle mesh. In that case requesting
 * the other type of mesh will automatically generate the new format.
 * iPolygonMesh can use csPolygonMeshTools::Triangulate() and
 * csPolygonMeshTools::Polygonize() to help with that.
 */
struct iPolygonMesh : public iBase
{
  /// Get the number of vertices for this mesh.
  virtual int GetVertexCount () = 0;
  /// Get the pointer to the array of vertices.
  virtual csVector3* GetVertices () = 0;
  /// Get the number of polygons for this mesh.
  virtual int GetPolygonCount () = 0;
  /// Get the pointer to the array of polygons.
  virtual csMeshedPolygon* GetPolygons () = 0;
  /// Get the number of triangles for this mesh.
  virtual int GetTriangleCount () = 0;
  /// Get the triangle table for this mesh.
  virtual csTriangle* GetTriangles () = 0;
  /**
   * Cleanup: this is called by the polygon mesh user
   * when it is ready extracting the data from the iPolygonMesh.
   * This gives the polygon mesh a chance to clean up some stuff.
   */
  virtual void Cleanup () = 0;

  /**
   * Get flags for this polygon mesh. This is zero or more of the
   * following:
   * <ul>
   * <li>#CS_POLYMESH_CLOSED: mesh is closed.
   * <li>#CS_POLYMESH_NOTCLOSED: mesh is not closed.
   * <li>#CS_POLYMESH_CONVEX: mesh is convex.
   * <li>#CS_POLYMESH_NOTCONVEX: mesh is not convex.
   * <li>#CS_POLYMESH_DEFORMABLE: mesh is deformable.
   * </ul>
   * Note that if neither #CS_POLYMESH_CLOSED nor #CS_POLYMESH_NOTCLOSED
   * are set then the closed state is not known. Setting both is illegal.
   * Note that if neither #CS_POLYMESH_CONVEX nor #CS_POLYMESH_NOTCONVEX
   * are set then the convex state is not known. Setting both is illegal.
   */
  virtual csFlags& GetFlags () = 0;

  /**
   * When this number changes you know the polygon mesh has changed
   * (deformation has occured) since the last time you got another
   * number from this function.
   */
  virtual uint32 GetChangeNumber () const = 0;
};

/** @} */

#endif // __CS_IGEOM_POLYMESH_H__

