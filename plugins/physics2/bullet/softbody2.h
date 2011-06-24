#ifndef __CS_BULLET_SOFTBODY_H__
#define __CS_BULLET_SOFTBODY_H__

#include "bullet2.h"
#include "common2.h"
#include "collisionobject2.h"

class btSoftBody;

CS_PLUGIN_NAMESPACE_BEGIN (Bullet2)
{
using CS::Physics2::iRigidBody;

class csBulletSoftBody : public scfImplementationExt2<csBulletSoftBody, 
  csBulletCollisionObject, CS::Physics2::iSoftBody,
  CS::Physics2::Bullet2::iSoftBody>
{
  friend class csBulletRigidBody;
  friend class csBulletJoint;
private:
  //CS::Physics2::Bullet::BodyType bodyType;
  float friction;
  float density;
  float totalMass;
  bool bending;
  btSoftBody* btBody;   //Don't know if I should add this to rigidbody too.
  struct AnimatedAnchor
  {
    AnimatedAnchor (size_t vertexIndex, CS::Physics2::iAnchorAnimationControl* controller)
      : vertexIndex (vertexIndex), controller (controller) {}

    size_t vertexIndex;
    csRef<CS::Physics2::iAnchorAnimationControl> controller;
    btVector3 position;
  };
  csArray<AnimatedAnchor> animatedAnchors;

public:
  csBulletSoftBody (csBulletSystem* phySys, btSoftBody* body);
  virtual ~csBulletSoftBody ();

  //iCollisionObject
  virtual iCollisionObject* QueryCollisionObject () {return dynamic_cast<csBulletCollisionObject*> (this);}

  virtual void SetObjectType (CS::Collision2::CollisionObjectType type) {}
  virtual CS::Collision2::CollisionObjectType GetObjectType () {return CS::Collision2::COLLISION_OBJECT_PHYSICAL;}

  virtual void SetAttachedMovable (iMovable* movable) {csBulletCollisionObject::SetAttachedMovable (movable);}
  virtual iMovable* GetAttachedMovable () {return csBulletCollisionObject::GetAttachedMovable ();}

  virtual void SetTransform (const csOrthoTransform& trans);
  virtual csOrthoTransform GetTransform ();

  virtual void RebuildObject ();

  // btSoftBody don't use collision shape.
  virtual void AddCollider (CS::Collision2::iCollider* collider, const csOrthoTransform& relaTrans) {}
  virtual void RemoveCollider (CS::Collision2::iCollider* collider) {}
  virtual void RemoveCollider (size_t index) {}

  virtual CS::Collision2::iCollider* GetCollider (size_t index) {return NULL;}
  virtual size_t GetColliderCount () {return 0;}

  virtual void SetCollisionGroup (const char* name) {csBulletCollisionObject::SetCollisionGroup (name);}
  virtual const char* GetCollisionGroup () const {return csBulletCollisionObject::GetCollisionGroup ();}

  virtual void SetCollisionCallback (CS::Collision2::iCollisionCallback* cb) {collCb = cb;}
  virtual CS::Collision2::iCollisionCallback* GetCollisionCallback () {return collCb;}

  virtual bool Collide (iCollisionObject* otherObject) {return csBulletCollisionObject::Collide (otherObject);}
  virtual CS::Collision2::HitBeamResult HitBeam (const csVector3& start, const csVector3& end);

  btSoftBody* GetBulletSoftPointer () {return btBody;}
  virtual void RemoveBulletObject ();
  virtual void AddBulletObject ();

  //iPhysicalBody

  virtual CS::Physics2::PhysicalBodyType GetBodyType () const {return CS::Physics2::PhysicalBodyType::BODY_SOFT;}
  virtual iRigidBody* QueryRigidBody () {return NULL;}
  virtual CS::Physics2::iSoftBody* QuerySoftBody () {return dynamic_cast<CS::Physics2::iSoftBody*>(this);}

  virtual bool Disable ();
  virtual bool Enable ();
  virtual bool IsEnabled ();

  virtual void SetMass (float mass);
  virtual float GetMass ();

  virtual float GetDensity () const {return density;}
  virtual void SetDensity (float density);

  virtual float GetVolume ();

  virtual void AddForce (const csVector3& force);

  virtual csVector3 GetLinearVelocity (size_t index = 0) const;

  virtual void SetFriction (float friction);
  virtual float GetFriction () {return friction;}

  //iSoftBody
  virtual void SetVertexMass (float mass, size_t index);
  virtual float GetVertexMass (size_t index);

  virtual size_t GetVertexCount ();
  virtual csVector3 GetVertexPosition (size_t index) const;

  virtual void AnchorVertex (size_t vertexIndex);
  virtual void AnchorVertex (size_t vertexIndex,
    CS::Physics2::iRigidBody* body);
  virtual void AnchorVertex (size_t vertexIndex,
    CS::Physics2::iAnchorAnimationControl* controller);

  virtual void UpdateAnchor (size_t vertexIndex,
    csVector3& position);
  virtual void RemoveAnchor (size_t vertexIndex);

  virtual float GetRidigity ();
  virtual void SetRigidity (float rigidity);

  virtual void SetLinearVelocity (const csVector3& vel);
  virtual void SetLinearVelocity (const csVector3& velocity,
    size_t vertexIndex);

  virtual void SetWindVelocity (const csVector3& velocity);
  virtual const csVector3 GetWindVelocity () const;

  virtual void AddForce (const csVector3& force, size_t vertexIndex);

  virtual size_t GetTriangleCount ();

  virtual csTriangle GetTriangle (size_t index) const;

  virtual csVector3 GetVertexNormal (size_t index) const;

  //Bullet::iSoftBody

  virtual void DebugDraw (iView* rView);

  virtual void SetLinearStiff (float stiff);
  virtual void SetAngularStiff (float stiff);
  virtual void SetVolumeStiff (float stiff);

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
  virtual void SetBendingConstraint (bool bending);

  virtual void GenerateCluster (int iter);

  void UpdateAnchorPositions ();
  void UpdateAnchorInternalTick (btScalar timeStep);
};
}
CS_PLUGIN_NAMESPACE_END (Bullet2)
#endif