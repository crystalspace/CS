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

/**\file
 * Video preferences dialog.
 */

#include "csextern.h"

#include "cstypes.h"
#include "csutil/csstring.h"
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
 * video driver to display a dialog with video preferences. Once a
 * selection is made the renderer can be queried. To apply the new
 * setting, a restart of CS is required, though.
 */
class CS_CRYSTALSPACE_EXPORT csVideoPreferences
{
private:
  iObjectRegistry* object_reg;
  csRef<iVFS> vfs;
  csRef<iAws> aws;
  csRef<iGraphics3D> g3d;
  csRef<iGraphics2D> g2d;
  csRef<iFontServer> fontserv;

  csString mode;

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
   * wrong. 
   */
  bool Setup (iObjectRegistry* object_reg);

  /**
   * Handle an event for our requester. Return true if the requester is
   * finished.
   */
  bool HandleEvent (iEvent &Event);

  /**
   * Return the class ID of the requested renderer.
   */
  const char* GetModePlugin()
  { return mode; }
};

#endif // __CS_VIDPREFS_H__

