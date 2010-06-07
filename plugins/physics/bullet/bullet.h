/*
Copyright (C) 2007 by Jorrit Tyberghein

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

#ifndef __CS_BULLET_DYNAMICS_H__
#define __CS_BULLET_DYNAMICS_H__

#include "ivaria/dynamics.h"
#include "iutil/comp.h"
#include "csutil/scf.h"
#include "csutil/scf_implementation.h"
#include "csutil/csobject.h"
#include "csutil/nobjvec.h"
#include "csutil/weakrefarr.h"
#include "ivaria/bullet.h"

struct btSoftBodyWorldInfo;

CS_PLUGIN_NAMESPACE_BEGIN(Bullet)
{

class csBulletMotionState;
class csBulletDebugDraw;
class csBulletRigidBody;
class csBulletSoftBody;
class csBulletCollider;
class csBulletTerrainCollider;
class csBulletTerrainCellCollider;
class csBulletDefaultMoveCallback;

struct BulletBody
{
  virtual ~BulletBody () {}

  csBulletBodyType bodyType;
};

/**
 * This is the implementation for the actual plugin.
 * It is responsible for creating iDynamicSystem.
 */
class csBulletDynamics : public scfImplementation2<csBulletDynamics,
  iDynamics, iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRefArrayObject<iDynamicSystem> systems;
  csRefArray<iDynamicsStepCallback> stepCallbacks;

public:
  csBulletDynamics (iBase *iParent);
  virtual ~csBulletDynamics ();

  virtual csPtr<iDynamicSystem> CreateSystem ();
  virtual void RemoveSystem (iDynamicSystem* system);
  virtual void RemoveSystems ();
  virtual iDynamicSystem* FindSystem (const char *name);

  virtual void Step (float stepsize);

  virtual void AddStepCallback (iDynamicsStepCallback *callback)
  { stepCallbacks.Push (callback); }
  virtual void RemoveStepCallback (iDynamicsStepCallback *callback)
  { stepCallbacks.Delete (callback); }

  //  iComponent
  virtual bool Initialize (iObjectRegistry* object_reg);  
};

class csBulletDynamicsSystem : public scfImplementationExt2<
  csBulletDynamicsSystem, csObject, iDynamicSystem,
  iBulletDynamicSystem>
{
  friend class csBulletMotionState;
  friend class csBulletKinematicMotionState;
  friend class csBulletRigidBody;
  friend class csBulletSoftBody;
  friend class csBulletCollider;
  friend class csBulletJoint;
  friend class csBulletPivotJoint;
  friend class HeightMapCollider;

private:
  bool isSoftWorld;
  btDynamicsWorld* bulletWorld;
  btCollisionDispatcher* dispatcher;
  btDefaultCollisionConfiguration* configuration;
  btSequentialImpulseConstraintSolver* solver;
  btBroadphaseInterface* broadphase;
  btSoftBodyWorldInfo* softWorldInfo;

  csRefArrayObject<iRigidBody> dynamicBodies;
  csRefArrayObject<iRigidBody> colliderBodies;
  csRefArrayObject<iBulletSoftBody> softBodies;
  csRefArray<iJoint> joints;
  csRefArray<iBulletPivotJoint> pivotJoints;
  csRefArray<iBulletTerrainCollider> terrainColliders;

  csRef<csBulletDefaultMoveCallback> moveCb;

  bool gimpactRegistered;
  float internalScale;
  float inverseInternalScale;
  float worldTimeStep;
  size_t worldMaxSteps;
  float linearDampening;
  float angularDampening;
  bool autoDisableEnabled;
  float linearDisableThreshold;
  float angularDisableThreshold;
  float timeDisableThreshold;

  // For getting collision mesh data.
  csStringID baseId;
  csStringID colldetId;

  csBulletDebugDraw* debugDraw;

  void CheckCollisions();
  void CheckCollision(csBulletRigidBody& cs_obA, btCollisionObject *obB,
		      btPersistentManifold &contactManifold);

public:
  csBulletDynamicsSystem (iObjectRegistry* object_reg);
  virtual ~csBulletDynamicsSystem ();

  //-- iDynamicsSystem
  virtual iObject *QueryObject () { return (iObject*) this; }

  virtual void SetGravity (const csVector3& v);
  virtual const csVector3 GetGravity () const;
  virtual void SetLinearDampener (float d);
  virtual float GetLinearDampener () const;
  virtual void SetRollingDampener (float d);
  virtual float GetRollingDampener () const;
  virtual void EnableAutoDisable (bool enable);
  virtual bool AutoDisableEnabled ();
  virtual void SetAutoDisableParams (float linear, float angular, int steps,
    float time);
  virtual void Step (float stepsize);

  virtual csPtr<iRigidBody> CreateBody ();
  virtual void AddBody (iRigidBody* body);
  virtual void RemoveBody (iRigidBody* body);
  virtual iRigidBody *FindBody (const char *name);
  virtual iRigidBody *GetBody (unsigned int index);
  virtual int GetBodysCount ();

  virtual csPtr<iBodyGroup> CreateGroup ();
  virtual void RemoveGroup (iBodyGroup* group);

  virtual csPtr<iJoint> CreateJoint ();
  virtual void RemoveJoint (iJoint* joint);
  virtual iDynamicsMoveCallback* GetDefaultMoveCallback ();

  virtual bool AttachColliderConvexMesh (iMeshWrapper* mesh,
    const csOrthoTransform& trans, float friction,
    float elasticity, float softness = 0.01f);
  virtual bool AttachColliderMesh (iMeshWrapper* mesh,
    const csOrthoTransform& trans, float friction,
    float elasticity, float softness = 0.01f);
  virtual bool AttachColliderCylinder (float length, float radius,
    const csOrthoTransform& trans, float friction,
    float elasticity, float softness = 0.01f);
  virtual bool AttachColliderCapsule (float length, float radius,
    const csOrthoTransform& trans, float friction,
    float elasticity, float softness = 0.01f);
  virtual bool AttachColliderBox (const csVector3 &size,
    const csOrthoTransform& trans, float friction,
    float elasticity, float softness = 0.01f);
  virtual bool AttachColliderSphere (float radius, const csVector3 &offset,
    float friction, float elasticity, float softness = 0.01f);
  virtual bool AttachColliderPlane (const csPlane3 &plane, float friction,
    float elasticity, float softness = 0.01f);
  virtual void DestroyColliders ();
  virtual void DestroyCollider (iDynamicsSystemCollider* collider);
  virtual void AttachCollider (iDynamicsSystemCollider* collider);
  virtual csRef<iDynamicsSystemCollider> CreateCollider ();
  virtual csRef<iDynamicsSystemCollider> GetCollider (unsigned int index);
  virtual int GetColliderCount ();

  void RegisterGimpact ();

  //-- iBulletDynamicSystem
  virtual void DebugDraw (iView* view);
  virtual void SetDebugMode (csBulletDebugMode mode);
  virtual csBulletDebugMode GetDebugMode ();

  virtual bool HitBeam (const csVector3 &start,
			const csVector3 &end,
			csBulletHitBeamResult& result);

  virtual void SetInternalScale (float scale);
  virtual void SetStepParameters (float timeStep, size_t maxSteps,
				  size_t iterations);

  virtual void SetSoftBodyWorld (bool isSoftBodyWorld);
  virtual bool GetSoftBodyWorld ();
  virtual size_t GetSoftBodyCount ();
  virtual iBulletSoftBody* GetSoftBody (size_t index);

  virtual iBulletSoftBody* CreateRope (csVector3 start, csVector3 end,
				       uint segmentCount);
  virtual iBulletSoftBody* CreateCloth (csVector3 corner1, csVector3 corner2,
					csVector3 corner3, csVector3 corner4,
					uint segmentCount1, uint segmentCount2,
					bool withDiagonals = false);
  virtual iBulletSoftBody* CreateSoftBody (iGeneralFactoryState* genmeshFactory,
					   const csOrthoTransform& bodyTransform);
  virtual iBulletSoftBody* CreateSoftBody (csVector3* vertices, size_t vertexCount,
					   csTriangle* triangles, size_t triangleCount);
  virtual void RemoveSoftBody (iBulletSoftBody* body);

  virtual csPtr<iBulletPivotJoint> CreatePivotJoint ();
  virtual void RemovePivotJoint (iBulletPivotJoint* joint);

  virtual bool SaveBulletWorld (const char* filename);

  virtual iBulletTerrainCollider* AttachColliderTerrain (csLockedHeightData& heightData,
							 int gridWidth, int gridHeight,
							 csVector3 gridSize,
							 csOrthoTransform& transform,
							 float minimumHeight = 0,
							 float maximumHeight = 0);
  virtual iBulletTerrainCollider* AttachColliderTerrain (iTerrainCell* cell,
							 float minimumHeight = 0,
							 float maximumHeight = 0);
  virtual iBulletTerrainCollider* AttachColliderTerrain (iTerrainSystem* terrain,
							 float minimumHeight = 0,
							 float maximumHeight = 0);
  virtual void DestroyCollider (iBulletTerrainCollider* collider);
};

}
CS_PLUGIN_NAMESPACE_END(Bullet)

#endif //__CS_BULLET_DYNAMICS_H__
