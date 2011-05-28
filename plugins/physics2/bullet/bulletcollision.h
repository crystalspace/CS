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

class btCollisionObject;
class btCompoundShape;

CS_PLUGIN_NAMESPACE_BEGIN(Bullet2)
{

class csBulletSector;
class csBulletCollisionObject;
class csBulletTerrainObject;
class csBulletCollisionSystem;
class csBulletSoftBody;

class csBulletCollisionObject: public scfImplementationExt1<
  csBulletCollisionObject, csObject, iCollisionObject>
{
  friend class csBulletSector;
  friend class csBulletCollisionSystem;
  friend class csBulletMotionState;
  friend class csBulletKinematicMotionState;

  csBulletSector* sector;
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
  bool isPhysics;
  bool isTerrain;

public:
  csBulletCollisionObject (csBulletCollisionSystem* sys);
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

struct PointContactResult : public btCollisionWorld::ContactResultCallback
{
  csArray<CollisionData>& colls;
  PointContactResult(csArray<CollisionData>& collisions) : colls(collisions) 
  {
  }
  virtual	btScalar	addSingleResult (btManifoldPoint& cp,	const btCollisionObject* colObj0, int partId0,int index0,const btCollisionObject* colObj1,int partId1,int index1)
  {
    CollisionData data;
    data.penetration = cp.m_distance1;
    data.positionWorldOnA = cp.m_positionWorldOnA;
    data.positionWorldOnB = cp.m_positionWorldOnB;
    colls.Push (data);
    return 0;
  }
};

//Will also implement iPhysicalSector...
class csBulletSector: public scfImplementationExt3<
  csBulletSector, csObject, iCollisionSector, 
  CS::Physics::Bullet::iPhysicalSector,
  CS::Physics::iPhysicalSector>
{
  //TODO: Too many friend classes..I'd better user a function instead...
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
  csBulletPhysicalSector* phySector;
  csBulletCollisionSystem* sys;
  csVector3 gravity;
  csRefArrayObject<iCollisionObject> collisionObjects;
  csRefArrayObject<iRigidBody> rigidBodies;
  csRefArrayObject<iSoftBody> softBodies;
  csWeakRefArray<csBulletSoftBody> anchoredSoftBodies;
  csRefArray<iJoint> joints;
  csRefArray<CollisionPortal> portals;
  csArray<CollisionData> points;
  csBulletDebugDraw* debugDraw;
  btDynamicsWorld* bulletWorld;
  btCollisionDispatcher* dispatcher;
  btDefaultCollisionConfiguration* configuration;
  btSequentialImpulseConstraintSolver* solver;
  btBroadphaseInterface* broadphase;
  btSoftBodyWorldInfo* softWorldInfo;
  float internalScale;
  float inverseInternalScale;
  csStringID baseID;
  csStringID colldetID;

  bool isSoftWorld;

  float worldTimeStep;
  size_t worldMaxSteps;
  float linearDampening;
  float angularDampening;

  float linearDisableThreshold;
  float angularDisableThreshold;
  float timeDisableThreshold;

public:
  csBulletSector (iObjectRegistry* object_reg,
                  csBulletCollisionSystem* sys);
  virtual ~csBulletSector();

  virtual iObject* QueryObject () {return (iObject*) this;}
  //iCollisionSector
  virtual void SetInternalScale (float scale);
  virtual void SetGravity (const csVector3& v);
  virtual csVector3 GetGravity (){return gravity;}

  virtual void AddCollisionObject(iCollisionObject* object);
  virtual void RemoveCollisionObject(iCollisionObject* object);

  virtual void AddPortal(iPortal* portal);
  virtual void RemovePortal(iPortal* portal);

  virtual void SetSector(iSector* sector);
  virtual iSector* GetSector(){return sector;}

  virtual HitBeamResult HitBeam(const csVector3& start, 
    const csVector3& end);

  virtual HitBeamResult HitBeamPortal(const csVector3& start, 
    const csVector3& end);

  virtual bool CollisionTest(iCollisionObject* object, csArray<CollisionData>& collisions);

  /*virtual MoveResult MoveTest (iCollisionObject* object,
    const csOrthoTransform& fromWorld, const csOrthoTransform& toWorld);*/

  //iPhysicalSector
  virtual void SetSimulationSpeed (float speed);
  virtual void SetStepParameters (float timeStep,
    size_t maxSteps, size_t iterations);
  virtual void Step (float duration);

  virtual void SetLinearDampener (float d);
  virtual float GetLinearDampener () {return linearDampening;}

  virtual void SetRollingDampener (float d);
  virtual float GetRollingDampener () {return angularDampening;}

  virtual void SetAutoDisableParams (float linear,
    float angular, float time);

  virtual void AddRidigBody (iRigidBody* body);
  virtual void RemoveRigidBody (iRigidBody* body);

  virtual void AddSoftBody (iSoftBody* body);
  virtual void RemoveSoftBody (iSoftBody* body);

  //Bullet::iPhysicalSector
  virtual void SetSoftBodyEnabled (bool enabled);
  virtual bool GetSoftBodyEnabled () {return isSoftWorld;}

  //Currently will not use gimpact shape...
  //virtual void SetGimpactEnabled (bool enabled);
  //virtual bool GetGimpactEnabled ();

  virtual void SaveWorld (const char* filename);

  virtual void DebugDraw (iView* rview);
  virtual void SetDebugMode (DebugMode mode);
  virtual DebugMode GetDebugMode ();

  bool BulletCollide(btCollisionObject* objectA,
    btCollisionObject* objectB);

  HitBeamResult RigidHitBeam(btCollisionObject* object, 
			     const csVector3& start,
			     const csVector3& end);
};

class csBulletCollisionSystem: public scfImplementation2<
  csBulletCollisionSystem, iCollisionSystem, iComponent>
{

  iObjectRegistry* object_reg;
  csRefArray<CS::Collision::iCollider> colliders;
  csRefArray<iCollisionObject> objects;
  csRefArray<iCollisionActor> actors;
  csRefArray<csBulletSector> collSectors;
  //Todo: think about it..
  //csArray<CollisionGroup> groups;

  class CollisionGroupVector : csArray<CollisionGroup>
  {
  public:
    CollisionGroupVector () : csArray<CollisionGroup> () {}
    static int CompareKey (CollisionGroup const& item,
      char const* const& key)
    {
      return strcmp (item.name.GetData (), key);
    }
    static csArrayCmp<CollisionGroup, char const*>
      KeyCmp(char const* k)
    {
      return csArrayCmp<CollisionGroup, char const*> (k,CompareKey);
    }
  };

  CollisionGroupVector collGroups;
  csArray<size_t> collGroupMasks;

  void RegisterGimpact ();
public:
  csBulletCollisionSystem (iBase* iParent);
  virtual ~csBulletCollisionSystem ();

  // iComponent
  virtual bool Initialize (iObjectRegistry* object_reg);

  // iCollisionSystem
  virtual csPtr<iColliderConvexMesh> CreateColliderConvexMesh (iMeshWrapper* mesh);
  virtual csPtr<iColliderConcaveMesh> CreateColliderConcaveMesh 
      (iMeshWrapper* mesh, bool isStatic = false);
  virtual csPtr<iColliderConcaveMeshScaled> CreateColliderConcaveMeshScaled
      (iColliderConcaveMesh* collider, float scale);
  virtual csPtr<iColliderCylinder> CreateColliderCylinder (float length, float radius);
  virtual csPtr<iColliderBox> CreateColliderBox (const csVector3& size);
  virtual csPtr<iColliderSphere> CreateColliderSphere (float radius);
  virtual csPtr<iColliderCapsule> CreateColliderCapsule (float length, float radius);
  virtual csPtr<iColliderCapsule> CreateColliderCone (float length, float radius);
  virtual csPtr<iColliderPlane> CreateColliderPlane (const csPlane3& plane) = 0;
  virtual csPtr<iColliderTerrain> CreateColliderTerrain (const iTerrainSystem* terrain,
      float minHeight = 0, float maxHeight = 0);

  virtual csRef<iCollisionObject> CreateCollisionObject ();
  virtual csRef<iCollisionActor> CreateCollisionActor ();
  virtual csRef<iCollisionSector> CreateCollisionSector ();

  virtual CollisionGroup& CreateCollisionGroup (const char* name);
  virtual CollisionGroup& FindCollisionGroup (const char* name);

  virtual void SetGroupCollision (CollisionGroup& group1,
      CollisionGroup& group2, bool collide);
  virtual bool GetGroupCollision (CollisionGroup& group1,
      CollisionGroup& group2);

  virtual void DecomposeConcaveMesh (iCollisionObject* object, iMeshWrapper* mesh); 
};
}
CS_PLUGIN_NAMESPACE_END(Bullet2)

#endif
