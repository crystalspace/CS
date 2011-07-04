#ifndef __CS_BULLET_COLLISIONACTOR_H__
#define __CS_BULLET_COLLISIONACTOR_H__

#include "bullet2.h"
#include "common2.h"
#include "collisionobject2.h"

CS_PLUGIN_NAMESPACE_BEGIN (Bullet2)
{

class csBulletCollisionActor : public scfImplementationExt1<
  csBulletCollisionActor, csBulletCollisionObject, 
  CS::Collision2::iCollisionActor>
{
private:
  float verticalVelocity;
  float verticalOffset;
  float fallSpeed;
  float jumpSpeed;
  float speed;
  float maxJumpHeight;

  float maxSlopeRadians;
  float maxSlopeCosine;

  int upAxis;
  bool touchingContact;
  bool wasOnGround;
  bool wasJumping;

  csWeakRef<iCamera> camera;

  csVector3 currentPosition;
  csVector3 targetPosition;

  bool RecoverFromPenetration ();
  void StepUp ();
  void StepForwardAndStrafe (float dt);
  void StepDown (float dt);

public:
  csBulletCollisionActor (csBulletSystem* sys);
  ~csBulletCollisionActor ();

  //iCollisionObject
  virtual CS::Collision2::iCollisionObject* QueryCollisionObject () {return dynamic_cast<csBulletCollisionObject*> (this);}
  virtual CS::Physics2::iPhysicalBody* QueryPhysicalBody () {return NULL;}

  virtual void SetObjectType (CS::Collision2::CollisionObjectType type, bool forceRebuild = true) {}
  virtual CS::Collision2::CollisionObjectType GetObjectType () {return CS::Collision2::COLLISION_OBJECT_ACTOR;}

  virtual void SetAttachedMovable (iMovable* movable) {csBulletCollisionObject::SetAttachedMovable (movable);}
  virtual iMovable* GetAttachedMovable () {return csBulletCollisionObject::GetAttachedMovable ();}

  virtual void SetTransform (const csOrthoTransform& trans) 
  {
    csBulletCollisionObject::SetTransform (trans);
    if (camera) camera->SetTransform (trans);
  }
  virtual csOrthoTransform GetTransform () {return csBulletCollisionObject::GetTransform ();}

  virtual void RebuildObject () {csBulletCollisionObject::RebuildObject ();}

  virtual void AddCollider (CS::Collision2::iCollider* collider, const csOrthoTransform& relaTrans)
  { csBulletCollisionObject::AddCollider (collider, relaTrans);}
  virtual void RemoveCollider (CS::Collision2::iCollider* collider) {csBulletCollisionObject::RemoveCollider (collider);}
  virtual void RemoveCollider (size_t index) {csBulletCollisionObject::RemoveCollider (index);}

  virtual CS::Collision2::iCollider* GetCollider (size_t index) {return csBulletCollisionObject::GetCollider (index);}
  virtual size_t GetColliderCount () {return colliders.GetSize ();}

  virtual void SetCollisionGroup (const char* name) {csBulletCollisionObject::SetCollisionGroup (name);}
  virtual const char* GetCollisionGroup () const {return csBulletCollisionObject::GetCollisionGroup ();}

  virtual void SetCollisionCallback (CS::Collision2::iCollisionCallback* cb) {collCb = cb;}
  virtual CS::Collision2::iCollisionCallback* GetCollisionCallback () {return collCb;}

  virtual bool Collide (iCollisionObject* otherObject) {return csBulletCollisionObject::Collide (otherObject);}
  virtual CS::Collision2::HitBeamResult HitBeam (const csVector3& start, const csVector3& end)
  { return csBulletCollisionObject::HitBeam (start, end);}

  virtual size_t GetContactObjectsCount () {return contactObjects.GetSize ();}
  virtual CS::Collision2::iCollisionObject* GetContactObject (size_t index) {
    return csBulletCollisionObject::GetContactObject (index);}

  virtual void RemoveBulletObject () {csBulletCollisionObject::RemoveBulletObject ();}
  virtual void AddBulletObject () {csBulletCollisionObject::AddBulletObject ();}

  //iCollisionActor
  virtual bool IsOnGround ();

  virtual void SetRotation (const csMatrix3& rot);
  virtual void Rotate (const csVector3& v, float angle);

  void SetCamera (iCamera* camera);

  virtual void UpdateAction (float delta);

  virtual void SetUpAxis (int axis);

  virtual void SetVelocity (float speed) {this->speed = speed;}

  virtual void PreStep ();
  virtual void PlayerStep (float delta);

  virtual void SetFallSpeed (float fallSpeed) {this->fallSpeed = fallSpeed;}
  virtual void SetJumpSpeed (float jumpSpeed) {this->jumpSpeed = jumpSpeed;}

  virtual void SetMaxJumpHeight (float maxJumpHeight)
  {this->maxJumpHeight = maxJumpHeight;}
  virtual void Jump ();

  virtual void SetMaxSlope (float slopeRadians);
  virtual float GetMaxSlope () const {return maxSlopeRadians;}
};

}
CS_PLUGIN_NAMESPACE_END (Bullet2)
#endif