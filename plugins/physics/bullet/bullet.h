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

CS_PLUGIN_NAMESPACE_BEGIN(Bullet)
{

class csBulletMotionState;
class csBulletDebugDraw;

/**
* This is the implementation for a default dynamics move callback.
* It can update mesh.
*/
class csBulletDefaultMoveCallback : public scfImplementation1<
  csBulletDefaultMoveCallback, iDynamicsMoveCallback>
{
public:
  csBulletDefaultMoveCallback ();
  virtual ~csBulletDefaultMoveCallback ();

  virtual void Execute (iMeshWrapper* mesh, csOrthoTransform& t);
  virtual void Execute (csOrthoTransform& t);
  virtual void Execute (iLight*, csOrthoTransform&) {}
  virtual void Execute (iCamera*, csOrthoTransform&) {}
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
  csRefArray<iDynamicsStepCallback> step_callbacks;

  btCollisionDispatcher* dispatcher;
  btDefaultCollisionConfiguration* configuration;
  btSequentialImpulseConstraintSolver* solver;
  btBroadphaseInterface* broadphase;

public:
  csBulletDynamics (iBase *iParent);
  virtual ~csBulletDynamics ();

  virtual csPtr<iDynamicSystem> CreateSystem ();
  virtual void RemoveSystem (iDynamicSystem* system);
  virtual void RemoveSystems ();
  virtual iDynamicSystem* FindSystem (const char *name);
  virtual void Step (float stepsize);
  virtual void AddStepCallback (iDynamicsStepCallback *callback)
  { step_callbacks.Push (callback); }
  virtual void RemoveStepCallback (iDynamicsStepCallback *callback)
  { step_callbacks.Delete (callback); }

  //  iComponent
  virtual bool Initialize (iObjectRegistry* object_reg);
};

class csBulletDynamicsSystem : public scfImplementationExt2<
  csBulletDynamicsSystem, csObject, iDynamicSystem,
  iBulletDynamicSystem>
{
private:
  btDynamicsWorld* bullet_world;
  csVector3 gravity;
  csRefArrayObject<iRigidBody> bodies;
  csRefArray<iDynamicsSystemCollider> colliders;
  csRef<csBulletDefaultMoveCallback> move_cb;

  // For getting collision mesh data.
  csStringID base_id;
  csStringID colldet_id;

  csBulletDebugDraw* debugDraw;

public:
  csBulletDynamicsSystem (btDynamicsWorld* world,
      iObjectRegistry* object_reg);
  virtual ~csBulletDynamicsSystem ();

  btDynamicsWorld* GetWorld () const { return bullet_world; }

  csStringID GetBaseID () const { return base_id; }
  csStringID GetColldetID () const { return colldet_id; }

  virtual iObject *QueryObject () { return (iObject*)this; }
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
  virtual void RemoveBody (iRigidBody* body);
  virtual iRigidBody *FindBody (const char *name);
  virtual iRigidBody *GetBody (unsigned int index);
  virtual int GetBodysCount ();

  virtual csPtr<iBodyGroup> CreateGroup ();
  virtual void RemoveGroup (iBodyGroup* group);

  virtual csPtr<iJoint> CreateJoint ();
  virtual void RemoveJoint (iJoint* joint);
  virtual iDynamicsMoveCallback* GetDefaultMoveCallback ();

  virtual bool AttachColliderMesh (iMeshWrapper* mesh,
    const csOrthoTransform& trans, float friction,
    float elasticity, float softness = 0.01f);
  virtual bool AttachColliderCylinder (float length, float radius,
    const csOrthoTransform& trans, float friction,
    float elasticity, float softness = 0.01f);
  virtual bool AttachColliderBox (const csVector3 &size,
    const csOrthoTransform& trans, float friction,
    float elasticity, float softness = 0.01f);
  virtual bool AttachColliderSphere (float radius, const csVector3 &offset,
    float friction, float elasticity, float softness = 0.01f);
  virtual bool AttachColliderPlane (const csPlane3 &plane, float friction,
    float elasticity, float softness = 0.01f);
  virtual void DestroyColliders () { colliders.DeleteAll (); }
  virtual void DestroyCollider (iDynamicsSystemCollider* collider)
  { colliders.Delete (collider); }
  virtual void AttachCollider (iDynamicsSystemCollider* collider);
  virtual csRef<iDynamicsSystemCollider> CreateCollider ();
  virtual csRef<iDynamicsSystemCollider> GetCollider (unsigned int index) 
  {
    return csRef<iDynamicsSystemCollider> (colliders[index]);
  }
  virtual int GetColliderCount () 
  {
    return (int) colliders.GetSize ();
  }

  btDynamicsWorld *GetBulletSys () { return bullet_world; }

  virtual void DebugDraw (iView* view);
};

class csBulletRigidBody : public scfImplementationExt1<csBulletRigidBody,
  csObject, iRigidBody>
{
  btRigidBody* body; 
  csRef<iMeshWrapper> mesh;
  csBulletDynamicsSystem* ds;
  float mass;
  csBulletMotionState* motionState;

public: 
  csBulletRigidBody (csBulletDynamicsSystem* ds);
  virtual ~csBulletRigidBody ();

  btRigidBody* GetBulletBody () { return body; }

  virtual iObject* QueryObject (void) { return (iObject*)this; }

  virtual bool MakeStatic (void);
  virtual bool MakeDynamic (void);
  virtual bool IsStatic (void);
  virtual bool Disable (void);
  virtual bool Enable (void);
  virtual bool IsEnabled (void);  

  virtual csRef<iBodyGroup> GetGroup (void);

  virtual bool AttachColliderMesh (iMeshWrapper* mesh,
    const csOrthoTransform& trans, float friction, float density,
    float elasticity, float softness = 0.01f);

  virtual bool AttachColliderCylinder (float length, float radius,
    const csOrthoTransform& trans, float friction, float density,
    float elasticity, float softness = 0.01f);

  virtual bool AttachColliderBox (const csVector3 &size,
    const csOrthoTransform& trans, float friction, float density,
    float elasticity, float softness = 0.01f);

  virtual bool AttachColliderSphere (float radius, const csVector3 &offset,
    float friction, float density, float elasticity,
    float softness = 0.01f);

  virtual bool AttachColliderPlane (const csPlane3 &plane, float friction,
    float density, float elasticity, float softness = 0.01f);

  virtual void AttachCollider (iDynamicsSystemCollider* collider);

  virtual void DestroyColliders ();
  virtual void DestroyCollider (iDynamicsSystemCollider* collider);

  virtual void SetPosition (const csVector3& trans);
  virtual const csVector3 GetPosition () const;
  virtual void SetOrientation (const csMatrix3& trans);
  virtual const csMatrix3 GetOrientation () const;
  virtual void SetTransform (const csOrthoTransform& trans);
  virtual const csOrthoTransform GetTransform () const;
  virtual void SetLinearVelocity (const csVector3& vel);
  virtual const csVector3 GetLinearVelocity () const;
  virtual void SetAngularVelocity (const csVector3& vel);
  virtual const csVector3 GetAngularVelocity () const;

  virtual void SetProperties (float mass, const csVector3& center,
    const csMatrix3& inertia);
  virtual void GetProperties (float* mass, csVector3* center,
    csMatrix3* inertia);

  virtual float GetMass ();
  virtual csVector3 GetCenter ();
  virtual csMatrix3 GetInertia ();

  virtual void AdjustTotalMass (float targetmass);

  virtual void AddForce (const csVector3& force);
  virtual void AddTorque (const csVector3& force);
  virtual void AddRelForce (const csVector3& force);
  virtual void AddRelTorque (const csVector3& force) ;
  virtual void AddForceAtPos (const csVector3& force, const csVector3& pos);
  virtual void AddForceAtRelPos (const csVector3& force,
    const csVector3& pos);
  virtual void AddRelForceAtPos (const csVector3& force,
    const csVector3& pos);
  virtual void AddRelForceAtRelPos (const csVector3& force,
    const csVector3& pos);

  virtual const csVector3 GetForce () const;
  virtual const csVector3 GetTorque () const;

  virtual void AttachMesh (iMeshWrapper* mesh);
  virtual iMeshWrapper* GetAttachedMesh ();

  virtual void Update ();

  virtual void SetMoveCallback (iDynamicsMoveCallback* cb);
  virtual void SetCollisionCallback (iDynamicsCollisionCallback* cb);
  virtual void Collision (iRigidBody *other, const csVector3& pos,
      const csVector3& normal, float depth);
  virtual csRef<iDynamicsSystemCollider> GetCollider (unsigned int index);
  virtual int GetColliderCount ();

  virtual void AttachLight (iLight*) {}
  virtual iLight* GetAttachedLight () { return 0; }
  virtual void AttachCamera (iCamera*) {}
  virtual iCamera* GetAttachedCamera () { return 0; }
};

class csBulletCollider : public scfImplementation1<csBulletCollider,
  iDynamicsSystemCollider>
{
  btRigidBody* body;
  csBulletDynamicsSystem* ds;
  csColliderGeometryType geom_type;
  float mass;
  float friction;
  csBulletMotionState* motionState;
  btCollisionShape* shape;

  // Data we need to keep for the body so we can clean it up
  // later.
  btVector3* vertices;
  int* indices;

public:
  csBulletCollider (csBulletDynamicsSystem* dynsys);
  virtual ~csBulletCollider ();

  btRigidBody* GetBulletController () { return body; }

  virtual bool CreateSphereGeometry (const csSphere& sphere);
  virtual bool CreatePlaneGeometry (const csPlane3& plane);
  virtual bool CreateMeshGeometry (iMeshWrapper *mesh);
  virtual bool CreateBoxGeometry (const csVector3& box_size);
  virtual bool CreateCapsuleGeometry (float length, float radius);

  virtual void SetCollisionCallback (
    iDynamicsColliderCollisionCallback* cb);

  virtual void SetFriction (float friction);
  virtual void SetSoftness (float softness);
  virtual void SetDensity (float density);
  virtual void SetElasticity (float elasticity);
  virtual float GetFriction ();
  virtual float GetSoftness ();
  virtual float GetDensity ();
  virtual float GetElasticity ();

  virtual void FillWithColliderGeometry (
      csRef<iGeneralFactoryState> genmesh_fact);
  virtual csColliderGeometryType GetGeometryType () 
  { return geom_type; }
  virtual csOrthoTransform GetTransform ();
  virtual csOrthoTransform GetLocalTransform ();
  virtual void SetTransform (const csOrthoTransform& trans);
  virtual bool GetBoxGeometry (csVector3& size); 
  virtual bool GetSphereGeometry (csSphere& sphere);
  virtual bool GetPlaneGeometry (csPlane3& plane); 
  virtual bool GetCylinderGeometry (float& length, float& radius);
  virtual void MakeStatic ();
  virtual void MakeDynamic ();
  virtual bool IsStatic ();
};

}
CS_PLUGIN_NAMESPACE_END(Bullet)

#endif //__CS_BULLET_DYNAMICS_H__
