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

#define int8 ode_int8
#define uint8 ode_uint8
#define int32 ode_int32
#define uint32 ode_uint32
#include <ode/ode.h>
#undef ode_uint32
#undef ode_int32
#undef ode_uint8
#undef ode_int8

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
  csRef<iObjectRegistry> object_reg;

  static int geomclassnum;
  static dJointGroupID contactjoints;

  csObjVector systems;

public:
  SCF_DECLARE_IBASE;

  csODEDynamics (iBase* parent);
  virtual ~csODEDynamics ();
  bool Initialize (iObjectRegistry* object_reg);

  static int GetGeomClassNum() { return geomclassnum; }

  virtual csPtr<iDynamicSystem> CreateSystem ();
  virtual void RemoveSystem (iDynamicSystem* system);

  virtual void Step (float stepsize);

  static void NearCallback (void *data, dGeomID o1, dGeomID o2);
  static int CollideMeshMesh (dGeomID mesh1, dGeomID mesh2, int flags,
  	dContactGeom *contact, int skip);
  static int CollideMeshBox (dGeomID mesh, dGeomID box, int flags,
  	dContactGeom *contact, int skip);
  static int CollideMeshCylinder (dGeomID mesh, dGeomID cyl, int flags,
  	dContactGeom *contact, int skip);
  static int CollideMeshSphere (dGeomID mesh, dGeomID sphere, int flags,
  	dContactGeom *contact, int skip);
  static dColliderFn* CollideSelector (int num)
  {
    if (num == geomclassnum)
      return &CollideMeshMesh;
    if (num == dBoxClass)
      return &CollideMeshBox;
    if (num == dCCylinderClass)
      return &CollideMeshCylinder;
    if (num == dSphereClass)
      return &CollideMeshSphere;
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

  csRef<iCollideSystem> collidesys;
  csRef<iDynamicsMoveCallback> move_cb;

  csObjVector bodies;
  csObjVector groups;
  csObjVector joints;

public:
  SCF_DECLARE_IBASE;

  csODEDynamicSystem ();
  virtual ~csODEDynamicSystem ();

  dWorldID GetWorldID() { return worldID; }
  dSpaceID GetSpaceID() { return spaceID; }
  iCollideSystem* GetCollideSystem() { return collidesys; }

  virtual void SetGravity (const csVector3& v);
  virtual const csVector3 GetGravity () const;

  virtual void Step (float stepsize);

  virtual csPtr<iRigidBody> CreateBody ();
  virtual void RemoveBody (iRigidBody* body);

  virtual csPtr<iBodyGroup> CreateGroup ();
  virtual void RemoveGroup (iBodyGroup *group);

  virtual csPtr<iJoint> CreateJoint ();
  virtual void RemoveJoint (iJoint* joint);

  virtual iDynamicsMoveCallback* GetDefaultMoveCallback () { return move_cb; }
};

/**
 * odedynam implementation of iBodyGroup.  This will set a 
 * variable inside the body which will be compared against 
 * inside NearCallback
 */
class csODEBodyGroup : public iBodyGroup
{
  csObjVector bodies;

  csODEDynamicSystem* system;
 
public:
  SCF_DECLARE_IBASE;

  csODEBodyGroup (csODEDynamicSystem *sys); 
  virtual ~csODEBodyGroup ();

  void AddBody (iRigidBody *body);
  void RemoveBody (iRigidBody *body);
  bool BodyInGroup (iRigidBody *body);
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

  /* these must be ptrs to avoid circular referencing */
  iBodyGroup* collision_group;
  csODEDynamicSystem* dynsys;

  csRef<iMeshWrapper> mesh;
  csRef<iSkeletonBone> bone;
  csRef<iDynamicsMoveCallback> move_cb;
  csRef<iDynamicsCollisionCallback> coll_cb;

public:
  SCF_DECLARE_IBASE;

  csODERigidBody (csODEDynamicSystem* sys);
  virtual ~csODERigidBody ();

  inline dBodyID GetID() { return bodyID; }

  bool MakeStatic (void);
  bool IsStatic (void) { return statjoint != 0; }
  bool MakeDynamic (void);

  void SetGroup (iBodyGroup *group);
  void UnsetGroup () { collision_group = NULL; }
  csRef<iBodyGroup> GetGroup (void) { return collision_group; }

  bool AttachColliderMesh (iMeshWrapper* mesh,
  	const csOrthoTransform& trans, float friction, float density,
	float elasticity);
  bool AttachColliderCylinder (float length, float radius,
  	const csOrthoTransform& trans, float friction, float density,
	float elasticity);
  bool AttachColliderBox (const csVector3 &size,
  	const csOrthoTransform& trans, float friction, float density,
	float elasticity);
  bool AttachColliderSphere (float radius, const csVector3 &offset,
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
  csRef<iMeshWrapper> GetAttachedMesh () { return mesh; }
  void AttachBone (iSkeletonBone* bone);
  csRef<iSkeletonBone> GetAttachedBone () { return bone; }
  void SetMoveCallback (iDynamicsMoveCallback* cb);
  void SetCollisionCallback (iDynamicsCollisionCallback* cb);

  void Collision (iRigidBody *other);
  void Update ();
};

/**
 * This implements the joint.  It does this by determining
 * which type of ODE joint best represents that described 
 */
class csODEJoint : public iJoint
{
  dJointID jointID;
  csRef<iRigidBody> body[2];
  dBodyID bodyID[2];

  int transConstraint[3], rotConstraint[3];
  csVector3 maxTrans, minTrans, maxAngle, minAngle;

  csOrthoTransform transform;

  csODEDynamicSystem* dynsys;

public:
  SCF_DECLARE_IBASE;
  
  csODEJoint (csODEDynamicSystem *sys);
  virtual ~csODEJoint ();

  inline dJointID GetID () { return jointID; }

  void Attach (iRigidBody *body1, iRigidBody *body2);
  csRef<iRigidBody> GetAttachedBody (int body);

  void SetTransform (const csOrthoTransform &trans);
  csOrthoTransform GetTransform ();

  void SetTransConstraints (bool X, bool Y, bool Z);
  inline bool IsXTransConstrained () { return transConstraint[0]; }
  inline bool IsYTransConstrained () { return transConstraint[1]; }
  inline bool IsZTransConstrained () { return transConstraint[2]; }
  void SetMinimumDistance (const csVector3 &min);
  csVector3 GetMinimumDistance ();
  void SetMaximumDistance (const csVector3 &max);
  csVector3 GetMaximumDistance ();

  void SetRotConstraints (bool X, bool Y, bool Z);
  inline bool IsXRotConstrained () { return rotConstraint[0]; }
  inline bool IsYRotConstrained () { return rotConstraint[1]; }
  inline bool IsZRotConstrained () { return rotConstraint[2]; }
  void SetMinimumAngle (const csVector3 &min);
  csVector3 GetMinimumAngle ();
  void SetMaximumAngle (const csVector3 &max);
  csVector3 GetMaximumAngle ();

private:
  void BuildHinge (const csVector3 &axis, float min, float max);
  void BuildHinge2 (const csVector3 &axis1, float min1, float max1, 
  	const csVector3 &axis2, float min2, float max2);
  void BuildSlider (const csVector3 &axis, float min, float max);
  void BuildJoint ();

};

/**
 * This is the implementation for a default dynamics move callback.
 * It can update mesh and bone.
 */
class csODEDefaultMoveCallback : public iDynamicsMoveCallback
{
public:
  SCF_DECLARE_IBASE;

  csODEDefaultMoveCallback ();
  virtual ~csODEDefaultMoveCallback ();

  void Execute (iMeshWrapper* mesh, csOrthoTransform& t);
  void Execute (iSkeletonBone* bone, csOrthoTransform& t);
};

#endif // __GAME_ODEDYNAMICS_H__

