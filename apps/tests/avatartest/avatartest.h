/*
  Copyright (C) 2009-10 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef __AVATARTEST_H__
#define __AVATARTEST_H__

#include "cstool/csdemoapplication.h"
#include "imesh/animesh.h"
#include "imesh/animnode/basicskelanim.h"
#include "imesh/animnode/ik.h"
#include "imesh/animnode/lookat.h"
#include "imesh/animnode/ragdoll.h"
#include "ivaria/bullet.h"
#include "ivaria/decal.h"
#include "ivaria/dynamics.h"
#include "ivaria/dynamicsdebug.h"
#include "ivaria/softanim.h"

#define DYNDEBUG_NONE 1
#define DYNDEBUG_MIXED 2
#define DYNDEBUG_COLLIDER 3
#define DYNDEBUG_BULLET 4

// Base class to be implemented for all different scenes
class AvatarScene
{
 public:
  virtual ~AvatarScene () {}

  // Camera related
  virtual csVector3 GetCameraStart () = 0;
  virtual float GetCameraMinimumDistance () = 0;
  virtual csVector3 GetCameraTarget () = 0;

  // Dynamic simuation related
  virtual float GetSimulationSpeed () = 0;
  virtual bool HasPhysicalObjects () = 0;

  // From csBaseEventHandler
  virtual void Frame () = 0;
  virtual bool OnKeyboard (iEvent &event) = 0;
  virtual bool OnMouseDown (iEvent &event) = 0;

  // Creation of objects
  virtual bool CreateAvatar () = 0;

  // User interaction with the scene
  virtual void ResetScene () = 0;

  // Display of information on the state of the scene
  virtual void UpdateStateDescription () = 0;

  // Animesh objects
  csRef<CS::Mesh::iAnimatedMeshFactory> animeshFactory;
  csRef<CS::Mesh::iAnimatedMesh> animesh;
};

class AvatarTest : public CS::Demo::DemoApplication
{
  friend class FrankieScene;
  friend class KrystalScene;
  friend class SintelScene;

private:
  // Current scene
  AvatarScene* avatarScene;
  int avatarSceneType;

  // Physics related
  bool physicsEnabled;
  csRef<iDynamics> dynamics;
  csRef<iDynamicSystem> dynamicSystem;
  csRef<CS::Physics::Bullet::iDynamicSystem> bulletDynamicSystem;
  csRef<CS::Debug::iDynamicsDebuggerManager> debuggerManager;
  csRef<CS::Debug::iDynamicSystemDebugger> dynamicsDebugger;
  int dynamicsDebugMode;

  // Soft bodies related
  csRef<CS::Animation::iSoftBodyAnimationControlType> softBodyAnimationType;
  csRef<CS::Animation::iSoftBodyAnimationControlFactory> softBodyAnimationFactory;
  bool softBodiesEnabled;

  // Animation node plugin managers
  csRef<CS::Animation::iSkeletonIKManager> IKManager;
  csRef<CS::Animation::iSkeletonLookAtManager> lookAtManager;
  csRef<CS::Animation::iSkeletonBasicNodesManager> basicNodesManager;
  csRef<CS::Animation::iSkeletonRagdollManager> ragdollManager;

  // Decal textures
  csRef<iDecalManager> decalManager;

  //-- csBaseEventHandler
  void Frame ();
  bool OnKeyboard (iEvent &event);
  bool OnMouseDown (iEvent &event);

  //-- CS::Demo::CameraManager
  csVector3 GetCameraStart ();
  float GetCameraMinimumDistance ();
  csVector3 GetCameraTarget ();

  // HitBeam test for mouse pointing at the animesh
  bool HitBeamAnimatedMesh (csVector3& isect, csVector3& direction, int& triangle);

 public:
  AvatarTest ();
  ~AvatarTest ();

  //-- csApplicationFramework
  bool OnInitialize (int argc, char* argv[]);
  bool Application ();
};

#endif // __AVATARTEST_H__

