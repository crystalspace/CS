/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein
    Written by Alex Pfaffe.

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
#ifndef __CS_RAPCOL_H__
#define __CS_RAPCOL_H__

#include "csgeom/math3d.h"
#include "csgeom/matrix3.h"
#include "csgeom/vector3.h"
#include "csgeom/box.h"
#include "ivaria/collider.h"

class csReversibleTransform;

class csCdModel;
class csCdBBox;
struct csCdTriangle;
struct csCollisionPair;
struct iPolygonMesh;
class PathPolygonMesh;

/// Low level collision detection using the RAPID algorithm.
class csRapidCollider : public iCollider
{
private:
  friend class csCdBBox;

  /// The internal collision object.
  csCdModel* m_pCollisionModel;

  /// Array of vertices.
  csVector3* vertices;

  /// Get top level bounding box.
  const csCdBBox* GetBbox () const;

  /// Delete and free memory of this objects oriented bounding box.
  void DestroyBbox ();

  /// Recursively test collisions of bounding boxes.
  static int CollideRecursive (csCdBBox *b1, csCdBBox *b2,
  	const csMatrix3& R, const csVector3& T,
	csVector3* vertices1, csVector3* vertices2);

  /**
   * Statistics, to allow early bailout.
   * If the number of triangles tested is too high the BBox structure
   * probably isn't very good.
   */
  static int trianglesTested;		// TEMPORARY.
  /// The number of boxes tested.
  static int boxesTested;		// TEMPORARY.
  /**
   * If bbox is less than this size, dont bother testing further,
   * just return with the results so far.
   */
  static float minBBoxDiam;
  /// Number of levels to test.
  static int testLevel;
  /// Test only up to the 1st hit.
  static bool firstHit;

  void GeometryInitialize (iPolygonMesh *mesh);

public:
  /**
    * Global variables
    * Matrix, and Vector used for collision testing.
    */
  CS_DECLARE_STATIC_CLASSVAR (mR, GetMR, csMatrix3)
  CS_DECLARE_STATIC_CLASSVAR (mT, GetMT, csVector3)

  static int numHits;

  /// Create a collider based on geometry.
  csRapidCollider (iPolygonMesh* mesh);

  /// Destroy the RAPID collider object
  virtual ~csRapidCollider();

  /**
   * Check if this collider collides with pOtherCollider.
   * Returns true if collision detected and adds the pair to the collisions
   * hists vector.
   * This collider and pOtherCollider must be of comparable subclasses, if
   * not false is returned.
   */
  bool Collide (csRapidCollider &pOtherCollider,
                        const csReversibleTransform *pThisTransform = 0,
                        const csReversibleTransform *pOtherTransform = 0);

  /// Query the array with collisions (and their count).
  static csCollisionPair *GetCollisions ();

  static void CollideReset ();
  static void SetFirstHit (bool fh) { firstHit = fh; }
  static bool GetFirstHit () { return firstHit; }
  static int Report (csRapidCollider **id1, csRapidCollider **id2);
  const csVector3 &GetRadius() const;

  SCF_DECLARE_IBASE;
};


#endif // __CS_RAPCOL_H__

