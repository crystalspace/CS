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

#ifndef __SNDTEST_H__
#define __SNDTEST_H__

#include <crystalspace.h>
#include <isndsys/ss_renderer.h>
#include <isndsys/ss_loader.h>

/**
 * The main class for the sound test.
 */
class SndTest : public csApplicationFramework, public csBaseEventHandler
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

  /// The sound renderer.
  csRef<iSndSysRenderer> sndrenderer;

  /// The sound loader.
  csRef<iSndSysLoader> sndloader;

  /// A pointer to the sector the camera will be in.
  iSector* room;

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

  /// Here we will create our little, simple world.
  void CreateRoom ();

public:

  /// Construct our game. This will just set the application ID for now.
  SndTest ();

  /// Destructor.
  ~SndTest ();

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
   * by SndTest to use Crystal Space), then this routine fires up the main
   * event loop. This is where everything starts. This loop will  basically
   * start firing events which actually causes Crystal Space to function.
   * Only when the program exits this function will return.
   */
  bool Application ();

};

#endif // __SNDTEST_H__
