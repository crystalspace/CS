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

/// Maximal number of vertices in output (clipped) polygons
#define MAX_OUTPUT_VERTICES	64

/**
 * Clipper return codes.<p>
 * The clipper routines return one of the values below so that we can
 * distinguish between the cases when input polygon is completely outside
 * the clipping polygon (thus it is not visible), completely inside the
 * clipping polygon (thus it has not changed) and partially outside,
 * partially inside (thus it was clipped).
 */

/// The input polygon is completely outside of clipper polygon
#define CS_CLIP_OUTSIDE		0
/// The input polygon is completely inside (thus has not changed)
#define CS_CLIP_INSIDE		1
/// The input polygon was partially inside, partially outside
#define CS_CLIP_CLIPPED		2

/**
 * The clipper can output additional information about each vertex in
 * output polygon. This is used for generating U/V/Z values and so on,
 * if they are needed.
 */
struct csVertexStatus
{
  /// The type of vertex: unmodified original, on the edge of original, other
  unsigned char Type;
  /// Original vertex number (for CS_VERTEX_ORIGINAL and CS_VERTEX_ONEDGE)
  unsigned char Vertex;
  /// Additional information for CS_VERTEX_ONEDGE (0..1, the 't' parameter)
  float Pos;
};

/**
 * The following are possible values for csVertexStatus.Type field.
 * The csVertexStatus is used by csClipper:Clip() routine which is
 * able to output a status structure corresponding to each output vertex.
 * This information is usually used to clip other information associated
 * with polygon vertices (i.e. z, u, v and such).
 */
/// The output vertex is one of the input vertices
#define CS_VERTEX_ORIGINAL	0
/// The output vertex is located on one of the edges of the original polygon
#define CS_VERTEX_ONEDGE	1
/// The output vertex is located somewhere inside the original polygon
#define CS_VERTEX_INSIDE	2

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
   * Clip a set of 2D points and return in 'OutPolygon' which is expected
   * to contain space at least for MAX_OUTPUT_VERTICES elements.
   * Returns one of CS_CLIP_XXX values defined above.
   */
  virtual UByte Clip (csVector2 *InPolygon, int InCount,
    csVector2 *OutPolygon, int &OutCount) = 0;

  /**
   * Clip a set of 2D points.
   * On output Count is set to number of vertices in output polygon.
   * The output array is expected to contain space for at least
   * MAX_OUTPUT_VERTICES elements. The bounding box is set to the
   * minimal rectangle that contains the output polygon.
   * Returns one of CS_CLIP_XXX values defined above.
   */
  virtual UByte Clip (csVector2 *InPolygon, int InCount,
    csVector2 *OutPolygon, int &OutCount, csBox &BoundingBox) = 0;

  /**
   * Same as above but provides additional information on each output
   * vertex. The information type can be: vertex is one of original vertices,
   * vertex is on the edge of the original polygon and vertex is arbitrary
   * located inside the original polygon. Both OutPolygon and OutStatus
   * arrays are expected to have enough storage for at least
   * MAX_OUTPUT_VERTICES elements.
   */
  virtual UByte Clip (csVector2 *InPolygon, int InCount,
    csVector2 *OutPolygon, int &OutCount, csVertexStatus *OutStatus) = 0;

  /**
   * Classify some bounding box against this clipper.
   * This function returns:<p>
   * <ul>
   * <li> -1 if box is not visible.
   * <li> 0 if box is partially visible.
   * <li> 1 if box is entirely visible.
   */
  virtual int ClassifyBox (csBox &box) = 0;

  /// Return true if given point is inside (or on bound) of clipper polygon.
  virtual bool IsInside (float x, float y) = 0;

  /// Same as above but takes a csVector2
  inline bool IsInside (const csVector2 &v)
  { return IsInside (v.x, v.y); }

  /// Return number of vertices for this clipper polygon.
  virtual int GetNumVertices () = 0;

  /// Return a pointer to the array of csVector2's
  virtual csVector2 *GetClipPoly () = 0;

  /// Wrapper function: clip a polygon in-place.
  UByte Clip (csVector2 *InPolygon, int &InOutCount, csBox &BoundingBox);
};

/**
 * The csBoxClipper class is able to clip convex polygons to a rectangle
 * (such as the screen).
 */
class csBoxClipper : public csClipper
{
  /// The clipping region
  csBox region;
  /// The vertices of clipping region (for GetClipPoly ())
  csVector2 ClipBox [4];

  /// Helper function for constructors
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
  csBoxClipper (float x1, float y1, float x2, float y2) : region (x1,y1,x2,y2)
  { InitClipBox (); }

  /// Simple clipping
  virtual UByte Clip (csVector2 *InPolygon, int InCount,
    csVector2 *OutPolygon, int &OutCount);

  /// Clip and compute the bounding box
  virtual UByte Clip (csVector2 *InPolygon, int InCount,
    csVector2 *OutPolygon, int &OutCount, csBox &BoundingBox);

  /// Clip and return additional information about each vertex
  virtual UByte Clip (csVector2 *InPolygon, int InCount,
    csVector2 *OutPolygon, int &OutCount, csVertexStatus *OutStatus);

  /// Classify some bounding box against this clipper.
  virtual int ClassifyBox (csBox &box);

  /// Return true if given point is inside (or on bound) of clipper polygon.
  virtual bool IsInside (float x, float y)
  { return region.In (x,y); }

  /// Return number of vertices for this clipper polygon.
  virtual int GetNumVertices ()
  { return 4; }

  /// Return a pointer to the array of csVector2's
  virtual csVector2 *GetClipPoly ()
  { return ClipBox; }
};

/**
 * The csPolygonClipper class can be used for clipping any polygon against
 * any other convex polygon. The clipper object should be used, if possible,
 * for many polygons (for example, a 3D sprite can initialize a clipper
 * object then clip all of its triangle against it at once) as the
 * initialization of clipper polygon involves some (although not too
 * expensive) calculations.
 * The clipping polygon *should* be convex since the routine does not
 * expect any line to intersect the edge of clipping polygon more than twice.
 */
class csPolygonClipper : public csClipper
{
  friend class Dumper;

  /// Equation for all edges of clipping polygon
  csVector2 *ClipData;
  /// Clipper polygon itself
  csVector2 *ClipPoly;
  /// A pointer to the pooled polygon (so that we can free it later).
  csPoly2D *ClipPoly2D;
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

  /// Simple clipping
  virtual UByte Clip (csVector2 *InPolygon, int InCount,
    csVector2 *OutPolygon, int &OutCount);

  /// Clip and compute the bounding box
  virtual UByte Clip (csVector2 *InPolygon, int InCount,
    csVector2 *OutPolygon, int &OutCount, csBox &BoundingBox);

  /// Clip and return additional information about each vertex
  virtual UByte Clip (csVector2 *InPolygon, int InCount,
    csVector2 *OutPolygon, int &OutCount, csVertexStatus *OutStatus);

  /// Classify some bounding box against this clipper.
  virtual int ClassifyBox (csBox &box);

  /// Return true if given point is inside (or on bound) of clipper polygon.
  virtual bool IsInside (float x, float y);

  /// Return number of vertices for this clipper polygon.
  virtual int GetNumVertices () { return ClipPolyVertices; }

  /// Return a pointer to the array of csVector2's
  virtual csVector2 *GetClipPoly ()
  { return ClipPoly; }
};

#endif // __POLYCLIP_H__
