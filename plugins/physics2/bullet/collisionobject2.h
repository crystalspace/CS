#ifndef __CS_BULLET_COLLISIONOBJECT_H__
#define __CS_BULLET_COLLISIONOBJECT_H__

#include "bullet2.h"
#include "common2.h"
#include "colliders2.h"

CS_PLUGIN_NAMESPACE_BEGIN(Bullet2)
{

struct CS::Physics2::iPhysicalBody;

class csBulletCollisionObject: public scfImplementationExt1<
  csBulletCollisionObject, csObject, CS::Collision2::iCollisionObject>
{
  friend class csBulletSector;
  friend class csBulletSystem;
  friend class csBulletMotionState;
  friend class csBulletKinematicMotionState;
  friend class csBulletJoint;

protected:
  csBulletSector* sector;
  csBulletSystem* system;
  csRefArray<csBulletCollider> colliders;
  csArray<csOrthoTransform> relaTransforms;
  csWeakRef<iMovable> movable;
  btCollisionObject* btObject;
  btTransform transform;
  csBulletMotionState* motionState;
  csRef<CS::Collision2::iCollisionCallback> collCb;
  btCompoundShape* compoundShape;
  CS::Collision2::CollisionObjectType type;
  CS::Collision2::CollisionGroup collGroup;
  int haveStaticColliders;
  bool insideWorld;
  bool shapeChanged;
  bool isTerrain;

public:
  csBulletCollisionObject (csBulletSystem* sys);
  virtual ~csBulletCollisionObject ();

  virtual iObject* QueryObject (void) { return (iObject*) this; }
  virtual iCollisionObject* QueryCollisionObject () {return dynamic_cast<iCollisionObject*> (this);}
  virtual CS::Physics2::iPhysicalBody* QueryPhysicalBody () {return NULL;}

  virtual void SetObjectType (CS::Collision2::CollisionObjectType type, bool forceRebuild = true);
  virtual CS::Collision2::CollisionObjectType GetObjectType () {return type;}

  virtual void SetAttachedMovable (iMovable* movable){this->movable = movable;}
  virtual iMovable* GetAttachedMovable (){return movable;}

  virtual void SetTransform (const csOrthoTransform& trans);
  virtual csOrthoTransform GetTransform ();

  virtual void AddCollider (CS::Collision2::iCollider* collider, const csOrthoTransform& relaTrans);
  virtual void RemoveCollider (CS::Collision2::iCollider* collider);
  virtual void RemoveCollider (size_t index);

  virtual CS::Collision2::iCollider* GetCollider (size_t index) ;
  virtual size_t GetColliderCount () {return colliders.GetSize ();}

  virtual void RebuildObject ();

  virtual void SetCollisionGroup (const char* name);
  virtual const char* GetCollisionGroup () const {return collGroup.name.GetData ();}

  virtual void SetCollisionCallback (CS::Collision2::iCollisionCallback* cb) {collCb = cb;}
  virtual CS::Collision2::iCollisionCallback* GetCollisionCallback () {return collCb;}

  virtual bool Collide (iCollisionObject* otherObject);
  virtual CS::Collision2::HitBeamResult HitBeam (const csVector3& start, const csVector3& end);

  btCollisionObject* GetBulletCollisionPointer () {return btObject;}
  virtual void RemoveBulletObject ();
  virtual void AddBulletObject ();
};

//TODO collision actor

/*
class csBulletCollisionActor : public scfImplementationExt1<
  csBulletCollisionActor, csBulletCollisionObject, iCollisionActor>
{
public:
  csBulletCollisionActor (csBulletCollisionSystem* sys);
  ~csBulletCollisionActor ();

  //iCollisionObject
  virtual void SetObjectType (CollisionObjectType type);
  virtual CollisionObjectType GetObjectType () {return type;}

  virtual void SetTransform (const csOrthoTransform& trans);
  virtual csOrthoTransform GetTransform ();

  virtual void RebuildObject ();

  virtual bool Collide (iCollisionObject* otherObject);
  virtual HitBeamResult HitBeam (const csVector3& start, const csVector3& end);

  //iCollisionActor
  virtual bool IsOnGround ();
  virtual csVector3 GetRotation ();
  virtual void SetRotation ();

  virtual void UpdateAction (float delta);

  virtual void SetUpAxis (int axis);

  virtual void SetVelocity (const csVector3& dir);
  virtual void SetVelocityForTimeInterval (const csVector3& velo, float timeInterval);

  virtual void PreStep ();
  virtual void PlayerStep (float delta);

  virtual void SetFallSpeed (float fallSpeed);
  virtual void SetJumpSpeed (float jumpSpeed);

  virtual void SetMaxJumpHeight (float maxJumpHeight);
  virtual void Jump ();

  virtual void SetMaxSlope (float slopeRadians);
  virtual float GetMaxSlope ();
};
*/
}
CS_PLUGIN_NAMESPACE_END (Bullet2)
#endif