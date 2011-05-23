#ifndef __CS_BULLET_COLLISION_H__
#define __CS_BULLET_COLLISION_H__

#include "iutil/comp.h"
#include "csutil/scf.h"
#include "csutil/scf_implementation.h"
#include "ivaria/collision2.h"
#include "ivaria/view.h"
#include "iengine/movable.h"
#include "common.h"
#include "bulletcolliders.h"

CS_PLUGIN_NAMESPACE_BEGIN(Bullet2)
{

class csBulletCollisionSector;
class csBulletCollisionObject;
class csBulletTerrainObject;
class csBulletCollisionSystem;

class csBulletCollisionObject:
  public scfImplementation1<csBulletCollisionObject, iCollisionObject>
{
  friend class csBulletCollisionSector;
  friend class csBulletCollisionSystem;
  friend class csBulletMotionState;
  friend class csBulletKinematicMotionState;

  csBulletCollisionSector* sector;
  csBulletCollisionSystem* collSys;
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
  bool compoundChanged;

public:
  csBulletCollisionObject(CollisionObjectType type, csBulletCollisionSystem* sys);
  virtual ~csBulletCollisionObject();

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
};

class csBulletTerrainObject:
  public scfImplementation1<csBulletTerrainObject, iCollisionObject>
{
  csBulletCollisionSector* sector;
  csBulletCollisionSystem* collSys;
  csArray<btRigidBody*> bodies;
  csArray<btTransform> transforms;
  csRef<csBulletColliderTerrain> terrainCollider;
  csRef<iMovable> movable;
  csRef<iCollisionCallback> collCb;
  //CollisionObjectType type;
  CollisionGroup collGroup;
  csOrthoTransform totalTransform;
  bool insideWorld;
public:
  csBulletTerrainObject(csBulletCollisionSystem* sys);
  virtual ~csBulletTerrainObject();

  virtual void SetObjectType (CollisionObjectType type) {}   
  virtual CollisionObjectType GetObjectType () {return COLLISION_OBJECT_TERRAIN;}

  virtual void SetAttachedMovable (iMovable* movable){this->movable = movable;}
  virtual iMovable* GetAttachedMovable (){return movable;}

  virtual void SetTransform (const csOrthoTransform& trans);
  virtual csOrthoTransform GetTransform ();

  virtual void AddCollider (CS::Collision::iCollider* collider, const csOrthoTransform& relaTrans);
  virtual void RemoveCollider (CS::Collision::iCollider* collider);
  virtual void RemoveCollider (size_t index);

  virtual CS::Collision::iCollider* GetCollider (size_t index);
  virtual size_t GetColliderCount () {return 1;}

  virtual void RebuildObject ();

  virtual void SetCollisionGroup (CollisionGroup group);
  virtual const char* GetCollisionGroup (){return collGroup.name.GetData ();}

  //terrain is a static object. Does it need a collision callback?
  virtual void SetCollisionCallback (iCollisionCallback* cb) {collCb = cb;}    
  virtual iCollisionCallback* GetCollisionCallback () {return collCb;}

  virtual bool Collide (iCollisionObject* otherObject);
  virtual HitBeamResult HitBeam (const csVector3& start, const csVector3& end);
};

class csBulletCollisionSector:
  public scfImplementation1<csBulletCollisionSector, iCollisionSector>
{
  friend class csBulletCollisionObject;
  friend class csBulletTerrainObject;
  friend class csBulletKinematicMotionState;
  friend class csBulletMotionState;
  friend class csBulletCollider;
  friend class csBulletColliderBox;
  friend class csBulletColliderSphere;
  friend class csBulletColliderCylinder;
  friend class csBulletColliderCapsule;
  friend class csBulletColliderConvexMesh;
  friend class csBulletColliderConcaveMesh;
  friend class csBulletColliderTerrain;
  friend class csBulletColliderPlane;
  friend class csBulletColliderCone;
  friend class HeightMapCollider;

  struct CollisionPortal
  {
    iPortal* portal;
    csBulletCollisionObject* ghostPortal1;
    csBulletCollisionObject* ghostPortal2;
  };
  iSector* sector;
  csBulletCollisionSystem* sys;
  csVector3 gravity;
  csRefArray<iCollisionObject> collisionObjects;
  csRefArray<CollisionPortal> portals;
  csArray<CollisionData> points;
  //csRefArray<iCollisionObject> collisionPortals;
  csBulletDebugDraw* debugDraw;
  btDynamicsWorld* bulletWorld;
  btCollisionDispatcher* dispatcher;
  btDefaultCollisionConfiguration* configuration;
  btSequentialImpulseConstraintSolver* solver;
  btBroadphaseInterface* broadphase;
  float internalScale;
  float inverseInternalScale;
  csStringID baseID;
  csStringID colldetID;

public:
  csBulletCollisionSector ();
  virtual ~csBulletCollisionSector();

  virtual void SetInternalScale (float scale);
  virtual void SetGravity (const csVector3& v);
  virtual csVector3 GetGravity (){return gravity;}

  virtual void AddCollisionObject(iCollisionObject* object);
  virtual void RemoveCollisionObject(iCollisionObject* object);

  virtual void AddPortal(iPortal& portal);
  virtual void RemovePortal(iPortal* portal);

  virtual void SetSector(iSector* sector);
  virtual iSector* GetSector(){return sector;}

  virtual HitBeamResult HitBeam(const csVector3& start, 
    const csVector3& end);

  virtual HitBeamResult HitBeamPortal(const csVector3& start, 
    const csVector3& end);

  virtual bool CollisionTest(iCollisionObject* object, csArray<CollisionData>& collisions);

  virtual MoveResult MoveTest (iCollisionObject* object,
    const csOrthoTransform& fromWorld, const csOrthoTransform& toWorld);

  bool RigidCollide(iCollisionObject* objectA,
    iCollisionObject* objectB);

  HitBeamResult RigidHitBeam(iCollisionObject* object, 
			     const csVector3& start,
			     const csVector3& end);
};

class csBulletCollisionSystem:
  public scfImplementationExt1<csBulletCollisionSystem, csObject, iCollisionSystem>
{

  csRefArray<CS::Collision::iCollider> colliders;
  csRefArray<iCollisionObject> objects;
  csRefArray<csBulletCollisionSector> collSectors;
  //Todo: think about it..
  //csArray<CollisionGroup> groups;

  class CollisionGroupVector : csArray<CollisionGroup>
  {
  public:
    CollisionGroupVector() : csArray<CollisionGroup>() {}
    static int CompareKey(CollisionGroup const& item,
      char const* const& key)
    {
      return strcmp (item.name.GetData(), key);
    }
    static csArrayCmp<CollisionGroup, char const*>
      KeyCmp(char const* k)
    {
      return csArrayCmp<CollisionGroup, char const*>(k,CompareKey);
    }
  };

  CollisionGroupVector collGroups;
  csArray<size_t> collGroupMasks;

  void RegisterGimpact();
public:
  csBulletCollisionSystem(iObjectRegistry* object_reg);
  virtual ~csBulletCollisionSystem();

  virtual csPtr<iColliderConvexMesh> CreateColliderConvexMesh (iMeshWrapper* mesh);
  virtual csPtr<iColliderConcaveMesh> CreateColliderConcaveMesh 
      (iMeshWrapper* mesh, bool isStatic = false);
  virtual csPtr<iColliderConcaveMeshScaled> CreateColliderConcaveMeshScaled
      (iColliderConcaveMesh* collider, float scale);
  virtual csPtr<iColliderCylinder> CreateColliderCylinder(float length, float radius);
  virtual csPtr<iColliderBox> CreateColliderBox(const csVector3& size);
  virtual csPtr<iColliderSphere> CreateColliderSphere(float radius);
  virtual csPtr<iColliderCapsule> CreateColliderCapsule(float length, float radius);
  virtual csPtr<iColliderPlane> CreateColliderPlane(const csPlane3& plain) = 0;
  virtual csPtr<iColliderTerrain> CreateColliderTerrain(const iTerrainSystem* terrain,
      float minHeight = 0, float maxHeight = 0);

  virtual csPtr<iCollisionObject> CreateCollisionObject();
  virtual csPtr<iCollisionActor> CreateCollisionActor();
  virtual csPtr<iCollisionSector> CreateCollisionSector();

  virtual CollisionGroup& CreateCollisionGroup(const char* name);
  virtual CollisionGroup& FindCollisionGroup(const char* name);

  virtual void SetGroupCollision (CollisionGroup& group1,
      CollisionGroup& group2, bool collide);
  virtual bool GetGroupCollision (CollisionGroup& group1,
      CollisionGroup& group2);

  virtual void DecomposeConcaveMesh (iCollisionObject* object, iMeshWrapper* mesh); 
};
}
CS_PLUGIN_NAMESPACE_END(Bullet2)

#endif
