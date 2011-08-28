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
  friend class csBulletColliderTerrain;
  friend class csBulletCollisionActor;

protected:
  csRefArray<csBulletCollider> colliders;
  csRefArray<csBulletCollisionObject> contactObjects;
  csArray<csOrthoTransform> relaTransforms;
  csArray<CS::Physics2::iJoint*> joints;
  CS::Collision2::CollisionGroup collGroup;
  csWeakRef<iMovable> movable;
  csWeakRef<iCamera> camera;
  csRef<CS::Collision2::iCollisionCallback> collCb;
  CS::Collision2::CollisionObjectType type;

  btTransform transform;
  btTransform invPricipalAxis;
  btQuaternion portalWarp;

  csBulletSector* sector;
  csBulletSystem* system;
  btCollisionObject* btObject;
  csBulletMotionState* motionState;
  btCompoundShape* compoundShape;
  csBulletCollisionObject* objectOrigin;
  csBulletCollisionObject* objectCopy;

  short haveStaticColliders;
  bool insideWorld;
  bool shapeChanged;
  bool isTerrain;

public:
  csBulletCollisionObject (csBulletSystem* sys);
  virtual ~csBulletCollisionObject ();

  virtual iObject* QueryObject (void) { return (iObject*) this; }
  virtual CS::Collision2::iCollisionObject* QueryCollisionObject () {
    return dynamic_cast<CS::Collision2::iCollisionObject*> (this);}
  virtual CS::Physics2::iPhysicalBody* QueryPhysicalBody () {return NULL;}

  virtual void SetObjectType (CS::Collision2::CollisionObjectType type, bool forceRebuild = true);
  virtual CS::Collision2::CollisionObjectType GetObjectType () {return type;}

  virtual void SetAttachedMovable (iMovable* movable){this->movable = movable;}
  virtual iMovable* GetAttachedMovable (){return movable;}

  virtual void SetAttachedCamera (iCamera* camera){this->camera = camera;}
  virtual iCamera* GetAttachedCamera (){return camera;}

  virtual void SetTransform (const csOrthoTransform& trans);
  virtual csOrthoTransform GetTransform ();

  virtual void AddCollider (CS::Collision2::iCollider* collider, const csOrthoTransform& relaTrans
    = csOrthoTransform (csMatrix3 (), csVector3 (0)));
  virtual void RemoveCollider (CS::Collision2::iCollider* collider);
  virtual void RemoveCollider (size_t index);

  virtual CS::Collision2::iCollider* GetCollider (size_t index) ;
  virtual size_t GetColliderCount () {return colliders.GetSize ();}

  virtual void RebuildObject ();

  virtual void SetCollisionGroup (const char* name);
  virtual const char* GetCollisionGroup () const {return collGroup.name.GetData ();}

  virtual void SetCollisionCallback (CS::Collision2::iCollisionCallback* cb) {collCb = cb;}
  virtual CS::Collision2::iCollisionCallback* GetCollisionCallback () {return collCb;}

  virtual bool Collide (CS::Collision2::iCollisionObject* otherObject);
  virtual CS::Collision2::HitBeamResult HitBeam (const csVector3& start, const csVector3& end);

  virtual size_t GetContactObjectsCount ();
  virtual CS::Collision2::iCollisionObject* GetContactObject (size_t index);

  btCollisionObject* GetBulletCollisionPointer () {return btObject;}
  virtual bool RemoveBulletObject ();
  virtual bool AddBulletObject ();
  void RemoveObjectCopy () {
    csBulletSector* sec = objectCopy->sector;
    sec->RemoveCollisionObject (objectCopy);
    objectCopy = NULL;
  }
};
}
CS_PLUGIN_NAMESPACE_END (Bullet2)
#endif