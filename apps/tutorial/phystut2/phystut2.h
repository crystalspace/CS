#ifndef __PHYSTUT2_H__
#define __PHYSTUT2_H__

#include "cstool/demoapplication.h"
#include "ivaria/physical2.h"
#include "ivaria/bullet2.h"
#include "ivaria/collision2.h"
#include "imesh/animesh.h"
#include "imesh/animnode/ragdoll2.h"

class Simple : public CS::Utility::DemoApplication
{
private:
  csRef<CS::Collision2::iCollisionSystem> collisionSystem;
  csRef<CS::Physics2::iPhysicalSystem> physicalSystem;
  csRef<CS::Collision2::iCollisionSector> collisionSector;
  csRef<CS::Physics2::iPhysicalSector> physicalSector;
  csRef<CS::Physics2::Bullet2::iPhysicalSector> bulletSector;
  csRef<CS::Physics2::iSoftBodyAnimationControlFactory> softBodyAnimationFactory;
  csRefArray<CS::Physics2::iRigidBody> dynamicBodies;
  bool isSoftBodyWorld;

  // Meshes
  csRef<iMeshFactoryWrapper> boxFact;
  csRef<iMeshFactoryWrapper> meshFact;
  csRef<CS::Collision2::iColliderConcaveMesh> mainCollider;


  // Environments
  int environment;
  csRef<iMeshWrapper> walls;

  // Configuration related
  int solver;
  bool autodisable;
  csString phys_engine_name;
  int phys_engine_id;
  bool do_bullet_debug;
  bool do_soft_debug;
  float remainingStepDuration;

  // Dynamic simulation related
  bool allStatic;
  bool pauseDynamic;
  float dynamicSpeed;

  // Camera related
  CS::Physics2::Bullet2::DebugMode debugMode;
  int physicalCameraMode;
  csRef<CS::Physics2::iRigidBody> cameraBody;
  csRef<CS::Collision2::iCollisionActor> cameraActor;

  // Ragdoll related
  csRef<CS::Animation::iSkeletonRagdollNodeManager2> ragdollManager;

  // Dragging related
  bool dragging;
  bool softDragging;
  csRef<CS::Physics2::iJoint> dragJoint;
  csRef<CS::Physics2::iSoftBody> draggedBody;
  
  size_t draggedVertex;
  float dragDistance;
  float linearDampening, angularDampening;

  // Cut & Paste related
  csRef<CS::Physics2::iPhysicalBody> clipboardBody;
  csRef<iMovable> clipboardMovable;

  // Collider
  csOrthoTransform localTrans;

  // Ghost
  csRef<CS::Collision2::iCollisionObject> ghostObject;

private:
  void Frame ();
  bool OnKeyboard (iEvent &event);

  bool OnMouseDown (iEvent &event);
  bool OnMouseUp (iEvent &event);

  // Camera
  void UpdateCameraMode ();

  // Spawning objects
  bool SpawnStarCollider ();
  CS::Physics2::iRigidBody* SpawnBox (bool setVelocity = true);
  CS::Physics2::iRigidBody* SpawnSphere (bool setVelocity = true);
  CS::Physics2::iRigidBody* SpawnCone (bool setVelocity = true);
  CS::Physics2::iRigidBody* SpawnCylinder (bool setVelocity = true);
  CS::Physics2::iRigidBody* SpawnCapsule (float length = rand() % 3 / 50.f + .7f,
    float radius = rand() % 10 / 50.f + .2f, bool setVelocity = true);
  CS::Collision2::iCollisionObject* SpawnConcaveMesh ();
  CS::Physics2::iRigidBody* SpawnConvexMesh (bool setVelocity = true);
  CS::Physics2::iRigidBody* SpawnCompound (bool setVelocity = true);
  CS::Physics2::iJoint* SpawnJointed ();
  CS::Physics2::iRigidBody* SpawnFilterBody (bool setVelocity = true);
  void SpawnChain ();
  void LoadFrankieRagdoll ();
  void LoadKrystalRagdoll ();
  void SpawnFrankieRagdoll ();
  void SpawnKrystalRagdoll ();
  void SpawnRope ();
  CS::Physics2::iSoftBody* SpawnCloth ();
  CS::Physics2::iSoftBody* SpawnSoftBody (bool setVelocity = true);

  void CreateBoxRoom ();
  void CreatePortalRoom ();
  void CreateTerrainRoom ();

  void CreateGhostCylinder ();
  void GripContactBodies ();

public:
  Simple ();
  virtual ~Simple ();

  void PrintHelp ();
  bool OnInitialize (int argc, char* argv[]);
  bool Application ();

  friend class MouseAnchorAnimationControl;
  csRef<CS::Physics2::iAnchorAnimationControl> grabAnimationControl;
};

class MouseAnchorAnimationControl : public scfImplementation1
  <MouseAnchorAnimationControl, CS::Physics2::iAnchorAnimationControl>
{
public:
  MouseAnchorAnimationControl (Simple* simple)
    : scfImplementationType (this), simple (simple) {}

  csVector3 GetAnchorPosition () const;

private:
  Simple* simple;
};

#endif
