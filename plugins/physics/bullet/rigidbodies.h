/*
  Copyright (C) 2010 Christian Van Brussel, Institute of Information
      and Communication Technologies, Electronics and Applied Mathematics
      at Universite catholique de Louvain, Belgium
      http://www.uclouvain.be/en-icteam.html

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_BULLET_RIGIDBODIES_H__
#define __CS_BULLET_RIGIDBODIES_H__

#include "bullet.h"
#include "common.h"

CS_PLUGIN_NAMESPACE_BEGIN(Bullet)
{

class csBulletRigidBody : public scfImplementationExt2<csBulletRigidBody,
    csObject, ::iRigidBody, iRigidBody>
{
  friend class csBulletMotionState;
  friend class csBulletKinematicMotionState;
  friend class csBulletDynamicsSystem;
  friend class csBulletCollider;
  friend class csBulletJoint;
  friend class csBulletPivotJoint;
  friend class csBulletSoftBody;

  CS::Physics::Bullet::BodyType bodyType;

  csBulletDynamicsSystem* dynSys;
  btRigidBody* body;
  csBulletMotionState* motionState;
  csRefArray<csBulletCollider> colliders;

  csRef<iDynamicsMoveCallback> moveCb;
  csRef<iDynamicsCollisionCallback> collCb;
  csRef<iKinematicCallback> kinematicCb;

  btCompoundShape* compoundShape;
  CS::Physics::Bullet::BodyState dynamicState;
  bool customMass;
  float mass;
  bool compoundChanged;
  bool insideWorld;
  float linearDampening;
  float angularDampening;

  csRef<iMeshWrapper> mesh;
  csRef<iLight> light;
  csRef<iCamera> camera;

  csArray<btCollisionObject*> contactObjects;
  csArray<btCollisionObject*> lastContactObjects;

  void RebuildBody ();

public: 
  csBulletRigidBody (csBulletDynamicsSystem* dynSys, bool isStatic = false);
  virtual ~csBulletRigidBody ();

  //-- CS::Physics::Bullet::iBody
  virtual CS::Physics::Bullet::BodyType GetType () const
  { return bodyType; }
  virtual ::iRigidBody* QueryRigidBody ()
  { return this; }
  virtual CS::Physics::Bullet::iSoftBody* QuerySoftBody ()
  { return 0; }
  virtual CS::Physics::Bullet::iTerrainCollider* QueryTerrainCollider ()
  { return 0; }

  //-- iRigidBody
  virtual iObject* QueryObject (void) { return (iObject*) this; }

  virtual bool MakeStatic (void);
  virtual bool MakeDynamic (void);
  virtual bool IsStatic (void);
  virtual bool Disable (void);
  virtual bool Enable (void);
  virtual bool IsEnabled (void);  

  virtual csRef<iBodyGroup> GetGroup (void);

  virtual bool AttachColliderConvexMesh (iMeshWrapper* mesh,
    const csOrthoTransform& trans, float friction, float density,
    float elasticity, float softness = 0.01f);

  virtual bool AttachColliderMesh (iMeshWrapper* mesh,
    const csOrthoTransform& trans, float friction, float density,
    float elasticity, float softness = 0.01f);

  virtual bool AttachColliderCylinder (float length, float radius,
    const csOrthoTransform& trans, float friction, float density,
    float elasticity, float softness = 0.01f);

  virtual bool AttachColliderCapsule (float length, float radius,
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
  virtual void AttachLight (iLight* light);
  virtual iLight* GetAttachedLight ();
  virtual void AttachCamera (iCamera* camera);
  virtual iCamera* GetAttachedCamera ();

  virtual void Update ();

  virtual void SetMoveCallback (iDynamicsMoveCallback* cb);
  virtual void SetCollisionCallback (iDynamicsCollisionCallback* cb);
  virtual void Collision (::iRigidBody *other, const csVector3& pos,
      const csVector3& normal, float depth);
  virtual csRef<iDynamicsSystemCollider> GetCollider (unsigned int index);
  virtual int GetColliderCount ();

  //-- CS::Physics::Bullet::iRigidBody
  virtual void MakeKinematic ();
  virtual CS::Physics::Bullet::BodyState GetDynamicState () const;
  virtual void SetDynamicState (CS::Physics::Bullet::BodyState state);

  virtual void SetKinematicCallback (iKinematicCallback* callback);
  virtual iKinematicCallback* GetKinematicCallback ();

  virtual void SetLinearDampener (float d);
  virtual float GetLinearDampener () const;
  virtual void SetRollingDampener (float d);
  virtual float GetRollingDampener () const;
};

/**
 * This is the implementation for a default dynamics move callback.
 * It can update the position of the attached mesh, light or camera.
 */
class csBulletDefaultMoveCallback : public scfImplementation1<
  csBulletDefaultMoveCallback, iDynamicsMoveCallback>
{
public:
  csBulletDefaultMoveCallback ();
  virtual ~csBulletDefaultMoveCallback ();

  void Execute (iMovable* movable, csOrthoTransform& t);
  virtual void Execute (iMeshWrapper* mesh, csOrthoTransform& t);
  virtual void Execute (iLight* light, csOrthoTransform& t);
  virtual void Execute (iCamera* camera, csOrthoTransform& t);
  virtual void Execute (csOrthoTransform& t);
};

/**
 * This is the implementation for a default kinematics move callback.
 * It can update the position of the rigid body from the state of the
 * attached mesh, light or camera.
 */
class csBulletDefaultKinematicCallback : public scfImplementation1<
  csBulletDefaultKinematicCallback, iKinematicCallback>
{
public:
  csBulletDefaultKinematicCallback ();
  virtual ~csBulletDefaultKinematicCallback ();

  virtual void GetBodyTransform (::iRigidBody* body, csOrthoTransform& transform) const;
};

}
CS_PLUGIN_NAMESPACE_END(Bullet)

#endif //__CS_BULLET_RIGIDBODIES_H__
