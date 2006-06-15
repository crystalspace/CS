/*
Copyright (C) 2001 by Jorrit Tyberghein
Piotr Obrzut

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


/**
* This is the implementation for the actual plugin.
* It is responsible for creating iDynamicSystem.
*/
class csBulletDynamics : public scfImplementation2<csBulletDynamics, iDynamics, iComponent>
{
private:

  iObjectRegistry* object_reg;
  csRefArrayObject<iDynamicSystem> systems;

public:

  csBulletDynamics (iBase *iParent);
  virtual ~csBulletDynamics ();

  csPtr<iDynamicSystem> CreateSystem ();

  void RemoveSystem (iDynamicSystem* system);

  void RemoveSystems ();

  iDynamicSystem* FindSystem (const char *name);

  void Step (float stepsize);

  // -- iComponent
  virtual bool Initialize (iObjectRegistry* object_reg);

};

class csBulletDynamicsSystem : public scfImplementationExt1<csBulletDynamicsSystem, csObject, iDynamicSystem>
{
  CcdPhysicsEnvironment *bullet_sys;
  csVector3 gravity;
  csRefArrayObject<iRigidBody> bodies;
  csRefArray<iDynamicsSystemCollider> colliders;

public:

  csBulletDynamicsSystem ();
  virtual ~csBulletDynamicsSystem ();

  iObject *QueryObject ()
  {return (iObject*)this;}
  void SetGravity (const csVector3& v);
  const csVector3 GetGravity () const;
  void SetLinearDampener (float d);
  float GetLinearDampener () const;
  void SetRollingDampener (float d);
  float GetRollingDampener () const;
  void EnableAutoDisable (bool enable);
  bool AutoDisableEnabled ();
  void SetAutoDisableParams (float linear, float angular, int steps,
    float time);
  void Step (float stepsize);

  csPtr<iRigidBody> CreateBody ();
  void RemoveBody (iRigidBody* body);
  iRigidBody *FindBody (const char *name);
  iRigidBody *GetBody (unsigned int index);
  int GetBodysCount ();

  csPtr<iBodyGroup> CreateGroup ();
  void RemoveGroup (iBodyGroup* group);

  csPtr<iJoint> CreateJoint ();
  void RemoveJoint (iJoint* joint);
  iDynamicsMoveCallback* GetDefaultMoveCallback ();

  bool AttachColliderMesh (iMeshWrapper* mesh,
    const csOrthoTransform& trans, float friction,
    float elasticity, float softness = 0.01f);
  bool AttachColliderCylinder (float length, float radius,
    const csOrthoTransform& trans, float friction,
    float elasticity, float softness = 0.01f);
  bool AttachColliderBox (const csVector3 &size,
    const csOrthoTransform& trans, float friction,
    float elasticity, float softness = 0.01f);
  bool AttachColliderSphere (float radius, const csVector3 &offset,
    float friction, float elasticity, float softness = 0.01f);
  bool AttachColliderPlane (const csPlane3 &plane, float friction,
    float elasticity, float softness = 0.01f);
  void DestroyColliders ()
  {colliders.DeleteAll ();}
  void DestroyCollider (iDynamicsSystemCollider* collider)
  {colliders.Delete (collider);}
  void AttachCollider (iDynamicsSystemCollider* collider);
  csRef<iDynamicsSystemCollider> CreateCollider ();
  csRef<iDynamicsSystemCollider> GetCollider (unsigned int index) 
  {return colliders[index];}
  int GetColliderCount () 
  {return (int) colliders.GetSize ();}

  CcdPhysicsEnvironment *GetBulletSys () 
  {return bullet_sys;}
};

/**
* This is the implementation for a default dynamics move callback.
* It can update mesh.
*/
class csBulletDefaultMoveCallback : 
  public scfImplementation1<csBulletDefaultMoveCallback, iDynamicsMoveCallback>
{
public:

  csBulletDefaultMoveCallback ();
  virtual ~csBulletDefaultMoveCallback ();

  void Execute (iMeshWrapper* mesh, csOrthoTransform& t);
  void Execute (csOrthoTransform& t);
  void Execute (iLight*, csOrthoTransform&) {}
  void Execute (iCamera*, csOrthoTransform&) {}
};

class	MyMotionState : public PHY_IMotionState

{
public:
  MyMotionState();

  virtual ~MyMotionState();

  virtual void	getWorldPosition(float& posX,float& posY,float& posZ);
  virtual void	getWorldScaling(float& scaleX,float& scaleY,float& scaleZ);
  virtual void	getWorldOrientation(float& quatIma0,float& quatIma1,float& quatIma2,float& quatReal);

  virtual void	setWorldPosition(float posX,float posY,float posZ);
  virtual	void	setWorldOrientation(float quatIma0,float quatIma1,float quatIma2,float quatReal);

  virtual	void	calculateWorldTransformations();

  SimdTransform	m_worldTransform;

};

class csBulletRigidBody : public scfImplementationExt1<csBulletRigidBody, csObject, iRigidBody>
{
  CcdPhysicsController *pc; 
  MyMotionState *ms;
  csRef<iMeshWrapper> mesh;
  csRef<iDynamicsMoveCallback> move_cb;
  csBulletDynamicsSystem* ds;

  float mass;

public: 

  csBulletRigidBody (csBulletDynamicsSystem* ds);
  virtual ~csBulletRigidBody ();

  CcdPhysicsController *GetBulletBody () {return pc;}

  iObject *QueryObject (void)
  {return (iObject*)this;}

  bool MakeStatic (void);
  bool MakeDynamic (void);
  bool IsStatic (void);
  bool Disable (void);
  bool Enable (void);
  bool IsEnabled (void);  

  csRef<iBodyGroup> GetGroup (void);

  bool AttachColliderMesh (iMeshWrapper* mesh,
    const csOrthoTransform& trans, float friction, float density,
    float elasticity, float softness = 0.01f);

  bool AttachColliderCylinder (float length, float radius,
    const csOrthoTransform& trans, float friction, float density,
    float elasticity, float softness = 0.01f);

  bool AttachColliderBox (const csVector3 &size,
    const csOrthoTransform& trans, float friction, float density,
    float elasticity, float softness = 0.01f);

  bool AttachColliderSphere (float radius, const csVector3 &offset,
    float friction, float density, float elasticity,
    float softness = 0.01f);

  bool AttachColliderPlane (const csPlane3 &plane, float friction,
    float density, float elasticity, float softness = 0.01f);

  void AttachCollider (iDynamicsSystemCollider* collider);

  void DestroyColliders ();
  void DestroyCollider (iDynamicsSystemCollider* collider);

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

  float GetMass ();

  csVector3 GetCenter ();

  csMatrix3 GetInertia ();

  void AdjustTotalMass (float targetmass);

  void AddForce (const csVector3& force);

  void AddTorque (const csVector3& force);

  void AddRelForce (const csVector3& force);

  void AddRelTorque (const csVector3& force) ;

  void AddForceAtPos (const csVector3& force, const csVector3& pos);

  void AddForceAtRelPos (const csVector3& force,
    const csVector3& pos);

  void AddRelForceAtPos (const csVector3& force,
    const csVector3& pos);

  void AddRelForceAtRelPos (const csVector3& force,
    const csVector3& pos);

  const csVector3 GetForce () const;

  const csVector3 GetTorque () const;

  void AttachMesh (iMeshWrapper* mesh);

  void Update ();

  iMeshWrapper* GetAttachedMesh ();

  void SetMoveCallback (iDynamicsMoveCallback* cb);
  void SetCollisionCallback (iDynamicsCollisionCallback* cb);
  void Collision (iRigidBody *other);
  csRef<iDynamicsSystemCollider> GetCollider (unsigned int index);
  int GetColliderCount ();

  void AttachLight (iLight*) {};
  iLight* GetAttachedLight () {return NULL;}
  void AttachCamera (iCamera*) {}
  iCamera* GetAttachedCamera (){return NULL;}

protected:

  void ResetShape ();
};

class csBulletCollider : public scfImplementation1<csBulletCollider, iDynamicsSystemCollider>
{
  CcdPhysicsController* pc;
  csBulletDynamicsSystem* ds;
  csColliderGeometryType geom_type;
  MyMotionState *ms;

public:

  csBulletCollider (csBulletDynamicsSystem* dynsys);
  virtual ~csBulletCollider ();

  CcdPhysicsController* GetBulletController () {return pc;}

  bool CreateSphereGeometry (const csSphere& sphere);
  bool CreatePlaneGeometry (const csPlane3& plane);
  bool CreateMeshGeometry (iMeshWrapper *mesh);
  bool CreateBoxGeometry (const csVector3& box_size);
  bool CreateCCylinderGeometry (float length, float radius);

  void SetCollisionCallback (
    iDynamicsColliderCollisionCallback* cb);

  void SetFriction (float friction);
  void SetSoftness (float softness);
  void SetDensity (float density);
  void SetElasticity (float elasticity);
  float GetFriction ();
  float GetSoftness ();
  float GetDensity ();
  float GetElasticity ();

  void FillWithColliderGeometry (csRef<iGeneralFactoryState> genmesh_fact);
  csColliderGeometryType GetGeometryType () 
  {return geom_type;};
  csOrthoTransform GetTransform ();
  csOrthoTransform GetLocalTransform ();
  void SetTransform (const csOrthoTransform& trans);
  bool GetBoxGeometry (csVector3& size); 
  bool GetSphereGeometry (csSphere& sphere);
  bool GetPlaneGeometry (csPlane3& plane); 
  bool GetCylinderGeometry (float& length, float& radius);
  void MakeStatic ();
  void MakeDynamic ();
  bool IsStatic ();

protected:
    void ResetShape ();
};

#endif //__CS_BULLET_DYNAMICS_H__
