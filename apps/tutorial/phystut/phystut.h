/*
    Copyright (C) 2001 by Jorrit Tyberghein

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __PHYSTUT_H__
#define __PHYSTUT_H__

#include "cstool/demoapplication.h"
#include "ivaria/dynamics.h"
#include "ivaria/bullet.h"
#include "ivaria/ode.h"
#include "ivaria/dynamicsdebug.h"
#include "ivaria/softanim.h"
#include "imesh/animesh.h"
#include "imesh/animnode/ragdoll.h"

class Simple : public CS::Utility::DemoApplication
{
private:
  // Physics related
  csRef<iDynamics> dynamics;
  csRef<iDynamicSystem> dynamicSystem;
  csRef<CS::Physics::Bullet::iDynamicSystem> bulletDynamicSystem;
  csRef<CS::Debug::iDynamicsDebuggerManager> debuggerManager;
  csRef<CS::Debug::iDynamicSystemDebugger> dynamicsDebugger;
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
  csRef<iRigidBody> cameraBody;

  // Ragdoll related
  csRef<CS::Animation::iSkeletonRagdollNodeManager> ragdollManager;
  CS::Animation::StateID ragdollState;
  csRef<iMeshWrapper> ragdollMesh;

  // Dragging related
  bool dragging;
  bool softDragging;
  csRef<CS::Physics::Bullet::iPivotJoint> dragJoint;
  csRef<CS::Physics::Bullet::iSoftBody> draggedBody;
  size_t draggedVertex;
  float dragDistance;
  float linearDampening, angularDampening;

  // Cut & Paste related
  csRef<iRigidBody> clipboardBody;
  csRef<iMeshWrapper> clipboardMesh;

  //-- csBaseEventHandler
  void Frame ();
  bool OnKeyboard (iEvent &event);
  bool OnMouseDown (iEvent &event);
  bool OnMouseUp (iEvent &event);

  // Camera
  void UpdateCameraMode ();

  // Spawning objects
  bool SpawnStarCollider ();
  iRigidBody* SpawnBox ();
  iRigidBody* SpawnSphere ();
  iRigidBody* SpawnCylinder ();
  iRigidBody* SpawnCapsule (float length = rand() % 3 / 50.f + .7f,
			    float radius = rand() % 10 / 50.f + .2f);
  iRigidBody* SpawnMesh ();
  iRigidBody* SpawnConvexMesh ();
  iRigidBody* SpawnCompound ();
  iJoint* SpawnJointed ();
  void SpawnChain ();
  void LoadRagdoll ();
  void SpawnRagdoll ();
  void SpawnRope ();
  void SpawnCloth ();
  void SpawnSoftBody ();

  void CreateWalls (const csVector3& radius);
  void CreateTerrain ();

public:
  Simple ();
  ~Simple ();

  //-- CS::Utility::DemoApplication
  void PrintHelp ();
  bool OnInitialize (int argc, char* argv[]);
  bool Application ();

  friend class MouseAnchorAnimationControl;
  csRef<CS::Physics::Bullet::iAnchorAnimationControl> grabAnimationControl;
};

class MouseAnchorAnimationControl : public scfImplementation1
<MouseAnchorAnimationControl, CS::Physics::Bullet::iAnchorAnimationControl>
{
 public:
  MouseAnchorAnimationControl (Simple* simple)
    : scfImplementationType (this), simple (simple) {}

  csVector3 GetAnchorPosition () const;

 private:
  Simple* simple;
};

#endif // __PHYSTUT_H__
