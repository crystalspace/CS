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
 * Crystal Space demo application framework class and tools
 */

/**
 * \addtogroup appframe
 * @{ */

// All these files commonly needed are already included, so that the
// applications only need to include the files relevant for them.
#include "cstool/csapplicationframework.h"
#include "cstool/simplestaticlighter.h"
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
#include "iutil/virtclk.h"
#include "iutil/visualdebug.h"
#include "ivideo/graph2d.h"
#include "ivaria/view.h"

class csPixmap;

namespace CS {
namespace Demo {

// ------------------------ HUDManager ------------------------

/**
 * A generic tool managing the display of a minimal text-based HUD, eg for applications
 * implementing CS::Demo::DemoApplication or providing a user interface through the keyboard.
 *
 * The HUD consists of the Crystal Space logo, the list of available keyboard and mouse actions
 * that can be used to interact with the demo, and a list of strings
 * describing the current state of the application.
 *
 * You need to setup or update the list of keys and current states (keyDescriptions and
 * stateDescriptions) whenever they change. The description of the state is augmented with
 * informations such as the current Frames Per Second.
 * \warning Initialize() must be called before any other operation.
 */
class CS_CRYSTALSPACE_EXPORT HUDManager : public csBaseEventHandler
{
 public:
  /**
   * Constructor.
   */
  HUDManager ();
  /**
   * Destructor.
   */
  ~HUDManager ();

  /**
   * Initialize this HUD manager
   */
  void Initialize (iObjectRegistry* registry);

  //-- csBaseEventHandler
  void Frame ();

  /**
   * Switch to the next page describing the list of available keyboard keys. This is useful
   * when the list of available keyboard keys is too big and needs to be split in several
   * different pages.
   */
  void SwitchKeysPage ();

  /**
   * Display a 2D text with a shadow. Additional parameters can be defined,
   * they will be formated into the text string by using the cs_snprintf()-style
   * formatting directives.
   */
  void WriteShadow (int x, int y, int color, const char *str,...);
  /**
   * Display a 2D text. Additional parameters can be defined,
   * they will be formated into the text string by using the cs_snprintf()-style
   * formatting directives.
   */
  void Write (int x, int y, int fg, int color, const char *str,...);

  /**
   * Set whether or not the HUD will be displayed. If not enabled, then this manager
   * will not be active at all.
   */
  void SetEnabled (bool enabled);

  /**
   * Get whether or not the HUD is displayed. If not enabled, then this manager
   * will not be active at all.
   */
  bool GetEnabled ();

  /**
   * Array of string describing the available user keys (eg 'd: toggle debug mode').
   * By default the array is filled with the keys available to move the camera. You
   * can call csStringArray::DeleteAll() if you don't want these keys to be displayed.
   */
  csStringArray keyDescriptions;

  /**
   * Array of string describing the state of the application (eg 'Debug mode enabled').
   */
  csStringArray stateDescriptions;

 private:
  // Reference to the 3D graphics
  csRef<iGraphics3D> g3d;
  // Reference to the 2D graphics
  csRef<iGraphics2D> g2d;
  // Reference to the virtual clock
  csRef<iVirtualClock> vc;

  // Reference to the font used to display information
  csRef<iFont> font;
  // Crystal Space logo
  csPixmap* cslogo;

  // Whether or not the HUD is displayed
  bool enabled;

  // Computing of frames per second
  uint frameCount;
  int frameTime;
  float currentFPS;

  // Current page of available keys to be displayed
  uint currentKeyPage;
  uint maxKeys;
};

// ------------------------ CameraManager ------------------------

/**
 * Various camera modes which can be used with CS::Demo::CameraManager.
 */
enum CameraMode
{
  CAMERA_NO_MOVE = 0,     /*!< The application will manage the camera by itself */
  CAMERA_MOVE_FREE,       /*!< The camera is free to move */
  CAMERA_MOVE_LOOKAT,     /*!< The camera is free to move but keeps looking at the target */
  CAMERA_ROTATE           /*!< The camera rotates around the target */
};

/**
 * A generic tool to control the motion of the camera through the keyboard
 * and/or the mouse.
 *
 * To use it, you need to create one CS::Demo::CameraManager, initialize it with
 * Initialize(), specify the camera to be controlled with SetCamera(), and configure the
 * behavior of the manager eg by defining the camera mode through SetCameraMode(). After that,
 * the camera manager will remain active until it is destroyed.
 *
 * \warning Initialize() must be called before any other operation.
 */
class CS_CRYSTALSPACE_EXPORT CameraManager : public csBaseEventHandler
{
 public:
  /**
   * Constructor.
   */
  CameraManager ();
  ~CameraManager ();

  /**
   * Initialize this camera helper
   */
  void Initialize (iObjectRegistry* registry);

  /// Set the camera to be controlled by this manager. This can be nullptr.
  void SetCamera (iCamera* camera);
  /// Get the camera controlled by this manager, or nullptr if there are none.
  iCamera* GetCamera ();

  /**
   * Update the position of the camera. You don't need to call this, but it can be used
   * to control the exact moment where the camera is updated (eg before the rendering of
   * the view). If you don't call this, then the camera will still be updated automatically
   * once per frame, but it may or not have a one frame delay depending on if it is updated
   * after or before the rendering of the view.
   */
  void UpdateCamera ();

  /// Set the camera mode to be used. The default value is CS::Demo::CAMERA_MOVE_NORMAL.
  void SetCameraMode (CameraMode cameraMode);
  /// Return the current camera mode.
  CameraMode GetCameraMode ();

  /**
   * Set the start position of the camera. This position is used when ResetCamera() is called.
   * The default value is 'csVector3 (0.0f, 0.0f, -3.0f)'.
   */
  void SetStartPosition (csVector3 position);

  /**
   * Get the start position of the camera. This position is used when ResetCamera() is called.
   */
  csVector3 GetStartPosition ();

  /**
   * Set the target of the camera, ie what it is looking at. This is relevant
   * only for the CS::Demo::CAMERA_MOVE_LOOKAT and CS::Demo::CAMERA_ROTATE camera
   * modes. The default value is 'csVector3 (0.0f, 0.0f, 0.0f)'.
   */
  void SetCameraTarget (csVector3 position);

  /**
   * Get the target of the camera, ie what it is looking at. This is relevant
   * only for the CS::Demo::CAMERA_MOVE_LOOKAT and CS::Demo::CAMERA_ROTATE camera
   * modes.
   */
  csVector3 GetCameraTarget ();

  /**
   * Set the closest distance there can be between the camera and its
   * target. This is relevant only for the CS::Demo::CAMERA_MOVE_LOOKAT and
   * CS::Demo::CAMERA_ROTATE camera modes. The default value is \a 0.1f.
   */
  void SetCameraMinimumDistance (float distance);

  /**
   * Get the closest distance there can be between the camera and its
   * target. This is relevant only for the CS::Demo::CAMERA_MOVE_LOOKAT and
   * CS::Demo::CAMERA_ROTATE camera modes.
   */
  float GetCameraMinimumDistance ();

  /**
   * Set whether the camera can be moved or not through the mouse.
   * The default value is true.
   * If enabled, then the camera will be moved when the user drags
   * the mouse while holding one of the following button:
   * - left button: the camera is moved sideways
   * - right button: the camera is rotated around the target returned
   * by CameraManager::GetCameraTarget().
   * - middle button: the camera is moved forward and backward. The camera
   * cannot get closer than CameraManager::GetCameraMinimumDistance().
   */
  void SetMouseMoveEnabled (bool enabled);

  /**
   * Return whether the camera can be moved or not through the mouse.
   */
  bool GetMouseMoveEnabled ();

  /**
   * Reset the camera position to the position returned by
   * CameraManager::GetStartPosition().
   */
  void ResetCamera ();

  /**
   * Set the speed of the camera's motion, in unit per second. The default value is 5.
   * Note that the camera moves ten times faster when the CTRL key is pressed. 
   */
  void SetMotionSpeed (float speed);

  /**
   * Get the speed of the camera's motion, in unit per second.
   */
  float GetMotionSpeed ();

  /**
   * Set the rotation speed of the camera, in radian per second. The default value is 2.
   * Note that the camera rotates five times faster when the CTRL key is pressed. 
   */
  void SetRotationSpeed (float speed);

  /**
   * Get the speed of the camera's motion, in radian per second.
   */
  float GetRotationSpeed ();

  //-- csBaseEventHandler
  void Frame ();
  bool OnMouseDown (iEvent &event);
  bool OnMouseUp (iEvent &event);
  bool OnMouseMove (iEvent &event);

 private:
  void UpdatePositionParameters (const csVector3& newPosition);
  void ApplyPositionParameters ();

  // Reference to the keyboard driver
  csRef<iKeyboardDriver> kbd;
  // Reference to the virtual clock
  csRef<iVirtualClock> vc;
  // Reference to the mouse driver
  csRef<iMouseDriver> mouse;

  // Reference to the camera
  csRef<iCamera> camera;

  CameraMode cameraMode;
  bool mouseMoveEnabled;

  csVector3 startPosition;
  csVector3 cameraTarget;
  float minimumDistance;

  csVector3 panCameraTarget;
  float cameraDistance;
  float cameraYaw;
  float cameraPitch;

  bool cameraModePan;
  bool cameraModeRotate;
  bool cameraModeZoom;

  float motionSpeed;
  float rotationSpeed;

  bool wasUpdated;

  int previousMouseX, previousMouseY;
};

// ------------------------ DemoApplication ------------------------

/**
 * Crystal Space demo application framework class. This class and its companions
 * provide the basic functionalities for Crystal Space's demo and test
 * applications. Most demos, tests and tutorials would want to use this class
 * in order to simplify their code, make it more pertinent for the user, and
 * unify the graphical and general layout between all applications.
 *
 * The basic functionalities provided by this class are:
 * - creation of the main objects of the engine
 * - default creation of the scene
 * - management of the camera (class CS::Demo::CameraManager).
 * - display of the available keys, Crystal Space logo and other informations
 * (class CS::Demo::HUDManager).
 * - management of the command line's help (class CS::Demo::CommandLineHelper).
 *
 * Here is an example for the most simple use of this class:
 * \code
 * //--------------------------
 * // example.h
 *
 * #include "cstool/csdemoapplication.h"
 *
 * class MyApp : public CS::Demo::DemoApplication
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
 */
class CS_CRYSTALSPACE_EXPORT DemoApplication : public csApplicationFramework,
  public csBaseEventHandler
{
 private:
  /// Whether the previous mouse cusor position was initialized
  bool mouseInitialized;

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

  /// Visual debugger
  csRef<CS::Debug::iVisualDebugger> visualDebugger;

  //-- csBaseEventHandler
  /**
   * Base implementation of the method inherited from csBaseEventHandler. It initializes the
   * previous position of the mouse cursor, update the camera manager, renders the view and
   * displays the visual debugging information if any.
   */
  virtual void Frame ();

  /// Base implementation of the method inherited from csBaseEventHandler. It checks for the
  /// "Help" or "Esc" keys.
  virtual bool OnKeyboard (iEvent &event);

  /**
   * Base implementation of the method inherited from csBaseEventHandler. It simply updates
   * the \a previousMouse position.
   */
  virtual bool OnMouseMove (iEvent &event);

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

  /// HUD manager
  HUDManager hudManager;

  /// Camera manager
  CameraManager cameraManager;

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
   * main view used for rendering, and initializes the HUD and the camera managers.
   */
  virtual bool Application ();

  /**
   * Base implementation of the method inherited from csApplicationFramework. It
   * closes the frame printer.
   */
  virtual void OnExit ();
};

} // namespace Demo
} // namespace CS

/** @} */

#endif // __CSDEMOAPPLICATION_H__
