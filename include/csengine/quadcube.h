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

#ifndef QUADCUBE_H
#define QUADCUBE_H

class Dumper;
class csClipper;

#include "csgeom/quadtree.h"
#include "csengine/pol2d.h"

/**
 * Subclass of csQuadtree which adds perspective correction
 * of 3d polygons.
 */
class csQuadtreePersp : public csQuadtree
{
private:
  /// Perspective project a polygon. Return true if visible.
  bool DoPerspective (csVector3* verts, int num_verts,
    csPolygon2D& persp);

public:
  /// Constructor.
  csQuadtreePersp (const csBox& box, int depth) : csQuadtree (box, depth) { }

  /**
   * Insert a polygon/frustrum into the quad-tree.
   * The optional 'clipper' will be used to clip the resulting polygon.
   */
  bool InsertPolygon (csVector3* verts, int num_verts, csClipper* clipper = NULL);

  /**
   * Test for polygon/frustrum visibility with the quad-tree.
   * The optional 'clipper' will be used to clip the resulting polygon.
   */
  bool TestPolygon (csVector3* verts, int num_verts, csClipper* clipper = NULL);

  /**
   * Test for point visibility with the quad-tree.
   */
  int TestPoint (const csVector3& point);
};

/**
 * A quad-cube is a set of six quad-trees arranged in a cube.
 */
class csQuadcube
{
  friend class Dumper;

private:
  /// The six quad-trees.
  csQuadtreePersp* trees[6];

public:
  /// Make the quad-cube for the box.
  csQuadcube (int depth);

  /**
   * Destroy this quad-cube
   */
  ~csQuadcube ();

  /**
   * Make cube empty.
   */
  void MakeEmpty ();

  /**
   * Is the cube full?
   */
  bool IsFull ();

  /**
   * Insert a polygon into the quad-cube.
   * Return true if the cube was modified (i.e. if parts of the
   * polygon were visible.<p>
   * The polygon does not actually need to be a polygon. It can
   * be a general frustrum. Note that the frustrum is assumed
   * to start at (0,0,0).
   */
  bool InsertPolygon (csVector3* verts, int num_verts);

  /**
   * Test for polygon visibility with the quad-cube.
   * Return true if polygon is visible.<p>
   * The polygon does not actually need to be a polygon. It can
   * be a general frustrum. Note that the frustrum is assumed
   * to start at (0,0,0).
   */
  bool TestPolygon (csVector3* verts, int num_verts);

  /**
   * Test for point visibility with the quad-cube.
   */
  int TestPoint (const csVector3& point);
};

#endif /*QUADCUBE_H*/

