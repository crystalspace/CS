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

#ifndef __CS_PMTOOLS_H__
#define __CS_PMTOOLS_H__

/**\file 
 */
/**
 * \addtogroup geom_utils
 * @{ */

class csVector3;
class csPlane3;
struct iPolygonMesh;

#include "csextern.h"

#include "csutil/array.h"
#include "csgeom/polymesh.h"

/**
 * A definition of one edge.
 */
struct csPolygonMeshEdge
{
  // Indices of the two vertices forming the edge.
  // vt1 < vt2.
  int vt1, vt2;
  // Indices of the two polygons sharing this edge.
  // If poly2 == -1 then this is an edge with only one polygon attached.
  int poly1, poly2;

  // Active or not. If this flag is true the edge is active in the sense
  // that it actually connects two non-coplanar polygons (or is only
  // connected to one polygon). Edges that are not active are not relevant
  // for outline calculation.
  bool active;
};

/**
 * A set of tools to work with iPolygonMesh instances.
 */
class CS_CSGEOM_EXPORT csPolygonMeshTools
{
public:
  /**
   * This function will calculate normals for all polygons in the mesh.
   * The given array of 'normals' should be big enough to have normals
   * for the number of polygons as defined in the mesh itself.
   */
  static void CalculateNormals (iPolygonMesh* mesh, csVector3* normals);

  /**
   * This function will calculate planes for all polygons in the mesh.
   * The given array of 'planes' should be big enough to have planes
   * for the number of polygons as defined in the mesh itself.
   */
  static void CalculatePlanes (iPolygonMesh* mesh, csPlane3* planes);

  /**
   * Create a table of edges for this mesh. The resulting table may later
   * be deleted with 'delete[]'. Note that every edge will only connect
   * two polygons. If more polygons connect to an edge then the edge
   * will be duplicated. The 'active' flag of the returned edges is not
   * calculated by this routine. Use 'CheckActiveEdges()' for that.
   */
  static csPolygonMeshEdge* CalculateEdges (iPolygonMesh*, int& num_edges);

  /**
   * This function will check all edges and mark them as active if the
   * two polygons are not co-planar. This function will return the number
   * of active edges.
   */
  static int CheckActiveEdges (csPolygonMeshEdge* edges, int num_edges,
  	csPlane3* planes);

  /**
   * Given a table of edges (as calculated with CalculateEdges()), a
   * table of planes (as calculated with CalculatePlanes()), and a position
   * in space. This function will calculate an outline that is valid from
   * that position. This outline will be given as an array of bool indicating
   * which vertex indices that are used (so these have to be transformed from
   * 3D to 2D) and also an array of double vertex indices (every set of two
   * vertex indices forms one edge) that form the outline.
   * This function will also return a radius. As long as the position
   * doesn't move outside this radius the outline will be valid.
   * The two input tables should have enough space for the returned
   * number of edges and vertex indices. The safest way is to allocate
   * double the amount of vertices as there are active edges in the input
   * edge table and enough vertices as the polygon mesh supports.
   * <br>
   * Note: this function requires that the given edges are marked as
   * active or not (use CheckActiveEdges()).
   * <br>
   * Note: num_outline_edges will be the amount of edges (which means
   * that there will be twice as much vertices in the 'outline_edges' table.
   */
  static void CalculateOutline (csPolygonMeshEdge* edges, int num_edges,
  	csPlane3* planes, int num_vertices,
	const csVector3& pos,
	int* outline_edges, int& num_outline_edges,
	bool* outline_verts,
	float& valid_radius);

  /**
   * Test whether a polygon mesh is closed.
   * \remark This function works best if vertices are shared.
   */
  static bool IsMeshClosed (iPolygonMesh* polyMesh);

  /**
   * Test whether a polygon mesh is convex. Note! This is NOT a fast
   * function. Use with care.
   * \remark This function works best if vertices are shared.
   */
  static bool IsMeshConvex (iPolygonMesh* polyMesh);

  /**
   * Close a polygon mesh.
   * The current implementation is rather naive; it just returns all faces,
   * but flipped. The returned table is a table of indices that are used
   * in the returned polygons. Don't forget to delete[] that table!
   */
  static void CloseMesh (iPolygonMesh* polyMesh, 
    csArray<csMeshedPolygon>& newPolys, int*& vertidx, int& vertidx_len);

  /**
   * Triangulate a mesh from the polygon mesh data in the iPolygonMesh.
   * Returns a table of triangles (delete with delete[]).
   */
  static void Triangulate (iPolygonMesh* polymesh,
  	csTriangle*& tris, int& tri_count);

  /**
   * Take a polygon mesh that has a valid set of triangles and generate
   * a polygon table for that (delete with delete[]). Note that the
   * polygons will point inside the triangle table!
   */
  static void Polygonize (iPolygonMesh* polymesh,
  	csMeshedPolygon*& polygons, int& poly_count);
};

/** @} */

#endif // __CS_PMTOOLS_H__

