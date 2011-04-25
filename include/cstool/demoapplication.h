/*
  Copyright (C) 2010-11 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

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
#ifndef __CSDEMOAPPLICATION_H__
#define __CSDEMOAPPLICATION_H__

/**\file 
 * Crystal Space demo application framework class
 */

/**
 * \addtogroup appframe
 * @{ */

// All these files commonly needed are already included, so that the
// applications only need to include the files relevant for them.
#include "cssysdef.h"

#include "cstool/csapplicationframework.h"
#include "cstool/numberedfilenamehelper.h"
#include "cstool/simplestaticlighter.h"
#include "csutil/cfgacc.h"
#include "csutil/cmdhelp.h"
#include "csutil/common_handlers.h"
#include "csutil/csbaseeventh.h"
#include "csutil/event.h"
#include "csutil/hash.h"
#include "csutil/stringarray.h"

#include "iengine/camera.h"
#include "iengine/scenenode.h"
#include "iengine/sector.h"
#include "iengine/movable.h"
#include "imesh/object.h"
#include "iutil/cmdline.h"
#include "iutil/csinput.h"
#include "iutil/stringarray.h"
#include "iutil/virtclk.h"
#include "iutil/visualdebug.h"
#include "ivideo/graph2d.h"
#include "ivaria/cameramanager.h"
#include "ivaria/hudmanager.h"
#include "ivaria/view.h"

namespace CS {
namespace Utility {

/**
 * Crystal Space demo application framework class. This class and its companions
 * provide the basic functionalities for Crystal Space's demo and test
 * applications. Most demos, tests and tutorials would want to use this class
 * in order to simplify their code, make it more pertinent for the user, and
 * unify the graphical and general layout between all applications.
 *
 * The functionalities provided by this class are:
 * - creation of the main objects of the engine
 * - default creation of the scene
 * - basic interaction with the HUD and camera managers
 * - screenshots
 *
 * Here is an example for the most simple use of this class:
 * \code
 * //--------------------------
 * // example.h
 *
 * #include "cstool/csdemoapplication.h"
 *
 * class MyApp : public CS::Utility::DemoApplication
 * {
 * public:
 *   MyApp();
 *   bool Application ();
 * };
 *
 * //--------------------------
 * // example.cpp
 *
 * #include "cssysdef.h"
 * #include "example.h"
 *
 * MyApp::MyApp ()
 *   : DemoApplication ("CrystalSpace.MyApp", "myapplication", "myapplication",
 *		          "Description of 'myapplication'.")
 * {
 *   // Configure the options for DemoApplication
 * }
 *
 * bool MyApp::Application ()
 * {
 *   // Default behavior from DemoApplication
 *   if (!DemoApplication::Application ())
 *     return false;
 *
 *   // Initialize here your application
 *
 *   // Default behavior from DemoApplication for the creation of the scene
 *   if (!CreateRoom ())
 *     return false;
 *
 *   // Create here the main objects of your scene
 *
 *   // Run the application
 *   Run();
 *
 *   return true;
 * }
 *
 * //--------------------------
 * // main.cpp
 *
 * #include "cssysdef.h"
 * #include "example.h"
 *
 * CS_IMPLEMENT_APPLICATION
 * 
 * int main (int argc, char* argv[]) 
 * {
 *   return MyApp ().Main (argc, argv);
 * }
 * \endcode
 *
 * \sa CS::Utility::iCameraManager CS::Utility::iHUDManager
 */
class CS_CRYSTALSPACE_EXPORT DemoApplication : public csApplicationFramework,
  public csBaseEventHandler
{
 private:
  /// Whether the previous mouse cusor position was initialized
  bool mouseInitialized;

  /// The file helper to find new filenames to save screenshots
  CS::NumberedFilenameHelper screenshotHelper;

  /// Image format of the screenshots
  csString screenshotFormat;

 protected:
  /// Reference to the engine
  csRef<iEngine> engine;
  /// Reference to the loader
  csRef<iLoader> loader;
  /// Reference to the 3D graphics
  csRef<iGraphics3D> g3d;
  /// Reference to the 2D graphics
  csRef<iGraphics2D> g2d;
  /// Reference to the keyboard driver
  csRef<iKeyboardDriver> kbd;
  /// Reference to the virtual clock
  csRef<iVirtualClock> vc;
  /// Reference to the view
  csRef<iView> view;
  /// Reference to the frame printer
  csRef<FramePrinter> printer;
  /// Reference to the virtual file system
  csRef<iVFS> vfs;
  /// Reference to the mouse driver
  csRef<iMouseDriver> mouse;

  /// Previous position of the mouse cursor during the last frame. It can be used to
  /// know the distance travelled by comparing it to "mouse->GetLastX/Y".
  csVector2 previousMouse;

  /// Reference to the main sector
  csRef<iSector> room;

  /// Reference to the default HUD manager
  csRef<CS::Utility::iHUDManager> hudManager;

  /// Reference to the default camera manager
  csRef<CS::Utility::iCameraManager> cameraManager;

  /// Visual debugger
  // TODO: remove
  csRef<CS::Debug::iVisualDebugger> visualDebugger;

  /// Access to the configuration manager system
  csConfigAccess config;

  //-- csBaseEventHandler
  /**
   * Base implementation of the method inherited from csBaseEventHandler. It initializes the
   * previous position of the mouse cursor, renders the view and displays the visual debugging
   * information if any.
   */
  virtual void Frame ();

  /**
   * Base implementation of the method inherited from csBaseEventHandler. It checks for the
   * "Help" or "Esc" keys.
   */
  virtual bool OnKeyboard (iEvent& event);

  /**
   * Base implementation of the method inherited from csBaseEventHandler. It simply updates
   * the \a previousMouse position.
   */
  virtual bool OnMouseMove (iEvent& event);

  /**
   * Default initialization for the creation of the main sector (DemoApplication::room). It
   * creates a sector of name "room", adds a grey uniform background far away, initializes
   * the camera and add some lights.
   */
  virtual bool CreateRoom ();

  /**
   * Print the help message for this application on standard output. You may need to implement
   * this method in order to print out the commanline options of your application.
   */
  virtual void PrintHelp ();

  /**
   * Return VFS path of an additional config file to load at initialization
   * Override this if you wish to use an application-specific config file on
   * top of <tt>demoapplication.cfg</tt>.
   */
  virtual const char* GetApplicationConfigFile()
  { return nullptr; }

 public:
  /**
   * Constructor.
   * \param applicationName Name of the application, used to set
   * csApplicationFramework::SetApplicationName().
   */
  DemoApplication (const char* applicationName);
  ~DemoApplication ();

  /**
   * Base implementation of the method inherited from csApplicationFramework.
   * It loads all base plugins, and registers itself to the event queue.
   */
  virtual bool OnInitialize (int argc, char* argv[]);

  /**
   * Base implementation of the method inherited from csApplicationFramework. It
   * opens the main application, find references to the engine objects, creates the
   * main view used for rendering, and initializes the HUD manager.
   */
  virtual bool Application ();

  /**
   * Base implementation of the method inherited from csApplicationFramework. It
   * closes the frame printer.
   */
  virtual void OnExit ();
};

} //namespace Utility
} //namespace CS

/** @} */

#endif // __CSDEMOAPPLICATION_H__
