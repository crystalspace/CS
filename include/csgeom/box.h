/*
    Copyright (C) 1998,1999 by Jorrit Tyberghein
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

#ifndef __CS_BOX_H
#define __CS_BOX_H

#include "types.h"	// for bool
#include "math2d.h"
#include "math3d.h"

/**
 * The maximum value that a coordinate in the bounding box can use.
 * This is considered the 'infinity' value used for empty bounding boxes.
 */
#define CS_BOUNDINGBOX_MAXVALUE 1000000000.

/**
 * A bounding box in 2D space.
 * In order to operate correctly, this bounding box assumes that all values
 * entered or compared against lie within the range 
 * (-CS_BOUNDINGBOX_MAXVALUE, CS_BOUNDINGBOX_MAXVALUE).  It is not
 * recommended to use points outside of this range.
 */
class csBox
{
protected:
  /// The top-left coordinate of the bounding box.
  csVector2 minbox;
  /// The bottom-right coordinate of the bounding box.
  csVector2 maxbox;

public:
  ///
  float MinX () const { return minbox.x; }
  ///
  float MinY () const { return minbox.y; }
  ///
  float MaxX () const { return maxbox.x; }
  ///
  float MaxY () const { return maxbox.y; }
  ///
  const csVector2& Min () const { return minbox; }
  ///
  const csVector2& Max () const { return maxbox; }

  /**
   * Return every corner of this bounding box from 0
   * to 3. This contrasts with Min() and Max() because
   * those are only the min and max corners.
   */
  csVector2 GetCorner (int corner) const;

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
  bool Intersect (csVector2* poly, int num_poly)
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
  bool Overlap (const csBox& box) const
  {
    if (maxbox.x < box.minbox.x || minbox.x > box.maxbox.x) return false;
    if (maxbox.y < box.minbox.y || minbox.y > box.maxbox.y) return false;
    return true;
  }

  /// Test if this box is empty.
  bool Empty () const
  {
    if (minbox.x > maxbox.x) return true;
    if (minbox.y > maxbox.y) return true;
    return false;
  }

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

  //-----
  // Maintenance Note: The csBox constructors and Set() appear at this point
  // in the file, rather than earlier, in order to appease the OpenStep 4.2
  // compiler.  Specifically, the problem is that the compiler botches code
  // generation if an unseen method (which is later declared inline) is
  // called from within another inline method.  For instance, if the
  // constructors were listed at the top of the file, rather than here, the
  // compiler would see calls to Empty() and StartBoundingBox() before seeing
  // declarations for them.  In such a situation, the buggy compiler
  // generates a broken object file.  The simple work-around of textually
  // reorganizing the file ensures that the declarations for Empty() and
  // StartBoundingBox() are seen before they are called.
  //-----

  /// Initialize this box to empty.
  csBox () : minbox (CS_BOUNDINGBOX_MAXVALUE, CS_BOUNDINGBOX_MAXVALUE),
	     maxbox (-CS_BOUNDINGBOX_MAXVALUE, -CS_BOUNDINGBOX_MAXVALUE) {}

  /// Initialize this box with one point.
  csBox (const csVector2& v) : minbox (v.x, v.y), maxbox (v.x, v.y) {}

  /// Initialize this box with the given values.
  csBox (float x1, float y1, float x2, float y2) :
    minbox (x1, y1), maxbox (x2, y2)
  { if (Empty ()) StartBoundingBox (); }

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

  /// Compute the union of two bounding boxes.
  csBox& operator+= (const csBox& box);
  /// Compute the union of a point with this bounding box.
  csBox& operator+= (const csVector2& point);
  /// Compute the intersection of two bounding boxes.
  csBox& operator*= (const csBox& box);

  /// Compute the union of two bounding boxes.
  friend csBox operator+ (const csBox& box1, const csBox& box2);
  /// Compute the union of a bounding box and a point.
  friend csBox operator+ (const csBox& box, const csVector2& point);
  /// Compute the intersection of two bounding boxes.
  friend csBox operator* (const csBox& box1, const csBox& box2);

  /// Tests if two bounding boxes are equal.
  friend bool operator== (const csBox& box1, const csBox& box2);
  /// Tests if two bounding boxes are unequal.
  friend bool operator!= (const csBox& box1, const csBox& box2);
  /// Tests if box1 is a subset of box2.
  friend bool operator< (const csBox& box1, const csBox& box2);
  /// Tests if box1 is a superset of box2.
  friend bool operator> (const csBox& box1, const csBox& box2);
  /// Tests if a point is contained in a box.
  friend bool operator< (const csVector2& point, const csBox& box);
};

/**
 * A bounding box in 3D space.
 * In order to operate correctly, this bounding box assumes that all values
 * entered or compared against lie within the range 
 * (-CS_BOUNDINGBOX_MAXVALUE, CS_BOUNDINGBOX_MAXVALUE).  It is not
 * recommended to use points outside of this range.
 */
class csBox3
{
protected:
  /// The top-left of this bounding box.
  csVector3 minbox;
  /// The bottom-right.
  csVector3 maxbox;

public:
  ///
  float MinX () const { return minbox.x; }
  ///
  float MinY () const { return minbox.y; }
  ///
  float MinZ () const { return minbox.z; }
  ///
  float MaxX () const { return maxbox.x; }
  ///
  float MaxY () const { return maxbox.y; }
  ///
  float MaxZ () const { return maxbox.z; }
  ///
  const csVector3& Min () const { return minbox; }
  ///
  const csVector3& Max () const { return maxbox; }

  /**
   * Return every corner of this bounding box from 0
   * to 7. This contrasts with Min() and Max() because
   * those are only the min and max corners.
   */
  csVector3 GetCorner (int corner) const;

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

  //-----
  // Maintenance Note: The csBox3 constructors and Set() appear at this point
  // in the file, rather than earlier, in order to appease the OpenStep 4.2
  // compiler.  Specifically, the problem is that the compiler botches code
  // generation if an unseen method (which is later declared inline) is
  // called from within another inline method.  For instance, if the
  // constructors were listed at the top of the file, rather than here, the
  // compiler would see calls to Empty() and StartBoundingBox() before seeing
  // declarations for them.  In such a situation, the buggy compiler
  // generated a broken object file.  The simple work-around of textually
  // reorganizing the file ensures that the declarations for Empty() and
  // StartBoundingBox() are seen before they are called.
  //-----

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

  /// Compute the union of two bounding boxes.
  csBox3& operator+= (const csBox3& box);
  /// Compute the union of a point with this bounding box.
  csBox3& operator+= (const csVector3& point);
  /// Compute the intersection of two bounding boxes.
  csBox3& operator*= (const csBox3& box);

  /// Compute the union of two bounding boxes.
  friend csBox3 operator+ (const csBox3& box1, const csBox3& box2);
  /// Compute the union of a bounding box and a point.
  friend csBox3 operator+ (const csBox3& box, const csVector3& point);
  /// Compute the intersection of two bounding boxes.
  friend csBox3 operator* (const csBox3& box1, const csBox3& box2);

  /// Tests if two bounding boxes are equal.
  friend bool operator== (const csBox3& box1, const csBox3& box2);
  /// Tests if two bounding boxes are unequal.
  friend bool operator!= (const csBox3& box1, const csBox3& box2);
  /// Tests if box1 is a subset of box2.
  friend bool operator< (const csBox3& box1, const csBox3& box2);
  /// Tests if box1 is a superset of box2.
  friend bool operator> (const csBox3& box1, const csBox3& box2);
  /// Tests if a point is contained in a box.
  friend bool operator< (const csVector3& point, const csBox3& box);
};

#endif // __CS_BOX_H
