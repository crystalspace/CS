/*
    Copyright (C) 2007 by Jorrit Tyberghein

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

#ifndef __CS_TRIMESHTOOLS_H__
#define __CS_TRIMESHTOOLS_H__


#include "csextern.h"

#include "csgeom/trimesh.h"
#include "csutil/array.h"

struct csTriangle;

/**
 * A triangle with minimum/maximum information on x.
 */
struct CS_CRYSTALSPACE_EXPORT csTriangleMinMax : public csTriangle
{
  float minx, maxx;
};

/**\file 
 * Set of tools to work with iTriangleMesh instances.
 */
/**
 * \addtogroup geom_utils
 * @{ */

class csVector3;
class csPlane3;
struct iTriangleMesh;

/**
 * A definition of one edge.
 */
struct CS_CRYSTALSPACE_EXPORT csTriangleMeshEdge : 
  public CS::Memory::CustomAllocated
{
  /**
   * Indices of the two vertices forming the edge.
   * vt1 < vt2.
   */
  int vt1, vt2;
  /**
   * Indices of the two triangles sharing this edge.
   * If tri2 == -1 then this is an edge with only one triangle attached.
   */
  int tri1, tri2;

  /**
   * Active or not. If this flag is true the edge is active in the sense
   * that it actually connects two non-coplanar triangles (or is only
   * connected to one triangle). Edges that are not active are not relevant
   * for outline calculation.
   */
  bool active;
};

/**
 * A set of tools to work with iTriangleMesh instances.
 */
class CS_CRYSTALSPACE_EXPORT csTriangleMeshTools
{
private:
  static void CalculatePlanes (csVector3* vertices,
  	csTriangleMinMax* tris, size_t num_tris, csPlane3* planes);

public:
  /**
   * This function will calculate normals for all triangles in the mesh.
   * The given array of 'normals' should be big enough to have normals
   * for the number of triangles as defined in the mesh itself.
   */
  static void CalculateNormals (iTriangleMesh* mesh, csVector3* normals);

  /**
   * This function will calculate planes for all triangles in the mesh.
   * The given array of 'planes' should be big enough to have planes
   * for the number of triangles as defined in the mesh itself.
   */
  static void CalculatePlanes (iTriangleMesh* mesh, csPlane3* planes);

  /**
   * Create a table of edges for this mesh. The resulting table may later
   * be deleted with 'delete[]'. Note that every edge will only connect
   * two triangles. If more triangles connect to an edge then the edge
   * will be duplicated. The 'active' flag of the returned edges is not
   * calculated by this routine. Use 'CheckActiveEdges()' for that.
   */
  static csTriangleMeshEdge* CalculateEdges (iTriangleMesh*, size_t& num_edges);

  /**
   * This function will check all edges and mark them as active if the
   * two triangles are not co-planar. This function will return the number
   * of active edges.
   */
  static size_t CheckActiveEdges (csTriangleMeshEdge* edges, size_t num_edges,
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
  static void CalculateOutline (csTriangleMeshEdge* edges, size_t num_edges,
  	csPlane3* planes, size_t num_vertices,
	const csVector3& pos,
	size_t* outline_edges, size_t& num_outline_edges,
	bool* outline_verts,
	float& valid_radius);

  /**
   * Test whether a triangle mesh is closed.
   * \remark This function works best if vertices are shared.
   */
  static bool IsMeshClosed (iTriangleMesh* trimesh);

  /**
   * Test whether a triangle mesh is convex. Note! This is NOT a fast
   * function. Use with care.
   * \remark This function works best if vertices are shared.
   */
  static bool IsMeshConvex (iTriangleMesh* trimesh);

  /**
   * Close a triangle mesh.
   * The current implementation is rather naive; it just returns all faces,
   * but flipped.
   */
  static void CloseMesh (iTriangleMesh* trimesh, csArray<csTriangle>& newtris);

  /**
   * Take a polygon mesh and sort triangles on maximum x coordinate.
   * That means that the first triangle in the returned array will
   * have a minimum x coordinates that is lower then further triangles.
   * This is useful for the SortedIn() routine below. This routine
   * will also calculate planes.
   * When done delete the returned arrays.
   */
  static void SortTrianglesX (iTriangleMesh* trimesh,
  	csTriangleMinMax*& tris, size_t& tri_count,
	csPlane3*& planes);

  /**
   * Test if a point is in a closed mesh. The mesh is defined by an
   * array of triangles which should be sorted on x using the SortTrianglesX()
   * function. This function does not check if the mesh is really closed.
   * This function also needs an array of planes. You can calculate that
   * with CalculatePlanes().
   */
  static bool PointInClosedMesh (const csVector3& point,
  	csVector3* vertices,
  	csTriangleMinMax* tris, size_t tri_count,
	csPlane3* planes);

  /**
   * Test if a line is in a closed mesh. The mesh is defined by an
   * array of triangles which should be sorted on x using the SortTrianglesX()
   * function. This function does not check if the mesh is really closed.
   * This function also needs an array of planes. You can calculate that
   * with CalculatePlanes().
   * This function does not check if the two points are actually in
   * the object. If they are not then you will actually reverse the check
   * and this function will return true if the line is completely outside
   * the object. Basically this function tests if the line intersects
   * some polygon in the object and it will return false if it does.
   */
  static bool LineInClosedMesh (const csVector3& p1, const csVector3& p2,
  	csVector3* vertices,
  	csTriangleMinMax* tris, size_t tri_count,
	csPlane3* planes);

  /**
   * Test if a box is in a closed mesh. The mesh is defined by an
   * array of triangles which should be sorted on x using the SortTrianglesX()
   * function. This function does not check if the mesh is really closed.
   * This function also needs an array of planes. You can calculate that
   * with CalculatePlanes().
   * This function does not check if the eight corner points are actually in
   * the object. If they are not then you will actually reverse the check
   * and this function will return true if the box is completely outside
   * the object. Basically this function tests if the box intersects
   * some polygon in the object and it will return false if it does.
   */
  static bool BoxInClosedMesh (const csBox3& box,
  	csVector3* vertices,
  	csTriangleMinMax* tris, size_t tri_count,
	csPlane3* planes);

  /**
   * Create a table of vertex connections for this mesh. The resulting
   * data structure must be freed by user after using it. 
   */
  static csArray<csArray<int> > *CalculateVertexConnections (
		  				iTriangleMesh* mesh);
};

/** @} */

#endif // __CS_TRIMESHTOOLS_H__

