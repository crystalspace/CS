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

#ifndef __CS_ICONSOLE_H__
#define __CS_ICONSOLE_H__

#include "iplugin.h"
#include "csutil/scf.h"

class csRect;
class csString;
struct iTextureManager;
struct iCursor;

/// These constants are for use with the ScrollTo() member function below
enum ScrollConst
{
  csConPageUp = -1,
  csConPageDown = -2,
  csConVeryTop = -3,
  csConVeryBottom = -4
};

enum CursorConst
{
  csConNoCursor = 0,
  csConLineCursor,
  csConBlockCursor,
  csConCustomCursor
};

/**
 * This is the Crystal Space Console interface.  It is an output only system.
 * It can be used in conjunction with the iConsoleInput interface in iconinp.h
 * to form an interactive console.
 *
 * These some ideas for future additions, but they might be better suited for
 * a subclass of iConsole:
 *
 * Alpha transparency (not directly supported by 2D driver)
 * Background texture(s) (may not be available during startup)
 */
SCF_VERSION(iConsole, 0, 0, 3);
struct iConsole : public iPlugIn
{
  /// Show the console   !DEPRECATED!
  virtual void Show() = 0;
  /// Hide the console   !DEPRECATED!
  virtual void Hide() = 0;

  /// Print a string to the console
  virtual void PutText(const char *text) = 0;

  /// Return a line from the buffer (-1 = current line)
  virtual const csString *GetText(int line = -1) const = 0;

  /// Update the console on the window.
  virtual void Draw(csRect *rect = NULL) = 0;

  /// Return true if console is active  !DEPRECATED!
  virtual bool IsActive() const = 0;

  /// Clear console
  virtual void Clear() = 0;

  /// Set the buffer size in lines
  virtual void SetBufferSize(int maxlines) = 0;

  /// Retrieve the console colors from the current palette
  virtual void CacheColors(iTextureManager *txtmgr) = 0;

  /// Get the foreground color
  virtual void GetForeground(int &red, int &green, int &blue) const = 0;
  /// Set the foreground color.  CacheColor() must be called before it goes into effect!
  virtual void SetForeground(int red, int green, int blue) = 0;

  /// Get the background color
  virtual void GetBackground(int &red, int &green, int &blue) const = 0;
  /// Set the background color.  CacheColor() must be called before it goes into effect!
  virtual void SetBackground(int red, int green, int blue) = 0;

  /// Retrieve the current coordinates and width/height of the console rectangle
  virtual void GetPosition(int &x, int &y, int &width, int &height) const = 0;
  /// Set the coordinates and/or width/height of the console rectangle.  
  /// -1 is interpreted as "use current value" for all four parameters.
  /// Other negative values are not legal.
  virtual void SetPosition(int x, int y, int width = -1, int height = -1) = 0;

  /// Invalidates part of the console for redrawal
  virtual void Invalidate(csRect &area) = 0;

  /// Retrieve the transparency setting
  virtual bool GetTransparency() const = 0;
  /// Set transparency
  virtual void SetTransparency(bool trans) = 0;

  /// Gets the ID of current font.
  virtual int GetFontID() const = 0;
  /// Sets the type of the font.
  virtual void SetFontID(int FontID) = 0;

  /// Get the current top line being displayed
  virtual int GetTopLine() const = 0;
  /** Set the current top line, or use of the constants above for scrolling
   * if snap is true, the console returns to the very bottom of the display
   * when a new line is printed
   */
  virtual void ScrollTo(int topline, bool snap = true) = 0;

  /// Retrieve the cursor position in text coordinates
  virtual void GetCursorPos(int &x, int &y) const = 0;
  /// Set the cursor position in text coordinates
  virtual void SetCursorPos(int x, int y) = 0;
  /**
   * Retrieve the cursor style, and if it's a custom cursor 
   * and custom is not NULL, it will return the texture handle
   * of the cursor.  Also returns whether the cursor flashes
   */
  virtual int GetCursorStyle(bool &flashing, iCursor **custom = NULL) const = 0;
  /**
   * Assign the cursor style, whether it flashes, and if custom,
   * assign the iCursor object to use.
   */
  virtual void SetCursorStyle(int style, bool flashing = true, iCursor *custom = NULL) = 0;

};

#endif // ! __CS_ICONSOLE_H__
