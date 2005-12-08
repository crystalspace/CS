/*
    Copyright (C) 2005 by Jorrit Tyberghein

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

#ifndef __STARTME_H__
#define __STARTME_H__

#include <crystalspace.h>

#define DEMO_MESH_Z	      14.0f
#define DEMO_MESH_MINX	      -6.0f
#define DEMO_MESH_MAXX	      6.0f
#define DEMO_MESH_MINY	      -5.0f
#define DEMO_MESH_MAXY	      5.0f

#define MAIN_LIGHT_ON	      csColor (.3f, .3f, .3f)
#define MAIN_LIGHT_OFF	      csColor (.05f, .05f, .05f)
#define POINTER_LIGHT_ON      csColor (.5f, 1, .4f)
#define POINTER_LIGHT_OFF     csColor (0, 0, 0)

struct DemoData
{
  const char* name;
  const char* exec;
  const char* args;
  csRef<iString> description;
  const char* image;
  csRef<iMeshWrapper> mesh;
  float spinning_speed;

  DemoData () : spinning_speed (0.0f) { }
};

struct StarInfo
{
  csRef<iMeshWrapper> star;
  csRef<iGeneralMeshState> stars_state;
  float r;	// Value between 0 and 1 indicating animation state.
  bool inqueue;	// True if in queue.

  StarInfo () : r (0), inqueue (false) { }
};

/**
 * This is the Crystal Space Demo Launcher program.
 */
class StartMe : public csApplicationFramework, public csBaseEventHandler
{
private:

  /// A pointer to the 3D engine.
  csRef<iEngine> engine;

  /// A pointer to the map loader plugin.
  csRef<iLoader> loader;

  /// A pointer to the 3D renderer plugin.
  csRef<iGraphics3D> g3d;

  /// The virtual file system.
  csRef<iVFS> vfs;

  /// Collision Detection System.
  csRef<iCollideSystem> cdsys;

  /// A pointer to the keyboard driver.
  csRef<iKeyboardDriver> kbd;

  /// A pointer to the mouse driver.
  csRef<iMouseDriver> mouse;

  /// A pointer to the virtual clock.
  csRef<iVirtualClock> vc;

  /// A pointer to the view which contains the camera.
  csRef<iView> view;

  /// A dynamic light (our pointer).
  csRef<iLight> pointer_light;
  /// The main light.
  csRef<iLight> main_light;

  /// Our font.
  csRef<iFont> font;
  int font_fg, font_bg;

  /// A pointer to the configuration manager.
  csRef<iConfigManager> confman;

  /// A pointer to the sector the camera will be in.
  iSector* room;

  // For the demos.
  csArray<DemoData> demos;
  csRef<iMeshFactoryWrapper> box_fact;
  size_t last_selected;
  size_t description_selected;

  // For the stars.
  csArray<StarInfo> stars;
  csRef<iMeshFactoryWrapper> star_fact;
  size_t cur_star, star_count;
  csTicks star_timeout;
  csTicks star_ticks;
  csArray<int> star_queue;
  float star_fade1, star_fade2, star_maxage;

  virtual bool OnKeyboard (iEvent&);
  virtual bool OnMouseDown (iEvent &event);

  /**
   * Setup everything that needs to be rendered on screen. This routine
   * is called from the event handler in response to a cscmdProcess
   * broadcast message.
   */
  void ProcessFrame ();

  /**
   * Finally render the screen. This routine is called from the event
   * handler in response to a cscmdFinalProcess broadcast message.
   */
  void FinishFrame ();

  /// Here we will create our little, simple world.
  void CreateRoom ();

  /// Load textures.
  bool LoadTextures ();

  /// Load configuration from file.
  void LoadConfig ();

  /// Create a demo mesh.
  csPtr<iMeshWrapper> CreateDemoMesh (const char* name, const csVector3& pos);

  bool InDescriptionMode () const { return description_selected != (size_t)-1; }
  /// Go to description mode.
  void EnterDescriptionMode ();
  /// Leave description mode.
  void LeaveDescriptionMode ();

public:

  /// Construct our game. This will just set the application ID for now.
  StartMe ();

  /// Destructor.
  virtual ~StartMe ();

  /// Final cleanup.
  void OnExit ();

  /**
   * Main initialization routine. This routine will set up some basic stuff
   * (like load all needed plugins, setup the event handler, ...).
   * In case of failure this routine will return false. You can assume
   * that the error message has been reported to the user.
   */
  bool OnInitialize (int argc, char* argv[]);

  /**
   * Run the application.
   * First, there are some more initialization (everything that is needed 
   * by StartMe to use Crystal Space), then this routine fires up the main
   * event loop. This is where everything starts. This loop will  basically
   * start firing events which actually causes Crystal Space to function.
   * Only when the program exits this function will return.
   */
  bool Application ();

  CS_EVENTHANDLER_NAMES ("crystalspace.startme")
  CS_EVENTHANDLER_NIL_CONSTRAINTS
};

#endif // __STARTME_H__
