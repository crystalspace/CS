/*
  Copyright (C) 2010 Alexandru - Teodor Voicu

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
#ifndef __HAIRTEST_H__
#define __HAIRTEST_H__

#include "crystalspace.h"

#include "cstool/csdemoapplication.h"
#include "imesh/animesh.h"
#include "imesh/ragdoll.h"
#include "imesh/lookat.h"
#include "imesh/basicskelanim.h"
#include "ivaria/dynamics.h"
#include "ivaria/bullet.h"
#include "ivaria/dynamicsdebug.h"

#include "imaterial/furmaterial.h"

#include "ivaria/icegui.h"

#define DYNDEBUG_NONE 1
#define DYNDEBUG_MIXED 2
#define DYNDEBUG_COLLIDER 3

// Base class to be implemented for all different models
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

  // Animesh
  csRef<iAnimatedMeshFactory> animeshFactory;
  csRef<iAnimatedMesh> animesh;
};

class HairTest : public csDemoApplication
{
  friend class KrystalScene;

private:
  AvatarScene* avatarScene;

  // Physics related
  bool physicsEnabled;
  csRef<iDynamics> dynamics;
  csRef<iDynamicSystem> dynamicSystem;
  csRef<iBulletDynamicSystem> bulletDynamicSystem;
  csRef<iDynamicsDebuggerManager> debuggerManager;
  csRef<iDynamicSystemDebugger> dynamicsDebugger;
  int dynamicsDebugMode;

  // Animation node plugin managers
  csRef<iSkeletonLookAtManager2> lookAtManager;
  csRef<iSkeletonBasicNodesManager2> basicNodesManager;
  csRef<iSkeletonRagdollManager2> ragdollManager;

  // Material plugin manager
  csRef<iFurMaterialType> furMaterialType;
  csRef<iFurMaterial> furMaterial;

  // GUI related
  csRef<iVFS> vfs;
  csRef<iCEGUI> cegui;

  bool mouseMovement;

  //-- csBaseEventHandler
  void Frame ();
  bool OnKeyboard (iEvent &event);
  bool OnMouseDown (iEvent &event);
  bool OnMouseUp (iEvent &event);
  bool OnMouseMove (iEvent &event);

  int lastMouseX, lastMouseY;

  // Switch Active Colliders displayed
  void SwitchDynamics();
public:
  HairTest ();
  ~HairTest ();

  // Handle exit button clicked event
  bool OnExitButtonClicked (const CEGUI::EventArgs& e);
  bool OnCollidersButtonClicked (const CEGUI::EventArgs& e);
  
  //-- csApplicationFramework
  bool OnInitialize (int argc, char* argv[]);
  bool Application ();

  // Declare the name of this event handler.
  CS_EVENTHANDLER_NAMES("application.hairtest")
      
  /* Declare that we're not terribly interested in having events
     delivered to us before or after other modules, plugins, etc. */
  CS_EVENTHANDLER_NIL_CONSTRAINTS
};

#endif // __HAIRTEST_H__

