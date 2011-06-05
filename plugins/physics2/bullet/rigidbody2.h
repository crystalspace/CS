#ifndef __CS_BULLET_RIGIDBODY_H__
#define __CS_BULLET_RIGIDBODY_H__

#include "bullet2.h"
#include "common2.h"

CS_PLUGIN_NAMESPACE_BEGIN (Bullet2)
{
class csBulletPhysicalSystem;

class csBulletRigidBody : public scfImplementationExt1<
  csBulletRigidBody, csBulletCollisionObject, 
  CS::Physics::iRigidBody>
{
private:
  btRigidBody* btBody;
  //CS::Physics::PhysicalBodyType bodyType;
  CS::Physics::RigidBodyState physicalState;
  float density;
  float linearDampening;
  float angularDampening;
  float friction;
  float softness;
  float elasticity;

  csRef<iKinematicCallback> kinematicCb;

public:
  csBulletRigidBody (csBulletSystem* phySys);
  ~csBulletRigidBody ();

  //iCollisionObject

  //virtual void SetTransform (const csOrthoTransform& trans);
  //virtual csOrthoTransform GetTransform ();

  virtual void RebuildObject ();

  //virtual bool Collide (iCollisionObject* otherObject);
  //virtual HitBeamResult HitBeam (const csVector3& start, const csVector3& end);

  btRigidBody* GetBulletRigidPointer () {return btBody;}
  virtual void RemoveBulletObject ();
  virtual void AddBulletObject ();

  //iPhysicalBody

  virtual PhysicalBodyType GetBodyType () {return BODY_RIGID;}
  virtual iRigidBody* QueryRigidBody () {return this;}
  virtual iSoftBody* QuerySoftBody () {return NULL;}

  virtual bool Disable ();
  virtual bool Enable ();
  virtual bool IsEnabled ();

  virtual float GetMass ();

  virtual float GetDensity () {return density;}
  virtual void SetDensity (float density);

  virtual float GetVolume ();

  virtual void AddForce (const csVector3& force);

  virtual void SetLinearVelocity (const csVector3& vel);
  virtual csVector3 GetLinearVelocity (size_t index = 0) const;

  virtual void SetFriction (float friction);
  virtual float GetFriction () {return friction;}

  //iRigidBody
  virtual iCollisionObject* QueryCollisionObject () {return dynamic_cast<iPhysicalBody*> (this);}

  virtual RigidBodyState GetState () {return physicalState;}
  virtual bool SetState (RigidBodyState state);

  virtual void SetElasticity (float elasticity);
  virtual float GetElasticity () {return elasticity;}

  virtual void SetAngularVelocity (const csVector3& vel);
  virtual csVector3 GetAngularVelocity () const;

  virtual void AddTorque (const csVector3& torque);

  virtual void AddRelForce (const csVector3& force);
  virtual void AddRelTorque (const csVector3& torque);

  virtual void AddForceAtPos (const csVector3& force,
      const csVector3& pos);
  virtual void AddForceAtRelPos (const csVector3& force,
      const csVector3& pos);

  virtual void AddRelForceAtPos (const csVector3& force,
      const csVector3& pos);
  virtual void AddRelForceAtRelPos (const csVector3& force,
      const csVector3& pos);

  virtual csVector3 GetForce () const;
  virtual csVector3 GetTorque () const;

  virtual void SetKinematicCallback (iKinematicCallback* cb) {kinematicCb = cb;}
  virtual iKinematicCallback* GetKinematicCallback () {return kinematicCb;}

  virtual void SetLinearDampener (float d);
  virtual float GetLinearDampener () {return linearDampening;}

  virtual void SetRollingDampener (float d);
  virtual float GetRollingDampener () {return angularDampening;}
};
}
CS_PLUGIN_NAMESPACE_END (Bullet2)

#endif