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

#include "csutil/custom_new_disable.h"
#include <CEGUI.h>
#include "csutil/custom_new_enable.h"

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
#define DYNDEBUG_BULLET 4

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

  // Creation of objects
  virtual bool CreateAvatar () = 0;

  // User interaction with the scene
  virtual void ResetScene () = 0;

  // Display of information on the state of the scene
  virtual void UpdateStateDescription () = 0;

  // Switch fur dynamics
  virtual void SwitchFurPhysics() = 0;

  // Animesh
  csRef<iAnimatedMeshFactory> animeshFactory;
  csRef<iAnimatedMesh> animesh;
  // Fur material
  csRef<iFurMaterial> furMaterial;
};

class HairTest : public CS::Demo::DemoApplication
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

  // GUI related
  csRef<iVFS> vfs;
  csRef<iCEGUI> cegui;
  csRef<iShaderVarStringSet> svStrings;
  CEGUI::Scrollbar* sliderShiftR;
  CEGUI::Scrollbar* sliderWidthR;
  CEGUI::Scrollbar* sliderAbsorption;
  CEGUI::Scrollbar* sliderEccentricity;
  CEGUI::Scrollbar* sliderGlintScale;
  CEGUI::Scrollbar* sliderCausticWidth;
  CEGUI::Scrollbar* sliderCausticMerge;
  CEGUI::Scrollbar* sliderR;
  CEGUI::Scrollbar* sliderG;
  CEGUI::Scrollbar* sliderB;
  CEGUI::Scrollbar* sliderGuideLOD;
  CEGUI::Scrollbar* sliderStrandLOD;
  CEGUI::Scrollbar* sliderOverallLOD;

  //-- csBaseEventHandler
  void Frame ();
  bool OnKeyboard (iEvent &event);
  bool OnMouseDown (iEvent &event);

  // Switch Active Colliders displayed
  void SwitchDynamics();

  //-- csDemoApplication
  csVector3 GetCameraStart ();
  float GetCameraMinimumDistance ();
  csVector3 GetCameraTarget ();

public:
  HairTest ();
  ~HairTest ();

  // Handle exit button clicked event
  bool OnExitButtonClicked (const CEGUI::EventArgs& e);
  bool OnCollidersButtonClicked (const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedShiftR (const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedWidthR (const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedAbsorption (const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedEccentricity (const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedGlintScale (const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedCausticWidth (const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedCausticMerge (const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedR (const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedG (const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedB (const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedGuideLOD (const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedStrandLOD (const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedOverallLOD (const CEGUI::EventArgs& e);
  bool OnPhysicsButtonClicked (const CEGUI::EventArgs& e);

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

