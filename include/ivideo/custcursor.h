/*
    Copyright (C)2003 by Neil Mosafi

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

#include "csgfx/rgbpixel.h"
#include "csgeom/vector2.h"

struct iConfigFile;
struct iGraphics3D;
struct iImage;


/// The default custom cursor name
#define CSCURSOR_Default "default"
/// The custom cursor name used when a mouse button is pressed
#define CSCURSOR_MouseDown "MouseDown"

SCF_VERSION (iCursor, 0, 1, 0);

/**
 * This interface is used to access the custom cursor plugin, which
 * handles processing for displaying pixmaps as cursors. Any number
 * of cursors can be set, indexed by key strings, along with
 * hotspots, and transparency. Supports any static iImage.
 */
struct iCursor : public iBase
{
  
  /**
   * Must be called before custom cursors will be displayed.  If you want to
   * use software emulation mode on all platforms, set the ForceEmulation
   * argument to true.
   */
  virtual bool Setup (iGraphics3D *, bool ForceEmulation = false) = 0;

  /// Load cursor settings from a configuration file
  virtual bool ParseConfigFile (iConfigFile*) = 0;

  /**
   * Adds or replaces a cursor called name.  Currently you can only register an
   * iImage - a simple pixmap will then be created for use in emulation mode.
   * The 'transparency' can range from 0 (completely opaque) to 255 (completely
   * transparent).
   */
  virtual void SetCursor (const char *name, iImage *image, 
    csRGBcolor* keycolor = 0, csVector2 hotspot = csVector2 (0,0),
    uint8 transparency = 0, csRGBcolor fg = csRGBcolor (255,255,255),
    csRGBcolor bg = csRGBcolor (0,0,0)) = 0;
      
  /// Sets the hotspot (center) of the specified cursor on the pixmap.
  virtual void SetHotSpot (const char *name, csVector2 hotspot) = 0;

  /**
   * Sets the transparency of the specified cursor.  The 'transparency'
   * can range from 0 (completely opaque) to 255 (completely transparent) This
   * will only work if the video driver supports transparency for cursors, or
   * the cursor is being emulated.
   */
  virtual void SetTransparency(const char *name, uint8 transparancy) = 0;
  
  /// Set key colour of the specified cursor.
  virtual void SetKeyColor (const char *name, csRGBcolor) = 0;

  /**
   * Set the foreground and background colors of the cursor.  These will only
   * be used for graphics drivers which support only monochrome cursors.
   */
  virtual void SetColor (const char *name, csRGBcolor fg, csRGBcolor bg) = 0;

  /**
   * Get cursor image of the specified cursor.  The returned reference is
   * invalid -- check with csRef<>::IsValid() -- if the cursor is not
   * registered.  This getter safely can be used to check if a cursor is
   * defined or not without switching cursors.
   */
  virtual csRef<iImage> GetCursorImage (const char *name) const = 0;

  /**
   * Get the hotspot (center) of the specified cursor on the pixmap.
   * Returns default 0,0 if there is no cursor with this name
   */
  virtual csVector2 GetHotSpot (const char *name) const = 0;

  /**
   * Get the transparency of the specified cursor.  Transparency can range from
   * 0 (completely opaque) to 255 (completely transparent).  Returns default 0
   * if there is no cursor with this name.
   */
  virtual uint8 GetTransparency (const char *name) const = 0;
  
  /**
   * Get key colour of the specified cursor.
   * Returns default 0 if the cursor has no keycolor set or there is no 
   * cursor with this name
   */
  virtual const csRGBcolor* GetKeyColor (const char *name) const = 0;

  /**
   * Get the foreground color of the cursor.  These will only
   * be used for graphics drivers which support only monochrome cursors.
   * Returns default 255,255,255 if there is no cursor with this name.
   */
  virtual csRGBcolor GetFGColor (const char *name) const = 0;

  /**
   * Get the background color of the cursor.  These will only
   * be used for graphics drivers which support only monochrome cursors.
   * Returns default 0,0,0 if there is no cursor with this name.
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
