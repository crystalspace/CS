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

#ifndef __CS_TRIMESHLOD_H__
#define __CS_TRIMESHLOD_H__

#include "csextern.h"

#include "csgeom/math3d.h"
#include "csgeom/trimesh.h"

class csTriangleVerticesCost;

/**
 * The representation of a vertex in a triangle mesh.
 * This is basically used as a temporary structure to be able to
 * calculate the cost of collapsing this vertex more quickly.
 */
class CS_CSGEOM_EXPORT csTriangleVertexCost : public csTriangleVertex
{
public:
  /// True if already deleted.
  bool deleted;

  /// Precalculated minimal cost of collapsing this vertex to some other.
  float cost;
  /// Vertex to collapse to with minimal cost.
  int to_vertex;

  ///
  csTriangleVertexCost () : deleted (false) { }
  ///
  ~csTriangleVertexCost () { }
  ///
  bool DelVertex (int idx);
  ///
  void ReplaceVertex (int old, int replace);

  /**
   * Calculate the minimal cost of collapsing this vertex to some other.
   * Also remember which other vertex was selected for collapsing to.
   */
  void CalculateCost (csTriangleVerticesCost* vertices);
};

/**
 * A class which holds vertices and connectivity information for a triangle
 * mesh. This is a general vertices structure but it is mostly useful
 * for LOD generation since every vertex contains information which
 * helps selecting the best vertices for collapsing.
 */
class CS_CSGEOM_EXPORT csTriangleVerticesCost
{
private:
  csTriangleVertexCost* vertices;
  int num_vertices;

public:
  /// Build vertex table for a triangle mesh.
  csTriangleVerticesCost (csTriangleMesh* mesh, csVector3* verts,
  	int num_verts);
  ///
  ~csTriangleVerticesCost ();
  /**
   * Update vertex table for a given set of vertices (with the same number as
   * at init).
   */
  void UpdateVertices (csVector3* verts);

  ///
  int GetVertexCount () { return num_vertices; }
  ///
  csTriangleVertexCost& GetVertex (int idx) { return vertices[idx]; }

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
class CS_CSGEOM_EXPORT csTriangleMeshLOD
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
   * calling this function. Don't expect any useful information here.
   */
  static void CalculateLOD (csTriangleMesh* mesh, csTriangleVerticesCost* verts,
  	int* translate, int* emerge_from);
};


#endif // __CS_TRIMESHLOD_H__

