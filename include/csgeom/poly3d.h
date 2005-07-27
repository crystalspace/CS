/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#ifndef __CS_POLY3D_H__
#define __CS_POLY3D_H__

/**\file 
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"

#include "csgeom/plane3.h"
#include "csgeom/vector3.h"
#include "csutil/dirtyaccessarray.h"

class csPoly2D;

// Values returned by classify.
enum
{
  CS_POL_SAME_PLANE = 0,
  CS_POL_FRONT = 1,
  CS_POL_BACK = 2,
  CS_POL_SPLIT_NEEDED = 3
};

/**
 * The following class represents a general 3D polygon.
 */
class CS_CRYSTALSPACE_EXPORT csPoly3D
{
protected:
  /// The 3D vertices.
  csDirtyAccessArray<csVector3> vertices;

public:
  /**
   * Make a new empty polygon.
   */
  csPoly3D (size_t start_size = 10);

  /// Copy constructor.
  csPoly3D (const csPoly3D& copy);

  /// Destructor.
  virtual ~csPoly3D ();

  /**
   * Initialize the polygon to empty.
   */
  void MakeEmpty ();

  /**
   * Get the number of vertices.
   */
  inline size_t GetVertexCount () const { return vertices.Length (); }

  /**
   * Get the array with all vertices.
   */
  inline const csVector3* GetVertices () const { return vertices.GetArray (); }

  /**
   * Get the array with all vertices.
   */
  inline csVector3* GetVertices () { return vertices.GetArray (); }

  /**
   * Get the specified vertex.
   */
  inline const csVector3* GetVertex (size_t i) const
  {
    if (i >= vertices.Length ()) return 0;
    return &(vertices.GetArray ()[i]);
  }

  /**
   * Get the specified vertex.
   */
  inline csVector3& operator[] (size_t i)
  {
    return vertices[i];
  }

  /**
   * Get the specified vertex.
   */
  inline const csVector3& operator[] (size_t i) const
  {
    return vertices[i];
  }

  /**
   * Get the first vertex.
   */
  inline const csVector3* GetFirst () const
  { 
    if (vertices.Length ()<=0) return 0;  
    else return vertices.GetArray ();
  }

  /**
   * Get the last vertex.
   */
  inline const csVector3* GetLast () const
  { 
    if (vertices.Length ()<=0) return 0; 
    else return 
      &(vertices.GetArray ())[vertices.Length ()-1]; 
  }

  /**
   * Test if this vector is inside the polygon.
   */
  bool In (const csVector3& v) const;

  /**
   * Test if a vector is inside the given polygon.
   */
  static bool In (csVector3* poly, size_t num_poly, const csVector3& v);

  /**
   * Make room for at least the specified number of vertices.
   */
  void MakeRoom (size_t new_max);

  /**
   * Set the number of vertices.
   */
  inline void SetVertexCount (size_t n) 
  { 
    MakeRoom (n);
    vertices.SetLength (n); 
  }

  /**
   * Add a vertex (3D) to the polygon.
   * Return index of added vertex.
   */
  inline size_t AddVertex (const csVector3& v) { return AddVertex (v.x, v.y, v.z); }

  /**
   * Add a vertex (3D) to the polygon.
   * Return index of added vertex.
   */
  size_t AddVertex (float x, float y, float z);

  /**
   * Set all polygon vertices at once.  Copies the array.
   */
  inline void SetVertices (csVector3 const* v, size_t num)
  {
    MakeRoom (num);
    memcpy (vertices.GetArray (), v, num * sizeof (csVector3));
  }

  /**
   * Project this polygon onto a X plane as seen from some
   * point in space. Fills the given 2D polygon with the projection
   * on the plane. This function assumes that there actually is
   * a projection. If the polygon to project comes on the same plane
   * as 'point' then it will return false (no valid projection).
   */
  bool ProjectXPlane (const csVector3& point, float plane_x,
  	csPoly2D* poly2d) const;

  /**
   * Project this polygon onto a Y plane as seen from some
   * point in space. Fills the given 2D polygon with the projection
   * on the plane. This function assumes that there actually is
   * a projection. If the polygon to project comes on the same plane
   * as 'point' then it will return false (no valid projection).
   */
  bool ProjectYPlane (const csVector3& point, float plane_y,
  	csPoly2D* poly2d) const;

  /**
   * Project this polygon onto a Z plane as seen from some
   * point in space. Fills the given 2D polygon with the projection
   * on the plane. This function assumes that there actually is
   * a projection. If the polygon to project comes on the same plane
   * as 'point' then it will return false (no valid projection).
   */
  bool ProjectZPlane (const csVector3& point, float plane_z,
  	csPoly2D* poly2d) const;

  /**
   * Project this polygon onto an axis-aligned plane as seen from some
   * point in space. Fills the given 2D polygon with the projection
   * on the plane. This function assumes that there actually is
   * a projection. Plane_nr is 0 for the X plane, 1 for Y, and 2 for Z.
   * Or one of the CS_AXIX_ constants.
   */
  inline bool ProjectAxisPlane (const csVector3& point, int plane_nr,
	float plane_pos, csPoly2D* poly2d) const
  {
    switch (plane_nr)
    {
      case CS_AXIS_X: return ProjectXPlane (point, plane_pos, poly2d);
      case CS_AXIS_Y: return ProjectYPlane (point, plane_pos, poly2d);
      case CS_AXIS_Z: return ProjectZPlane (point, plane_pos, poly2d);
    }
    return false;
  }

  /**
   * Static function to classify a polygon with regards to a plane.
   * If this poly is on same plane it returns CS_POL_SAME_PLANE.
   * If this poly is completely in front of the given plane it returnes
   * CS_POL_FRONT. If this poly is completely back of the given plane it
   * returnes CS_POL_BACK. Otherwise it returns CS_POL_SPLIT_NEEDED.
   */
  static int Classify (const csPlane3& pl,
  	const csVector3* vertices, size_t num_vertices);

  /**
   * Classify this polygon with regards to a plane.
   * If this poly is on same plane it returns CS_POL_SAME_PLANE. If this poly is
   * completely in front of the given plane it returnes CS_POL_FRONT. If this
   * poly is completely back of the given plane it returnes CS_POL_BACK.
   * Otherwise it returns CS_POL_SPLIT_NEEDED.
   */
  inline int Classify (const csPlane3& pl) const
  {
    return Classify (pl, vertices.GetArray (), vertices.Length ());
  }

  /// Same as Classify() but for X plane only.
  int ClassifyX (float x) const;

  /// Same as Classify() but for Y plane only.
  int ClassifyY (float y) const;

  /// Same as Classify() but for Z plane only.
  int ClassifyZ (float z) const;

  /// Same as Classify() but for a given axis plane.
  inline int ClassifyAxis (int axis, float where) const
  {
    switch (axis)
    {
      case CS_AXIS_X: return ClassifyX (where);
      case CS_AXIS_Y: return ClassifyY (where);
      case CS_AXIS_Z: return ClassifyZ (where);
    }
    return 0;
  }

  /**
   * Test if this polygon is axis aligned and return
   * the axis (one of CS_AXIS_ constants). The location
   * of the axis is returned in 'where'.
   * Returns CS_AXIS_NONE if the polygon is not axis aligned.
   * The epsilon will be used to test if two coordinates are close.
   */
  int IsAxisAligned (float& where, float epsilon = SMALL_EPSILON) const;

  /**
   * Calculate the main axis of the normal.
   * Returns one of the CS_AXIS_ constants;
   */
  int ComputeMainNormalAxis () const;

  /// Cut this polygon with a plane and only keep the front side.
  void CutToPlane (const csPlane3& split_plane);

  /// Split this polygon with the given plane (A,B,C,D).
  void SplitWithPlane (csPoly3D& front, csPoly3D& back,
  	const csPlane3& split_plane) const;

  /// Split this polygon to the x-plane.
  void SplitWithPlaneX (csPoly3D& front, csPoly3D& back, float x) const;

  /// Split this polygon to the y-plane.
  void SplitWithPlaneY (csPoly3D& front, csPoly3D& back, float y) const;

  /// Split this polygon to the z-plane.
  void SplitWithPlaneZ (csPoly3D& front, csPoly3D& back, float z) const;

  /// Compute the normal of a polygon.
  static csVector3 ComputeNormal (const csVector3* vertices, size_t num);

  /// Compute the normal of a polygon.
  static csVector3 ComputeNormal (const csArray<csVector3>& poly);

  /// Compute the normal of an indexed polygon.
  static csVector3 ComputeNormal (int* poly, size_t num, csVector3* vertices);

  /// Compute the normal of this polygon.
  inline csVector3 ComputeNormal () const
  {
    return ComputeNormal (vertices.GetArray (), vertices.Length ());
  }

  /// Compute the plane of a polygon.
  static csPlane3 ComputePlane (const csVector3* vertices, size_t num);

  /// Compute the plane of a polygon.
  static csPlane3 ComputePlane (const csArray<csVector3>& poly);

  /// Compute the plane of an indexed polygon.
  static csPlane3 ComputePlane (int* poly, size_t num, csVector3* vertices);

  /// Compute the plane of this polygon.
  inline csPlane3 ComputePlane () const
  {
    return ComputePlane (vertices.GetArray (), vertices.Length ());
  }

  /**
   * Calculate the area of this polygon.
   */
  float GetArea() const;

  /**
   * Compute and get the central vertex of this polygon.
   */
  csVector3 GetCenter () const;
};

/// This structure is used by csVector3Array::CompressVertices().
struct csCompressVertex
{
  size_t orig_idx;
  float x, y, z;
  size_t new_idx;
  bool used;
};

/**
 * This is actually the same class as csPoly3D. But it has been
 * renamed to make it clear that it is for other uses. It also
 * adds some functionality specific to that use. In particular
 * this class is more used to hold an unordered collection of 3D vectors.
 */
class CS_CRYSTALSPACE_EXPORT csVector3Array : public csPoly3D
{
public:
  csVector3Array (size_t start_size = 10) : csPoly3D (start_size) { }

  /**
   * Add a vertex but first check if it isn't already present
   * in the array. Return the index that the vertex was added too.
   */
  inline size_t AddVertexSmart (const csVector3& v)
  { return AddVertexSmart (v.x, v.y, v.z); }

  /**
   * Add a vertex but first check if it isn't already present
   * in the array. Return the index that the vertex was added too.
   */
  size_t AddVertexSmart (float x, float y, float z);

  /**
   * Compress an array of vertices (i.e. remove all duplicated
   * vertices). Returns an array of csCompressVertex which can be
   * used to map from the old index to the new one. 'new_count'
   * will be set to the new number of unique vertices (and 'new_vertices'
   * will be the new vertex table with that size). The size
   * of the returned array is 'num_vertices' though since it has
   * to be indexed with the original vertex array.
   * If this function returns 0 there is nothing to do (i.e. no duplicate
   * vertices). Otherwise you have to 'delete[]' the returned array.
   */
  static csCompressVertex* CompressVertices (csVector3* vertices,
	size_t num_vertices, csVector3*& new_vertices, size_t& new_count);

  /**
   * Compress an array of vertices (i.e. remove all duplicated
   * vertices). Returns an array of csCompressVertex which can be
   * used to map from the old index to the new one. The 'vertices'
   * table will be modified with the new compressed vertices.
   * If this function returns 0 there is nothing to do (i.e. no duplicate
   * vertices). Otherwise you have to 'delete[]' the returned array.
   */
  static csCompressVertex* CompressVertices (csArray<csVector3>& vertices);
};

/** @} */

#endif // __CS_POLY3D_H__
