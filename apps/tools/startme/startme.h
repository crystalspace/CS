/*
    Copyright (C) 2011 by Jorrit Tyberghein and Jelle Hellemans

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

#include <string>

#include "csutil/custom_new_disable.h"
#include <CEGUI.h>
#include "csutil/custom_new_enable.h"

#include "ivaria/icegui.h"

struct DemoData
{
  const char* name;
  const char* exec;
  const char* args;
  std::string description;
  const char* image;
  CEGUI::Window* window;

  DemoData () : window (0) { }
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

  /// A pointer to the CEGUI plugin.
  csRef<iCEGUI> cegui;

  /// A pointer to the configuration manager.
  csRef<iConfigManager> confman;

  csRef<iNativeWindow> natwin;

  /// A pointer to the sector the camera will be in.
  iSector* room;

  // For the demos.
  csArray<DemoData> demos;

  csRef<FramePrinter> printer;

  virtual bool OnKeyboard (iEvent&);

  float position, lastPosition;
  enum {
    ROTATE_NORMAL = 0,
    ROTATE_SELECTING,
    ROTATE_SEARCHING
  } rotationStatus;
  float rotationSpeed;

  /**
   * Setup everything that needs to be rendered on screen. This routine
   * is called from the event handler in response to a cscmdProcess
   * broadcast message.
   */
  void Frame ();

  /// Here we will create our little, simple world.
  void CreateRoom ();

  /// Load configuration from file.
  void LoadConfig ();

  bool OnClick (const CEGUI::EventArgs& e);

  bool OnLogoClicked (const CEGUI::EventArgs& e);

  bool OnMouseMove (iEvent& ev);

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

   // Declare the name of this event handler.
  CS_EVENTHANDLER_NAMES("application.startme")
      
  /* Declare that we want to receive events *after* the CEGUI plugin. */
  virtual const csHandlerID * GenericPrec (csRef<iEventHandlerRegistry> &r1, 
    csRef<iEventNameRegistry> &r2, csEventID event) const 
  {
    static csHandlerID precConstraint[2];
    
    precConstraint[0] = r1->GetGenericID("crystalspace.cegui");
    precConstraint[1] = CS_HANDLERLIST_END;
    return precConstraint;
  }

  CS_EVENTHANDLER_DEFAULT_INSTANCE_CONSTRAINTS
};

#endif // __STARTME_H__
