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

#ifndef __SIMPCD_H__
#define __SIMPCD_H__

#include <crystalspace.h>

/**
* This is the main class of this Tutorial. It contains the
* basic initialization code and the main event handler.
*
* csApplicationFramework provides a handy object-oriented wrapper around the
* Crystal Space initialization and start-up functions.
*
* csBaseEventHandler provides a base object which does absolutely nothing
* with the events that are sent to it.
*/
class Simple : public csApplicationFramework, public csBaseEventHandler
{
private:
  /// A pointer to the 3D engine.
  csRef<iEngine> engine;

  /// A pointer to the map loader plugin.
  csRef<iLoader> loader;

  /// A pointer to the 3D renderer plugin.
  csRef<iGraphics3D> g3d;

  /// A pointer to the keyboard driver.
  csRef<iKeyboardDriver> kbd;

  /// A pointer to the virtual clock.
  csRef<iVirtualClock> vc;

  /// A pointer to the view which contains the camera.
  csRef<iView> view;

  /// The render manager, cares about selecting lights+meshes to render
  csRef<iRenderManager> rm;

  /// The collision plugin, creates colliders
  csRef<iCollideSystem> cdsys;

  /// A pointer to the sector the camera will be in.
  iSector* room;

  /// The anchor sprite
  csRef<iMeshWrapper> parent_sprite;
  
  /// Determines in which direction sprite1 will rotate
  float rot1_direction;
  
  /// The first of the two colliding meshes
  csRef<iMeshWrapper> sprite1;
  
  /// sprite1's collider
  iCollider* sprite1_col;

  /// Determines in which direction sprite2 will rotate
  float rot2_direction;

  /// The second of the two colliding meshes
  csRef<iMeshWrapper> sprite2;

  /// sprite2's collider
  iCollider* sprite2_col;

  /// Current orientation of the camera.
  float rotX, rotY;

  /// Event handlers to draw and print the 3D canvas on each frame
  csRef<FrameBegin3DDraw> drawer;
  csRef<FramePrinter> printer;

  /// Helper function to create a collider for a mesh
  iCollider* InitCollider (iMeshWrapper* mesh);

public:
  bool SetupModules ();

  /**
  * Handle keyboard events - ie key presses and releases.
  * This routine is called from the event handler in response to a 
  * csevKeyboard event.
  */
  bool OnKeyboard (iEvent&);

  /**
  * Setup everything that needs to be rendered on screen. This routine
  * is called from the event handler in response to a csevFrame
  * message, and is called in the "logic" phase (meaning that all
  * event handlers for 3D, 2D, Console, Debug, and Frame phases
  * will be called after this one).
  */
  void Frame ();

  /// Here we will create our little, simple world.
  void CreateRoom ();

  /// Construct our game. This will just set the application ID for now.
  Simple ();

  /// Destructor.
  ~Simple ();

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
  * by Simple1 to use Crystal Space), then this routine fires up the main
  * event loop. This is where everything starts. This loop will  basically
  * start firing events which actually causes Crystal Space to function.
  * Only when the program exits this function will return.
  */
  bool Application ();

  /* Declare the name by which this class is identified to the event scheduler.
  * Declare that we want to receive the frame event in the "LOGIC" phase,
  * and that we're not terribly interested in having other events
  * delivered to us before or after other modules, plugins, etc. */
  CS_EVENTHANDLER_PHASE_LOGIC("application.simpcd")
};

#endif // __SIMPCD_H__
