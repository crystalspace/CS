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

#ifndef __GMTRIANGLE_H__
#define __GMTRIANGLE_H__

#include "ivideo/graph3d.h"
#include "csgeom/math3d.h"

/**
 * A mesh of triangles. Note that a mesh of triangles is only valid
 * if used in combination with a vertex or edge table. Every triangle is then
 * a set of three indices in that table.
 */
class csTriangleMesh
{
private:
  /// The triangles.
  csTriangle* triangles;
  int num_triangles;
  int max_triangles;

public:
  ///
  csTriangleMesh () : triangles (NULL), num_triangles (0), max_triangles (0) { }
  ///
  csTriangleMesh (const csTriangleMesh& mesh);
  ///
  ~csTriangleMesh ();

  /// Add a triangle to the mesh.
  void AddTriangle (int a, int b, int c);
  /// Query the array of triangles.
  csTriangle* GetTriangles () { return triangles; }
  ///
  csTriangle& GetTriangle (int i) { return triangles[i]; }
  /// Query the number of triangles.
  int GetTriangleCount () { return num_triangles; }

  /// Clear the mesh of triangles.
  void Clear ();
  /// Reset the mesh of triangles (don't deallocate internal structures yet).
  void Reset ();
  /// Set the size of the triangle list.
  void SetSize (int count);
  /// Set the triangle array.  The array is copied.
  void SetTriangles (csTriangle const* trigs, int count);
};

class csTriangleVertices;

/**
 * The representation of a vertex in a triangle mesh.
 * This is basicly used as a temporary structure to be able to
 * calculate the cost of collapsing this vertex more quickly.
 */
class csTriangleVertex
{
public:
  /// Position of this vertex in 3D space.
  csVector3 pos;
  /// Index of this vertex.
  int idx;

  /// Triangles that this vertex is connected to.
  int* con_triangles;
  /// Number of triangles.
  int num_con_triangles;
  int max_con_triangles;

  /// Other vertices that this vertex is connected to.
  int* con_vertices;
  /// Number of vertices.
  int num_con_vertices;
  int max_con_vertices;

  ///
  csTriangleVertex () : con_triangles (NULL),
  	num_con_triangles (0), max_con_triangles (0),
  	con_vertices (NULL), num_con_vertices (0), max_con_vertices (0) { }
  ///
  ~csTriangleVertex () { delete [] con_triangles; delete [] con_vertices; }
  ///
  void AddTriangle (int idx);
  ///
  void AddVertex (int idx);
};

/**
 * A class which holds vertices and connectivity information for a triangle
 * mesh.
 */
class csTriangleVertices
{
private:
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
  int GetVertexCount () { return num_vertices; }
  ///
  csTriangleVertex& GetVertex (int idx) { return vertices[idx]; }
};

#endif /*__GMTRIANGLE_H__*/

