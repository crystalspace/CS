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

#ifndef __VOLUME_H__
#define __VOLUME_H__

#include "types.h"
#include "csgeom/math3d.h"

/**
 * A convex volume. This is basicly a set of
 * infinite planes. The intersection of the positive sides
 * of all planes defines the volume. Note that the volume
 * can be infinite.
 */
class csVolume
{
private:
  csPlane* planes;
  int num_planes;
  int max_planes;

  void Clear ();

public:
  /// Create a new infinite volume.
  csVolume ();

  /// Copy constructor.
  csVolume (const csVolume &copy);

  ///
  virtual ~csVolume () { Clear (); }

  /**
   * Add a plane to this volume.
   * The plane is copied internally and can be reused after
   * calling this function.
   */
  void AddPlane (csPlane& plane);

  /**
   * Add a plane to this volume.
   */
  void AddPlane (float A, float B, float C, float D);

  /**
   * Get the number of planes.
   */
  int GetNumPlanes () { return num_planes; }

  /**
   * Get a plane.
   */
  csPlane& GetPlane (int idx) { return planes[idx]; }

  /**
   * Intersect with another volume.
   * Returns new volume.
   */
  csVolume Intersect (const csVolume& volume);

  /**
   * Intersect a convex polygon with this volume.
   * Returns the part of the polygon that is inside the volume.
   * Caller should delete returned array of csVector3.
   * This function returns NULL if there is no intersection.
   */
  csVector3* Intersect (csVector3* poly, int num_in, int& num_out);

  /// Returns true if volume intersects with target.
  bool Intersects (csVolume& target);

  /**
   * Tests the orientation of this volume with respect
   * to the given plane. If the plane cuts right through
   * the volume then it returns 0. If the volume is completely
   * to the positive side of the plane then it returns > 0.
   * Otherwise it returns < 0.
   */
  int ClassifyToPlane (const csPlane& cutter);

  /**
   * Cut this volume by a plane and retain everything on the
   * positive side of it.
   */
  void CutByPlane (const csPlane& cutter);

  /// Return true if a point lies within volume.
  bool Contains (const csVector3& p);

  /// Return true if volume is empty.
  bool IsEmpty ();

  /// Return true if volume is infinite (no planes).
  bool IsInfinite () { return planes == NULL; }

  /// Make volume infinite.
  void MakeInfinite () { Clear (); }
};

#endif // __CSVOLUME_H__
