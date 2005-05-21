/*
    Copyright (C) 1998-2002 by Jorrit Tyberghein
    Largely rewritten by Ivan Avramovic <ivan@avramovic.com>

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

#ifndef __CS_BOX_H__
#define __CS_BOX_H__

/**\file 
 * Bounding boxes for 2D and 3D space.
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"

#include "cstypes.h"	// for bool
#include "csrect.h"
#include "vector2.h"
#include "vector3.h"
#include "segment.h"

#include "csutil/array.h"
#include "csutil/csstring.h"

class csPlane3;
class csTransform;
class csPoly2D;

/**
 * The maximum value that a coordinate in the bounding box can use.
 * This is considered the 'infinity' value used for empty bounding boxes.
 */
#define CS_BOUNDINGBOX_MAXVALUE 1000000000.

/**\name Corner indices
 * For csBox2::GetCorner().
 * @{ */
/// min X, min Y
#define CS_BOX_CORNER_xy 0
/// min X, max Y
#define CS_BOX_CORNER_xY 1
/// max X, min Y
#define CS_BOX_CORNER_Xy 2
/// max X, max Y
#define CS_BOX_CORNER_XY 3
/// center
#define CS_BOX_CENTER2 4
/** @} */

/**
 * \name Indices of edges for csBox2.
 * Index e+1 is opposite edge of e (with e even).
 * @{ */
/// from min X, min Y to max X, min Y
#define CS_BOX_EDGE_xy_Xy 0
/// from max X, min Y to min X, min Y
#define CS_BOX_EDGE_Xy_xy 1
/// from max X, min Y to max X, max Y
#define CS_BOX_EDGE_Xy_XY 2
/// from max X, max Y to max X, min Y
#define CS_BOX_EDGE_XY_Xy 3
/// from max X, max Y to min X, max Y
#define CS_BOX_EDGE_XY_xY 4
/// from min X, max Y to max X, max Y
#define CS_BOX_EDGE_xY_XY 5
/// from min X, max Y to min X, min Y
#define CS_BOX_EDGE_xY_xy 6
/// from min X, min Y to min X, max Y
#define CS_BOX_EDGE_xy_xY 7
/** @} */

/**
 * A bounding box in 2D space.
 * In order to operate correctly, this bounding box assumes that all values
 * entered or compared against lie within the range
 * (-#CS_BOUNDINGBOX_MAXVALUE, #CS_BOUNDINGBOX_MAXVALUE).  It is not
 * recommended to use points outside of this range.
 */
class CS_CRYSTALSPACE_EXPORT csBox2
{
private:
  struct bEdge
  {
    uint8 v1, v2; // Indices of vertex in bounding box (CS_BOX_CORNER_...)
  };
  // Index by edge number. Edge e and e+1 with e even are opposite edges.
  // (CS_BOX_EDGE_...)
  static bEdge edges[8];

protected:
  /// The top-left coordinate of the bounding box.
  csVector2 minbox;
  /// The bottom-right coordinate of the bounding box.
  csVector2 maxbox;

public:
  /// Get the minimum X value of the box
  float MinX () const { return minbox.x; }
  /// Get the minimum Y value of the box
  float MinY () const { return minbox.y; }
  /// Get the maximum X value of the box
  float MaxX () const { return maxbox.x; }
  /// Get the maximum Y value of the box
  float MaxY () const { return maxbox.y; }
  /// Get Min component for 0 (x) or 1 (y).
  float Min (int idx) const { return idx ? minbox.y : minbox.x; }
  /// Get Max component for 0 (x) or 1 (y).
  float Max (int idx) const { return idx ? maxbox.y : maxbox.x; }
  /// Get the 2d vector of minimum (x, y) values
  const csVector2& Min () const { return minbox; }
  /// Get the 2d vector of maximum (x, y) values
  const csVector2& Max () const { return maxbox; }
  /// Compute area of box
  float Area () const { return (MaxX()-MinX())*(MaxY()-MinY())); }

  /**
   * Return every corner of this bounding box from 0
   * to 3. This contrasts with Min() and Max() because
   * those are only the min and max corners.
   * Corner 0 = xy, 1 = xY, 2 = Xy, 3 = XY.
   * Use #CS_BOX_CORNER_xy etc. defines.
   * #CS_BOX_CENTER2 also works.
   */
  csVector2 GetCorner (int corner) const;

  /**
   * Get the center of this box.
   */
  csVector2 GetCenter () const { return (minbox+maxbox)/2; }

  /**
   * Set the center of this box. This will not change the size
   * of the box but just relocate the center.
   */
  void SetCenter (const csVector2& c);

  /**
   * Set the size of the box but keep the center intact.
   */
  void SetSize (const csVector2& s);

  /**
   * Given an edge index (#CS_BOX_EDGE_xy_Xy etc.) return the two vertices
   * (index #CS_BOX_CORNER_xy etc.).
   */
  void GetEdgeInfo (int edge, int& v1, int& v2) const
  {
    v1 = edges[edge].v1;
    v2 = edges[edge].v2;
  }

  /**
   * Return every edge (segment) of this bounding box
   * from 0 to 7 (#CS_BOX_EDGE_xy_Xy etc.).
   */
  csSegment2 GetEdge (int edge) const
  {
    return csSegment2 (GetCorner (edges[edge].v1), GetCorner (edges[edge].v2));
  }

  /**
   * Return every edge (segment) of this bounding box
   * from 0 to 7 (#CS_BOX_EDGE_xy_Xy etc.).
   */
  void GetEdge (int edge, csSegment2& e) const
  {
    e.SetStart (GetCorner (edges[edge].v1));
    e.SetEnd (GetCorner (edges[edge].v2));
  }

  /**
   * Test if a polygon if visible in the box. This
   * function does not test the case where the box is
   * fully contained in the polygon. But all other
   * cases are tested.
   */
  static bool Intersect (float minx, float miny, float maxx, float maxy,
    csVector2* poly, int num_poly);

  /**
   * Test if a polygon if visible in the box. This
   * function does not test the case where the box is
   * fully contained in the polygon. But all other
   * cases are tested.
   */
  static bool Intersect (const csVector2& minbox, const csVector2& maxbox,
    csVector2* poly, int num_poly)
  {
    return Intersect (minbox.x, minbox.y, maxbox.x, maxbox.y, poly, num_poly);
  }

  /**
   * Test if a polygon if visible in the box. This
   * function does not test the case where the box is
   * fully contained in the polygon. But all other
   * cases are tested.
   */
  bool Intersect (csVector2* poly, int num_poly) const
  {
    return Intersect (minbox, maxbox, poly, num_poly);
  }

  /// Test if the given coordinate is in this box.
  bool In (float x, float y) const
  {
    if (x < minbox.x || x > maxbox.x) return false;
    if (y < minbox.y || y > maxbox.y) return false;
    return true;
  }

  /// Test if the given coordinate is in this box.
  bool In (const csVector2& v) const
  {
    return In (v.x, v.y);
  }

  /// Test if this box overlaps with the given box.
  bool Overlap (const csBox2& box) const
  {
    if (maxbox.x < box.minbox.x || minbox.x > box.maxbox.x) return false;
    if (maxbox.y < box.minbox.y || minbox.y > box.maxbox.y) return false;
    return true;
  }

  /// Test if this box contains the other box.
  bool Contains (const csBox2& box) const
  {
    return (box.minbox.x >= minbox.x && box.maxbox.x <= maxbox.x) &&
           (box.minbox.y >= minbox.y && box.maxbox.y <= maxbox.y);
  }

  /// Test if this box is empty.
  bool Empty () const
  {
    if (minbox.x > maxbox.x) return true;
    if (minbox.y > maxbox.y) return true;
    return false;
  }

  /**
   * Calculate the squared distance between (0,0) and the box
   * This routine is extremely efficient.
   */
  float SquaredOriginDist () const;

  /**
   * Calculate the squared distance between (0,0) and the point
   * on the box which is furthest away from (0,0).
   * This routine is extremely efficient.
   */
  float SquaredOriginMaxDist () const;

  /// Initialize this box to empty.
  void StartBoundingBox ()
  {
    minbox.x =  CS_BOUNDINGBOX_MAXVALUE;  minbox.y =  CS_BOUNDINGBOX_MAXVALUE;
    maxbox.x = -CS_BOUNDINGBOX_MAXVALUE;  maxbox.y = -CS_BOUNDINGBOX_MAXVALUE;
  }

  /// Initialize this box to one vertex.
  void StartBoundingBox (const csVector2& v)
  {
    minbox = v;
    maxbox = v;
  }

  /// Same but given some coordinates.
  void StartBoundingBox (float x, float y)
  {
    minbox.x = maxbox.x = x;
    minbox.y = maxbox.y = y;
  }

  /// Add a new vertex and recalculate the bounding box.
  void AddBoundingVertex (float x, float y)
  {
    if (x < minbox.x) minbox.x = x;  if (x > maxbox.x) maxbox.x = x;
    if (y < minbox.y) minbox.y = y;  if (y > maxbox.y) maxbox.y = y;
  }

  /// Add a new vertex and recalculate the bounding box.
  void AddBoundingVertex (const csVector2& v)
  {
    AddBoundingVertex (v.x, v.y);
  }

  /**
   * Add a new vertex and recalculate the bounding box.
   * This version is a little more optimal. It assumes however
   * that at least one point has been added to the bounding box.
   */
  void AddBoundingVertexSmart (float x, float y)
  {
    if (x < minbox.x) minbox.x = x; else if (x > maxbox.x) maxbox.x = x;
    if (y < minbox.y) minbox.y = y; else if (y > maxbox.y) maxbox.y = y;
  }

  /**
   * Add a new vertex and recalculate the bounding box.
   * This version is a little more optimal. It assumes however
   * that at least one point has been added to the bounding box.
   */
  void AddBoundingVertexSmart (const csVector2& v)
  {
    AddBoundingVertexSmart (v.x, v.y);
  }

  /**
   * Add a new vertex and recalculate the bounding box.
   * Return true if the box was modified.
   */
  bool AddBoundingVertexTest (float x, float y)
  {
    bool rc = false;
    if (x < minbox.x) { minbox.x = x; rc = true; }
    if (x > maxbox.x) { maxbox.x = x; rc = true; }
    if (y < minbox.y) { minbox.y = y; rc = true; }
    if (y > maxbox.y) { maxbox.y = y; rc = true; }
    return rc;
  }

  /**
   * Add a new vertex and recalculate the bounding box.
   * Return true if the box was modified.
   */
  bool AddBoundingVertexTest (const csVector2& v)
  {
    return AddBoundingVertexTest (v.x, v.y);
  }

  /**
   * Add a new vertex and recalculate the bounding box.
   * This version is a little more optimal. It assumes however
   * that at least one point has been added to the bounding box.
   * Return true if the box was modified.
   */
  bool AddBoundingVertexSmartTest (float x, float y)
  {
    bool rc = false;
    if (x < minbox.x) { minbox.x = x; rc = true; }
    else if (x > maxbox.x) { maxbox.x = x; rc = true; }
    if (y < minbox.y) { minbox.y = y; rc = true; }
    else if (y > maxbox.y) { maxbox.y = y; rc = true; }
    return rc;
  }

  /**
   * Add a new vertex and recalculate the bounding box.
   * This version is a little more optimal. It assumes however
   * that at least one point has been added to the bounding box.
   * Return true if the box was modified.
   */
  bool AddBoundingVertexSmartTest (const csVector2& v)
  {
    return AddBoundingVertexSmartTest (v.x, v.y);
  }

  /// Initialize this box to empty.
  csBox2 () : minbox (CS_BOUNDINGBOX_MAXVALUE, CS_BOUNDINGBOX_MAXVALUE),
	     maxbox (-CS_BOUNDINGBOX_MAXVALUE, -CS_BOUNDINGBOX_MAXVALUE) {}

  /// Initialize this box with one point.
  csBox2 (const csVector2& v) : minbox (v.x, v.y), maxbox (v.x, v.y) {}

  /// Initialize this box with the given values.
  csBox2 (float x1, float y1, float x2, float y2) :
    minbox (x1, y1), maxbox (x2, y2)
  { if (Empty ()) StartBoundingBox (); }

  /// Initialize this box from the given csRect.
  csBox2 (const csRect& r) : minbox (r.xmin, r.ymin), maxbox (r.xmax, r.ymax)
  { }
  
  /// Sets the bounds of the box with the given values.
  void Set (const csVector2& bmin, const csVector2& bmax)
  {
    minbox = bmin;
    maxbox = bmax;
  }

  /// Sets the bounds of the box with the given values.
  void Set (float x1, float y1, float x2, float y2)
  {
    if (x1>x2 || y1>y2) StartBoundingBox();
    else { minbox.x = x1;  minbox.y = y1;  maxbox.x = x2;  maxbox.y = y2; }
  }

  /// Set Min component for 0 (x) or 1 (y).
  void SetMin (int idx, float val)
  {
    if (idx == 1) minbox.y = val;
    else minbox.x = val;
  }

  /// Set Max component for 0 (x) or 1 (y).
  void SetMax (int idx, float val)
  {
    if (idx == 1) maxbox.y = val;
    else maxbox.x = val;
  }

  /**
   * Return a textual representation of the box in the form
   * "(minx,miny)-(maxx,maxy)".
   */
  csString Description() const;

  /// Compute the union of two bounding boxes.
  csBox2& operator+= (const csBox2& box);
  /// Compute the union of a point with this bounding box.
  csBox2& operator+= (const csVector2& point);
  /// Compute the intersection of two bounding boxes.
  csBox2& operator*= (const csBox2& box);
  /// Test if the two boxes have an intersection.
  bool TestIntersect (const csBox2& box) const;

  /// Compute the union of two bounding boxes.
  friend CS_CRYSTALSPACE_EXPORT csBox2 operator+ (const csBox2& box1, 
    const csBox2& box2);
  /// Compute the union of a bounding box and a point.
  friend CS_CRYSTALSPACE_EXPORT csBox2 operator+ (const csBox2& box, 
    const csVector2& point);
  /// Compute the intersection of two bounding boxes.
  friend CS_CRYSTALSPACE_EXPORT csBox2 operator* (const csBox2& box1, 
    const csBox2& box2);

  /// Tests if two bounding boxes are equal.
  friend CS_CRYSTALSPACE_EXPORT bool operator== (const csBox2& box1, 
    const csBox2& box2);
  /// Tests if two bounding boxes are unequal.
  friend CS_CRYSTALSPACE_EXPORT bool operator!= (const csBox2& box1, 
    const csBox2& box2);
  /// Tests if box1 is a subset of box2.
  friend CS_CRYSTALSPACE_EXPORT bool operator< (const csBox2& box1, 
    const csBox2& box2);
  /// Tests if box1 is a superset of box2.
  friend CS_CRYSTALSPACE_EXPORT bool operator> (const csBox2& box1, 
    const csBox2& box2);
  /// Tests if a point is contained in a box.
  friend CS_CRYSTALSPACE_EXPORT bool operator< (const csVector2& point, 
    const csBox2& box);
};

/**
 * \name Indices of corner vertices for csBox3.
 * Used by csBox3::GetCorner().
 * @{ */
/// min X, min Y, min Z
#define CS_BOX_CORNER_xyz 0
/// min X, min Y, max Z
#define CS_BOX_CORNER_xyZ 1
/// min X, max Y, min Z
#define CS_BOX_CORNER_xYz 2
/// min X, max Y, max Z
#define CS_BOX_CORNER_xYZ 3
/// min X, min Y, min Z
#define CS_BOX_CORNER_Xyz 4
/// max X, min Y, max Z
#define CS_BOX_CORNER_XyZ 5
/// max X, max Y, min Z
#define CS_BOX_CORNER_XYz 6
/// max X, max Y, max Z
#define CS_BOX_CORNER_XYZ 7
/// center
#define CS_BOX_CENTER3 8
/** @} */

/**
 * \name Indices of faces for csBox3.
 * Used by csBox3::GetSide().
 * @{ */
/// min X
#define CS_BOX_SIDE_x 0
/// max X
#define CS_BOX_SIDE_X 1
/// min Y
#define CS_BOX_SIDE_y 2
/// max Y
#define CS_BOX_SIDE_Y 3
/// min Z
#define CS_BOX_SIDE_z 4
/// max Z
#define CS_BOX_SIDE_Z 5
/// inside
#define CS_BOX_INSIDE 6
/** @} */

/**
 * \name Indices of edges for csBox3.
 * Index e+1 is opposite edge of e (with e even).
 * @{ */
/// from max X, min Y, min Z to min X, min Y, min Z
#define CS_BOX_EDGE_Xyz_xyz 0
/// from min X, min Y, min Z to max X, min Y, min Z
#define CS_BOX_EDGE_xyz_Xyz 1
/// from min X, min Y, min Z to min X, max Y, min Z
#define CS_BOX_EDGE_xyz_xYz 2
/// from min X, max Y, min Z to min X, min Y, min Z
#define CS_BOX_EDGE_xYz_xyz 3
/// from min X, max Y, min Z to max X, max Y, min Z
#define CS_BOX_EDGE_xYz_XYz 4
/// from max X, max Y, min Z to min X, max Y, min Z
#define CS_BOX_EDGE_XYz_xYz 5
/// from max X, max Y, min Z to max X, min Y, min Z
#define CS_BOX_EDGE_XYz_Xyz 6
/// from max X, min Y, min Z to max X, max Y, min Z
#define CS_BOX_EDGE_Xyz_XYz 7
/// from max X, min Y, min Z to max X, min Y, max Z
#define CS_BOX_EDGE_Xyz_XyZ 8
/// from max X, min Y, max Z to max X, min Y, min Z
#define CS_BOX_EDGE_XyZ_Xyz 9
/// from max X, min Y, max Z to max X, max Y, max Z
#define CS_BOX_EDGE_XyZ_XYZ 10
/// from max X, max Y, max Z to max X, min Y, max Z
#define CS_BOX_EDGE_XYZ_XyZ 11
/// from max X, max Y, max Z to max X, max Y, min Z
#define CS_BOX_EDGE_XYZ_XYz 12
/// from max X, max Y, min Z to max X, max Y, max Z
#define CS_BOX_EDGE_XYz_XYZ 13
/// from max X, max Y, max Z to min X, max Y, max Z
#define CS_BOX_EDGE_XYZ_xYZ 14
/// from min X, max Y, max Z to max X, max Y, max Z
#define CS_BOX_EDGE_xYZ_XYZ 15
/// from min X, max Y, max Z to min X, max Y, min Z
#define CS_BOX_EDGE_xYZ_xYz 16
/// from min X, max Y, min Z to min X, max Y, max Z
#define CS_BOX_EDGE_xYz_xYZ 17
/// from min X, max Y, max Z to min X, min Y, max Z
#define CS_BOX_EDGE_xYZ_xyZ 18
/// from min X, min Y, max Z to min X, max Y, max Z
#define CS_BOX_EDGE_xyZ_xYZ 19
/// from min X, min Y, max Z to min X, min Y, min Z
#define CS_BOX_EDGE_xyZ_xyz 20
/// from min X, min Y, min Z to min X, min Y, max Z
#define CS_BOX_EDGE_xyz_xyZ 21
/// from min X, min Y, max Z to max X, min Y, max Z
#define CS_BOX_EDGE_xyZ_XyZ 22
/// from max X, min Y, max Z to min X, min Y, max Z
#define CS_BOX_EDGE_XyZ_xyZ 23
/** @} */

/**
 * A bounding box in 3D space.
 * In order to operate correctly, this bounding box assumes that all values
 * entered or compared against lie within the range
 * (-#CS_BOUNDINGBOX_MAXVALUE, #CS_BOUNDINGBOX_MAXVALUE).  It is not
 * recommended to use points outside of this range.
 */
class CS_CRYSTALSPACE_EXPORT csBox3
{
protected:
  /// The top-left of this bounding box.
  csVector3 minbox;
  /// The bottom-right.
  csVector3 maxbox;
  /** \internal
   * A csBox3 edge.
   */
  struct bEdge
  {
    uint8 v1, v2; // Indices of vertex in bounding box (CS_BOX_CORNER_...)
    uint8 fl, fr; // Indices of left/right faces sharing edge (CS_BOX_SIDE_...)
  };
  /// Indices of four clock-wise edges (0..23)
  typedef uint8 bFace[4];	
  /**
   * Index by edge number. Edge e and e+1 with e even are opposite edges.
   * (CS_BOX_EDGE_...) 
   */
  static bEdge edges[24];
  /// Index by CS_BOX_SIDE_? number.
  static bFace faces[6];
public:
  /// Get the minimum X value of the box
  float MinX () const { return minbox.x; }
  /// Get the minimum Y value of the box
  float MinY () const { return minbox.y; }
  /// Get the minimum Z value of the box
  float MinZ () const { return minbox.z; }
  /// Get the maximum X value of the box
  float MaxX () const { return maxbox.x; }
  /// Get the maximum Y value of the box
  float MaxY () const { return maxbox.y; }
  /// Get the maximum Z value of the box
  float MaxZ () const { return maxbox.z; }
  /// Get Min component for 0 (x), 1 (y), or 2 (z).
  float Min (int idx) const
  { return idx == 1 ? minbox.y : idx == 0 ? minbox.x : minbox.z; }
  /// Get Max component for 0 (x), 1 (y), or 2 (z).
  float Max (int idx) const
  { return idx == 1 ? maxbox.y : idx == 0 ? maxbox.x : maxbox.z; }
  /// Get the 3d vector of minimum (x, y, z) values
  const csVector3& Min () const { return minbox; }
  /// Get the 3d vector of maximum (x, y, z) values
  const csVector3& Max () const { return maxbox; }
  /// Compute volume of box
  float Volume () const
  { return (MaxX()-MinX())*(MaxY()-MinY())*(MaxZ()-MinZ()); }

  /**
   * Return every corner of this bounding box from 0
   * to 7. This contrasts with Min() and Max() because
   * those are only the min and max corners.
   * Corner 0 = xyz, 1 = xyZ, 2 = xYz, 3 = xYZ,
   *        4 = Xyz, 5 = XyZ, 6 = XYz, 7 = XYZ.
   * Use #CS_BOX_CORNER_xyz etc. defines.
   * #CS_BOX_CENTER3 also works.
   */
  csVector3 GetCorner (int corner) const;

  /**
   * Given an edge index (#CS_BOX_EDGE_Xyz_xyz etc.) return the two vertices
   * (index #CS_BOX_CORNER_xyz, etc.) and left/right faces
   * (#CS_BOX_SIDE_x, etc.).
   */
  void GetEdgeInfo (int edge, int& v1, int& v2, int& fleft, int& fright) const
  {
    v1 = edges[edge].v1;
    v2 = edges[edge].v2;
    fleft = edges[edge].fl;
    fright = edges[edge].fr;
  }

  /**
   * Given a face index (#CS_BOX_SIDE_x etc.) return the four edges oriented
   * clockwise around this face (#CS_BOX_EDGE_Xyz_xyz etc.).
   */
  uint8* GetFaceEdges (int face) const
  {
    return faces[face];
  }

  /**
   * Get the center of this box.
   */
  csVector3 GetCenter () const { return (minbox+maxbox)/2; }

  /**
   * Set the center of this box. This will not change the size
   * of the box but just relocate the center.
   */
  void SetCenter (const csVector3& c);

  /**
   * Set the size of the box but keep the center intact.
   */
  void SetSize (const csVector3& s);

  /**
   * Get a side of this box as a 2D box.
   * Use #CS_BOX_SIDE_x etc. defines.
   */
  csBox2 GetSide (int side) const;

  /**
   * Get axis aligned plane information from a side of this box.
   * Side is one of #CS_BOX_SIDE_x. Axis will be one of #CS_AXIS_X.
   */
  void GetAxisPlane (int side, int& axis, float& where) const;

  /**
   * Fill the array (which should be three long at least)
   * with all visible sides (#CS_BOX_SIDE_x etc. defines) as seen
   * from the given point.
   * Returns the number of visible sides.
   */
  int GetVisibleSides (const csVector3& pos, int* visible_sides) const;

  /**
   * Static function to get the 'other' side (i.e. CS_BOX_SIDE_X
   * to CS_BOX_SIDE_x, ...).
   */
  static int OtherSide (int side)
  {
    return side ^ 1;
  }

  /**
   * Return every edge (segment) of this bounding box
   * from 0 to 23 (use one of the #CS_BOX_EDGE_Xyz_xyz etc. indices).
   * The returned edge is undefined for any other index.
   */
  csSegment3 GetEdge (int edge) const
  {
    return csSegment3 (GetCorner (edges[edge].v1), GetCorner (edges[edge].v2));
  }

  /**
   * Return every edge (segment) of this bounding box
   * from 0 to 23 (use one of the #CS_BOX_EDGE_Xyz_xyz etc. indices).
   * The returned edge is undefined for any other index.
   */
  void GetEdge (int edge, csSegment3& e) const
  {
    e.SetStart (GetCorner (edges[edge].v1));
    e.SetEnd (GetCorner (edges[edge].v2));
  }

  /// Test if the given coordinate is in this box.
  bool In (float x, float y, float z) const
  {
    if (x < minbox.x || x > maxbox.x) return false;
    if (y < minbox.y || y > maxbox.y) return false;
    if (z < minbox.z || z > maxbox.z) return false;
    return true;
  }

  /// Test if the given coordinate is in this box.
  bool In (const csVector3& v) const
  {
    return In (v.x, v.y, v.z);
  }

  /// Test if this box overlaps with the given box.
  bool Overlap (const csBox3& box) const
  {
    if (maxbox.x < box.minbox.x || minbox.x > box.maxbox.x) return false;
    if (maxbox.y < box.minbox.y || minbox.y > box.maxbox.y) return false;
    if (maxbox.z < box.minbox.z || minbox.z > box.maxbox.z) return false;
    return true;
  }

  /// Test if this box contains the other box.
  bool Contains (const csBox3& box) const
  {
    return (box.minbox.x >= minbox.x && box.maxbox.x <= maxbox.x) &&
           (box.minbox.y >= minbox.y && box.maxbox.y <= maxbox.y) &&
           (box.minbox.z >= minbox.z && box.maxbox.z <= maxbox.z);
  }

  /// Test if this box is empty.
  bool Empty () const
  {
    if (minbox.x > maxbox.x) return true;
    if (minbox.y > maxbox.y) return true;
    if (minbox.z > maxbox.z) return true;
    return false;
  }

  /// Initialize this box to empty.
  void StartBoundingBox ()
  {
    minbox.x =  CS_BOUNDINGBOX_MAXVALUE;
    minbox.y =  CS_BOUNDINGBOX_MAXVALUE;
    minbox.z =  CS_BOUNDINGBOX_MAXVALUE;
    maxbox.x = -CS_BOUNDINGBOX_MAXVALUE;
    maxbox.y = -CS_BOUNDINGBOX_MAXVALUE;
    maxbox.z = -CS_BOUNDINGBOX_MAXVALUE;
  }

  /// Initialize this box to one vertex.
  void StartBoundingBox (const csVector3& v)
  {
    minbox = v; maxbox = v;
  }

  /// Add a new vertex and recalculate the bounding box.
  void AddBoundingVertex (float x, float y, float z)
  {
    if (x < minbox.x) minbox.x = x; if (x > maxbox.x) maxbox.x = x;
    if (y < minbox.y) minbox.y = y; if (y > maxbox.y) maxbox.y = y;
    if (z < minbox.z) minbox.z = z; if (z > maxbox.z) maxbox.z = z;
  }

  /// Add a new vertex and recalculate the bounding box.
  void AddBoundingVertex (const csVector3& v)
  {
    AddBoundingVertex (v.x, v.y, v.z);
  }

  /**
   * Add a new vertex and recalculate the bounding box.
   * This version is a little more optimal. It assumes however
   * that at least one point has been added to the bounding box.
   */
  void AddBoundingVertexSmart (float x, float y, float z)
  {
    if (x < minbox.x) minbox.x = x; else if (x > maxbox.x) maxbox.x = x;
    if (y < minbox.y) minbox.y = y; else if (y > maxbox.y) maxbox.y = y;
    if (z < minbox.z) minbox.z = z; else if (z > maxbox.z) maxbox.z = z;
  }

  /**
   * Add a new vertex and recalculate the bounding box.
   * This version is a little more optimal. It assumes however
   * that at least one point has been added to the bounding box.
   */
  void AddBoundingVertexSmart (const csVector3& v)
  {
    AddBoundingVertexSmart (v.x, v.y, v.z);
  }

  /**
   * Add a new vertex and recalculate the bounding box.
   * Returns true if box was modified.
   */
  bool AddBoundingVertexTest (float x, float y, float z)
  {
    bool rc = false;
    if (x < minbox.x) { minbox.x = x; rc = true; }
    if (x > maxbox.x) { maxbox.x = x; rc = true; }
    if (y < minbox.y) { minbox.y = y; rc = true; }
    if (y > maxbox.y) { maxbox.y = y; rc = true; }
    if (z < minbox.z) { minbox.z = z; rc = true; }
    if (z > maxbox.z) { maxbox.z = z; rc = true; }
    return rc;
  }

  /**
   * Add a new vertex and recalculate the bounding box.
   * Returns true if box was modified.
   */
  bool AddBoundingVertexTest (const csVector3& v)
  {
    return AddBoundingVertexTest (v.x, v.y, v.z);
  }

  /**
   * Add a new vertex and recalculate the bounding box.
   * This version is a little more optimal. It assumes however
   * that at least one point has been added to the bounding box.
   * Returns true if box was modified.
   */
  bool AddBoundingVertexSmartTest (float x, float y, float z)
  {
    bool rc = false;
    if (x < minbox.x) { minbox.x = x; rc = true; }
    else if (x > maxbox.x) { maxbox.x = x; rc = true; }
    if (y < minbox.y) { minbox.y = y; rc = true; }
    else if (y > maxbox.y) { maxbox.y = y; rc = true; }
    if (z < minbox.z) { minbox.z = z; rc = true; }
    else if (z > maxbox.z) { maxbox.z = z; rc = true; }
    return rc;
  }

  /**
   * Add a new vertex and recalculate the bounding box.
   * This version is a little more optimal. It assumes however
   * that at least one point has been added to the bounding box.
   * Returns true if box was modified.
   */
  bool AddBoundingVertexSmartTest (const csVector3& v)
  {
    return AddBoundingVertexSmartTest (v.x, v.y, v.z);
  }

  /// Initialize this box to empty.
  csBox3 () :
    minbox ( CS_BOUNDINGBOX_MAXVALUE,
             CS_BOUNDINGBOX_MAXVALUE,
	     CS_BOUNDINGBOX_MAXVALUE),
    maxbox (-CS_BOUNDINGBOX_MAXVALUE,
            -CS_BOUNDINGBOX_MAXVALUE,
	    -CS_BOUNDINGBOX_MAXVALUE) {}

  /// Initialize this box with one point.
  csBox3 (const csVector3& v) : minbox (v), maxbox (v) { }

  /// Initialize this box with two points.
  csBox3 (const csVector3& v1, const csVector3& v2) :
  	minbox (v1), maxbox (v2)
  { if (Empty ()) StartBoundingBox (); }

  /// Initialize this box with the given values.
  csBox3 (float x1, float y1, float z1, float x2, float y2, float z2) :
    minbox (x1, y1, z1), maxbox (x2, y2, z2)
  { if (Empty ()) StartBoundingBox (); }

  /// Sets the bounds of the box with the given values.
  void Set (const csVector3& bmin, const csVector3& bmax)
  {
    minbox = bmin;
    maxbox = bmax;
  }

  /// Sets the bounds of the box with the given values.
  void Set (float x1, float y1, float z1, float x2, float y2, float z2)
  {
    if (x1>x2 || y1>y2 || z1>z2) StartBoundingBox();
    else
    {
      minbox.x = x1; minbox.y = y1; minbox.z = z1;
      maxbox.x = x2; maxbox.y = y2; maxbox.z = z2;
    }
  }

  /// Set Min component for 0 (x), 1 (y), or 2 (z).
  void SetMin (int idx, float val)
  {
    if (idx == 1) minbox.y = val;
    else if (idx == 0) minbox.x = val;
    else minbox.z = val;
  }

  /// Set Max component for 0 (x), 1 (y), or 2 (z).
  void SetMax (int idx, float val)
  {
    if (idx == 1) maxbox.y = val;
    else if (idx == 0) maxbox.x = val;
    else maxbox.z = val;
  }

  /**
   * Return a textual representation of the box in the form
   * "(minx,miny,minz)-(maxx,maxy,maxz)".
   */
  csString Description() const;

  /**
   * Split this box along an axis and construct two new boxes.
   */
  void Split (int axis, float where, csBox3& bl, csBox3& br) const
  {
    switch (axis)
    {
      case CS_AXIS_X:
        bl.Set (minbox.x, minbox.y, minbox.z,
      	        where,    maxbox.y, maxbox.z);
        br.Set (where,    minbox.y, minbox.z,
      	        maxbox.x, maxbox.y, maxbox.z);
        break;
      case CS_AXIS_Y:
        bl.Set (minbox.x, minbox.y, minbox.z,
      	        maxbox.x, where,    maxbox.z);
        br.Set (minbox.x, where,    minbox.z,
      	        maxbox.x, maxbox.y, maxbox.z);
        break;
      case CS_AXIS_Z:
        bl.Set (minbox.x, minbox.y, minbox.z,
      	        maxbox.x, maxbox.y, where);
        br.Set (minbox.x, minbox.y, where,
      	        maxbox.x, maxbox.y, maxbox.z);
        break;
    }
  }

  /**
   * Test if this box intersects with the given axis aligned plane.
   * Returns < 0 if box is completely in left half.
   * Returns > 0 if box is completely in right half.
   * Returns 0 if box is intersected.
   */
  int TestSplit (int axis, float where) const
  {
    if (maxbox[axis] < where) return -1;
    if (minbox[axis] > where) return 1;
    return 0;
  }

  /**
   * Test if this box is adjacent to the other on the X side.
   */
  bool AdjacentX (const csBox3& other, float epsilon = SMALL_EPSILON) const;

  /**
   * Test if this box is adjacent to the other on the Y side.
   */
  bool AdjacentY (const csBox3& other, float epsilon = SMALL_EPSILON) const;

  /**
   * Test if this box is adjacent to the other on the Z side.
   */
  bool AdjacentZ (const csBox3& other, float epsilon = SMALL_EPSILON) const;

  /**
   * Test if this box is adjacent to the other one.
   * Return -1 if not adjacent or else any of the #CS_BOX_SIDE_x etc.
   * flags to indicate the side of this box that the other
   * box is adjacent with.
   * The epsilon value is used to decide when adjacency is ok.
   */
  int Adjacent (const csBox3& other, float epsilon = SMALL_EPSILON) const;

  /**
   * Assume that 3D space is divided into 27 areas. One is inside
   * the box. The other 26 are rectangular segments around the box.
   * This function will calculate the right segment for a given point
   * and return that.
   */
  int CalculatePointSegment (const csVector3& pos) const;

  /**
   * Get a convex outline (not a polygon unless projected to 2D)
   * for for this box as seen from the given position.
   * The coordinates returned are world space coordinates.
   * Note that you need place for at least six vectors in the array.
   * If you set bVisible true, you will get all visible corners - this
   * could be up to 7.
   */
  void GetConvexOutline (const csVector3& pos,
  	csVector3* array, int& num_array, bool bVisible=false) const;

  /**
   * Test if this box is between two others.
   */
  bool Between (const csBox3& box1, const csBox3& box2) const;

  /**
   * Calculate the minimum manhattan distance between this box
   * and another one.
   */
  void ManhattanDistance (const csBox3& other, csVector3& dist) const;

  /**
   * Calculate the squared distance between (0,0,0) and the box
   * This routine is extremely efficient.
   */
  float SquaredOriginDist () const;

  /**
   * Calculate the squared distance between (0,0,0) and the point
   * on the box which is furthest away from (0,0,0).
   * This routine is extremely efficient.
   */
  float SquaredOriginMaxDist () const;

  /**
   * Project this box to a 2D bounding box given the view point
   * transformation and also the field-of-view and shift values (for
   * perspective projection). The transform should transform from world
   * to camera space (using Other2This). The minimum and maximum z
   * are also calculated. If the bounding box is behind the camera
   * then the 'sbox' will not be calculated (min_z and max_z are
   * still calculated) and the function will return false.
   * If the camera is inside the transformed box then this function will
   * return true and a conservative screen space bounding box is returned.
   */
  bool ProjectBox (const csTransform& trans, float fov, float sx, float sy,
  	csBox2& sbox, float& min_z, float& max_z) const;

  /**
   * Project this box to the 2D outline given the view point
   * transformation and also the field-of-view and shift values (for
   * perspective correction). The minimum and maximum z are also
   * calculated. If the box is fully behind the camera
   * then false is returned and this function will not do anything.
   * If the box is partially behind the camera you will get an outline
   * that is conservatively correct (i.e. it will overestimate the box).
   */
  bool ProjectOutline (const csTransform& trans, float fov, float sx, float sy,
  	csPoly2D& poly, float& min_z, float& max_z) const;

  /**
   * Project this box to the 2D outline given the origin and an axis aligned
   * plane. If this fails (because some of the points cannot be projected)
   * then it will return false. Note that this function will NOT clear
   * the input array. So it will add the projected vertices after the
   * vertices that may already be there.
   */
  bool ProjectOutline (const csVector3& origin,
	int axis, float where, csArray<csVector2>& poly) const;

  /**
   * Project this box to the 2D outline given the origin and an axis aligned
   * plane. If this fails (because some of the points cannot be projected)
   * then it will return false.
   */
  bool ProjectOutline (const csVector3& origin,
	int axis, float where, csPoly2D& poly) const;

  /**
   * Project this box to the 2D outline given the view point
   * transformation and also the field-of-view and shift values (for
   * perspective correction). The minimum and maximum z are also
   * calculated. If the box is fully behind the camera
   * then false is returned and this function will not do anything.
   * If the box is partially behind the camera you will get an outline
   * that is conservatively correct (i.e. it will overestimate the box).
   * In addition to the outline this function also returns the projected
   * screen-space box. So it is a combination of ProjectBox() and
   * ProjectOutline().
   */
  bool ProjectBoxAndOutline (const csTransform& trans, float fov,
  	float sx, float sy, csBox2& sbox, csPoly2D& poly,
	float& min_z, float& max_z) const;

  /// Compute the union of two bounding boxes.
  csBox3& operator+= (const csBox3& box);
  /// Compute the union of a point with this bounding box.
  csBox3& operator+= (const csVector3& point);
  /// Compute the intersection of two bounding boxes.
  csBox3& operator*= (const csBox3& box);
  /// Test if the two boxes have an intersection.
  bool TestIntersect (const csBox3& box) const;

  /// Compute the union of two bounding boxes.
  friend CS_CRYSTALSPACE_EXPORT csBox3 operator+ (const csBox3& box1, 
    const csBox3& box2);
  /// Compute the union of a bounding box and a point.
  friend CS_CRYSTALSPACE_EXPORT csBox3 operator+ (const csBox3& box, 
    const csVector3& point);
  /// Compute the intersection of two bounding boxes.
  friend CS_CRYSTALSPACE_EXPORT csBox3 operator* (const csBox3& box1, 
    const csBox3& box2);

  /// Tests if two bounding boxes are equal.
  friend CS_CRYSTALSPACE_EXPORT bool operator== (const csBox3& box1, 
    const csBox3& box2);
  /// Tests if two bounding boxes are unequal.
  friend CS_CRYSTALSPACE_EXPORT bool operator!= (const csBox3& box1, 
    const csBox3& box2);
  /// Tests if box1 is a subset of box2.
  friend CS_CRYSTALSPACE_EXPORT bool operator< (const csBox3& box1, 
    const csBox3& box2);
  /// Tests if box1 is a superset of box2.
  friend CS_CRYSTALSPACE_EXPORT bool operator> (const csBox3& box1, 
    const csBox3& box2);
  /// Tests if a point is contained in a box.
  friend CS_CRYSTALSPACE_EXPORT bool operator< (const csVector3& point, 
    const csBox3& box);
};

/** @} */

#endif // __CS_BOX_H__
