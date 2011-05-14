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

#include "imesh/animesh.h"
#include "imesh/animnode/lookat.h"
#include "imesh/animnode/ragdoll.h"
#include "imesh/animnode/speed.h"
#include "imesh/furmesh.h"
#include "ivaria/dynamics.h"
#include "ivaria/dynamicsdebug.h"
#include "ivaria/icegui.h"

#include "cstool/demoapplication.h"

// Different states for the display of the physical objects
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
  virtual void UpdateStateDescription () {}

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

class HairTest : public CS::Utility::DemoApplication
{
  friend class KrystalScene;
  friend class FrankieScene;

private:
  AvatarScene* avatarScene;
  int avatarSceneType;

  // Physics related
  bool physicsEnabled;
  bool furMeshEnabled;
  csRef<iDynamics> dynamics;
  csRef<iDynamicSystem> dynamicSystem;
  csRef<CS::Physics::Bullet::iDynamicSystem> bulletDynamicSystem;
  csRef<CS::Debug::iDynamicsDebuggerManager> debuggerManager;
  csRef<CS::Debug::iDynamicSystemDebugger> dynamicsDebugger;
  int dynamicsDebugMode;
  csRef<iSaver> saver;

  // Animation node plugin managers
  csRef<CS::Animation::iSkeletonLookAtNodeManager> lookAtManager;
  csRef<CS::Animation::iSkeletonSpeedNodeManager> basicNodesManager;
  csRef<CS::Animation::iSkeletonRagdollNodeManager> ragdollManager;

  // GUI related
  csRef<iVFS> vfs;
  csRef<iCEGUI> cegui;
  csRef<iShaderVarStringSet> svStrings;
  CEGUI::Scrollbar* sliderR;
  CEGUI::Scrollbar* sliderG;
  CEGUI::Scrollbar* sliderB;
  CEGUI::Scrollbar* sliderA;
  CEGUI::Scrollbar* sliderPointiness;
  CEGUI::Scrollbar* sliderStrandWidth;
  CEGUI::Scrollbar* sliderControlPointsDeviation;
  CEGUI::Scrollbar* sliderGuideLOD;
  CEGUI::Scrollbar* sliderStrandLOD;
  CEGUI::Scrollbar* sliderControlPointsLOD;
  CEGUI::Scrollbar* sliderOverallLOD;
  CEGUI::Combobox* objectComboBox;

  CEGUI::Window* form;
  CEGUI::Window* stddlg;

  //-- Load dialog functions
  void LoadTexture(const char* filename, const char* path, 
    const char* shaderVariable); 

  void StdDlgUpdateLists(const char* filename);

  bool StdDlgOkButton (const CEGUI::EventArgs& e);
  bool StdDlgCancleButton (const CEGUI::EventArgs& e);
  bool StdDlgFileSelect (const CEGUI::EventArgs& e);
  bool StdDlgDirSelect (const CEGUI::EventArgs& e);
  bool StdDlgDirChange (const CEGUI::EventArgs& e);

  bool LoadDiffuseMap (const CEGUI::EventArgs& e);
  bool UnloadDiffuseMap (const CEGUI::EventArgs& e);
  bool LoadTextureMap (const CEGUI::EventArgs& e);
  bool UnloadTextureMap (const CEGUI::EventArgs& e);
  bool LoadColorMap (const CEGUI::EventArgs& e);
  bool UnloadColorMap (const CEGUI::EventArgs& e);

  //-- csBaseEventHandler
  void Frame ();
  bool OnKeyboard (iEvent &event);
  bool OnMouseDown (iEvent &event);

  // Switch Active Colliders displayed
  void SwitchDynamics();

public:
  HairTest ();
  ~HairTest ();

  void PrintHelp ();

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
  bool OnEventThumbTrackEndedR (const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedG (const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedB (const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedA (const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedPointiness (const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedStrandWidth (const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedControlPointsDeviation(const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedGuideLOD (const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedStrandLOD (const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedControlPointsLOD (const CEGUI::EventArgs& e);
  bool OnEventThumbTrackEndedOverallLOD (const CEGUI::EventArgs& e);
  bool OnPhysicsButtonClicked (const CEGUI::EventArgs& e);
  bool OnEnableButtonClicked (const CEGUI::EventArgs& e);
  bool OnEventListSelectionChanged (const CEGUI::EventArgs& e);

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

