/*
    Copyright (C) 1998 by Jorrit Tyberghein
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

#ifndef MATH2D_H
#define MATH2D_H

#define CS_POLY_IN 1
#define CS_POLY_ON 0
#define CS_POLY_OUT -1

#include "types.h"	// for bool

class csVector2;
class csBox;

/**
 * A 2D vector.
 */
class csVector2
{
public:
  ///
  float x;
  ///
  float y;

  /// Make a new vector. No initialization is done.
  csVector2 () {}

  /// Make a new vector and initialize with the given values.
  csVector2 (float x, float y) { csVector2::x = x; csVector2::y = y; }

  /// Set vector to given values.
  inline void Set (float ix, float iy)
  { x = ix; y = iy; }

  /// Return the norm (magnitude) of a 2D vector.
  static float Norm (const csVector2& v);

  /// Return the norm (magnitude) of this vector.
  float Norm () const;

  /// Rotate vector around the origin by a given angle in radians.
  void Rotate (float angle);

  /// Add another vector to this vector.
  csVector2& operator+= (const csVector2& v) 
  { x += v.x;  y += v.y;  return *this; }

  /// Subtract another vector from this vector.
  csVector2& operator-= (const csVector2& v)
  { x -= v.x;  y -= v.y;  return *this; }

  /// Multiply this vector by a scalar.
  csVector2& operator*= (float f) { x *= f;  y *= f;  return *this; }

  /// Divide this vector by a scalar.
  csVector2& operator/= (float f) { x /= f;  y /= f;  return *this; }

  /// Unary + operator.
  inline csVector2 operator+ () const { return *this; }

  /// Unary - operator.
  inline csVector2 operator- () const { return csVector2(-x,-y); }

  /// Add two vectors.
  friend csVector2 operator+ (const csVector2& v1, const csVector2& v2);
  /// Subtract two vectors.
  friend csVector2 operator- (const csVector2& v1, const csVector2& v2);
  /// Take the dot product of two vectors.
  friend float operator* (const csVector2& v1, const csVector2& v2);
  /// Multiply a vector and a scalar.
  friend csVector2 operator* (const csVector2& v, float f);
  /// Multiply a vector and a scalar.
  friend csVector2 operator* (float f, const csVector2& v);
  /// Divide a vector by a scalar.
  friend csVector2 operator/ (const csVector2& v, float f);
  /// Check if two vectors are equal.
  friend bool operator== (const csVector2& v1, const csVector2& v2);
  /// Check if two vectors are not equal.
  friend bool operator!= (const csVector2& v1, const csVector2& v2);
};

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
  /// The top-left x coordinate of the bounding box.
  float minx;
  /// The top-left y coordinate of the bounding box.
  float miny;
  /// The bottom-right x coordinate of the bounding box.
  float maxx;
  /// The bottom-right y coordinate of the bounding box.
  float maxy;

public:
  /// Initialize this box to empty.
  csBox () : minx( CS_BOUNDINGBOX_MAXVALUE), miny( CS_BOUNDINGBOX_MAXVALUE),
	     maxx(-CS_BOUNDINGBOX_MAXVALUE), maxy(-CS_BOUNDINGBOX_MAXVALUE) {}

  /// Initialize this box with one point.
  csBox (const csVector2& v) : minx(v.x), miny(v.y), maxx(v.x), maxy(v.y) {}

  /// Initialize this box with the given values.
  csBox (float x1, float y1, float x2, float y2) :
    minx(x1), miny(y1), maxx(x2), maxy(y2)
  { if (Empty ()) StartBoundingBox (); }

  /// Sets the bounds of the box with the given values.
  void Set (float x1, float y1, float x2, float y2)
  {
    if (x1>x2 || y1>y2) StartBoundingBox();
    else { minx = x1;  miny = y1;  maxx = x2;  maxy = y2; }
  }

  ///
  float MinX () const { return minx; }
  ///
  float MinY () const { return miny; }
  ///
  float MaxX () const { return maxx; }
  ///
  float MaxY () const { return maxy; }

  /// Test if the given coordinate is in this box.
  bool In (float x, float y) const
  {
    if (x < minx || x > maxx) return false;
    if (y < miny || y > maxy) return false;
    return true;
  }

  /// Test if this box overlaps with the given box.
  bool Overlap (const csBox& box) const
  {
    if (maxx < box.minx || minx > box.maxx) return false;
    if (maxy < box.miny || miny > box.maxy) return false;
    return true;
  }

  /// Test if this box is empty.
  bool Empty () const
  {
    if (minx > maxx) return true;
    if (miny > maxy) return true;
    return false;
  }

  /// Initialize this box to empty.
  void StartBoundingBox ()
  {
    minx =  CS_BOUNDINGBOX_MAXVALUE;  miny =  CS_BOUNDINGBOX_MAXVALUE;
    maxx = -CS_BOUNDINGBOX_MAXVALUE;  maxy = -CS_BOUNDINGBOX_MAXVALUE;
  }

  /// Initialize this box to one vertex.
  void StartBoundingBox (csVector2& v)
  {
    minx = v.x;  miny = v.y;
    maxx = v.x;  maxy = v.y;
  }

  /// Add a new vertex and recalculate the bounding box.
  void AddBoundingVertex (float x, float y)
  {
    if (x < minx) minx = x;  if (x > maxx) maxx = x;
    if (y < miny) miny = y;  if (y > maxy) maxy = y;
  }

  /**
   * Add a new vertex and recalculate the bounding box.
   * This version is a little more optimal. It assumes however
   * that at least one point has been added to the bounding box.
   */
  void AddBoundingVertexSmart (float x, float y)
  {
    if (x < minx) minx = x; else if (x > maxx) maxx = x;
    if (y < miny) miny = y; else if (y > maxy) maxy = y;
  }

  /// Add a new vertex and recalculate the bounding box.
  void AddBoundingVertex (const csVector2& v)
  {
    if (v.x < minx) minx = v.x;  if (v.x > maxx) maxx = v.x;
    if (v.y < miny) miny = v.y;  if (v.y > maxy) maxy = v.y;
  }

  /// Add a new vertex and recalculate the bounding box.
  void AddBoundingVertexSmart (const csVector2& v)
  {
    if (v.x < minx) minx = v.x; else if (v.x > maxx) maxx = v.x;
    if (v.y < miny) miny = v.y; else if (v.y > maxy) maxy = v.y;
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
 * Various functions in 2D, such as 2D vector functions.
 * This is a static class and contains only static member functions.
 */
class csMath2
{
public:
  /**
   * Calculates which side of a line a given point is on.
   * Returns -1 if point v is left of line segment 's1-s2',
   *          1 if point v is right of segment 's1-s2'
   *       or 0 if point v lies on segment 's1-s2'.
   */
  static int WhichSide2D (const csVector2& v, 
                          const csVector2& s1, const csVector2& s2);

  /**
   * Calculates whether a vector lies inside a given 2D polygon.
   * Return CS_POLY_IN, CS_POLY_OUT, or CS_POLY_ON for this vector with
   * respect to the given polygon. The polygon is given as an array of 2D
   * vectors with a bounding box.
   * WARNING: does no safety checking for P or bounding_box.
   */
  static int InPoly2D (const csVector2& v, 
                       csVector2* P, int n, csBox* bounding_box);

  /**
   * Calculates 2 x the area of a given triangle.
   * Returns twice the signed area of the triangle determined by a,b,c,
   * positive if a,b,c are oriented ccw, and negative if cw.
   */
  static float Area2 (float ax, float ay,
                      float bx, float by, 
                      float cx, float cy);

  /**
   * Calculates whether a point lies to the right of a given line.
   * Returns true iff c is strictly to the right of the directed
   * line through a to b.
   */
  static bool Right (float ax, float ay,
                     float bx, float by,
                     float cx, float cy);

  /**
   * Calculates whether a point lies to the left of a given line.
   * Returns true iff c is strictly to the left of the directed
   * line through a to b.
   */
  static bool Left (float ax, float ay,
                    float bx, float by,
                    float cx, float cy);

};

/** 
 * Some functions to perform various intersection calculations with 2D
 * line segments.  This is a static class and contains only static member
 * functions.
 */
class csIntersect2
{
public:
  /**
   * Compute the intersection of the 2D segments.  Return true if they
   * intersect, with the intersection point returned in isect,  and the
   * distance from a1 of the intersection in dist.
   */
  static bool Segments (
    const csVector2& a1, const csVector2& a2, // first segment
    const csVector2& b1, const csVector2& b2, // second segment
    csVector2& isect, float& dist);         // intersection point and distance

  /**
   * Compute the intersection of a 2D segment and a line.  Return true if they
   * intersect, with the intersection point returned in isect,  and the
   * distance from a1 of the intersection in dist.
   */
  static bool SegmentLine (
    const csVector2& a1, const csVector2& a2, // first segment
    const csVector2& b1, const csVector2& b2, // second line
    csVector2& isect, float& dist);         // intersection point and distance

  /**
   * Compute the intersection of 2D lines.  Return true if they
   * intersect, with the intersection point returned in isect.
   */
  static bool Lines (
    const csVector2& a1, const csVector2& a2, // first line
    const csVector2& b1, const csVector2& b2, // second line
    csVector2& isect);                      // intersection point
};

#endif /*MATH_H*/
