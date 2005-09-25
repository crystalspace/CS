/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef __CS_IGEOM_CLIP2D_H__
#define __CS_IGEOM_CLIP2D_H__

/**\file
 * 2D clipping interface
 */

/**
 * \addtogroup geom_utils
 * @{ */

#include "csutil/scf_interface.h"

class csBox2;
class csVector2;

/// Maximal number of vertices in output (clipped) polygons
#define MAX_OUTPUT_VERTICES	64

/** \name Clipper return codes
 * The clipper routines return one of <code>#CS_CLIP_OUTSIDE</code>, 
 * <code>#CS_CLIP_INSIDE</code>, <code>#CS_CLIP_CLIPPED</code> so that we can 
 * distinguish between the cases when input polygon is completely outside the 
 * clipping polygon (thus it is not visible), completely inside the clipping 
 * polygon (thus it has not changed) and partially outside, partially inside 
 * (thus it was clipped).
 * @{ */
enum
{
  /// The input polygon is completely outside of clipper polygon
  CS_CLIP_OUTSIDE = 0,
  /// The input polygon is completely inside (thus has not changed)
  CS_CLIP_INSIDE = 1,
  /// The input polygon was partially inside, partially outside
  CS_CLIP_CLIPPED = 2
};
/** @} */

/**
 * \name Additional vertex informations
 * @{ */
/**
 * The clipper can output additional information about each vertex in
 * output polygon. This is used for generating U/V/Z values and so on,
 * if they are needed. 
 */
struct csVertexStatus
{
  /**
   * The type of vertex<p>
   * One of CS_VERTEX_*.
   */
  unsigned char Type;
  /// Original vertex number (for #CS_VERTEX_ORIGINAL and #CS_VERTEX_ONEDGE)
  size_t Vertex;
  /// Additional information for #CS_VERTEX_ONEDGE (0..1, the 't' parameter)
  float Pos;
};

enum
{
  /// The output vertex is one of the input vertices
  CS_VERTEX_ORIGINAL = 0,
  /// The output vertex is located on one of the edges of the original polygon
  CS_VERTEX_ONEDGE = 1,
  /// The output vertex is located somewhere inside the original polygon
  CS_VERTEX_INSIDE = 2
};
/** @} */


/**
 * This interfaces represents a 2D clipper for polygons.
 */
struct iClipper2D : public virtual iBase
{
  SCF_INTERFACE(iClipper2D, 2, 0, 0);
  /**
   * Clip a set of 2D points and return in 'OutPolygon' which is expected
   * to contain space at least for #MAX_OUTPUT_VERTICES elements.
   * \return One of \link #CS_CLIP_OUTSIDE CS_CLIP_*\endlink.
   */
  virtual uint8 Clip (csVector2 *InPolygon, size_t InCount,
    csVector2 *OutPolygon, size_t &OutCount) = 0;

  /**
   * Clip a set of 2D points.
   * On output OutCount is set to number of vertices in output polygon.
   * The output array is expected to contain space for at least
   * #MAX_OUTPUT_VERTICES elements. The bounding box is set to the
   * minimal rectangle that contains the output polygon.
   * \return One of \link #CS_CLIP_OUTSIDE CS_CLIP_*\endlink.
   */
  virtual uint8 Clip (csVector2 *InPolygon, size_t InCount,
    csVector2 *OutPolygon, size_t &OutCount, csBox2 &BoundingBox) = 0;

  /**
   * Clips a set of 2D points and provides additional information on each
   * output vertex. The information type can be: vertex is one of original
   * vertices, vertex is on the edge of the original polygon and vertex is
   * arbitrary located inside the original polygon. Both OutPolygon and
   * OutStatus arrays are expected to have enough storage for at least
   * #MAX_OUTPUT_VERTICES elements.
   * \return One of \link #CS_CLIP_OUTSIDE CS_CLIP_*\endlink.
   */
  virtual uint8 Clip (csVector2 *InPolygon, size_t InCount,
    csVector2 *OutPolygon, size_t &OutCount, csVertexStatus *OutStatus) = 0;

  /**
   * Wrapper function: clip a polygon in-place.
   * \return One of \link #CS_CLIP_OUTSIDE CS_CLIP_*\endlink.
   */
  virtual uint8 ClipInPlace (csVector2 *InPolygon, size_t &InOutCount,
			     csBox2 &BoundingBox) = 0;

  /**
   * Classify some bounding box against this clipper.
   * This function returns:<p>
   * <ul>
   * <li> -1 if box is not visible.
   * <li> 0 if box is partially visible.
   * <li> 1 if box is entirely visible.
   * </ul>
   */
  virtual int ClassifyBox (const csBox2 &box) = 0;

  /// Return true if given point is inside (or on bound) of clipper polygon.
  virtual bool IsInside (const csVector2& v) = 0;

  /// Return number of vertices for this clipper polygon.
  virtual size_t GetVertexCount () = 0;

  /// Return a pointer to the array of csVector2's
  virtual csVector2 *GetClipPoly () = 0;

  /// The type of a clipper
  enum ClipperType
  {
    /// This clipper contains a polygon.
    clipperPoly,
    /// This clipper contains a box. Can be used for some optimizations.
    clipperBox
  };
  /**
   * Retrieve the type of this clipper.
   */
  virtual ClipperType GetClipperType() const = 0;
};

/** @} */

#endif // __CS_IGEOM_CLIP2D_H__
