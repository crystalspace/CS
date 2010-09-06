/*
  Copyright (C) 2010 Alexandru - Teodor Voicu
      Faculty of Automatic Control and Computer Science of the "Politehnica"
      University of Bucharest
      http://csite.cs.pub.ro/index.php/en/

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

#include "imesh/furmesh.h"
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

  // Kill the avatar
  virtual void KillAvatar () = 0;

  // Display of information on the state of the scene
  virtual void UpdateStateDescription () = 0;

  // Switch fur dynamics
  virtual void SwitchFurPhysics() = 0;

  // Save fur
  virtual void SaveFur() = 0;

  // Animesh
  csRef<CS::Mesh::iAnimatedMeshFactory> animeshFactory;
  csRef<CS::Mesh::iAnimatedMesh> animesh;
  // Fur material
  csRef<CS::Mesh::iFurMesh> furMesh;
};

class HairTest : public CS::Demo::DemoApplication
{
  friend class KrystalScene;
  friend class FrankieScene;

private:
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
  csRef<iSaver> saver;

  // Animation node plugin managers
  csRef<CS::Animation::iSkeletonLookAtManager> lookAtManager;
  csRef<CS::Animation::iSkeletonSpeedNodeManager> basicNodesManager;
  csRef<CS::Animation::iSkeletonRagdollManager> ragdollManager;

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
  CEGUI::Scrollbar* sliderControlPointsLOD;
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

  // Save fur
  void SaveFactory(iMeshFactoryWrapper* meshfactwrap, const char * filename);
  void SaveObject(iMeshWrapper* meshwrap, const char * filename);

  // Handle exit button clicked event
  bool OnSaveButtonClicked (const CEGUI::EventArgs& e);
  bool OnExitButtonClicked (const CEGUI::EventArgs& e);
  bool OnCollidersButtonClicked (const CEGUI::EventArgs& e);
  bool OnSceneButtonClicked (const CEGUI::EventArgs& e);
  bool OnKillButtonClicked (const CEGUI::EventArgs& e);
  bool OnResetButtonClicked (const CEGUI::EventArgs& e);
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
  bool OnEventThumbTrackEndedControlPointsLOD (const CEGUI::EventArgs& e);
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

private:
  void SwitchScenes();
};

#endif // __HAIRTEST_H__

