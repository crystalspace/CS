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

#ifndef SOLIDCUBE_H
#define SOLIDCUBE_H

class Dumper;
class csClipper;

#include "csengine/solidbsp.h"
#include "csengine/pol2d.h"

/**
 * Subclass of csSolidBsp which adds perspective correction
 * of 3d polygons.
 */
class csSolidBspPersp : public csSolidBsp
{
private:
  /// Perspective project a polygon. Return true if visible.
  bool DoPerspective (csVector3* verts, int num_verts,
    csPolygon2D& persp);

public:
  /// Constructor.
  csSolidBspPersp () : csSolidBsp () { }

  /**
   * Insert a polygon/frustum into the solid bsp.
   */
  bool InsertPolygon (csVector3* verts, int num_verts);

  /**
   * Test for polygon/frustum visibility with the solid bsp.
   */
  bool TestPolygon (csVector3* verts, int num_verts);
};

/**
 * A solid bsp cube is a set of six solid bsp's arranged in a cube.
 */
class csSolidBspCube
{
private:
  /// The six solid bsp trees.
  csSolidBspPersp* trees[6];

public:
  /// Make the solid bsp cube for the box.
  csSolidBspCube ();

  /**
   * Destroy this solid bsp cube.
   */
  ~csSolidBspCube ();

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

#endif /*SOLIDCUBE_H*/

