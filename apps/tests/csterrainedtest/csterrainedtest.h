/*
    Copyright (C) 2010 by Jelle Hellemans

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

#ifndef __TERRAINED_H__
#define __TERRAINED_H__

#include <crystalspace.h>

struct iTerrainModifier;

/**
 * This is the main class of this Tutorial. It contains the
 * basic initialization code and the main event handler.
 *
 * csApplicationFramework provides a handy object-oriented wrapper around the
 * Crystal Space initialization and start-up functions.
 *
 * csBaseEventHandler provides a base object which does absolutely nothing
 * with the events that are sent to it.
 */
class TerrainEd : public csApplicationFramework, public csBaseEventHandler
{
private:
  csRef<iTerrainFactory> factory;
  csRef<iTerrainSystem> terrain;
  csRef<iTerrainModifier> mod;
  float rectSize;
  float rectHeight;

  csRefArray<iTerrainModifier> undoStack;

  void UpdateModifier(iEvent& ev);

private:
  int mouse_x, mouse_y;

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
  csRef<FramePrinter> printer;

public:
  bool SetupModules ();

  bool OnKeyboard (iEvent&);

  bool OnMouseClick (iEvent& ev);

  bool OnMouseDown (iEvent& ev);

  bool OnMouseMove (iEvent& ev);
  
  void Frame ();
  
  /// Here we will create our little, simple world.
  void CreateRoom ();
    
  /// Construct our game. This will just set the application ID for now.
  TerrainEd ();

  /// Destructor.
  ~TerrainEd ();

  /// Final cleanup.
  void OnExit ();

  bool OnInitialize (int argc, char* argv[]);

  bool Application ();

  CS_EVENTHANDLER_PHASE_LOGIC("application.terrained")
};

#endif // __TERRAINED_H__
