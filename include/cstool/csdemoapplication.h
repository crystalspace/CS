/*
  Copyright (C) 2010 Christian Van Brussel, Communications and Remote
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

// All these files commonly needed are already included
#include "iengine/sector.h"
#include "iengine/camera.h"
#include "iengine/scenenode.h"
#include "iengine/movable.h"
#include "ivideo/graph2d.h"
#include "ivaria/view.h"
#include "csutil/event.h"
#include "csutil/csbaseeventh.h"
#include "csutil/common_handlers.h"
#include "csutil/stringarray.h"
#include "csutil/cmdhelp.h"
#include "iutil/cmdline.h"
#include "iutil/csinput.h"
#include "iutil/virtclk.h"
#include "imesh/object.h"
#include "cstool/csapplicationframework.h"
#include "cstool/simplestaticlighter.h"

class csPixmap;

enum csDemoCameraMode
{
  CSDEMO_CAMERA_NONE = 0,         /*!< The application will manage the camera by itself */
  CSDEMO_CAMERA_MOVE_NORMAL,      /*!< The camera is free to move */
  CSDEMO_CAMERA_MOVE_ORIGIN,      /*!< The camera moves relatively to the origin of the scene */
  CSDEMO_CAMERA_ROTATE_ORIGIN     /*!< The camera can only rotate relatively to the origin of the scene */
};

class csDemoCommandLineHelper
{
 public:
  csDemoCommandLineHelper (const char* applicationCommand,
			   const char* applicationCommandUsage,
			   const char* applicationDescription);

  void WriteHelp (iObjectRegistry* registry);

  /// Command line options displayed when the '-help' option is used
  struct CommandOption
  {
    /// Constructor
    CommandOption (const char* option, const char* description)
    : option (option), description (description) {}

    /// Name of the option
    csString option;
    /// Description of the option
    csString description;
  };

  /// Array of command line options displayed when the '-help' option is used
  csArray<CommandOption> commandOptions;

 private:
  // Command line help
  csString applicationCommand;
  csString applicationCommandUsage;
  csString applicationDescription;
};

/**
 * Crystal Space demo application framework class. This is a base class
 * providing the basic functionalities for Crystal Space's demo and test
 * applications. Most demos, tests and tutorials would want to use this class
 * in order to simplify their code, make it more pertinent for the user, and
 * unify the graphical and general layout between all applications.
 *
 * The basic functionalities provided by this class are:
 * - creation of the main objects of the engine
 * - default creation of the scene
 * - management of the camera
 * - display of the available keys
 * - display of the information on the state of the application
 * - display of Crystal Space's logo
 * - management of the command line's help
 *
 * Here is an example for the most simple use of this class:
 * \code
 * //--------------------------
 * // Example.h
 *
 * #include "cstool/csdemoapplication.h"
 *
 * class MyApp : public csDemoApplication
 * {
 * public:
 *   MyApp();
 *   bool Application ();
 * };
 *
 * //--------------------------
 * // Example.cpp
 *
 * #include "cssysdef.h"
 * #include "Example.h"
 *
 * MyApp::MyApp ()
 *   : csDemoApplication ("CrystalSpace.MyApp", "myapplication", "myapplication",
 *		          "Description of 'myapplication'.")
 * {
 *   // Configure the options for csDemoApplication
 * }
 *
 * bool MyApp::Application ()
 * {
 *   // Default behavior from csDemoApplication
 *   if (!csDemoApplication::Application ())
 *     return false;
 *
 *   // Initialize here your application
 *
 *   // Default behavior from csDemoApplication for the creation of the scene
 *   if (!CreateRoom ())
 *     return false;
 *
 *   // Create here the main objects of the scene
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
 * #include "Example.h"
 *
 * CS_IMPLEMENT_APPLICATION
 * 
 * int main (int argc, char* argv[]) 
 * {
 *   return MyApp ().Main (argc, argv);
 * }
 * \endcode
 */
class CS_CRYSTALSPACE_EXPORT csDemoApplication : public csApplicationFramework,
  public csBaseEventHandler
{
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
  /// Reference to the font used to display information
  csRef<iFont> font;
  /// Reference to the main sector
  csRef<iSector> room;

  /// Base implementation of a method inherited from csBaseEventHandler
  virtual void Frame ();
  /// Base implementation of a method inherited from csBaseEventHandler
  virtual bool OnKeyboard (iEvent &event);
  /// Base implementation of a method inherited from csBaseEventHandler
  virtual bool OnMouseDown (iEvent &event);
  /// Base implementation of a method inherited from csBaseEventHandler
  virtual bool OnMouseUp (iEvent &event);
  /// Base implementation of a method inherited from csBaseEventHandler
  virtual bool OnMouseMove (iEvent &event);

  /// Camera mode
  csDemoCameraMode cameraMode;

  /**
   * Basic initialization for the creation of the main sector. It creates
   * a background far away, initializes the camera, and adds a few lights.
   */
  virtual bool CreateRoom ();

  /// Display of a 2D text with a shadow
  void WriteShadow (int x, int y, int color, const char *str,...);
  /// Display of a 2D text
  void Write (int x, int y, int fg, int color, const char *str,...);

  /// Array of string describing the available keys
  csStringArray keyDescriptions;
  /// Array of string describing the state of the application
  csStringArray stateDescriptions;

  // Command line help
  csDemoCommandLineHelper commandLineHelper;

 public:
  /**
   * Constructor. The parameters of this constructor are used mainly to display
   * information when the '-help' option is used.
   * \param applicationName Name of the application, used to set
   * csApplicationFramework::SetApplicationName().
   * \param applicationCommand Name of the executable
   * \param applicationCommandUsage Syntax to use the executable (eg "myapp
   * <OPTIONS> filename"). Additional examples of use can also be added.
   * \param applicationDescription Description of the application
   */
  csDemoApplication (const char* applicationName, const char* applicationCommand,
		     const char* applicationCommandUsage,
		     const char* applicationDescription);
  ~csDemoApplication ();

  /// Base implementation of a method inherited from csApplicationFramework
  virtual void OnExit ();
  /// Base implementation of a method inherited from csApplicationFramework
  virtual bool OnInitialize (int argc, char* argv[]);
  /// Base implementation of a method inherited from csApplicationFramework
  virtual bool Application ();

 private:
  // Camera related
  void ResetCamera ();
  void UpdateCamera ();
  void FixCameraForOrigin (const csVector3 & desiredOrigin);

  csVector3 cameraTarget;
  float cameraDist;
  float cameraYaw;
  float cameraPitch;

  bool cameraModePan;
  bool cameraModeRotate;
  bool cameraModeZoom;

  int lastMouseX, lastMouseY;

  // Crystal Space logo
  csPixmap* cslogo;

  // Computing of frames per second
  uint frameCount;
  int frameTime;
  float currentFPS;
};

/** @} */

#endif // __CSDEMOAPPLICATION_H__
