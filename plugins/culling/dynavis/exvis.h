/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_EXVIS_H__
#define __CS_EXVIS_H__

#include "csgeom/vector2.h"
#include "csgeom/math2d.h"
#include "iutil/dbghelp.h"

struct iPolygonMesh;
struct iMovable;
struct iCamera;
class csPlane3;
class csBoxClipper;

struct csExVisObj
{
  void* obj;
  int totpix;
  int vispix;
};

/**
 * Exact visibility culler. This class is mainly a debugging tool
 * for Dynavis. It will try to compute exact visibility by rendering
 * the polygons of the write objects for all occluders to a buffer
 * with Z-buffer.
 */
class csExactCuller
{
private:
  int width, height;
  uint32* scr_buffer;	// The total screen buffer. Contains object indices.
  float* z_buffer;	// Z buffer.
  csBoxClipper* boxclip;

  int num_objects;
  int max_objects;
  csExVisObj* objects;

  /**
   * Insert a polygon in the buffer.
   * This function will not do any backface culling and it will work
   * perfectly in all orientations. Polygon has to be convex.
   * Returns the total number of pixels in totpix and the number
   * of visible pixels (z-buf) in vispix.
   * M, N, and O are used for getting the Z value at every pixel.
   */
  void InsertPolygon (csVector2* tr_verts, size_t num_verts,
  	float M, float N, float O, uint32 obj_number, int& totpix);

public:
  /// Create a new exact culler with the given dimensions.
  csExactCuller (int w, int h);
  /// Destroy the exact culler.
  ~csExactCuller ();

  /// Add an object to this culler.
  void AddObject (void* obj, iPolygonMesh* polymesh, iMovable* movable,
  	iCamera* camera, const csPlane3* planes);

  /// Do the visibility test.
  void VisTest ();

  /**
   * Get the status for one object.
   * Returns the amount of pixels that are visible and also the
   * amount of total pixels that would have been rendered if object
   * was fully visible.
   */
  void GetObjectStatus (void* obj, int& vispix, int& totpix);
};

#endif // __CS_EXVIS_H__

