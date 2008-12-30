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
#include "csutil/scf_implementation.h"

#include "csgeom/tri.h"
#include "csgeom/vector3.h"
#include "csgeom/box.h"
#include "igeom/trimesh.h"

#include "csutil/array.h"
#include "csutil/flags.h"
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
class CS_CRYSTALSPACE_EXPORT csTriangleMesh :
  public scfImplementation1<csTriangleMesh, iTriangleMesh>
{
protected:
  /// The triangles.
  csDirtyAccessArray<csTriangle> triangles;
  // The vertices.
  csDirtyAccessArray<csVector3> vertices;

  uint32 change_nr;
  csFlags flags;

public:
  ///
  csTriangleMesh () : scfImplementationType (this), change_nr (0) { }
  ///
  csTriangleMesh (const csTriangleMesh& mesh);
  ///
  virtual ~csTriangleMesh ();

  /// Add a vertex to the mesh.
  void AddVertex (const csVector3& v);
  /// Get the number of vertices for this mesh.
  virtual size_t GetVertexCount () { return vertices.GetSize (); }
  /// Get the number of vertices for this mesh.
  size_t GetVertexCount () const { return vertices.GetSize (); }
  /// Get the pointer to the array of vertices.
  virtual csVector3* GetVertices () { return vertices.GetArray (); }
  /// Get the pointer to the array of vertices.
  const csVector3* GetVertices () const { return vertices.GetArray (); }

  /// Add a triangle to the mesh.
  void AddTriangle (int a, int b, int c);
	/// Add another triangle mesh to this one.
	void AddTriangleMesh(const csTriangleMesh& tm);
  /// Query the array of triangles.
  virtual csTriangle* GetTriangles () { return triangles.GetArray (); }
  /// Query the array of triangles.
  const csTriangle* GetTriangles () const { return triangles.GetArray (); }
  ///
  csTriangle& GetTriangle (int i) { return triangles[i]; }
  /// Query the number of triangles.
  size_t GetTriangleCount () const { return triangles.GetSize (); }
  /// Query the number of triangles.
  virtual size_t GetTriangleCount () { return triangles.GetSize (); }

  /// Clear the mesh of triangles.
  void Clear ();
  /// Set the size of the triangle list.
  void SetSize (int count);
  /// Set the triangle array.  The array is copied.
  void SetTriangles (csTriangle const* trigs, int count);

  virtual void Lock () { }
  virtual void Unlock () { }
  virtual csFlags& GetFlags () { return flags; }
  virtual uint32 GetChangeNumber () const { return change_nr; }

	/// Adds another triangle mesh to this one
	csTriangleMesh& operator+=(const csTriangleMesh& tm);
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

/**
 * A convenience triangle mesh implementation that represents a cube.
 */
class CS_CRYSTALSPACE_EXPORT csTriangleMeshBox :
  public scfImplementation1<csTriangleMeshBox,iTriangleMesh>
{
private:
  csVector3 vertices[8];
  csTriangle triangles[12];
  uint32 change_nr;
  csFlags flags;

public:
  /**
   * Construct a cube triangle mesh.
   */
  csTriangleMeshBox (const csBox3& box) : scfImplementationType(this)
  {
    change_nr = 0;
    triangles[0].Set (4, 5, 1);
    triangles[1].Set (4, 1, 0);
    triangles[2].Set (5, 7, 3);
    triangles[3].Set (5, 3, 1);
    triangles[4].Set (7, 6, 2);
    triangles[5].Set (7, 2, 3);
    triangles[6].Set (6, 4, 0);
    triangles[7].Set (6, 0, 2);
    triangles[8].Set (6, 7, 5);
    triangles[9].Set (6, 5, 4);
    triangles[10].Set (0, 1, 3);
    triangles[11].Set (0, 3, 2);
    SetBox (box);

    flags.SetAll (CS_TRIMESH_CLOSED | CS_TRIMESH_CONVEX);
  }

  virtual ~csTriangleMeshBox ()
  {
  }

  /**
   * Set the box.
   */
  void SetBox (const csBox3& box)
  {
    change_nr++;
    vertices[0] = box.GetCorner (0);
    vertices[1] = box.GetCorner (1);
    vertices[2] = box.GetCorner (2);
    vertices[3] = box.GetCorner (3);
    vertices[4] = box.GetCorner (4);
    vertices[5] = box.GetCorner (5);
    vertices[6] = box.GetCorner (6);
    vertices[7] = box.GetCorner (7);
  }

  virtual size_t GetVertexCount () { return 8; }
  virtual csVector3* GetVertices () { return vertices; }
  virtual size_t GetTriangleCount () { return 12; }
  virtual csTriangle* GetTriangles () { return triangles; }
  virtual void Lock () { }
  virtual void Unlock () { }
  virtual csFlags& GetFlags () { return flags; }
  virtual uint32 GetChangeNumber () const { return change_nr; }
};

/**
 * A convenience triangle mesh which takes vertex and triangle
 * pointers from another source. Take care of object life time
 * when using this class; i.e. make sure the real owner of the
 * vertex and triangle data is not destroyed at a time when
 * this class is still in use.
 */
class CS_CRYSTALSPACE_EXPORT csTriangleMeshPointer :
  public scfImplementation1<csTriangleMeshPointer,iTriangleMesh>
{
private:
  csVector3* vertices;
  size_t num_vertices;
  csTriangle* triangles;
  size_t num_triangles;
  uint32 change_nr;
  csFlags flags;

public:
  /**
   * Construct a triangle mesh.
   */
  csTriangleMeshPointer (csVector3* vertices, size_t num_vertices,
      csTriangle* triangles, size_t num_triangles)
    : scfImplementationType(this)
  {
    change_nr = 0;
    csTriangleMeshPointer::vertices = vertices;
    csTriangleMeshPointer::num_vertices = num_vertices;
    csTriangleMeshPointer::triangles = triangles;
    csTriangleMeshPointer::num_triangles = num_triangles;
  }

  virtual ~csTriangleMeshPointer ()
  {
  }

  virtual size_t GetVertexCount () { return num_vertices; }
  virtual csVector3* GetVertices () { return vertices; }
  virtual size_t GetTriangleCount () { return num_triangles; }
  virtual csTriangle* GetTriangles () { return triangles; }
  virtual void Lock () { }
  virtual void Unlock () { }
  virtual csFlags& GetFlags () { return flags; }
  virtual uint32 GetChangeNumber () const { return change_nr; }
};

/** @} */

#endif // __CS_TRIMESH_H__

