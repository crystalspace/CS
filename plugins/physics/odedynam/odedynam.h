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


#ifndef __CS_ODEDYNAMICS_H__
#define __CS_ODEDYNAMICS_H__

#include "csgeom/vector3.h"
#include "csutil/nobjvec.h"
#include "csutil/garray.h"
#include "csutil/refarr.h"
#include "csutil/hashmap.h"
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
#include "ivaria/ode.h"

struct iObjectRegistry;
struct iMeshWrapper;
struct iSkeletonBone;
class csPolygonTree;

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
  csRef<iCollideSystem> collsys;
  csRef<iCollider> collider;
  float friction;
  dReal aabb[6];

  colliderdata (iCollideSystem* cs, iCollider* c, float f ) 
  {
    collsys=cs; collider=c; friction=f;
    aabb[0]=-dInfinity;	aabb[1]=dInfinity;	
    aabb[2]=-dInfinity;	aabb[3]=dInfinity;	
    aabb[4]=-dInfinity;	aabb[5]=dInfinity;
  }
  ~colliderdata() 
  {
  }
};

typedef csDirtyAccessArray<dGeomID> csGeomList;

struct ContactEntry
{
  dGeomID o1, o2;
  csVector3 point;
  csVector3 dir;
};

struct MeshInfo
{
  iMeshWrapper* mesh;
  csPolygonTree* tree;
};

typedef csDirtyAccessArray<ContactEntry> csContactList;

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

  csRefArrayObject<iDynamicSystem> systems;
  float erp, cfm;

  bool rateenabled;
  float steptime, limittime;
  float total_elapsed;

  bool stepfast;
  int sfiter;
  bool fastobjects;

public:
  SCF_DECLARE_IBASE;

  csODEDynamics (iBase* parent);
  virtual ~csODEDynamics ();
  bool Initialize (iObjectRegistry* object_reg);

  static int GetGeomClassNum() { return geomclassnum; }

  virtual csPtr<iDynamicSystem> CreateSystem ();
  virtual void RemoveSystem (iDynamicSystem* system);
  virtual iDynamicSystem* FindSystem (const char *name);

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
  static dColliderFn* CollideSelector (int num);
  static void GetAABB (dGeomID g, dReal aabb[6]);

  void SetGlobalERP (float erp);
  float GlobalERP () { return erp; }
  void SetGlobalCFM (float cfm);
  float GlobalCFM () { return cfm; }
  void EnableStepFast (bool enable);
  bool StepFastEnabled () { return stepfast; }
  void SetStepFastIterations (int iter);
  int StepFastIterations () { return sfiter; }
  void EnableFrameRate (bool enable) { rateenabled = enable; }
  bool FrameRateEnabled () { return rateenabled; }
  void SetFrameRate (float hz) { steptime = 1.0 / hz; }
  float FrameRate () { return 1.0 / steptime; }
  void SetFrameLimit (float hz) { limittime = 1.0 / hz; }
  float FrameLimit () { return 1.0 / limittime; }
  void EnableFastObjects (bool enable) { fastobjects = enable; }
  bool FastObjectsEnabled () { return false; }

  struct ODEDynamicState : public iODEDynamicState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csODEDynamics);
    void SetGlobalERP (float erp)
    { scfParent->SetGlobalERP (erp); }
    float GlobalERP ()
    { return scfParent->GlobalERP (); }
    void SetGlobalCFM (float cfm) 
    { scfParent->SetGlobalCFM (cfm); }
    float GlobalCFM () 
    { return scfParent->GlobalCFM (); }
    void EnableStepFast (bool enable)
    { scfParent->EnableStepFast (enable); }
    bool StepFastEnabled () 
    { return scfParent->StepFastEnabled (); }
    void SetStepFastIterations (int iter)
    { scfParent->SetStepFastIterations (iter); }
    int StepFastIterations ()
    { return scfParent->StepFastIterations (); }
    void EnableFrameRate (bool enable)
    { scfParent->EnableFrameRate (enable); }
    bool FrameRateEnabled ()
    { return scfParent->FrameRateEnabled (); }
    void SetFrameRate (float hz)
    { scfParent->SetFrameRate (hz); }
    float FrameRate ()
    { return scfParent->FrameRate (); }
    void SetFrameLimit (float hz)
    { scfParent->SetFrameLimit (hz); }
    float FrameLimit ()
    { return scfParent->FrameLimit (); }
    void EnableFastObjects (bool enable)
    { scfParent->EnableFastObjects (enable); }
    bool FastObjectsEnabled ()
    { return scfParent->FastObjectsEnabled (); }
  } scfiODEDynamicState;

  struct Component : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csODEDynamics);
    virtual bool Initialize (iObjectRegistry* object_reg)
    {
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;
};

class csODEBodyGroup;
class csODEJoint;

/**
 * This is the implementation for the dynamics core.
 * It handles all bookkeeping for rigid bodies and joints.
 * It also handles collision response.
 * Collision detection is done in another plugin.
 */
class csODEDynamicSystem : public csObject
{
private:
  dWorldID worldID;
  dSpaceID spaceID;
  float roll_damp, lin_damp;

  csRef<iCollideSystem> collidesys;
  csRef<iDynamicsMoveCallback> move_cb;

  csRefArrayObject<iRigidBody> bodies;
  csRefArray<csODEBodyGroup> groups;
  csRefArray<csODEJoint> joints;

  csGeomList geoms;

  bool rateenabled;
  float steptime, limittime;
  float total_elapsed;

  bool stepfast;
  int sfiter;
  bool fastobjects;

public:
  SCF_DECLARE_IBASE_EXT (csObject);

  struct DynamicSystem : public iDynamicSystem {
    SCF_DECLARE_EMBEDDED_IBASE (csODEDynamicSystem);
    iObject *QueryObject ()
    { return scfParent; }
    void SetGravity (const csVector3 &v)
    { scfParent->SetGravity (v); }
    const csVector3 GetGravity () const 
    { return scfParent->GetGravity (); }
    void SetLinearDampener (float d)
    { scfParent->SetLinearDampener (d); }
    float GetLinearDampener () const 
    { return scfParent->GetLinearDampener (); }
    void SetRollingDampener (float d)
    { scfParent->SetRollingDampener (d); }
    float GetRollingDampener () const 
    { return scfParent->GetRollingDampener (); }
    void Step (float stepsize)
    { scfParent->Step (stepsize); }
    csPtr<iRigidBody> CreateBody ()
    { return scfParent->CreateBody (); }
    void RemoveBody (iRigidBody *body)
    { scfParent->RemoveBody (body); }
    iRigidBody *FindBody (const char *name)
    { return scfParent->FindBody (name); }
    csPtr<iBodyGroup> CreateGroup ()
    { return scfParent->CreateGroup (); }
    void RemoveGroup (iBodyGroup *group)
    { scfParent->RemoveGroup (group); }
    csPtr<iJoint> CreateJoint ()
    { return scfParent->CreateJoint (); }
    void RemoveJoint (iJoint *joint)
    { scfParent->RemoveJoint (joint); }
    iDynamicsMoveCallback* GetDefaultMoveCallback ()
    { return scfParent->GetDefaultMoveCallback (); }
	bool AttachColliderMesh (iMeshWrapper* mesh, const csOrthoTransform& trans,
	    float friction, float elasticity, float softness)
	{ return scfParent->AttachColliderMesh (mesh, trans, friction, elasticity, softness); }
	bool AttachColliderCylinder (float length, float radius,
  	    const csOrthoTransform& trans, float friction, float elasticity, float softness)
    { return scfParent->AttachColliderCylinder (length, radius, trans, friction, elasticity, softness); }
	bool AttachColliderBox (const csVector3 &size, const csOrthoTransform&
	    trans, float friction, float elasticity, float softness)
    { return scfParent->AttachColliderBox (size, trans, friction, elasticity, softness); }
    bool AttachColliderSphere (float radius, const csVector3 &offset,
  	    float friction, float elasticity, float softness)
    { return scfParent->AttachColliderSphere (radius, offset, friction, elasticity, softness); }
    bool AttachColliderPlane (const csPlane3 &plane, float friction,
        float elasticity, float softness)
    { return scfParent->AttachColliderPlane (plane, friction, elasticity, softness); }
  } scfiDynamicSystem;
  friend struct DynamicSystem;

  void SetERP (float erp) { dWorldSetERP (worldID, erp); }
  float ERP () { return dWorldGetERP (worldID); }
  void SetCFM (float cfm) { dWorldSetCFM (worldID, cfm); }
  float CFM () { return dWorldGetCFM (worldID); }
  void EnableStepFast (bool enable) { stepfast = enable; }
  bool StepFastEnabled () { return stepfast; }
  void SetStepFastIterations (int iter) { sfiter = iter; }
  int StepFastIterations () { return sfiter; }
  void EnableFrameRate (bool enable) { rateenabled = enable; }
  bool FrameRateEnabled () { return rateenabled; }
  void SetFrameRate (float hz) { steptime = 1.0 / hz; }
  float FrameRate () { return 1.0 / steptime; }
  void SetFrameLimit (float hz) { limittime = 1.0 / hz; }
  float FrameLimit () { return 1.0 / limittime; }
  void EnableFastObjects (bool enable) { fastobjects = enable; }
  bool FastObjectsEnabled () { return false; }

  struct ODEDynamicSystemState : public iODEDynamicSystemState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csODEDynamicSystem);
    void SetERP (float erp)
    { scfParent->SetERP (erp); }
    float ERP ()
    { return scfParent->ERP (); }
    void SetCFM (float cfm) 
    { scfParent->SetCFM (cfm); }
    float CFM () 
    { return scfParent->CFM (); }
    void EnableStepFast (bool enable)
    { scfParent->EnableStepFast (enable); }
    bool StepFastEnabled () 
    { return scfParent->StepFastEnabled (); }
    void SetStepFastIterations (int iter)
    { scfParent->SetStepFastIterations (iter); }
    int StepFastIterations ()
    { return scfParent->StepFastIterations (); }
    void EnableFrameRate (bool enable)
    { scfParent->EnableFrameRate (enable); }
    bool FrameRateEnabled ()
    { return scfParent->FrameRateEnabled (); }
    void SetFrameRate (float hz)
    { scfParent->SetFrameRate (hz); }
    float FrameRate ()
    { return scfParent->FrameRate (); }
    void SetFrameLimit (float hz)
    { scfParent->SetFrameLimit (hz); }
    float FrameLimit ()
    { return scfParent->FrameLimit (); }
    void EnableFastObjects (bool enable)
    { scfParent->EnableFastObjects (enable); }
    bool FastObjectsEnabled ()
    { return scfParent->FastObjectsEnabled (); }
  } scfiODEDynamicSystemState;


  csODEDynamicSystem (float erp, float cfm);
  virtual ~csODEDynamicSystem ();

  dWorldID GetWorldID() { return worldID; }
  dSpaceID GetSpaceID() { return spaceID; }
  iCollideSystem* GetCollideSystem() { return collidesys; }

  virtual void SetGravity (const csVector3& v);
  virtual const csVector3 GetGravity () const;
  virtual void SetRollingDampener (float d) { roll_damp = (d > 1) ? 1.0 : d; }
  virtual float GetRollingDampener () const { return roll_damp; }
  virtual void SetLinearDampener (float d) { lin_damp = (d > 1) ? 1.0 : d; }
  virtual float GetLinearDampener () const { return lin_damp; }

  virtual void Step (float stepsize);

  virtual csPtr<iRigidBody> CreateBody ();
  virtual void RemoveBody (iRigidBody* body);
  virtual iRigidBody *FindBody (const char *name);

  virtual csPtr<iBodyGroup> CreateGroup ();
  virtual void RemoveGroup (iBodyGroup *group);

  virtual csPtr<iJoint> CreateJoint ();
  virtual void RemoveJoint (iJoint* joint);

  virtual iDynamicsMoveCallback* GetDefaultMoveCallback () { return move_cb; }

  virtual bool AttachColliderMesh (iMeshWrapper* mesh,
  	const csOrthoTransform& trans, float friction, float elasticity, float softness);
  virtual bool AttachColliderCylinder (float length, float radius,
  	const csOrthoTransform& trans, float friction, float elasticity, float softness);
  virtual bool AttachColliderBox (const csVector3 &size,
  	const csOrthoTransform& trans, float friction, float elasticity, float softness);
  virtual bool AttachColliderSphere (float radius, const csVector3 &offset,
  	float friction, float elasticity, float softness);
  virtual bool AttachColliderPlane (const csPlane3 &plane, float friction,
    float elasticity, float softness);
};

class csODERigidBody;

/**
 * odedynam implementation of iBodyGroup.  This will set a 
 * variable inside the body which will be compared against 
 * inside NearCallback
 */
class csODEBodyGroup : public iBodyGroup
{
  csRefArray<iRigidBody> bodies;

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
class csODERigidBody : public csObject
{
private:
  dBodyID bodyID;
  dSpaceID groupID;
  csGeomList geoms;
  dJointID statjoint;

  /* these must be ptrs to avoid circular referencing */
  iBodyGroup* collision_group;
  csODEDynamicSystem* dynsys;

  csRef<iMeshWrapper> mesh;
  csRef<iSkeletonBone> bone;
  csRef<iDynamicsMoveCallback> move_cb;
  csRef<iDynamicsCollisionCallback> coll_cb;

public:
  SCF_DECLARE_IBASE_EXT (csObject);

  struct RigidBody : public iRigidBody 
  {
    SCF_DECLARE_EMBEDDED_IBASE (csODERigidBody);
    iObject *QueryObject ()
    { return scfParent; }
    bool MakeStatic (void) 
    { return scfParent->MakeStatic (); }
    bool MakeDynamic (void)
    { return scfParent->MakeDynamic (); }
    bool IsStatic (void) 
    { return scfParent->IsStatic (); }
    csRef<iBodyGroup> GetGroup (void) 
    { return scfParent->GetGroup (); }
    bool AttachColliderMesh (iMeshWrapper* mesh, const csOrthoTransform& trans, 
	float friction, float density, float elasticity, float softness)
    { return scfParent->AttachColliderMesh (mesh, trans, friction, density, elasticity, softness); }
    bool AttachColliderCylinder (float length, float radius, 
	const csOrthoTransform& trans, float friction, float density, float elasticity, float softness)
    { return scfParent->AttachColliderCylinder (length, radius, trans, friction, density, elasticity, softness); }
    bool AttachColliderBox (const csVector3 &size, 
	const csOrthoTransform& trans, float friction, float density, float elasticity, float softness)
    { return scfParent->AttachColliderBox (size, trans, friction, density, elasticity, softness); }
    bool AttachColliderSphere (float radius, const csVector3 &offset, 
	float friction, float density, float elasticity, float softness)
    { return scfParent->AttachColliderSphere (radius, offset, friction, density, elasticity, softness); }
    bool AttachColliderPlane (const csPlane3 &plane, float friction,
    float density, float elasticity, float softness)
	{ return scfParent->AttachColliderPlane (plane, friction, density, elasticity, softness); }
    void SetPosition (const csVector3& trans) 
    { scfParent->SetPosition (trans); }
    const csVector3 GetPosition () const
    { return scfParent->GetPosition (); }
    void SetOrientation (const csMatrix3& trans) 
    { scfParent->SetOrientation (trans); }
    const csMatrix3 GetOrientation () const 
    { return scfParent->GetOrientation (); }
    void SetTransform (const csOrthoTransform& trans) 
    { scfParent->SetTransform (trans); }
    const csOrthoTransform GetTransform () const 
    { return scfParent->GetTransform (); }
    void SetLinearVelocity (const csVector3& vel) 
    { scfParent->SetLinearVelocity (vel); }
    const csVector3 GetLinearVelocity () const 
    { return scfParent->GetLinearVelocity(); }
    void SetAngularVelocity (const csVector3& vel) 
    { scfParent->SetAngularVelocity (vel); }
    const csVector3 GetAngularVelocity () const 
    { return scfParent->GetAngularVelocity (); }
    void SetProperties (float mass, const csVector3& center, const csMatrix3& inertia) 
    { scfParent->SetProperties (mass, center, inertia); }
    void GetProperties (float* mass, csVector3* center, csMatrix3* inertia) 
    { scfParent->GetProperties (mass, center, inertia); }
    void AdjustTotalMass (float targetmass) 
    { scfParent->AdjustTotalMass (targetmass); }
    void AddForce (const csVector3& force) 
    { scfParent->AddForce (force); }
    void AddTorque (const csVector3& force) 
    { scfParent->AddTorque (force); }
    void AddRelForce (const csVector3& force) 
    { scfParent->AddRelForce (force); }
    void AddRelTorque (const csVector3& force) 
    { scfParent->AddRelTorque (force); }
    void AddForceAtPos (const csVector3& force, const csVector3& pos) 
    { scfParent->AddForceAtPos (force, pos); }
    void AddForceAtRelPos (const csVector3& force,
	const csVector3& pos) 
    { scfParent->AddForceAtRelPos (force, pos); }
    void AddRelForceAtPos (const csVector3& force,
  	const csVector3& pos) 
    { scfParent->AddRelForceAtPos (force, pos); }
    void AddRelForceAtRelPos (const csVector3& force,
  	const csVector3& pos) 
    { scfParent->AddRelForceAtRelPos (force, pos); }
    const csVector3 GetForce () const
    { return scfParent->GetForce (); }
    const csVector3 GetTorque () const
    { return scfParent->GetTorque (); }
    void AttachMesh (iMeshWrapper* mesh) 
    { scfParent->AttachMesh (mesh); }
    csRef<iMeshWrapper> GetAttachedMesh () 
    { return scfParent->GetAttachedMesh (); }
    void AttachBone (iSkeletonBone* bone) 
    { scfParent->AttachBone (bone); }
    csRef<iSkeletonBone> GetAttachedBone () 
    { return scfParent->GetAttachedBone (); }
    void SetMoveCallback (iDynamicsMoveCallback* cb) 
    { scfParent->SetMoveCallback (cb); }
    void SetCollisionCallback (iDynamicsCollisionCallback* cb) 
    { scfParent->SetCollisionCallback (cb); }
    void Collision (iRigidBody *other) 
    { scfParent->Collision (other); }
    void Update () 
    { scfParent->Update (); }
  } scfiRigidBody;
  friend struct RigidBody;

  csODERigidBody (csODEDynamicSystem* sys);
  virtual ~csODERigidBody ();

  inline dBodyID GetID() { return bodyID; }

  bool MakeStatic (void);
  bool IsStatic (void) { return statjoint != 0; }
  bool MakeDynamic (void);

  void SetGroup (iBodyGroup *group);
  void UnsetGroup () { collision_group = 0; }
  csRef<iBodyGroup> GetGroup (void) { return collision_group; }

  bool AttachColliderMesh (iMeshWrapper* mesh,
  	const csOrthoTransform& trans, float friction, float density,
	float elasticity, float softness);
  bool AttachColliderCylinder (float length, float radius,
  	const csOrthoTransform& trans, float friction, float density,
	float elasticity, float softness);
  bool AttachColliderBox (const csVector3 &size,
  	const csOrthoTransform& trans, float friction, float density,
	float elasticity, float softness);
  bool AttachColliderSphere (float radius, const csVector3 &offset,
  	float friction, float density, float elasticity, float softness);
  /// ODE planes are globally transformed, immobile, infinitely dense
  bool AttachColliderPlane (const csPlane3 &plane, float friction,
    float density, float elasticity, float softness);

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
  void Execute (csOrthoTransform& t);
};

#endif // __CS_ODEDYNAMICS_H__

