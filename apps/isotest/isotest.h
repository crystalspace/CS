/*
    Copyright (C) 2004 by Jorrit Tyberghein

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

#ifndef __ISOTEST_H__
#define __ISOTEST_H__

#include <crystalspace.h>

struct iEngine;
struct iLoader;
struct iGraphics3D;
struct iKeyboardDriver;
struct iVirtualClock;
struct iObjectRegistry;
struct iEvent;
struct iSector;
struct iView;
struct iMeshWrapper;
struct iLight;
struct iCamera;
struct iFont;

/**
 * Capture an isometric camera viewpoint.
 * After changing angle or distance, call SetupIsoView() on it.
 * Every frame the camera moves or changes angle or distance call
 *   CameraIsoLookat with the view as argument.
 */
struct IsoView
{
  /// offset to apply to the camera.
  csVector3 camera_offset;
  /// original camera offset.
  csVector3 original_offset;
  /// angle of rotation, in degrees, 0.0 is the original rotation.
  float angle;
  /// Zoom, used to select ortho projection boundaries
  float zoom;

  /// initialize with original offset of camera from the spot you look at.
  void SetOrigOffset(const csVector3& v, float z) 
  {
    camera_offset = original_offset = v; 
    angle = 0.f; 
    zoom = z;
  }
};

/**
* This is the main class of this application. It contains the
* basic initialization code and the main event handler.
*
* csApplicationFramework provides a handy object-oriented wrapper around the
* Crystal Space initialization and start-up functions.
*
* csBaseEventHandler provides a base object which does absolutely nothing
* with the events that are sent to it.
*/
class IsoTest : public csApplicationFramework, public csBaseEventHandler
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

  /// A pointer to the sector the camera will be in.
  iSector* room;

  csRef<iMeshWrapper> actor;
  iMeshWrapper* plane;
  csRef<iLight> actor_light;
  csRef<iFont> font;
  int current_view;
  IsoView views[4];
  /// is the main actor walking
  bool actor_is_walking;

  /// Event handlers to draw and print the 3D canvas on each frame
  csRef<FrameBegin3DDraw> drawer;
  csRef<FramePrinter> printer;

  bool CreateActor ();
  bool LoadMap ();

  /// make the camera look at given position using isometric viewpoint.
  void CameraIsoLookat(iCustomMatrixCamera* customCam, const IsoView& isoview, 
    const csVector3& lookat);
  /// setup an isometric view to be ready for display, call after rotating
  void SetupIsoView(IsoView& isoview);

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

  /// Construct our game. This will just set the application ID for now.
  IsoTest ();

  /// Destructor.
  ~IsoTest ();

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
  * Declare that we want to receive the frame event in the "3D" phase,
  * and that we're not terribly interested in having other events
  * delivered to us before or after other modules, plugins, etc. */
  CS_EVENTHANDLER_PHASE_3D("application.isotest")
};

#endif // __ISOTEST_H__

