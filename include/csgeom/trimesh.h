/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_TRIMESH_H__
#define __CS_TRIMESH_H__

#include "csextern.h"

#include "csgeom/tri.h"
#include "csgeom/vector3.h"

#include "csutil/array.h"
#include "csutil/dirtyaccessarray.h"

/**\file
 * Triangle mesh.
 */
/**\addtogroup geom_utils
 * @{ */

/**
 * A mesh of triangles. Note that a mesh of triangles is only valid
 * if used in combination with a vertex or edge table. Every triangle is then
 * a set of three indices in that table.
 */
class CS_CRYSTALSPACE_EXPORT csTriangleMesh
{
protected:
  /// The triangles.
  csDirtyAccessArray<csTriangle> triangles;

public:
  ///
  csTriangleMesh () { }
  ///
  csTriangleMesh (const csTriangleMesh& mesh);
  ///
  ~csTriangleMesh ();

  /// Add a triangle to the mesh.
  void AddTriangle (int a, int b, int c);
  /// Query the array of triangles.
  csTriangle* GetTriangles () { return triangles.GetArray (); }
  /// Query the array of triangles.
  const csTriangle* GetTriangles () const { return triangles.GetArray (); }
  ///
  csTriangle& GetTriangle (int i) { return triangles[i]; }
  /// Query the number of triangles.
  size_t GetTriangleCount () const { return triangles.Length (); }

  /// Clear the mesh of triangles.
  void Clear ();
  /// Set the size of the triangle list.
  void SetSize (int count);
  /// Set the triangle array.  The array is copied.
  void SetTriangles (csTriangle const* trigs, int count);
};

/**
 * The representation of a vertex in a triangle mesh.
 * This is basically used as a temporary structure to be able to
 * calculate the cost of collapsing this vertex more quickly.
 */
class CS_CRYSTALSPACE_EXPORT csTriangleVertex
{
public:
  /// Position of this vertex in 3D space.
  csVector3 pos;
  /// Index of this vertex.
  int idx;

  /// Triangles that this vertex is connected to.
  csArray<size_t> con_triangles;

  /// Other vertices that this vertex is connected to.
  csArray<int> con_vertices;

  ///
  csTriangleVertex () { }
  ///
  ~csTriangleVertex () { }
  ///
  void AddTriangle (size_t idx);
  ///
  void AddVertex (int idx);
};

/**
 * A class which holds vertices and connectivity information for a triangle
 * mesh.
 */
class CS_CRYSTALSPACE_EXPORT csTriangleVertices
{
protected:
  csTriangleVertex* vertices;
  int num_vertices;

public:
  /// Build vertex table for a triangle mesh.
  csTriangleVertices (csTriangleMesh* mesh, csVector3* verts, int num_verts);
  ///
  ~csTriangleVertices ();
  /**
   * Update vertex table for a given set of vertices (with the same number
   * as at init).
   */
  void UpdateVertices (csVector3* verts);

  ///
  int GetVertexCount () const { return num_vertices; }
  ///
  csTriangleVertex& GetVertex (int idx) { return vertices[idx]; }
};

/** @} */

#endif // __CS_TRIMESH_H__

