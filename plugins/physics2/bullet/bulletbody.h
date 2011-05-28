#ifndef __CS_BULLET_BODY_H__
#define __CS_BULLET_BODY_H__

#include "bulletphysics.h"
#include "common.h"

CS_PLUGIN_NAMESPACE_BEGIN (Bullet2)
{
class csBulletPhysicalSystem;

class csBulletRigidBody : public scfImplementationExt1<
  csBulletRigidBody, csBulletCollisionObject, 
  CS::Physics::iRigidBody>
{
private:
  btRigidBody* btBody;
public:
  csBulletRigidBody (iPhysicalSystem* phySys);
  ~csBulletRigidBody ();

  //iCollisionObject

  virtual void SetTransform (const csOrthoTransform& trans);
  virtual csOrthoTransform GetTransform ();

  virtual void RebuildObject ();

  //virtual bool Collide (iCollisionObject* otherObject);
  //virtual HitBeamResult HitBeam (const csVector3& start, const csVector3& end);

  btRigidBody* GetBulletRigidPointer () {return btBody;}
  virtual void RemoveBulletObject ();

  //iPhysicalBody

  virtual PhysicalBodyType GetBodyType () {return BODY_RIGID;}
  virtual iRigidBody* QueryRigidBody () {return this;}
  virtual iSoftBody* QuerySoftBody () {return NULL;}

  virtual bool Disable ();
  virtual bool Enable ();
  virtual bool IsEnabled ();

  virtual float GetMass ();
  virtual void SetMass (float mass);

  virtual float GetDensity ();
  virtual void SetDensity (float density);

  virtual float GetVolume ();

  virtual void AddForce (const csVector3& force);

  virtual void SetLinearVelocity (const csVector3& vel);
  virtual csVector3 GetLinearVelocity (size_t index = 0);

  virtual void SetFriction (float friction);
  virtual void GetFriction (float& friction);

  //iRigidBody
  virtual iCollisionObject* QueryCollisionObject ();

  virtual RigidBodyState GetState ();
  virtual void SetState (RigidBodyState state);

  virtual void SetElasticity (float elasticity);
  virtual void GetElasticity (float elasticity);

  virtual void SetAngularVelocity (const csVector3& vel);
  virtual csVector3 GetAngularVelocity () const;

  virtual void AddTorque (const csVector3& force);

  virtual void AddRelForce (const csVector3& force);
  virtual void AddRelTorque (const csVector3& force);

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

  virtual void SetKinematicCallback (iKinematicCallback* cb);
  virtual iKinematicCallback* GetKinematicCallback ();

  virtual void SetLinearDampener (float d);
  virtual float GetLinearDampener ();

  virtual void SetRollingDampener (float d);
  virtual float GetRollingDampener ();
};

class csBulletSoftBody : scfImplementationExt2<csBulletSoftBody, 
  csBulletCollisionObject, CS::Physics::iSoftBody,
  CS::Physics::Bullet::iSoftBody>
{
private:
  btSoftBody* btBody;
public:
  csBulletSoftBody (iPhysicalSystem* phySys);
  ~csBulletSoftBody ();

  //iCollisionObject

  virtual void SetTransform (const csOrthoTransform& trans);
  virtual csOrthoTransform GetTransform ();

  virtual void RebuildObject ();

  //virtual bool Collide (iCollisionObject* otherObject);
  //virtual HitBeamResult HitBeam (const csVector3& start, const csVector3& end);

  btSoftBody* GetBulletSoftPointer () {return btBody;}
  virtual void RemoveBulletObject ();

  //iPhysicalBody

  virtual PhysicalBodyType GetBodyType () {return BODY_SOFT;}
  virtual iRigidBody* QueryRigidBody () {return NULL;}
  virtual iSoftBody* QuerySoftBody () {return this;}

  virtual bool Disable ();
  virtual bool Enable ();
  virtual bool IsEnabled ();

  virtual float GetMass ();
  virtual void SetMass (float mass);

  virtual float GetDensity ();
  virtual void SetDensity (float density);

  virtual float GetVolume ();

  virtual void AddForce (const csVector3& force);

  virtual void SetLinearVelocity (const csVector3& vel);
  virtual csVector3 GetLinearVelocity (size_t index = 0);

  virtual void SetFriction (float friction);
  virtual void GetFriction (float& friction);

  //iSoftBody
  virtual void SetVertexMass (float mass, size_t index);
  virtual float GetVertexMass (size_t index);

  virtual size_t GetVertexCount ();
  virtual csVector3 GetVertexPosition (size_t index) const;

  virtual void AnchorVertex (size_t vertexIndex);
  virtual void AnchorVertex (size_t vertexIndex,
      iRigidBody* body);
  virtual void AnchorVertex (size_t vertexIndex,
      iAnchorAnimationControl* controller);

  virtual void UpdateAnchor (size_t vertexIndex,
      csVector3& position);
  virtual void RemoveAnchor (size_t vertexIndex);

  virtual void SetRigidity (float rigidity);
  virtual float GetRidigity ();

  virtual void SetLinearVelocity (const csVector3& velocity,
      size_t vertexIndex);

  virtual void setWindVelocity (const csVector3& velocity);
virtual const csVector3 getWindVelocity () const;

  virtual void AddForce (const csVector3& force, size_t vertexIndex);

  virtual size_t GetTriangleCount ();

  virtual csTriangle GetTriangle (size_t index) const;

  virtual csVector3 GetVertexNormal (size_t index) const;

  //Bullet::iSoftBody

  virtual void DebugDraw (iView* rView);

  virtual void SetLinearStiff (float stiff);
  virtual void SetAngularStiff (float stiff);
  virtual void SetVolumeStiff (float vol);

  virtual void ResetCollisionFlag ();

  virtual void SetClusterCollisionRS (bool cluster);
  virtual void SetClusterCollisionSS (bool cluster);

  virtual void SetSRHardness (float hardness);
  virtual void SetSKHardness (float hardness);
  virtual void SetSSHardness (float hardness);

  virtual void SetSRImpulse (float impulse);
  virtual void SetSKImpulse (float impulse);
  virtual void SetSSImpulse (float impulse);

  virtual void SetVeloCorrectionFactor (float factor);

  virtual void SetDamping (float damping);
  virtual void SetDrag (float drag);
  virtual void SetLift (float lift);
  virtual void SetPressure (float pressure);

  virtual void SetVolumeConversationCoefficient (float conversation);
  virtual void SetShapeMatchThreshold (float matching);

  virtual void SetRContactsHardness (float hardness);
  virtual void SetKContactsHardness (float hardness);
  virtual void SetSContactsHardness (float hardness);
  virtual void SetAnchorsHardness (float hardness);

  virtual void SetVeloSolverIterations (int iter);
  virtual void SetPositionIterations (int iter);
  virtual void SetDriftIterations (int iter);
  virtual void SetClusterIterations (int iter);

  virtual void SetShapeMatching (bool match);
};
}
CS_PLUGIN_NAMESPACE_END (Bullet2)

#endif