/*
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_IVIDEO_CUSTCURSOR_H__
#define __CS_IVIDEO_CUSTCURSOR_H__

#include "csutil/scf.h"
#include "csgeom/cspoint.h"
#include "igraphic/image.h"

/// The default custom cursor name
#define CSCURSOR_Default "default"
/// The custom cursor name used when a mouse button is pressed
#define CSCURSOR_MouseDown "MouseDown"

SCF_VERSION (iCursor, 0, 0, 1);

/**
 * This interface is used to access the custom cursor plugin, which
 * handles processing for displaying pixmaps as cursors. Any number
 * of cursors can be set, indexed by key strings, along with
 * hotspots, and alpha transparency. Supports both static
 * csSimplePixmap cursors and csAnimatedPixmap cursors.
 */
struct iCursor : public iBase
{
  
  /**
   * Must be called before custom cursors will be displayed.  If you want to
   * use software emulation mode on all platforms, set the forceEmulation
   * argument to true.
   */
  virtual bool Setup (iGraphics3D *, bool forceEmulation = false) = 0;

  /// Load cursor settings from a config file in VFS
  virtual bool ParseConfigFile (const char *iFile) = 0;

  /**
   * Adds or replaces a cursor called name. Currently you can only register
   * an iImage - a simple pixmap will then be created for use in emulation mode
   */
  virtual void SetCursor (const char *name, iImage *image, csRGBcolor keycolor, 
                          csPoint hotspot = csPoint (0,0), uint8 alpha = 0, 
                          csRGBcolor fg = csRGBcolor (255,255,255),
                          csRGBcolor bg = csRGBcolor (0,0,0)) = 0;
      
  /// Sets the hotspot (center) of the specified cursor on the pixmap.
  virtual void SetHotspot (const char *name, csPoint hotspot) = 0;

  /**
   * Sets the alpha transparency of the specified cursor.  This will only
   * work if the OS supports this, or you are in emulation mode
   */
  virtual void SetAlpha (const char *name, uint8 alpha) = 0;
  
  /// Set key colour of the specified cursor.
  virtual void SetKeyColor (const char *name, csRGBcolor col) = 0;

  /**
   * Set the foreground and background colors of the cursor.  These will only
   * be used when in OS mode on systems which only support monochrome cursors
   */
  virtual void SetColor (const char *name, csRGBcolor fg, csRGBcolor bg) = 0;

  /**
   * Get cursor image of the specified cursor
   * Returns a reference to NULL if there is no cursor with this name.  This
   * getter safely can be used to check if a cursor is defined or not without
   * switching
   */
  virtual const csRef<iImage> GetCursorImage (const char *name) const = 0;

  /**
   * Get the hotspot (center) of the specified cursor on the pixmap.
   * Returns default 0,0 if there is no cursor with this name
   */
  virtual csPoint GetHotspot (const char *name) const = 0;

  /**
   * Get the alpha transparency of the specified cursor.  
   * Returns default 0 if there is no cursor with this name
   */
  virtual uint8 GetAlpha (const char *name) const = 0;
  
  /**
   * Get key colour of the specified cursor.
   * Returns default 0,0,0 if there is no cursor with this name
   */
  virtual csRGBcolor GetKeyColor (const char *name) const = 0;

  /**
   * Get the foreground color of the cursor.  These will only be used when 
   * in OS mode on systems which only support monochrome cursors
   * Returns default 255,255,255 if there is no cursor with this name
   */
  virtual csRGBcolor GetFGColor (const char *name) const = 0;

  /**
   * Get the background color of the cursor.  These will only be used when 
   * in OS mode on systems which only support monochrome cursors
   * Returns default 0,0,0 if there is no cursor with this name
   */
  virtual csRGBcolor GetBGColor (const char *name) const = 0;

  /// Completely remove a cursor.
  virtual bool RemoveCursor (const char *name) = 0;

  /// Remove all cursors.
  virtual void RemoveAllCursors () = 0;

  /**
   * Switches the current cursor displayed to the specified cursor.
   * Disables automatic switching of cursors on mouse events;
   * 
   * The system defaults to CSCURSOR_Default.  In this mode, it will also
   * automatically switch to CSCURSOR_MouseDown when the mouse is pressed
   */
  virtual bool SwitchCursor (const char *name) = 0;
};

#endif //__CS_IVIDEO_CUSTCURSOR_H__
