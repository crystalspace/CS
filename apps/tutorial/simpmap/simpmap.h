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

#ifndef __SIMPMAP_H__
#define __SIMPMAP_H__

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

  /// A pointer to the collision detection system.
  csRef<iCollideSystem> cdsys;

  /// A pointer to the view which contains the camera.
  csRef<iView> view;

  /// A pointer to the sector the camera will be in.
  iSector* room;

  /// Our collider used for gravity and CD.
  csColliderActor collider_actor;

  /**
   * Handle keyboard events - ie key presses and releases.
   * This routine is called from the event handler in response to a 
   * csevKeyboard event.
   */
  bool OnKeyboard (iEvent&);

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

  /// Here we will load our world from a map file.
  bool LoadMap ();

public:

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

};

#endif // __SIMPMAP_H__
