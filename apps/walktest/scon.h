/*
    Simple console example
    Copyright (C) 1998 by Jorrit Tyberghein

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

#ifndef __SCON_H__
#define __SCON_H__

#include "support/console.h"

interface ITextureManager;

class csRect;

/// Two possible console modes
enum
{
  MESSAGE_MODE,
  CONSOLE_MODE
};

/**
 * The console.
 */
class SimpleConsole : public csConsole
{
protected:
  /// Text foreground color
  int console_fg;
  /// RGB version of above
  int console_fg_r, console_fg_g, console_fg_b;
  /// Console background color
  int console_bg;
  /// RGB version of above
  int console_bg_r, console_bg_g, console_bg_b;
  /// Colored console background? (what for? :-)
  bool console_colored_bg;
  /// Console mode
  int ConsoleMode;
  /// Select font (-1 = automatic, else one of cs_Font_...)
  int console_font;

  /// 'Work' ticker
  int current_work_done;
  /// Are we 'showing work' now?..
  int showing_work;

public:
  /// Create console object
  SimpleConsole ();
  /// Destroy console object
  virtual ~SimpleConsole ();

  /// Return true if console is active
  virtual bool IsActive () { return (ConsoleMode == CONSOLE_MODE); }
  /// Show the console
  virtual void Show ();
  /// Hide the console
  virtual void Hide ();

  /**
   * Test if the console is transparent and if we should allow
   * the renderer to draw.
   */
  bool IsTransparent () { return !console_colored_bg; }

  /**
   * Set transparency mode for console. 0 = off,
   * 1 = on, -1 = read from config.
   */
  void SetTransparent (int t = -1);

  /// Get the foreground color
  int get_fg () { return console_fg; }

  /// Get the background color
  int get_bg () { return console_bg; }

  /// Add a text line to message console (overlapped on rendering screen)
  void PutMessage (char *str,...);
  /// Replaces last message on the console
  void ReplaceLastMessage (char *str,...);
  /// Shows user that we're not hanging...
  void ShowWork (void);
  /// Add a text line to main console window
  virtual void PutText (char *str, ...);
  /// Print (if console is active) and execute a command
  virtual void ExecuteCommand (char *command);
  /// Clear console
  virtual void Clear ();
  /// Refresh console image
  virtual void Print (csRect* area);

  /// A character key has been pressed
  void AddChar(int c);
  /// Recalculate console colors
  void SetupColors (ITextureManager* txtmgr);

  /// Set the maximum number of lines (-1 = read from config file)
  void SetMaxLines (int ml = -1);

private:
  /// Time left until messages will scroll up
  long LineTime;
  /// Cursor state
  bool CursorState;
  /// Cursor switch time
  long CursorTime;

  /// Console contents
  char **Line;
  /// Number of lines on console
  int LineCount;
  /// Maximum lines on console
  int LineMax;
  /// Characters per line
  int LineSize;
  /// Lines changed since last 'Print'
  char *LinesChanged;

  /// Messages overlapped onto renderer screen
  char **LineMessage;
  /// Number of lines on message pad
  int LineMessageCount;
  /// Maximum lines on message pad
  int LineMessageMax;

  /// Currently edited command line
  char *LineCommand;
  /// Currently edited character
  int LineCommandCount;
  /// Maximal characters in edited line
  int LineCommandMax;

  /// History array
  char **History;
  /// Number of elements in history array
  int HistoryCount;
  /// Maximum history lines
  int HistoryMax;
  /// Current position in history array
  int HistoryCurrent;

  /// Unfinished line for PutText()
  char *buff_text;
};

extern void GfxWrite (int x, int y, int fg, int bg, char *str, ...);

#endif // __SCON_H__
