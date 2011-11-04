/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __SIMPLEPT_H__
#define __SIMPLEPT_H__

#include <crystalspace.h>

class Simple : public csApplicationFramework, public csBaseEventHandler
{
private:
  csRef<iEngine> engine;
  csRef<iLoader> loader;
  csRef<iGraphics3D> g3d;
  csRef<iKeyboardDriver> kbd;
  iSector* room;
  csRef<iView> view;
  csRef<iRenderManager> rm;
  csRef<iVirtualClock> vc;
  csRef<iMeshWrapper> genmesh;
  csRef<iGeneralFactoryState> factstate;
  csRef<iFont> font;
  csRef<FramePrinter> printer;

  CS::ShaderVarStringID svTexDiffuse;
  csRef<iTextureHandle> targetTex;
  csRef<iMaterialWrapper> targetMat;
  csRef<iView> targetView;

  void CreatePolygon (iGeneralFactoryState *th, int v1, int v2, int v3, int v4);

  int genmesh_resolution;
  csVector3 genmesh_scale;
  float* angle_table;
  float* angle_speed;
  csVector3* start_verts;
  bool CreateGenMesh (iMaterialWrapper* mat);
  void AnimateGenMesh (csTicks elapsed);

  struct Target
  {
    csRef<iTextureHandle> texh;
    csRenderTargetAttachment attachment;
    const char* format;
  };
  csArray<Target> targetTextures;
  size_t currentTarget;

  csString currentTargetStr;
  csString availableFormatsStr;
  size_t numAvailableformats;
  bool renderTargetState;
  
  void CreateTextures ();
  void CycleTarget();
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
  
  /// Here we will create our little, simple world.
  bool CreateRoom ();
    
  /// Construct our game. This will just set the application ID for now.
  Simple ();

  /// Destructor.
  ~Simple ();

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
  CS_EVENTHANDLER_PHASE_LOGIC("application.simple1")
};

#endif // __SIMPLEPT_H__

