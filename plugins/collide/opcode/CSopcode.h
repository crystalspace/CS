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

#ifndef __CS_OPCODE_PLUGIN_H__
#define __CS_OPCODE_PLUGIN_H__

#include "iutil/comp.h"
#include "csgeom/vector3.h"
#include "csutil/dirtyaccessarray.h"
#include "ivaria/collider.h"
#include "csgeom/transfrm.h"
#include "CSopcodecollider.h"
#include "Opcode.h"

struct iObjectRegistry;

/**
 * Opcode implementation of the collision detection system.
 */
class csOPCODECollideSystem : public iCollideSystem
{
public:
  Opcode::AABBTreeCollider TreeCollider;
  Opcode::RayCollider RayCol;
  Opcode::BVTCache ColCache;

  csDirtyAccessArray<csCollisionPair> pairs;
  csArray<int> collision_faces;
  csArray<csIntersectingTriangle> intersecting_triangles;
  iObjectRegistry *object_reg;
 
  static iObjectRegistry* rep_object_reg;
  static void OpcodeReportV (int severity, const char* message, 
    va_list args);

  SCF_DECLARE_IBASE;

  csOPCODECollideSystem (iBase* parent);
  virtual ~csOPCODECollideSystem ();
  bool Initialize (iObjectRegistry* iobject_reg);

  // Copy the collision detection pairs for the current Collide
  // to 'pairs'.
  void CopyCollisionPairs (csOPCODECollider* col1, csOPCODECollider* col2);

  virtual csPtr<iCollider> CreateCollider (iPolygonMesh* mesh);

  /**
   * Test collision between two colliders.
   */
  virtual bool Collide (
  	iCollider* collider1, const csReversibleTransform* trans1,
  	iCollider* collider2, const csReversibleTransform* trans2);

  bool CollideRaySegment (
  	iCollider* collider, const csReversibleTransform* trans,
	const csVector3& start, const csVector3& end, bool use_ray);
  virtual bool CollideRay (
  	iCollider* collider, const csReversibleTransform* trans,
	const csVector3& start, const csVector3& end)
  {
    return CollideRaySegment (collider, trans, start, end, true);
  }
  virtual bool CollideSegment (
  	iCollider* collider, const csReversibleTransform* trans,
	const csVector3& start, const csVector3& end)
  {
    return CollideRaySegment (collider, trans, start, end, false);
  }
  virtual const csArray<csIntersectingTriangle>& GetIntersectingTriangles ()
  	const
  {
    return intersecting_triangles;
  }

  /**
   * Get pointer to current array of collision pairs.
   */
  virtual csCollisionPair* GetCollisionPairs ();

  /**
   * Get number of collision pairs in array.
   */
  virtual size_t GetCollisionPairCount ();

  /**
   * Reset the array with collision pairs.
   */
  virtual void ResetCollisionPairs ();

  /**
   * Indicate if we are interested only in the first hit that is found.
   * This is only valid for CD algorithms that actually allow the
   * detection of multiple CD hit points.
   */
  virtual void SetOneHitOnly (bool o);

  /**
   * Return true if this CD system will only return the first hit
   * that is found. For CD systems that support multiple hits this
   * will return the value set by the SetOneHitOnly() function.
   * For CD systems that support one hit only this will always return true.
   */
  virtual bool GetOneHitOnly ();

  struct Component : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csOPCODECollideSystem);
    virtual bool Initialize (iObjectRegistry* object_reg)
    {
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;
};

#endif // __CS_OPCODE_PLUGIN_H__
