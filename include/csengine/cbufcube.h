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

#ifndef __CS_CBUFCUBE_H__
#define __CS_CBUFCUBE_H__

class csClipper;

#include "csengine/cbuffer.h"
#include "csengine/pol2d.h"
#include "csgeom/polyclip.h"

/**
 * Subclass of csCBuffer which adds perspective correction
 * of 3d polygons.
 */
class csCBufferPersp : public csCBuffer
{
private:
  /// Perspective project a polygon. Return true if visible.
  bool DoPerspective (csVector3* verts, int num_verts,
    csPolygon2D& persp);

public:
  /// Constructor.
  csCBufferPersp (int sx, int sy, int nlines) : csCBuffer (sx, sy, nlines) { }

  /**
   * Insert a polygon/frustum into the cube.
   * The optional 'clipper' will be used to clip the resulting polygon.
   */
  bool InsertPolygon (csVector3* verts, int num_verts, csClipper* clipper = NULL);

  /**
   * Test for polygon/frustum visibility with the cube.
   * The optional 'clipper' will be used to clip the resulting polygon.
   */
  bool TestPolygon (csVector3* verts, int num_verts, csClipper* clipper = NULL);
};

/**
 * A cbuffer cube is a set of six c-buffers arranged in a cube.
 */
class csCBufferCube
{
private:
  /// The six c-buffers.
  csCBufferPersp* trees[6];
  /// A box clipper to clip all the 2D polygons with.
  csBoxClipper* clipper;

public:
  /// Make the cube for the box.
  csCBufferCube (int dim);

  /**
   * Destroy this cube.
   */
  ~csCBufferCube ();

  /**
   * Make cube empty.
   */
  void MakeEmpty ();

  /**
   * Is the cube full?
   */
  bool IsFull ();

  /**
   * Insert a polygon into the cube.
   * Return true if the cube was modified (i.e. if parts of the
   * polygon were visible.<p>
   * The polygon does not actually need to be a polygon. It can
   * be a general frustum. Note that the frustum is assumed
   * to start at (0,0,0).
   */
  bool InsertPolygon (csVector3* verts, int num_verts);

  /**
   * Test for polygon visibility with the cube.
   * Return true if polygon is visible.<p>
   * The polygon does not actually need to be a polygon. It can
   * be a general frustum. Note that the frustum is assumed
   * to start at (0,0,0).
   */
  bool TestPolygon (csVector3* verts, int num_verts);
};

#endif // __CS_CBUFCUBE_H__

