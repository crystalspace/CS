/*
    Copyright (C) 2002 Anders Stenberg

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


#ifndef __GAME_ODEDYNAMICS_H__
#define __GAME_ODEDYNAMICS_H__

#include "csgeom/vector3.h"
#include "csutil/csobjvec.h"
#include "csutil/garray.h"
#include "csgeom/transfrm.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/skelbone.h"
#include "iutil/comp.h"
#include "ode/ode.h"
#include "ivaria/dynamics.h"

struct iObjectRegistry;
struct iMeshWrapper;
struct iSkeletonBone;



////////////////////////////////////////////
// NOTE
// There is lots of unused stuff in here involving iCollideSystem
// It was put there when I built the base for making mesh colliders
// The reason I left it in is because it does quite a lot of useful
// stuff conserning custom colliders in ODE
///////////////////////////////////////////



struct colliderdata
{
public:
  iCollideSystem* collsys;
  iCollider* collider;
  float friction;
  dReal aabb[6];

  colliderdata (iCollideSystem* cs, iCollider* c, float f ) 
  {
    collsys=cs; SCF_INC_REF(cs); collider=c; SCF_INC_REF(c); friction=f;
    aabb[0]=-dInfinity;	aabb[1]=dInfinity;	
    aabb[2]=-dInfinity;	aabb[3]=dInfinity;	
    aabb[4]=-dInfinity;	aabb[5]=dInfinity;
  }
  ~colliderdata() 
  {
    SCF_DEC_REF(collsys);
    SCF_DEC_REF(collider);
  }
};

CS_TYPEDEF_GROWING_ARRAY (csGeomList, dGeomID);

/**
 * This is the implementation for the actual plugin.
 * It is responsible for creating iDynamicSystem.
 */
class csODEDynamics : public iDynamics
{
private:
  iObjectRegistry* object_reg;

  static int geomclassnum;
  static dJointGroupID contactjoints;

  csObjVector systems;

public:
  SCF_DECLARE_IBASE;

  csODEDynamics (iBase* parent);
  virtual ~csODEDynamics ();
  bool Initialize (iObjectRegistry* object_reg);

  static int GetGeomClassNum() { return geomclassnum; }

  iDynamicSystem* CreateSystem ();
  void RemoveSystem (iDynamicSystem* system);

  void Step (float stepsize);

  static void NearCallback (void *data, dGeomID o1, dGeomID o2);
  static int CollideFunction (dGeomID o1, dGeomID o2, int flags,
  	dContactGeom *contact, int skip);
  static dColliderFn* CollideSelector (int num)
  {
    if (num==geomclassnum)
      return &CollideFunction;
    else
      return NULL;
  }
  static void GetAABB (dGeomID g, dReal aabb[6]);

  struct Component : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csODEDynamics);
    virtual bool Initialize (iObjectRegistry* object_reg)
    {
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;
};

/**
 * This is the implementation for the dynamics core.
 * It handles all bookkeeping for rigid bodies and joints.
 * It also handles collision response.
 * Collision detection is done in another plugin.
 */
class csODEDynamicSystem : public iDynamicSystem
{
private:
  dWorldID worldID;
  dSpaceID spaceID;

  iCollideSystem* collidesys;

  csObjVector bodies;

public:
  SCF_DECLARE_IBASE;

  csODEDynamicSystem ();
  virtual ~csODEDynamicSystem ();

  dWorldID GetWorldID() { return worldID; }
  dSpaceID GetSpaceID() { return spaceID; }
  iCollideSystem* GetCollideSystem() { return collidesys; }

  void SetGravity (const csVector3& v);
  const csVector3 GetGravity () const;

  void Step (float stepsize);

  iRigidBody* CreateBody ();
  void RemoveBody (iRigidBody* body);
};

/**
 * This is the implementation for a rigid body.
 * It keeps all properties for the body.
 * It can also be attached to a movable or a bone,
 * to automatically update it.
 */
class csODERigidBody : public iRigidBody
{
private:
  dBodyID bodyID;
  dGeomID groupID;
  csGeomList ids;
  dJointID statjoint;

  csODEDynamicSystem* dynsys;

  iMeshWrapper* mesh;
  iSkeletonBone* bone;

public:
  SCF_DECLARE_IBASE;

  csODERigidBody (csODEDynamicSystem* sys);
  virtual ~csODERigidBody ();

  inline dBodyID GetID() { return bodyID; }

  bool MakeStatic (void);
  bool MakeDynamic (void);

  bool AttachColliderMesh (iPolygonMesh* mesh,
  	csOrthoTransform& trans, float friction, float density,
	float elasticity);
  bool AttachColliderBox (csVector3 size,
  	csOrthoTransform& trans, float friction, float density,
	float elasticity);
  bool AttachColliderSphere (float radius, csVector3 offset,
  	float friction, float density, float elasticity);

  void SetPosition (const csVector3& trans);
  const csVector3 GetPosition () const;
  void SetOrientation (const csMatrix3& trans);
  const csMatrix3 GetOrientation () const;
  void SetTransform (const csOrthoTransform& trans);
  const csOrthoTransform GetTransform () const;
  void SetLinearVelocity (const csVector3& vel);
  const csVector3 GetLinearVelocity () const;
  void SetAngularVelocity (const csVector3& vel);
  const csVector3 GetAngularVelocity () const;

  void SetProperties (float mass, const csVector3& center,
  	const csMatrix3& inertia);
  void GetProperties (float* mass, csVector3* center,
    csMatrix3* inertia);
  void AdjustTotalMass (float targetmass);

  void AddForce (const csVector3& force);
  void AddTorque (const csVector3& force);
  void AddRelForce (const csVector3& force);
  void AddRelTorque (const csVector3& force) ;
  void AddForceAtPos (const csVector3& force, const csVector3& pos);
  void AddForceAtRelPos (const csVector3& force, const csVector3& pos);
  void AddRelForceAtPos (const csVector3& force, const csVector3& pos);
  void AddRelForceAtRelPos (const csVector3& force, const csVector3& pos);

  const csVector3 GetForce () const;
  const csVector3 GetTorque () const;

  /*const csVector3 GetRelForce () const;
  const csVector3 GetRelTorque () const;*/

  //int GetJointCount () const = 0;

  void AttachMesh (iMeshWrapper* mesh);
  void AttachBone (iSkeletonBone* bone);

  void Update ();
};


#endif // __GAME_ODEDYNAMICS_H__

