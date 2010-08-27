/*
    Copyright (C) 2010 by Jorrit Tyberghein, Eduardo Poyart

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

#ifndef __LODVIEW_H__
#define __LODVIEW_H__

class LodView : public csApplicationFramework, public csBaseEventHandler
{
private:
  /// A pointer to the 3D engine.
  csRef<iEngine> engine;

  /// A pointer to the map loader plugin.
  csRef<iLoader> loader;
  csRef<iThreadedLoader> tloader;
  csRef<iMeshFactoryWrapper> imeshfactw;

  /// A pointer to the VFS.
  csRef<iVFS> vfs;

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

  float rotX, rotY;

  csRef<FramePrinter> printer;

  csArray<csRef<iMeshWrapper> > sprites;
  int num_lod_levels;
  int lod_level;
  int prev_lod_level;
  csString filename;
  int num_sprites;
  bool use_multiple_sprites;
  int num_multiple;
  bool use_adaptive_LODs;

  /**
   * Handle keyboard events - ie key presses and releases.
   * This routine is called from the event handler in response to a 
   * csevKeyboard event.
   */
  bool OnKeyboard (iEvent&);

  /**
   * Setup everything that needs to be rendered on screen. This routine
   * is called from the event handler in response to a csevFrame
   * broadcast message.
   */
  void Frame ();
  
  csRef<iThreadReturn> loading;
  
  /// Load the file specified in filename
  void LoadLODs(const char* filename);
  
  /// Print out usage instructions to the console
  void Usage();
  
  /// Here we will create our little, simple world.
  void CreateRoom ();
  void CreateSmallRoom();
  void CreateLargeRoom();

  /// Here we will create our sprites.
  void CreateSprites();
  void CreateOneSprite(const csVector3& pos);
  void CreateManySprites(int rows, int cols);

  bool SetupModules ();
  
  /// Tell the mesh factory to force a new LOD level
  void UpdateLODLevel();

public:

  /// Construct our game. This will just set the application ID for now.
  LodView ();

  /// Destructor.
  ~LodView ();

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

  CS_EVENTHANDLER_NAMES("crystalspace.Lod")
  CS_EVENTHANDLER_NIL_CONSTRAINTS
};

#endif // __LODVIEW_H__
