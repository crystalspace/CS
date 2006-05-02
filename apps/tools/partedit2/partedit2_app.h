/*
    Copyright (C) 2006 by Christopher Nelson

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

#ifndef __PARTEDIT2_APP_H__
#define __PARTEDIT2_APP_H__

#include <crystalspace.h>

/**
 * The PartEdit2 Application
 */
class PartEdit2 : public csApplicationFramework, public csBaseEventHandler
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

  /// A pointer to the sector the camera will be in.
  iSector* room;
  
  /// The AWS2 plugin.
  csRef<iAws2> aws;

  /// Current orientation of the camera.
  float rotX, rotY;
  
  /** Handles initialization of script events. */
  void initializeScriptEvents();

 public:
  bool SetupModules ();

  /**
   * Handle keyboard events - ie key presses and releases.
   * This routine is called from the event handler in response to a 
   * csevKeyboard event.
   */
  bool OnKeyboard (iEvent&);
  
  /**
   * Handle events - foreward them to AWS2 for the most part.
   */
  bool HandleEvent (iEvent &Event);
  
  /**
   * Setup everything that needs to be rendered on screen. This routine
   * is called from the event handler in response to a csevProcess
   * broadcast message.
   */
  void ProcessFrame ();
  
  /// Here we will create our little, simple world.
  void CreateRoom ();
    
  /**
   * Finally render the screen. This routine is called from the event
   * handler in response to a csevFinalProcess broadcast message.
   */
  void FinishFrame ();

  /// Construct our game. This will just set the application ID for now.
  PartEdit2 ();

  /// Destructor.
  ~PartEdit2 ();

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
  
  // Declare the name of this event handler.
  CS_EVENTHANDLER_NAMES("application.partedit2")
      
  /* Declare that we're not terribly interested in having events
     delivered to us before or after other modules, plugins, etc. */
  CS_EVENTHANDLER_NIL_CONSTRAINTS
};

PartEdit2 *pe2App();

#endif // __PARTEDIT2_APP_H__
