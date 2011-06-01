#ifndef __CS_BULLET_PHYSICS_H__
#define __CS_BULLET_PHYSICS_H__

#include "iutil/comp.h"
#include "csutil/scf.h"
#include "csutil/scf_implementation.h"
#include "ivaria/collision2.h"
#include "ivaria/physical2.h"
#include "ivaria/bullet2.h"
#include "ivaria/view.h"
#include "iengine/movable.h"
#include "common2.h"
#include "colliders2.h"
#include "rigidbody2.h"
#include "softbody2.h"
#include "joint2.h"

class btCollisionObject;
class btCompoundShape;

CS_PLUGIN_NAMESPACE_BEGIN(Bullet2)
{

class csBulletSector;
class csBulletCollisionObject;
class csBulletTerrainObject;
class csBulletSystem;
class csBulletSoftBody;

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

  struct CollisionPortal
  {
    iPortal* portal;
    csBulletCollisionObject* ghostPortal1;
    csBulletCollisionObject* ghostPortal2;
  };
  iSector* sector;
  csBulletPhysicalSector* phySector;
  csBulletSystem* sys;
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
                  csBulletSystem* sys);
  virtual ~csBulletSector();

  virtual iObject* QueryObject () {return (iObject*) this;}
  //iCollisionSector
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

  virtual void StartProfile ();

  virtual void StopProfile ();

  virtual void DumpProfile (bool resetProfile = true);

  bool BulletCollide(btCollisionObject* objectA,
    btCollisionObject* objectB);

  HitBeamResult RigidHitBeam(btCollisionObject* object, 
			     const csVector3& start,
			     const csVector3& end);
};

class csBulletSystem: public scfImplementation3<
  csBulletSystem, CS::Collision::iCollisionSystem, 
  CS::Physics::iPhysicalSystem, iComponent>
{
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

  iObjectRegistry* object_reg;
  csRefArrayObject<CS::Collision::iCollider> colliders;
  csRefArrayObject<iCollisionObject> objects;
  csRefArrayObject<iRigidBody> rigidBodies;
  csRefArrayObject<iSoftBody> softBodies;
  csRefArrayObject<iJoint> joints;
  csRefArrayObject<iCollisionActor> actors;
  csRefArrayObject<csBulletSector> collSectors;
  btSoftBodyWorldInfo defaultInfo;
  float internalScale;
  float inverseInternalScale;
  csStringID baseID;
  csStringID colldetID;
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
  csBulletSystem (iBase* iParent);
  virtual ~csBulletSystem ();

  // iComponent
  virtual bool Initialize (iObjectRegistry* object_reg);

  // iCollisionSystem
  virtual void SetInternalScale (float scale);
  virtual csPtr<iColliderConvexMesh> CreateColliderConvexMesh (iMeshWrapper* mesh);
  virtual csPtr<iColliderConcaveMesh> CreateColliderConcaveMesh 
      (iMeshWrapper* mesh, bool isStatic = false);
  virtual csPtr<iColliderConcaveMeshScaled> CreateColliderConcaveMeshScaled
      (iColliderConcaveMesh* collider, float scale);
  virtual csRef<iColliderCylinder> CreateColliderCylinder (float length, float radius);
  virtual csRef<iColliderBox> CreateColliderBox (const csVector3& size);
  virtual csRef<iColliderSphere> CreateColliderSphere (float radius);
  virtual csRef<iColliderCapsule> CreateColliderCapsule (float length, float radius);
  virtual csRef<iColliderCapsule> CreateColliderCone (float length, float radius);
  virtual csRef<iColliderPlane> CreateColliderPlane (const csPlane3& plane) = 0;
  virtual csRef<iColliderTerrain> CreateColliderTerrain (const iTerrainSystem* terrain,
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

  //iPhysicalSystem
  virtual csRef<iRigidBody> CreateRigidBody ();

  virtual csRef<iJoint> CreateJoint ();
 
  virtual csRef<iSoftBody> CreateRope (csVector3 start,
      csVector3 end, size_t segmentCount);
  virtual csRef<iSoftBody> CreateRope (csVector3* vertices, size_t vertexCount);
  virtual csRef<iSoftBody> CreateCloth (csVector3 corner1, csVector3 corner2,
      csVector3 corner3, csVector3 corner4,
      size_t segmentCount1, size_t segmentCount2,
      bool withDiagonals = false);

  virtual csRef<iSoftBody> CreateSoftBody (iGeneralFactoryState* genmeshFactory);

  virtual csRef<iSoftBody> CreateSoftBody (csVector3* vertices,
      size_t vertexCount, csTriangle* triangles,
      size_t triangleCount);
};
}
CS_PLUGIN_NAMESPACE_END(Bullet2)

#endif
