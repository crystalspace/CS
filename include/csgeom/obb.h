/*
    Copyright (C) 2002 by Jorrit Tyberghein
    Copyright (C) 2002 by Daniel Duhprey

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

#ifndef __CS_OBB_H__
#define __CS_OBB_H__

/**\file 
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"

#include "csgeom/box.h"
#include "csgeom/matrix3.h"

class csVector3;
class csReversibleTransform;

/**
 * Oriented bounding box (OBB). This is basically
 * a csBox3 with a matrix to rotate it.
 */
class CS_CRYSTALSPACE_EXPORT csOBB : public csBox3
{
private:
  csMatrix3 mMat;

public:
  /**
   * Initialize the OBB to empty with identity transform.
   */
  csOBB () {}

  /**
   * Copy constructor.
   */
  csOBB (const csOBB &b) : csBox3 (b), mMat (b.mMat) { }

  /**
   * Initialize the OBB to the given AABB with identity transform.
   */
  csOBB (const csBox3 &b) : csBox3 (b) { }

  /**
   * Construct an OBB from the three given vectors. This will
   * setup the orientation.
   * dir1 are the two vertices furthest appart.
   * dir2 is the two vertices after moving them to the plane perpendicular
   * to dir1 and dir3 is the cross of dir1 and dir2.
   */
  csOBB (const csVector3 &dir1, const csVector3 &dir2,
  	const csVector3 &dir3);

  void AddBoundingVertex (const csVector3 &v);
  csVector3 GetCorner (int corner) const; 
  const csMatrix3 &GetMatrix () const { return mMat; }

  /**
   * Get the diameter of this OBB.
   */
  float Diameter ();

  /**
   * Get the volume of this OBB.
   */
  float Volume ();

  /**
   * Given the table of vertices find a csOBB that matches this table.
   * This is a faster version that FindOBBAccurate() but it is less
   * accurate.
   */
  void FindOBB (const csVector3 *vertex_table, int num, float eps = 0.0);

  /**
   * Given the table of vertices find a csOBB that matches this table.
   * This version is a lot slower than FindOBB but it gives a more accurate
   * OBB.
   */
  void FindOBBAccurate (const csVector3 *vertex_table, int num);
};

/**
 * Version of the csOBB with frozen corners (for optimization purposes).
 */
class CS_CRYSTALSPACE_EXPORT csOBBFrozen
{
private:
  csVector3 corners[8];

public:
  /**
   * Copy a normal OBB and freeze the corners.
   */
  void Copy (const csOBB& obb)
  {
    for (int i = 0 ; i < 8 ; i++)
    {
      corners[i] = obb.GetCorner (i);
    }
  }

  /**
   * Copy a normal OBB and freeze the corners.
   */
  void Copy (const csOBB& obb, const csReversibleTransform& trans);

  /**
   * Create an empty csOBBFrozen which is not initialized.
   */
  csOBBFrozen ()
  {
  }

  /**
   * Create a frozen OBB from a normal OBB.
   */
  csOBBFrozen (const csOBB& obb)
  {
    Copy (obb);
  }

  /**
   * Create a frozen OBB from a normal OBB.
   * The given transform is applied to the vertices
   * AFTER the matrix of the obb is applied (using
   * Other2This).
   */
  csOBBFrozen (const csOBB& obb, const csReversibleTransform& trans)
  {
    Copy (obb, trans);
  }

  /**
   * Get one corner from the OBB.
   */
  const csVector3& GetCorner (int corner) const
  {
    CS_ASSERT (corner >= 0 && corner < 8);
    return corners[corner];
  }

  /**
   * Project this OBB to a 2D screen space box.
   * Returns false if OBB is not on screen.
   */
  bool ProjectOBB (float fov, float sx, float sy, csBox2& sbox,
	float& min_z, float& max_z);
};

/** @} */

#endif // __CS_OBB_H__

