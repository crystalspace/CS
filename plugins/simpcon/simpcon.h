/*
    Simple Console
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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

#ifndef __SIMPCONS_H__
#define __SIMPCONS_H__

#include "isys/system.h"
#include "ivaria/conout.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/fontserv.h"
#include "csutil/csbase.h"
#include "csutil/scf.h"

class csRect;
struct iGraphics3D;
struct iGraphics2D;

/// Two possible console modes
enum
{
  MESSAGE_MODE,
  CONSOLE_MODE
};

/**
 * A simple console plugin.
 */
class csSimpleConsole : public iConsole
{
  /// Text foreground color
  int console_fg;
  /// RGB version of above
  int console_fg_r, console_fg_g, console_fg_b;
  /// Console background color
  int console_bg;
  /// RGB version of above
  int console_bg_r, console_bg_g, console_bg_b;
  /// Transparent console background?
  bool console_transparent_bg;
  /// Console mode
  int ConsoleMode;
  /// Select font
  iFont *console_font;
  /// The width and height of graphics canvas
  int FrameWidth, FrameHeight;
  /// The system driver
  iSystem *System;
  /// The 3D driver
  iGraphics3D *G3D;
  /// The 2D driver
  iGraphics2D *G2D;
  /// Cursor style
  int CursorStyle;
  /// Automatically update?
  bool Update;
  /// System is opened?
  bool SystemReady;
  /// Cursor position
  int CursorPos;
  /// Clear input on next character flag
  bool ClearInput;
  /// The client that should be notified
  iPlugIn *Client;
  /// Is the entire view invalid?
  bool InvalidAll;

  void GfxWrite (int x, int y, int fg, int bg, char *iText, ...);
  void PutMessage (bool advance, const char *iText);
  void FreeLineMessage ();
  void FreeBuffer ();
  void SetLineMessages (int iCount);
  void DisplayInput ();
  void CacheColors ();

public:
  DECLARE_IBASE;

  /// Create console object
  csSimpleConsole (iBase *iParent);
  /// Destroy console object
  virtual ~csSimpleConsole ();

  /// Initialize the plugin, and return success status
  virtual bool Initialize (iSystem *iSys);

  /// Intercept events
  virtual bool HandleEvent (iEvent &Event);

  //-------------------------- iConsole interface ----------------------------//

  /**
   * Put some text to the console. Console acts like a simple
   * TTY and should interpret basical symbols like '\n'.
   */
  virtual void PutText (int iMode, const char *iText);

  /// Return a line from the buffer (-1 = current line)
  virtual const char *GetLine (int iLine = -1) const;

  /**
   * Display the console and return the dirty rectangle.
   * The graphics driver should be in 2D draw mode.
   */
  virtual void Draw2D (csRect *oRect);

  /**
   * Update the 3D part of the console on the window.
   * The graphics driver should be in 3D draw mode.
   */
  virtual void Draw3D (csRect *) {}

  /**
   * Clear console. If wipe = false, it just moves the top line to the
   * current line; if wipe is true, it clears the buffer completely.
   */
  virtual void Clear (bool iWipe = false);

  /// Set the buffer size in lines
  virtual void SetBufferSize (int iMaxLines);

  /// Retrieve the transparency setting
  virtual bool GetTransparency () const
  { return console_transparent_bg; }
  /// Set transparency
  virtual void SetTransparency (bool iTransp);

  /// Gets the current font.
  virtual iFont *GetFont () const
  { return console_font; }
  /// Sets the type of the font.
  virtual void SetFont (iFont *Font)
  {
    if (console_font) console_font->DecRef ();
    console_font = Font;
    if (console_font) console_font->IncRef ();
  }

  /// Get the current top line being displayed
  virtual int GetTopLine () const
  { return 0; }
  /**
   * Set the current top line, or use of the constants above for scrolling.
   * If snap is true, the console returns to the very bottom of the display
   * when a new line is printed.
   */
  virtual void ScrollTo (int iTopLine, bool iSnap = true)
  { (void)iTopLine; (void)iSnap; }

  /// Retrieve the cursor style and whether the cursor flashes
  virtual int GetCursorStyle () const
  { return CursorStyle; }
  /// Assign the cursor style, and whether it flashes
  virtual void SetCursorStyle (int iStyle)
  { CursorStyle = iStyle; }

  /**
   * Show/hide the console. In 'hidden' state console should not display
   * anything at all (when Draw() is called) or draw some minimal information
   */
  virtual void SetVisible (bool iShow);
  /**
   * Query whether the console is visible or hidden.
   */
  virtual bool GetVisible ()
  { return (ConsoleMode == CONSOLE_MODE); }
  /**
   * Enable or disable automatic console updates.
   * When the console is in console auto-update mode, it automatically
   * calls BeginDraw/Console->Draw methods on every PutText call.
   * Otherwise it is your responsability to call Draw() at appropiate
   * times. Initially this mode is enabled.
   */
  virtual void AutoUpdate (bool iAutoUpdate)
  { Update = iAutoUpdate; }

  /// Set cursor horizontal position (-1 == follow output)
  virtual void SetCursorPos (int iCharNo);

  /// Query maximal line width in characters
  virtual int GetMaxLineWidth ()
  { return LineSize; }

  /**
   * Tell console that this object should be notified when console 
   * visibility status changes.
   */
  virtual void RegisterPlugin (iPlugIn *iClient)
  { Client = iClient; }

  /**
   * Implement simple extension commands.
   */
  virtual bool ConsoleExtension (const char *iCommand, ...)
  { (void)iCommand; return false; }

private:
  /// Time left until messages will scroll up
  cs_time LineTime;
  /// Cursor state
  bool CursorState;
  /// Cursor switch time
  cs_time CursorTime;

  /// Console contents
  char **Line;
  /// Current output line on console
  int LineNumber;
  /// Maximum lines on console
  int LineMax;
  /// Characters per line
  int LineSize;
  /// Lines changed since last 'Print'
  bool *LinesChanged;

  /// Messages overlapped onto renderer screen
  char **LineMessage;
  /// Current output line on message pad
  int LineMessageNumber;
  /// Maximum lines on message pad
  int LineMessageMax;

  /// Currently edited command line
  char *LineCommand;
  /// Currently edited character
  int LineCommandCount;
  /// Maximal characters in edited line
  int LineCommandMax;
};

#endif // __SIMPCONS_H__
