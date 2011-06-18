#ifndef __PHYSTUT2_H__
#define __PHYSTUT2_H__

#include "cstool/demoapplication.h"
#include "ivaria/physical2.h"
#include "ivaria/bullet2.h"
#include "ivaria/collision2.h"
#include "ivaria/softanim.h"
#include "imesh/animesh.h"
#include "imesh/animnode/ragdoll.h"

class Simple : public CS::Utility::DemoApplication
{
private:
  csRef<CS::Collision::iCollisionSystem> collisionSystem;
  csRef<CS::Physics::iPhysicalSystem> physicalSystem;
  csRef<CS::Collision::iCollisionSector> collisionSector;
  csRef<CS::Physics::iPhysicalSector> physicalSector;
  csRef<CS::Physics::Bullet::iPhysicalSector> bulletSector;
  csRef<CS::Animation::iSoftBodyAnimationControlFactory> softBodyAnimationFactory;
  bool isSoftBodyWorld;

  // Meshes
  csRef<iMeshFactoryWrapper> boxFact;
  csRef<iMeshFactoryWrapper> meshFact;

  // Environments
  int environment;
  csRef<iMeshWrapper> walls;

  // Configuration related
  int solver;
  bool autodisable;
  csString phys_engine_name;
  int phys_engine_id;
  bool do_bullet_debug;
  float remainingStepDuration;

  // Dynamic simulation related
  bool debugMode;
  bool allStatic;
  bool pauseDynamic;
  float dynamicSpeed;

  // Camera related
  int physicalCameraMode;
  csRef<CS::Physics::iRigidBody> cameraBody;
  float rotX, rotY, rotZ;

  // Ragdoll related
  csRef<CS::Animation::iSkeletonRagdollNodeManager> ragdollManager;
  CS::Animation::StateID ragdollState;
  csRef<iMeshWrapper> ragdollMesh;

  // Dragging related
  bool dragging;
  bool softDragging;
  csRef<CS::Physics::iJoint> dragJoint;
  csRef<CS::Physics::iSoftBody> draggedBody;
  size_t draggedVertex;
  float dragDistance;
  float linearDampening, angularDampening;

  // Cut & Paste related
  csRef<CS::Physics::iPhysicalBody> clipboardBody;
  csRef<iMovable> clipboardMovable;

  csOrthoTransform localTrans;

private:
  void Frame ();
  bool OnKeyboard (iEvent &event);

  bool OnMouseDown (iEvent &event);
  bool OnMouseUp (iEvent &event);

  // Camera
  void UpdateCameraMode ();

  // Spawning objects
  bool SpawnStarCollider ();
  CS::Physics::iRigidBody* SpawnBox ();
  CS::Physics::iRigidBody* SpawnSphere ();
  CS::Physics::iRigidBody* SpawnCone ();
  CS::Physics::iRigidBody* SpawnCylinder ();
  CS::Physics::iRigidBody* SpawnCapsule (float length = rand() % 3 / 50.f + .7f,
    float radius = rand() % 10 / 50.f + .2f);
  CS::Physics::iRigidBody* SpawnConcaveMesh ();
  CS::Physics::iRigidBody* SpawnConvexMesh ();
  CS::Physics::iRigidBody* SpawnCompound ();
  CS::Physics::iJoint* SpawnJointed ();
  void SpawnChain ();
  void LoadRagdoll ();
  void SpawnRagdoll ();
  void SpawnRope ();
  void SpawnCloth ();
  CS::Physics::iSoftBody* SpawnSoftBody ();

  void CreateWalls (const csVector3& radius);
  void CreateTerrain ();

public:
  Simple ();
  virtual ~Simple ();

  void PrintHelp ();
  bool OnInitialize (int argc, char* argv[]);
  bool Application ();

  friend class MouseAnchorAnimationControl;
  csRef<CS::Physics::iAnchorAnimationControl> grabAnimationControl;
};

class MouseAnchorAnimationControl : public scfImplementation1
  <MouseAnchorAnimationControl, CS::Physics::iAnchorAnimationControl>
{
public:
  MouseAnchorAnimationControl (Simple* simple)
    : scfImplementationType (this), simple (simple) {}

  csVector3 GetAnchorPosition () const;

private:
  Simple* simple;
};

#endif