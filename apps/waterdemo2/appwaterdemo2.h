/*
 * appwaterdemo2.h
 *
 * Declaration of AppWaterdemo2, the main application object.
 */

#ifndef __appwaterdemo2_h
#define __appwaterdemo2_h

#include <crystalspace.h>

#include "waterdemo2.h"
//#include <csutil/ref.h>
//#include <csutil/csbaseeventh.h>
//#include <cstool/csapplicationframework.h>
//#include <iengine/engine.h>
//#include <ivideo/graph3d.h>

class AppWaterdemo2 :
  public csApplicationFramework, public csBaseEventHandler
{
private:

  iSector* room;
  float rotX, rotY;
	
  csRef<iLoader> loader;
  csRef<iKeyboardDriver> kbd;
  csRef<iVirtualClock> vc;
  csRef<iMeshFactoryWrapper> fact;
  csRef<iMeshFactoryWrapper> waterfact;
  csRef<iView> view;

  /**
   * A reference to the 3D renderer plugin.
   */
  csRef<iGraphics3D> g3d;

  /**
   * A reference to the 3D engine plugin.
   */
  csRef<iEngine> engine;

  virtual void Frame();

  /**
   * Handle keyboard events, such as key presses and releases.  This routine is
   * called from the event handler in response to a csevKeyboard event.
   */
  virtual bool OnKeyboard(iEvent&);

  bool LoadMap ();
  
public:
  /**
   * Constructor.
   */
  AppWaterdemo2();

  /**
   * Destructor.
   */
  virtual ~AppWaterdemo2();

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
  CS_EVENTHANDLER_NAMES("application.waterdemo2")
      
  /* Declare that we're not terribly interested in having events
     delivered to us before or after other modules, plugins, etc. */
  CS_EVENTHANDLER_NIL_CONSTRAINTS
};

#endif // __appwaterdemo2_h
