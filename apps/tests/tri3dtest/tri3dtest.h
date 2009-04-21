/*
    Copyright (C) 2008 by Scott Johnson

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

#ifndef __TRI3DTEST_H__
#define __TRI3DTEST_H__

#include <crystalspace.h>
#include "csgeom/triangulate3d.h"

/**
 * This is the main class for the application.  It is copied almost entirely
 * from the Simple application.
 */

class Tri3DTest : public csApplicationFramework, public csBaseEventHandler
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

  /// Current orientation of the camera.
  float rotX, rotY;

  /// Event handlers to draw and print the 3D canvas on each frame
  csRef<FrameBegin3DDraw> drawer;
  csRef<FramePrinter> printer;

  /// The triangulated version of our mesh
  csTriangleMesh tm;

  /// The untriangulated version of our mesh
  //csSimpleRenderMesh* untrimesh;
  CS::Geometry::csContour3 untrimesh;

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

  /// Create a simple box mesh from triangles
  bool CreateBox(CS::Geometry::csContour3& mesh);
  
  /// Convert from a triangle mesh to a render mesh
  /// @TODO: Change to regular render mesh, rather than simple rm
  csSimpleRenderMesh ConvertToRenderMesh(const csTriangleMesh& t);

  /// Convert from a csContour to a render mesh
  /// @TODO: Change to regular render mesh, rather than simple rm
  csSimpleRenderMesh ConvertToRenderMesh(const CS::Geometry::csContour3& c);

  /// Construct our game. This will just set the application ID for now.
  Tri3DTest ();

  /// Destructor.
  ~Tri3DTest ();

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
   * Declare that we want to receive the frame event in the "LOGIC" phase,
   * and that we're not terribly interested in having other events
   * delivered to us before or after other modules, plugins, etc. */
  CS_EVENTHANDLER_PHASE_LOGIC("application.tri3dtest")
};

#endif // __TRI3DTEST_H__
