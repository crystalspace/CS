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

#ifndef __CS_POLYCLIP_H__
#define __CS_POLYCLIP_H__

/**\file 
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"

#include "csgeom/math2d.h"
#include "csgeom/polypool.h"
#include "igeom/clip2d.h"

/**
 * The csClipper class is an abstract parent to all 2D clipping objects.
 */
class CS_CRYSTALSPACE_EXPORT csClipper : public iClipper2D
{
protected:
  /// This variable holds a pool for 2D polygons as used by the clipper.
  static csPoly2DPool *polypool;

  /// Result of most recent clipping
  uint8 mrClipping;

public:
  static csPoly2DPool *GetSharedPool ();

public:
  /// Constructor.
  csClipper ();

  /// Destructor.
  virtual ~csClipper ();

  /// Wrapper function: clip a polygon in-place.
  virtual uint8 ClipInPlace (csVector2 *InPolygon, size_t &InOutCount,
  	csBox2 &BoundingBox);

  /// most recent Clipresult
  uint8 LastClipResult () { return mrClipping; }

  SCF_DECLARE_IBASE;
};

/**
 * The csBoxClipper class is able to clip convex polygons to a rectangle
 * (such as the screen).
 */
class CS_CRYSTALSPACE_EXPORT csBoxClipper : public csClipper
{
  /// The clipping region
  csBox2 region;
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
  csBoxClipper (const csBox2& b) : region (b)
  { InitClipBox (); }
  /// Initializes the clipper object to a rectangle with the given coords.
  csBoxClipper (float x1, float y1, float x2, float y2) : region (x1,y1,x2,y2)
  { InitClipBox (); }

  /// Simple clipping
  virtual uint8 Clip (csVector2 *InPolygon, size_t InCount,
    csVector2 *OutPolygon, size_t &OutCount);

  /// Clip and compute the bounding box
  virtual uint8 Clip (csVector2 *InPolygon, size_t InCount,
    csVector2 *OutPolygon, size_t &OutCount, csBox2 &BoundingBox);

  /// Clip and return additional information about each vertex
  virtual uint8 Clip (csVector2 *InPolygon, size_t InCount,
    csVector2 *OutPolygon, size_t &OutCount, csVertexStatus *OutStatus);

  /// Classify some bounding box against this clipper.
  virtual int ClassifyBox (const csBox2 &box);

  /// Return true if given point is inside (or on bound) of clipper polygon.
  virtual bool IsInside (const csVector2& v)
  { return region.In (v.x, v.y); }

  /// Return number of vertices for this clipper polygon.
  virtual size_t GetVertexCount ()
  { return 4; }

  /// Return a pointer to the array of csVector2's
  virtual csVector2 *GetClipPoly ()
  { return ClipBox; }

  virtual ClipperType GetClipperType() const { return clipperBox; }
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
class CS_CRYSTALSPACE_EXPORT csPolygonClipper : public csClipper
{
  /// Equation for all edges of clipping polygon
  csVector2 *ClipData;
  /// Clipper polygon itself
  csVector2 *ClipPoly;
  /// A pointer to the pooled polygon (so that we can free it later).
  csPoly2D *ClipPoly2D;
  /// Number of vertices in clipper polygon
  size_t ClipPolyVertices;
  /// Clipping polygon bounding box
  csBox2 ClipBox;

  /// Prepare clipping line equations
  void Prepare ();

public:
  /// Create a polygon clipper object from a 2D polygon.
  csPolygonClipper (csPoly2D *Clipper, bool mirror = false,
    bool copy = false);
  /// Create a polygon clipper object from a set of 2D vectors.
  csPolygonClipper (csVector2 *Clipper, size_t Count, bool mirror = false,
    bool copy = false);
  /// Destroy the polygon clipper object.
  virtual ~csPolygonClipper ();

  /// Simple clipping
  virtual uint8 Clip (csVector2 *InPolygon, size_t InCount,
    csVector2 *OutPolygon, size_t &OutCount);

  /// Clip and compute the bounding box
  virtual uint8 Clip (csVector2 *InPolygon, size_t InCount,
    csVector2 *OutPolygon, size_t &OutCount, csBox2 &BoundingBox);

  /// Clip and return additional information about each vertex
  virtual uint8 Clip (csVector2 *InPolygon, size_t InCount,
    csVector2 *OutPolygon, size_t &OutCount, csVertexStatus *OutStatus);

  /// Classify some bounding box against this clipper.
  virtual int ClassifyBox (const csBox2 &box);

  /// Return true if given point is inside (or on bound) of clipper polygon.
  virtual bool IsInside (const csVector2& v);

  /// Return number of vertices for this clipper polygon.
  virtual size_t GetVertexCount () { return ClipPolyVertices; }

  /// Return a pointer to the array of csVector2's
  virtual csVector2 *GetClipPoly ()
  { return ClipPoly; }

  virtual ClipperType GetClipperType() const { return clipperPoly; }
};
/** @} */


#endif // __CS_POLYCLIP_H__
