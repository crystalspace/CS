/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Written by Michael Dale Long
  
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

#ifndef __IVARIA_CONOUT_H__
#define __IVARIA_CONOUT_H__

#include <stdarg.h>
#include "isys/plugin.h"

class csRect;
struct iTextureManager;
struct iFont;

/// These constants are for use with the ScrollTo() member function below
enum
{
  csConPageUp = -1,
  csConPageDown = -2,
  csConVeryTop = -3,
  csConVeryBottom = -4
};

enum
{
  csConNoCursor = 0,
  csConNormalCursor,
  csConInsertCursor
};

SCF_VERSION (iConsoleOutput, 2, 0, 0);

/**
 * This is the Crystal Space Console interface.  It is an output only system.
 * It can be used in conjunction with the iConsoleInput interface to form an
 * interactive console.
 */
struct iConsoleOutput : public iBase
{
  /**
   * Put some text to the console. Console acts like a simple
   * TTY and should interpret basical symbols like '\n' and '\b'.
   * The '\r' character has a special meaning: it sets a flag that
   * tells console to clear the current line before next character
   * is output. That is, you can emmit non-persistent messages
   * this way: PutText ("some text\r"); This message will disappear
   * as soon as any other message will be sent to console.
   */
  virtual void PutText (int iMode, const char *iText) = 0;

  /// Return a line from the buffer (-1 = current line)
  virtual const char *GetLine (int iLine = -1) const = 0;

  /**
   * Display the console and return the dirty rectangle.
   * The graphics driver should be in 2D draw mode.
   */
  virtual void Draw2D (csRect *oRect = NULL) = 0;

  /**
   * Update the 3D part of the console on the window.
   * The graphics driver should be in 3D draw mode.
   */
  virtual void Draw3D (csRect *oRect = NULL) = 0;

  /**
   * Clear console. If wipe = false, it just moves the top line to the
   * current line; if wipe is true, it clears the buffer completely.
   */
  virtual void Clear (bool iWipe = false) = 0;

  /// Set the buffer size in lines
  virtual void SetBufferSize (int iMaxLines) = 0;

  /// Retrieve the transparency setting
  virtual bool GetTransparency () const = 0;
  /// Set transparency
  virtual void SetTransparency (bool iTransp) = 0;

  /// Gets the current font.
  virtual iFont *GetFont () const = 0;
  /// Sets the type of the font.
  virtual void SetFont (iFont *Font) = 0;

  /// Get the current top line being displayed
  virtual int GetTopLine () const = 0;
  /**
   * Set the current top line, or use of the constants above for scrolling.
   * If snap is true, the console returns to the very bottom of the display
   * when a new line is printed.
   */
  virtual void ScrollTo (int iTopLine, bool iSnap = true) = 0;

  /// Retrieve the cursor style
  virtual int GetCursorStyle () const = 0;
  /// Assign the cursor style
  virtual void SetCursorStyle (int iStyle) = 0;

  /**
   * Show/hide the console. In 'hidden' state console should not display
   * anything at all (when Draw() is called) or draw some minimal information
   */
  virtual void SetVisible (bool iShow) = 0;
  /**
   * Query whether the console is visible or hidden.
   */
  virtual bool GetVisible () = 0;

  /**
   * Enable or disable automatic console updates.
   * When the console is in console auto-update mode, it automatically
   * calls BeginDraw/Console->Draw methods on every PutText call.
   * Otherwise it is your responsability to call Draw() at appropiate
   * times. Initially this mode is enabled.
   */
  virtual void AutoUpdate (bool iAutoUpdate) = 0;

  /// Set cursor horizontal position (-1 == follow output)
  virtual void SetCursorPos (int iCharNo) = 0;

  /// Query maximal line width in characters
  virtual int GetMaxLineWidth () = 0;

  /**
   * Tell console that this object should be notified when console 
   * visibility status changes.
   */
  virtual void RegisterPlugin (iPlugIn *iClient) = 0;

  /// Implement simple extension commands.
  virtual bool ConsoleExtension (const char *iCommand, ...) = 0;

  /// Implement simple extension commands.
  virtual bool ConsoleExtensionV (const char *iCommand, va_list) = 0;
};

#endif // __IVARIA_CONOUT_H__
