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

/*
-------------------------------------------------------------------------
*
*           OPCODE collision detection plugin for CrystalSpace
*
*           OPCODE library was written by Pierre Terdiman
*                  ported to CS by Charles Quarra
*
-------------------------------------------------------------------------
*/

#ifndef __CS_OPCODECOL_H__
#define __CS_OPCODECOL_H__

#include "csgeom/math3d.h"
#include "csgeom/matrix3.h"
#include "csgeom/vector3.h"
#include "csgeom/box.h"
#include "ivaria/collider.h"
#include "Opcode.h"

class csReversibleTransform;

class csCdModel;
class csCdBBox;
struct csCdTriangle;
struct csCollisionPair;
struct iPolygonMesh;
class PathPolygonMesh;

/// Low level collision detection using the RAPID algorithm.
class csOPCODECollider : public iCollider
{
/// The internal model object.
public:
  Opcode::Model* m_pCollisionModel;
  IceMaths::Matrix4x4 transform;
  unsigned int* indexholder;
  Point *vertholder;

  Opcode::MeshInterface opcMeshInt;

private:
  void GeometryInitialize (iPolygonMesh *mesh);

  static void MeshCallback (udword triangle_index, 
    Opcode::VertexPointers& triangle, void* user_data);
public:
  /**
   * Global variables
   * Matrix, and Vector used for collision testing.
   */

  // CS_DECLARE_STATIC_CLASSVAR (mR, GetMR, csMatrix3)
  // CS_DECLARE_STATIC_CLASSVAR (mT, GetMT, csVector3)

  /// Create a collider based on geometry.
  csOPCODECollider (iPolygonMesh* mesh);

  /// Destroy the RAPID collider object
  virtual ~csOPCODECollider ();

  /**
   * Check if this collider collides with pOtherCollider.
   * Returns true if collision detected and adds the pair to the collisions
   * hists vector.
   * This collider and pOtherCollider must be of comparable subclasses, if
   * not false is returned.
   */
  bool Collide (csOPCODECollider &pOtherCollider,
    const csReversibleTransform *pThisTransform = 0,
    const csReversibleTransform *pOtherTransform = 0);

  /// Test collision with an array of colliders.
  bool CollideArray (
  	const csReversibleTransform *pThisTransform,
  	int num_colliders,
	  iCollider** colliders,
	  csReversibleTransform **transforms);

  int CollidePath (
  	const csReversibleTransform* thisTransform,
	  csVector3& newpos, int num_colliders,
    iCollider** colliders,
    csReversibleTransform** transforms);

  /// Query the array with collisions (and their count).
  static csCollisionPair *GetCollisions ();

  static void CollideReset ();
  static void SetFirstHit (bool fh);
  static bool GetFirstHit ();
  static int Report (csOPCODECollider **id1, csOPCODECollider **id2);
  const csVector3 &GetRadius () const;

  SCF_DECLARE_IBASE;
};

#endif // __CS_OPCODECOL_H__
