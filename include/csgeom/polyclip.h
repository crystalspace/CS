/*
    Crystal Space polygon clipping routines
    Copyright (C) 1998 by Jorrit Tyberghein
    Contributed by Ivan Avramovic <ivan@avramovic.com> and
                   Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __POLYCLIP_H__
#define __POLYCLIP_H__

#include "csgeom/math2d.h"
#include "csgeom/polypool.h"

class Dumper;

/**
 * The csClipper class is an abstract parent to all 2D clipping objects.
 */
class csClipper
{
protected:
  /// This variable holds a pool for 2D polygons as used by the clipper.
  static csPoly2DPool polypool;

public:
  /**
   * Clip a set of 2D points and return in 'dest_poly'.
   * 'dest_poly' must be big enough to hold the clipped polygon.
   * Return false if polygon is not visible (clipped away).
   */
  virtual bool Clip (csVector2 *Polygon, csVector2* dest_poly, int Count,
  	int &OutCount) = 0;

  /**
   * Clip a set of 2D points and return them in the same array.
   * On input MaxCount contains number of elements that Output can hold.
   * On output Count is set to number of vertices in output polygon.
   */
  virtual bool Clip (csVector2 *Polygon, int &Count, int MaxCount, 
                     csBox *BoundingBox) = 0;

  /**
   * Classify some bounding box against this clipper.
   * This function returns:<p>
   * <ul>
   * <li> -1 if box is not visible.
   * <li> 0 if box is partially visible.
   * <li> 1 if box is entirely visible.
   */
  virtual int ClassifyBox (csBox* box) = 0;

  /// Return true if given point is inside (or on bound) of clipper polygon.
  virtual bool IsInside (float x, float y) = 0;

  /// Return number of vertices for this clipper polygon.
  virtual int GetNumVertices () = 0;

  /// Return vertex at index for this clipper polygon.
  virtual const csVector2 GetVertex (int i) = 0;

  /// Return a pointer to the array of csVector2's
  virtual csVector2 *GetClipPoly () = 0;
};

/**
 * The csBoxClipper class is able to clip convex polygons to a rectangle
 * (such as the screen).
 */
class csBoxClipper : public csClipper
{
  ///
  csBox region;
  ///
  csVector2 ClipBox [4];

  ///
  inline void InitClipBox ()
  {
    ClipBox [0].Set (region.MinX (), region.MinY ());
    ClipBox [1].Set (region.MinX (), region.MaxY ());
    ClipBox [2].Set (region.MaxX (), region.MaxY ());
    ClipBox [3].Set (region.MaxX (), region.MinY ());
  }

public:
  /// Initializes the clipper object to the given bounding region.
  csBoxClipper (const csBox& b) : region (b)
  { InitClipBox (); }
  /// Initializes the clipper object to a rectangle with the given coords.
  csBoxClipper (float x1, float y1, float x2, float y2) : region(x1,y1,x2,y2)
  { InitClipBox (); }

  /// Clip a to dest_poly.
  virtual bool Clip (csVector2 *Polygon, csVector2* dest_poly, int Count,
  	int &OutCount);

  /**
   * Clip a set of 2D points and return them in the same array.
   * On input MaxCount contains number of elements that Output can hold.
   * On output Count is set to number of vertices in output polygon
   */
  virtual bool Clip (csVector2 *Polygon, int &Count, int MaxCount, 
                     csBox *BoundingBox);

  /// Classify some bounding box against this clipper.
  virtual int ClassifyBox (csBox* box);

  /// Return true if given point is inside (or on bound) of clipper polygon.
  virtual bool IsInside (float x, float y) { return region.In (x,y); }

  /// Return number of vertices for this clipper polygon.
  virtual int GetNumVertices () { return 4; }

  /// Return vertex at index for this clipper polygon.
  virtual const csVector2 GetVertex (int i)
  {
    switch (i)
    {
      case 0: return csVector2 (region.MinX (), region.MinY ());
      case 1: return csVector2 (region.MaxX (), region.MinY ());
      case 2: return csVector2 (region.MaxX (), region.MaxY ());
      default: return csVector2 (region.MinX (), region.MaxY ());
    }
  }

  /// Return a pointer to the array of csVector2's
  virtual csVector2 *GetClipPoly ()
  { return ClipBox; }
};

/**
 * The csPolygonClipper class can be used for clipping any convex polygon
 * with any other polygon. The clipper object should be used, if possible,
 * for many polygons (for example, a 3D sprite can initialize a clipper
 * object then clip all of its triangle against it at once) as the
 * initialization of clipper polygon involves some (although not too
 * expensive) calculations.
 * Both clipped and clipping polygons *should* be convex as the result
 * of intersection of two non-convex polygons can result in more than
 * one resulting polygon, and this class does not handle that.
 */
class csPolygonClipper : public csClipper
{
  friend class Dumper;

  /// Private structure for keeping pre-calculated some data
  struct SegData
  {
    float dx, dy;
  };

  /// Equation for all edges of clipping polygon
  SegData *ClipData;
  /// Clipper polygon itself
  csVector2 *ClipPoly;
  /// A pointer to the pooled polygon (so that we can free it later).
  csPoly2D* ClipPoly2D;
  /// Number of vertices in clipper polygon
  int ClipPolyVertices;
  /// Clipping polygon bounding box
  csBox ClipBox;

  /// Prepare clipping line equations
  void Prepare ();

public:
  /// Create a polygon clipper object from a 2D polygon.
  csPolygonClipper (csPoly2D *Clipper, bool mirror = false,
    bool copy = false);
  /// Create a polygon clipper object from a set of 2D vectors.
  csPolygonClipper (csVector2 *Clipper, int Count, bool mirror = false,
    bool copy = false);
  /// Destroy the polygon clipper object.
  virtual ~csPolygonClipper ();

  /// Clip to dest_poly.
  virtual bool Clip (csVector2 *Polygon, csVector2* dest_poly, int Count,
  	int &OutCount);

  /**
   * Clip a set of 2D points and return them in the same array.
   * On input MaxCount contains number of elements that Output can hold.
   * On output Count is set to number of vertices in output polygon.
   */
  virtual bool Clip (csVector2 *Polygon, int &Count, int MaxCount, 
                     csBox *BoundingBox);

  /// Classify some bounding box against this clipper.
  virtual int ClassifyBox (csBox* box);

  /// Return true if given point is inside (or on bound) of clipper polygon.
  virtual bool IsInside (float x, float y);

  /// Return number of vertices for this clipper polygon.
  virtual int GetNumVertices () { return ClipPolyVertices; }

  /// Return vertex at index for this clipper polygon.
  virtual const csVector2 GetVertex (int i) { return ClipPoly[i]; }

  /// Return a pointer to the array of csVector2's
  virtual csVector2 *GetClipPoly ()
  { return ClipPoly; }
};

#endif // __POLYCLIP_H__
