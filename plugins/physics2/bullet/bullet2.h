#ifndef __CS_BULLET_PHYSICS_H__
#define __CS_BULLET_PHYSICS_H__

#include "iutil/comp.h"
#include "csutil/scf.h"
#include "csutil/nobjvec.h"
#include "csutil/scf_implementation.h"
#include "ivaria/collision2.h"
#include "ivaria/physical2.h"
#include "ivaria/bullet2.h"
#include "ivaria/view.h"
#include "iengine/movable.h"
#include "csutil/csobject.h"

struct iSector;
class btCollisionObject;
class btCompoundShape;
class btDynamicsWorld;
class btCollisionDispatcher;
class btDefaultCollisionConfiguration;
class btSequentialImpulseConstraintSolver;
class btBroadphaseInterface;
struct btSoftBodyWorldInfo;

CS_PLUGIN_NAMESPACE_BEGIN (Bullet2)
{
class csBulletSystem;
class csBulletDebugDraw;
class csBulletRigidBody;
class csBulletSoftBody;
class csBulletCollisionObject;
class csBulletCollider;
class csBulletJoint;

using namespace CS::Collision2;
using namespace CS::Physics2;

//Will also implement iPhysicalSector...
class csBulletSector : public scfImplementationExt3<
  csBulletSector, csObject, CS::Collision2::iCollisionSector, 
  CS::Physics2::Bullet2::iPhysicalSector,
  CS::Physics2::iPhysicalSector>
{
  friend class csBulletCollisionObject;
  friend class csBulletRigidBody;
  friend class csBulletSoftBody;
  friend class csBulletJoint;
  friend class csBulletColliderTerrain;
  friend class csBulletKinematicMotionState;
  friend class csBulletMotionState;

  struct CollisionPortal
  {
    iPortal* portal;
    csBulletCollisionObject* ghostPortal1;
    csBulletCollisionObject* ghostPortal2;
  };
  iSector* sector;
  csBulletSystem* sys;
  csVector3 gravity;
  csRefArrayObject<csBulletCollisionObject> collisionObjects;
  csRefArrayObject<csBulletRigidBody> rigidBodies;
  csRefArrayObject<csBulletSoftBody> softBodies;
  csWeakRefArray<csBulletSoftBody> anchoredSoftBodies;
  csRefArray<csBulletJoint> joints;
  csArray<CollisionPortal> portals;
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
  csBulletSector (csBulletSystem* sys);
  virtual ~csBulletSector();

  virtual iObject* QueryObject () {return (iObject*) this;}
  //iCollisionSector
  virtual void SetGravity (const csVector3& v);
  virtual csVector3 GetGravity () const {return gravity;}

  virtual void AddCollisionObject(iCollisionObject* object);
  virtual void RemoveCollisionObject(iCollisionObject* object);

  virtual size_t GetCollisionObjectCount () {return collisionObjects.GetSize ();}
  virtual iCollisionObject* GetCollisionObject (size_t index);

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
  virtual float GetLinearDampener () const {return linearDampening;}

  virtual void SetRollingDampener (float d);
  virtual float GetRollingDampener () const {return angularDampening;}

  virtual void SetAutoDisableParams (float linear,
    float angular, float time);

  virtual void AddRigidBody (iRigidBody* body);
  virtual void RemoveRigidBody (iRigidBody* body);

  virtual size_t GetRigidBodyCount () {return rigidBodies.GetSize ();}
  virtual iRigidBody* GetRigidBody (size_t index);

  virtual void AddSoftBody (iSoftBody* body);
  virtual void RemoveSoftBody (iSoftBody* body);

  virtual size_t GetSoftBodyCount () {return softBodies.GetSize ();}
  virtual iSoftBody* GetSoftBody (size_t index);

  virtual void RemoveJoint (iJoint* joint);

  virtual void SetSoftBodyEnabled (bool enabled);
  virtual bool GetSoftBodyEnabled () {return isSoftWorld;}

  //Bullet::iPhysicalSector
  //Currently will not use gimpact shape...
  //virtual void SetGimpactEnabled (bool enabled);
  //virtual bool GetGimpactEnabled ();

  virtual bool SaveWorld (const char* filename);

  virtual void DebugDraw (iView* rview);
  virtual void SetDebugMode (CS::Physics2::Bullet2::DebugMode mode);
  virtual CS::Physics2::Bullet2::DebugMode GetDebugMode ();

  virtual void StartProfile ();

  virtual void StopProfile ();

  virtual void DumpProfile (bool resetProfile = true);

  bool BulletCollide(btCollisionObject* objectA,
    btCollisionObject* objectB);

  HitBeamResult RigidHitBeam(btCollisionObject* object, 
			     const csVector3& start,
			     const csVector3& end);

  void UpdateSoftBodies (float timeStep);
};

class csBulletSystem : public scfImplementation3<
  csBulletSystem, CS::Collision2::iCollisionSystem, 
  CS::Physics2::iPhysicalSystem, iComponent>
{
  friend class csBulletColliderConvexMesh;
  friend class csBulletColliderConcaveMesh;
private:
  iObjectRegistry* object_reg;
  /*csRefArrayObject<CS::Collision2::iCollider> colliders;
  csRefArrayObject<csBulletCollisionObject> objects;
  csRefArrayObject<csBulletRigidBody> rigidBodies;
  csRefArrayObject<csBulletSoftBody> softBodies;
  csRefArrayObject<csBulletJoint> joints;
  csRefArrayObject<iCollisionActor> actors;*/
  csRefArrayObject<iCollisionSector> collSectors;
  btSoftBodyWorldInfo* defaultInfo;
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
  virtual csRef<iColliderConvexMesh> CreateColliderConvexMesh (iMeshWrapper* mesh);
  virtual csRef<iColliderConcaveMesh> CreateColliderConcaveMesh (iMeshWrapper* mesh);
  virtual csRef<iColliderConcaveMeshScaled> CreateColliderConcaveMeshScaled
      (iColliderConcaveMesh* collider, float scale);
  virtual csRef<iColliderCylinder> CreateColliderCylinder (float length, float radius);
  virtual csRef<iColliderBox> CreateColliderBox (const csVector3& size);
  virtual csRef<iColliderSphere> CreateColliderSphere (float radius);
  virtual csRef<iColliderCapsule> CreateColliderCapsule (float length, float radius);
  virtual csRef<iColliderCone> CreateColliderCone (float length, float radius);
  virtual csRef<iColliderPlane> CreateColliderPlane (const csPlane3& plane);
  virtual csRef<iColliderTerrain> CreateColliderTerrain (iTerrainSystem* terrain,
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
  virtual csRef<iJoint> CreateRigidP2PJoint (const csVector3 position);
  virtual csRef<iJoint> CreateRigidSlideJoint (const csOrthoTransform trans,
    float minDist, float maxDist, float minAngle, float maxAngle, int axis);
  virtual csRef<iJoint> CreateRigidHingeJoint (const csVector3 position,
    float minAngle, float maxAngle, int axis);
  virtual csRef<iJoint> CreateSoftLinearJoint (const csVector3 position);
  virtual csRef<iJoint> CreateSoftAngularJoint (int axis);
 
  virtual csRef<iSoftBody> CreateRope (csVector3 start,
      csVector3 end, size_t segmentCount);
  virtual csRef<iSoftBody> CreateRope (csVector3* vertices, size_t vertexCount);
  virtual csRef<iSoftBody> CreateCloth (csVector3 corner1, csVector3 corner2,
      csVector3 corner3, csVector3 corner4,
      size_t segmentCount1, size_t segmentCount2,
      bool withDiagonals = false);

  virtual csRef<iSoftBody> CreateSoftBody (iGeneralFactoryState* genmeshFactory, 
    const csOrthoTransform& bodyTransform);

  virtual csRef<iSoftBody> CreateSoftBody (csVector3* vertices,
      size_t vertexCount, csTriangle* triangles,
      size_t triangleCount);
  float getInverseInternalScale() {return inverseInternalScale;}
  float getInternalScale() {return internalScale;}
};
}
CS_PLUGIN_NAMESPACE_END(Bullet2)

#endif
