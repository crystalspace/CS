/*
    Copyright (C) 1998,2001 by Jorrit Tyberghein

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

#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

#include "igraph3d.h"
#include "csgeom/math3d.h"

/**
 * A mesh of triangles. Note that a mesh of triangles is only valid
 * if used in combination with a vertex or edge table. Every triangle is then
 * a set of three indices in that table.
 */
class csTriangleMesh2
{
private:
  /// The triangles.
  csTriangle* triangles;
  int num_triangles;
  int max_triangles;

public:
  ///
  csTriangleMesh2 () : triangles (NULL), num_triangles (0), max_triangles (0) { }
  ///
  csTriangleMesh2 (const csTriangleMesh2& mesh);
  ///
  ~csTriangleMesh2 ();

  /// Add a triangle to the mesh.
  void AddTriangle (int a, int b, int c);
  /// Query the array of triangles.
  csTriangle* GetTriangles () { return triangles; }
  ///
  csTriangle& GetTriangle (int i) { return triangles[i]; }
  /// Query the number of triangles.
  int GetNumTriangles () { return num_triangles; }

  /// Clear the mesh of triangles.
  void Clear ();
  /// Reset the mesh of triangles (don't deallocate the internal structures yet).
  void Reset ();
};

class csTriangleVertices2;

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
  /// True if already deleted.
  bool deleted;

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

  /// Precalculated minimal cost of collapsing this vertex to some other.
  float cost;
  /// Vertex to collapse to with minimal cost.
  int to_vertex;

  ///
  csTriangleVertex () : deleted (false), con_triangles (NULL), num_con_triangles (0), max_con_triangles (0),
  	con_vertices (NULL), num_con_vertices (0), max_con_vertices (0) { }
  ///
  ~csTriangleVertex () { delete [] con_triangles; delete [] con_vertices; }
  ///
  void AddTriangle (int idx);
  ///
  void AddVertex (int idx);
  ///
  bool DelVertex (int idx);
  ///
  void ReplaceVertex (int old, int replace);

  /**
   * Calculate the minimal cost of collapsing this vertex to some other.
   * Also remember which other vertex was selected for collapsing to.
   */
  void CalculateCost (csTriangleVertices2* vertices);
};

/**
 * A class which holds vertices and connectivity information for a triangle
 * mesh. This is a general vertices structure but it is mostly useful
 * for LOD generation since every vertex contains information which
 * helps selecting the best vertices for collapsing.
 */
class csTriangleVertices2
{
private:
  csTriangleVertex* vertices;
  int num_vertices;

public:
  /// Build vertex table for a triangle mesh.
  csTriangleVertices2 (csTriangleMesh2* mesh, csVector3* verts, int num_verts);
  ///
  ~csTriangleVertices2 ();
  /// Update vertex table for a given set of vertices (with the same number as at init).
  void UpdateVertices (csVector3* verts);

  ///
  int GetNumVertices () { return num_vertices; }
  ///
  csTriangleVertex& GetVertex (int idx) { return vertices[idx]; }

  /// Calculate the cost of all vertices.
  void CalculateCost ();

  /// Return the vertex id with minimal cost.
  int GetMinimalCostVertex ();

  /// Dump connectivity information@@@ TEMPORARY
  void Dump ();
};

/**
 * A static class which performs the calculation
 * of the best order to do the collapsing.
 */
class csSpriteLOD
{
public:
  /**
   * For the given mesh and a set of vertices calculate the best
   * order in which to perform LOD reduction. This fills two arrays
   * (which should have the same size as the number of vertices in 'verts').
   * 'translate' contains a mapping from the old order of vertices to
   * the new one. The new ordering of vertices is done in a way so that
   * the first vertex is the one which is always present in the model
   * and with increasing detail, vertices are added in ascending vertex order.
   * 'emerge_from' contains (for a given index in the new order) from
   * which this vertex arises (or seen the other way around: to what this
   * vertex had collapsed).<p>
   *
   * Note. The given 'mesh' and 'verts' objects are no longer valid after
   * calling this function. Don't expect anything useful information here.
   */
  static void CalculateLOD (csTriangleMesh2* mesh, csTriangleVertices2* verts,
  	int* translate, int* emerge_from);
};


#endif /*__TRIANGLE_H__*/
