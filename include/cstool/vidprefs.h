/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_VIDPREFS_H__
#define __CS_VIDPREFS_H__

#include "csextern.h"

#include "cstypes.h"
#include "csutil/ref.h"

struct iObjectRegistry;
struct iEvent;
struct iVFS;
struct iAws;
struct iAwsCanvas;
struct iAwsSource;
struct iGraphics3D;
struct iGraphics2D;
struct iFontServer;
struct iImageIO;

/**
 * This class is a video preference editor. It uses AWS (will load
 * the plugin if not already present in memory) and the current
 * video driver to display a dialog with video preferences. At user
 * request it will replace the current video driver with the new
 * requested one (i.e. OpenGL instead of software) or else it will
 * change modes. Do NOT try to use this class while the engine is
 * already initialized. This will make all registered textures
 * and materials invalid.
 */
class CS_CRYSTALSPACE_EXPORT csVideoPreferences
{
private:
  iObjectRegistry* object_reg;
  csRef<iVFS> vfs;
  csRef<iImageIO> imageio;
  csRef<iAws> aws;
  csRef<iAwsCanvas> aws_canvas;
  csRef<iGraphics3D> g3d;
  csRef<iGraphics2D> g2d;
  csRef<iFontServer> fontserv;

  int mode;	// 0 for software, 1, for OpenGL (@@@ temporary)

  bool exit_loop;

  bool SetupWindow ();

  void SetSoftwareL (iAwsSource *source);
  void SetOpenGLL (iAwsSource *source);
  static void SetSoftware (intptr_t vidprefs, iAwsSource *source);
  static void SetOpenGL (intptr_t vidprefs, iAwsSource *source);

public:
  /**
   * Create the video requester.
   */
  csVideoPreferences ();

  /**
   * Destructor.
   */
  ~csVideoPreferences ();

  /**
   * Setup the video requester. Returns false if something went
   * wrong. If this function succeeded the application should stop
   * all event handling it normally does on its own. Instead it should
   * call HandleEvent() in this class. If this function returns true
   * then the video requester has finished.
   * <p>
   * IMPORTANT! After HandleEvent() returned true the video driver, the canvas
   * and the font server may have changed. You should take care to update
   * your local g3d/g2d/fontserv variables. Also make sure to release the
   * old g3d/g2d/fontserv you may already have.
   */
  bool Setup (iObjectRegistry* object_reg);

  /**
   * Handle an event for our requester. Return true if the requester is
   * finished.
   */
  bool HandleEvent (iEvent &Event);

  /**
   * Clean up the video requester.
   */
  void CleanUp ();

  /**
   * Cleanup the video requester and switch to the correct renderer
   * as requested by the user. This will essentially load the 3D renderer
   * and canvas and register them to the object registry.
   */
  void SelectMode ();
};

#endif // __CS_VIDPREFS_H__

