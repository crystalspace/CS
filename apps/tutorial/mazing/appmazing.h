/*
    Copyright (C) 2006 by Jorrit Tyberghein

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

#ifndef __appmazing_h
#define __appmazing_h

#include <crystalspace.h>
#include "gamestuff.h"

class AppMazing :
  public csApplicationFramework, public csBaseEventHandler
{
private:
  /**
   * A reference to the 3D renderer plugin.
   */
  csRef<iGraphics3D> g3d;

  /**
   * A reference to the 3D engine plugin.
   */
  csRef<iEngine> engine;

  /**
   * The main loader.
   */
  csRef<iLoader> loader;

  /**
   * The view.
   */
  csRef<iView> view;

  /**
   * The collision detection system.
   */
  csRef<iCollideSystem> cdsys;

  /**
   * The virtual clock.
   */
  csRef<iVirtualClock> vc;

  /**
   * The string registry. This is used for common constants in
   * Crystal Space.
   */
  csRef<iStringSet> strings;

  csRef<FramePrinter> printer;

  /**
   * Set up everything that needs to be rendered on screen.  This routine is
   * called from the event handler in response to a cscmdProcess broadcast
   * message.
   */
  virtual void Frame();

  /**
   * Handle keyboard events, such as key presses and releases.  This routine is
   * called from the event handler in response to a csevKeyboard event.
   */
  virtual bool OnKeyboard(iEvent&);

  /// The Game.
  Game game;

public:
  /**
   * Constructor.
   */
  AppMazing();

  /**
   * Destructor.
   */
  virtual ~AppMazing();

  iCamera* GetCamera () const { return view->GetCamera (); }
  iEngine* GetEngine () const { return engine; }
  iLoader* GetLoader () const { return loader; }
  iStringSet* GetStrings () const { return strings; }
  iCollideSystem* GetCollisionDetectionSystem () const { return cdsys; }
  Game& GetGame () { return game; }

  /**
   * Final cleanup.
   */
  virtual void OnExit();

  /**
   * Main initialization routine.  This routine should set up basic facilities
   * (such as loading startup-time plugins, etc.).  In case of failure this
   * routine will return false.  You can assume that the error message has been
   * reported to the user.
   */
  virtual bool OnInitialize(int argc, char* argv[]);

  /**
   * Run the application.  Performs additional initialization (if needed), and
   * then fires up the main run/event loop.  The loop will fire events which
   * actually causes Crystal Space to "run".  Only when the program exits does
   * this function return.
   */
  virtual bool Application();
  
  // Declare the name of this event handler.
  CS_EVENTHANDLER_NAMES("application.mazing")
      
  /* Declare that we're not terribly interested in having events
     delivered to us before or after other modules, plugins, etc. */
  CS_EVENTHANDLER_NIL_CONSTRAINTS
};

#endif // __appmazing_h
