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
#include "csutil/scf_implementation.h"
#include "ivaria/collider.h"
#include "csgeom/transfrm.h"
#include "imesh/terrain2.h"
#include "CSopcodecollider.h"
#include "csTerraFormerCollider.h"
#include "Opcode.h"

struct iObjectRegistry;

CS_PLUGIN_NAMESPACE_BEGIN(csOpcode)
{

/**
 * Opcode implementation of the collision detection system.
 */
class csOPCODECollideSystem :
  public scfImplementation2<csOPCODECollideSystem, iCollideSystem, iComponent>
{

  bool Collide (csOPCODECollider* collider1,
    const csReversibleTransform* trans1, csTerraFormerCollider* terraformer,
    const csReversibleTransform* trans2);
  
  bool Collide (csOPCODECollider* collider1, const csReversibleTransform*
    trans1, iTerrainSystem* terrain, const csReversibleTransform* terrainTrans);

  bool TestTriangleTerraFormer (csVector3 triangle[3],
    csTerraFormerCollider* c, csCollisionPair* pair);

public:
  Opcode::AABBTreeCollider TreeCollider;
  Opcode::RayCollider RayCol;
  Opcode::LSSCollider LSSCol;
  Opcode::OBBCollider OBBCol;
  Opcode::BVTCache ColCache;
  Opcode::LSSCache LSSCache;
  Opcode::OBBCache OBBCache;

  IceMaths::Matrix4x4 identity_matrix;
  
  csDirtyAccessArray<csCollisionPair> pairs;
  csArray<int> collision_faces;
  csArray<csIntersectingTriangle> intersecting_triangles;
  iObjectRegistry *object_reg;
  csStringID trianglemesh_id;
  csStringID basemesh_id;
 
  static iObjectRegistry* rep_object_reg;
  static void OpcodeReportV (int severity, const char* message, 
    va_list args);

  csOPCODECollideSystem (iBase* parent);
  virtual ~csOPCODECollideSystem ();
  bool Initialize (iObjectRegistry* iobject_reg);

  // Copy the collision detection pairs for the current Collide
  // to 'pairs'.
  void CopyCollisionPairs (csOPCODECollider* col1, csOPCODECollider* col2);

  void CopyCollisionPairs (csOPCODECollider* col2,
      csTerraFormerCollider* terraformer);

  virtual csStringID GetTriangleDataID () { return trianglemesh_id; }
  virtual csStringID GetBaseDataID () { return basemesh_id; }

  virtual csPtr<iCollider> CreateCollider (iTriangleMesh* mesh);
  virtual csPtr<iCollider> CreateCollider (iTerraFormer* mesh);
  virtual csPtr<iCollider> CreateCollider (iTerrainSystem* mesh);

  /**
   * Test collision between two colliders.
   */
  virtual bool Collide (
  	iCollider* collider1, const csReversibleTransform* trans1,
  	iCollider* collider2, const csReversibleTransform* trans2);

  bool CollideRaySegment (
  	iCollider* collider, const csReversibleTransform* trans,
	const csVector3& start, const csVector3& end, bool use_ray);
  bool CollideRaySegment (
  	csOPCODECollider* collider, const csReversibleTransform* trans,
	const csVector3& start, const csVector3& end, bool use_ray);
  bool CollideRaySegment (
  	iTerrainSystem* terrain, const csReversibleTransform* trans,
	const csVector3& start, const csVector3& end, bool use_ray);
  bool CollideRaySegment (
  	csTerraFormerCollider* terraformer, const csReversibleTransform* trans,
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
  
  virtual bool CollideLSS (
  	iCollider* collider, const csReversibleTransform* trans1,
	const csVector3& start, const csVector3& end, float radius,
  	const csReversibleTransform* trans2);
  bool CollideLSS (
  	csOPCODECollider* collider, const csReversibleTransform* trans1,
	const csVector3& start, const csVector3& end, float radius,
  	const csReversibleTransform* trans2);
  bool CollideLSS (
  	iTerrainSystem* terrain, const csReversibleTransform* trans1,
	const csVector3& start, const csVector3& end, float radius,
  	const csReversibleTransform* trans2);
  bool CollideLSS (
  	csTerraFormerCollider* terraformer, const csReversibleTransform* trans1,
	const csVector3& start, const csVector3& end, float radius,
  	const csReversibleTransform* trans2);
  
  virtual bool CollideOBB (
  	iCollider* collider, const csReversibleTransform* trans1,
  	const csVector3& min, const csVector3& max,
  	const csReversibleTransform* trans2);
  bool CollideOBB (
  	csOPCODECollider* collider, const csReversibleTransform* trans1,
  	const csVector3& min, const csVector3& max,
  	const csReversibleTransform* trans2);
  bool CollideOBB (
  	iTerrainSystem* terrain, const csReversibleTransform* trans1,
  	const csVector3& min, const csVector3& max,
  	const csReversibleTransform* trans2);
  bool CollideOBB (
  	csTerraFormerCollider* terraformer, const csReversibleTransform* trans1,
  	const csVector3& min, const csVector3& max,
  	const csReversibleTransform* trans2);
  
  virtual const csArray<csIntersectingTriangle>& GetIntersectingTriangles ()
  	const
  {
    return intersecting_triangles;
  }
  bool CalculateIntersections (csTerraFormerCollider* terraformer);
  bool TestSegmentTerraFormer (
    csTerraFormerCollider* terraformer,
    const IceMaths::Matrix4x4& transform,
    const csVector3& start, const csVector3& end);

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
};

}
CS_PLUGIN_NAMESPACE_END(csOpcode)

#endif // __CS_OPCODE_PLUGIN_H__
