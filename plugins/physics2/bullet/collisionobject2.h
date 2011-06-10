#ifndef __CS_BULLET_COLLISIONOBJECT_H__
#define __CS_BULLET_COLLISIONOBJECT_H__

CS_PLUGIN_NAMESPACE_BEGIN(Bullet2)
{

class csBulletCollisionObject: public scfImplementationExt1<
  csBulletCollisionObject, csObject, iCollisionObject>
{
  friend class csBulletSector;
  friend class csBulletSystem;
  friend class csBulletMotionState;
  friend class csBulletKinematicMotionState;

  csBulletSector* sector;
  csBulletSystem* system;
  csRefArray<csBulletCollider> colliders;
  csArray<csOrthoTransform> relaTransforms;
  csRef<iMovable> movable;
  btCollisionObject* btObject;
  btTransform transform;
  csBulletMotionState* motionState;
  csRef<iCollisionCallback> collCb;
  btCompoundShape* compoundShape;
  CollisionObjectType type;
  CollisionGroup collGroup;
  bool insideWorld;
  bool shapeChanged;
  bool isPhysics;
  bool isTerrain;

public:
  csBulletCollisionObject (csBulletSystem* sys);
  virtual ~csBulletCollisionObject ();

  virtual iObject* QueryObject (void) { return (iObject*) this; }

  virtual void SetObjectType (CollisionObjectType type);
  virtual CollisionObjectType GetObjectType () {return type;}

  virtual void SetAttachedMovable (iMovable* movable){this->movable = movable;}
  virtual iMovable* GetAttachedMovable (){return movable;}

  virtual void SetTransform (const csOrthoTransform& trans);
  virtual csOrthoTransform GetTransform ();

  virtual void AddCollider (CS::Collision::iCollider* collider, const csOrthoTransform& relaTrans);
  virtual void RemoveCollider (CS::Collision::iCollider* collider);
  virtual void RemoveCollider (size_t index);

  virtual CS::Collision::iCollider* GetCollider (size_t index) ;
  virtual size_t GetColliderCount () {return colliders.GetSize ();}

  virtual void RebuildObject ();

  virtual void SetCollisionGroup (const char* name);
  virtual const char* GetCollisionGroup () const {return collGroup.name.GetData ();}

  virtual void SetCollisionCallback (iCollisionCallback* cb) {collCb = cb;}
  virtual iCollisionCallback* GetCollisionCallback () {return collCb;}

  virtual bool Collide (iCollisionObject* otherObject);
  virtual HitBeamResult HitBeam (const csVector3& start, const csVector3& end);

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