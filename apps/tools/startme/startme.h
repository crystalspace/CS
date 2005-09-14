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

#define DEMO_MESH_Z 14.0f
#define DEMO_MESH_MINX -6.0f
#define DEMO_MESH_MAXX  6.0f
#define DEMO_MESH_MINY -5.0f
#define DEMO_MESH_MAXY  5.0f
#define DEMO_COUNT 12

#define STAR_COUNT 100	// Maximum number of stars.
#define STAR_NEWSTAR_TIMEOUT 10	// Milliseconds before a new star is born.
#define STAR_MAXAGE 0.5	// Maximum age of a star in seconds.
#define STAR_FADE_1 .2	// Value at which we stop fading in.
#define STAR_FADE_2 .4	// Value at which we start fading out again.

#define MAIN_LIGHT_ON csColor (.3, .3, .3)
#define MAIN_LIGHT_OFF csColor (.05, .05, .05)
#define POINTER_LIGHT_ON csColor (.5, 1, .4)
#define POINTER_LIGHT_OFF csColor (0, 0, 0)

struct DemoData
{
  const char* name;
  const char* command;
  const char* description;
  const char* image;
};

DemoData demos[DEMO_COUNT] =
{
  { "awstest",
  	"awstest.exe -silent",
	"AwsTest is a test application that demonstrates our built-in#\
window system. It nicely integrates with the 3D engine. You can see#\
a 3D view in the background and another 3D view on the same world#\
in a window.",
  	"startme_awstest.jpg" },
  { "ceguitest",
  	"ceguitest.exe -silent",
	"CeGUITest is a test application for a window system based on#\
CeGUI.",
  	"startme_ceguitest.jpg" },
  { "csdemo",
  	"csdemo.exe -silent -mode=800x600",
	"CsDemo is a generic demo application. We currently have only one#\
demo file which is 'demodata.zip'. Just let it run and watch.",
  	"startme_csdemo.jpg" },
  { "isotest",
  	"isotest.exe -silent -mode=800x600",
	"Crystal Space is a real 3D engine. However with some tweaking you#\
can force it into emulating something that resembles isometric#\
a bit. This example also uses stencil shadows.",
  	"startme_isotest.jpg" },
  { "lightningtest",
  	"lghtngtest.exe -silent -mode=800x600",
	"A small demo showing the lightning mesh",
  	"startme_lightningtest.jpg" },
  { "parallaxtest",
  	"walktest.exe -silent parallaxtest -mode=800x600",
	"This is WalkTest with the parallaxtest map. It shows parallax#\
mapping in Crystal Space. You need pretty good hardware to see#\
the effects. If you don't have the needed hardware support you will#\
see nothing special.",
  	"startme_parallaxtest.jpg" },
  { "particles",
  	"walktest.exe -silent particles -mode=800x600",
	"This is WalkTest with the particles map. It is a very simple demo#\
that shows two particle systems.",
  	"startme_particles.jpg" },
  { "partsys",
  	"walktest.exe -silent partsys -mode=800x600 -relight",
	"This is WalkTest with the partsys map. It is a very simple demo#\
that demonstrates several particle systems in Crystal Space.",
  	"startme_partsys.jpg" },
  { "terrain",
  	"walktest.exe -silent terrain -mode=800x600",
	"This is WalkTest with the terrain map. It demonstrates the landscape#\
engine in Crystal Space. There are two versions of this map. This one#\
is 'terrain' which looks nice but needs good hardware to run nicely.#\
'terrainf' is a lot faster.",
  	"startme_terrain.jpg" },
  { "terrainf",
  	"walktest.exe -silent terrainf -mode=800x600 -relight",
	"This is WalkTest with the terrainf map. It demonstrates the#\
landscape engine in Crystal Space. There are two versions of this#\
map. This one is 'terrainf' which is usually faster then 'terrain'#\
and doesn't require advanced 3D hardware.",
  	"startme_terrainf.jpg" },
  { "walktest",
  	"walktest.exe -silent -mode=800x600 -relight",
	"This is WalkTest with the default flarge map. This really is an#\
ugly map. It was completely created in a text editor. The purpose#\
of this map is to be a general test environment for the Crystal#\
Space developers. It contains a lot of objects and it can give you#\
some ideas on what is possible.",
  	"startme_walktest.jpg" },
  { "waterdemo",
  	"waterdemo.exe -silent -mode=800x600",
	"WaterDemo is a small demo of a water effect in Crystal Space.#\
Just press space bar to see the effect.",
  	"startme_waterdemo.jpg" }
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

  /// A pointer to the sector the camera will be in.
  iSector* room;

  // For the demos.
  csRef<iMeshFactoryWrapper> box_fact;
  csRef<iMeshWrapper> meshes[DEMO_COUNT];
  float spinning_speed[DEMO_COUNT];
  int last_selected;
  int description_selected;

  // For the stars.
  csRef<iMeshFactoryWrapper> star_fact;
  StarInfo stars[STAR_COUNT];
  int cur_star;
  csTicks star_ticks;
  csArray<int> star_queue;

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

  /// Create a demo mesh.
  csPtr<iMeshWrapper> CreateDemoMesh (const char* name, const csVector3& pos);

  bool InDescriptionMode () const { return description_selected != -1; }
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
};

#endif // __STARTME_H__
