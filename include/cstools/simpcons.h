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

#ifndef __SIMPCONS_H
#define __SIMPCONS_H

#include "isystem.h"
#include "iconsole.h"
#include "csutil/csbase.h"
#include "csutil/scf.h"

struct iTextureManager;
struct iCursor;
class csRect;
class csIniFile;
class csString;

/**
 * An abstract command interpreter.  Console calls upon this object when it
 * has a command line which needs to be interpreted.
 */
class csSimpleCommand : public csBase
{
public:
  virtual bool PerformLine( char* ) = 0;  // Should be (char const*)
};


/// Two possible console modes
enum
{
  MESSAGE_MODE,
  CONSOLE_MODE
};

/**
 * The console.
 */
class csSimpleConsole : public iConsole
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
  /// Transparent console background?
  bool console_transparent_bg;
  /// Console mode
  int ConsoleMode;
  /// Select font (-1 = automatic, else one of cs_Font_...)
  int console_font;
  /// The command handler
  csSimpleCommand* command_handler;
  /// The configuration file
  csIniFile *config;

public:
  DECLARE_IBASE;
  /// Create console object
  csSimpleConsole (iBase *base);
  csSimpleConsole (csIniFile *iConfig, csSimpleCommand* = 0);
  /// Destroy console object
  virtual ~csSimpleConsole ();

  /// Return true if console is active
  virtual bool IsActive () const { return (ConsoleMode == CONSOLE_MODE); }
  /// Show the console
  virtual void Show ();
  /// Hide the console
  virtual void Hide ();

  /**
   * Test if the console is transparent and if we should allow
   * the renderer to draw.
   */
  bool IsTransparent () { return console_transparent_bg; }

  /**
   * Set transparency mode for console.
   * 0 = off, 1 = on, -1 = read from config.
   */
  void SetTransparent (int t = -1);

  /// Get the foreground color
  int get_fg () { return console_fg; }

  /// Get the background color
  int get_bg () { return console_bg; }

  /// Add a text line to message console (overlapped on rendering screen)
  void PutMessage (bool advance, const char *str);
  /// Print (if console is active) and execute a command
  virtual void ExecuteCommand (char *command);
  /// Clear console
  virtual void Clear ();
  /// Refresh console image
  virtual void Print (csRect* area);

  /// A character key has been pressed
  void AddChar(int c);
  /// Recalculate console colors
  void SetupColors (iTextureManager* txtmgr);

  /// Set the maximum number of lines (-1 = read from config file)
  void SetMaxLines (int ml = -1);

  /// iConsole compatibility methods
  virtual bool Initialize(iSystem *system) { return (system->RegisterDriver("iConsole", this)); }
  virtual void PutText(const char *text);
  virtual const csString *GetText(int line = -1) const { return NULL; } /* Not supported */
  virtual void Draw(csRect *rect = NULL) { Print(rect); }
  virtual void SetBufferSize(int lines) { SetMaxLines(lines); }
  virtual void CacheColors(iTextureManager *txtmgr) { SetupColors(txtmgr); }
  virtual void GetForeground(int &red, int &green, int &blue) const { red = console_fg_r; green = console_fg_g; blue = console_fg_b; }
  virtual void SetForeground(int red, int green, int blue) { console_fg_r = red; console_fg_g = green; console_fg_b = blue; }
  virtual void GetBackground(int &red, int &green, int &blue) const  { red = console_bg_r; green = console_bg_g; blue = console_bg_b; }
  virtual void SetBackground(int red, int green, int blue)  { console_bg_r = red; console_bg_g = green; console_bg_b = blue; }
  virtual void GetPosition(int &, int &, int &, int &) const { /* Not supported */ }
  virtual void SetPosition(int, int, int = -1, int = -1) { /* Not supported */ }
  virtual void Invalidate(csRect &) { /* Not supported */ }
  virtual bool GetTransparency() const { return console_transparent_bg; }
  virtual void SetTransparency(bool trans) { console_transparent_bg = trans; }
  virtual int GetFontID() const { return console_font; }
  virtual void SetFontID(int FontID) { console_font = FontID; }
  virtual int GetTopLine() const { /* Not supported */ return -1; }
  virtual void ScrollTo(int, bool = true) { /* Not supported */ }
  virtual void GetCursorPos(int &, int &) const { /* Not supported */ }
  virtual void SetCursorPos(int, int) { /* Not supported */ }
  virtual int GetCursorStyle(bool &, iCursor ** = NULL) const { /* Not supported */ return -1; }
  virtual void SetCursorStyle(int, bool = true, iCursor * = NULL) { /* Not supported */ }

private:
  /// Time left until messages will scroll up
  time_t LineTime;
  /// Cursor state
  bool CursorState;
  /// Cursor switch time
  time_t CursorTime;

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

  /// History array
  char **History;
  /// Number of elements in history array
  int HistoryCount;
  /// Maximum history lines
  int HistoryMax;
  /// Current position in history array
  int HistoryCurrent;
};

extern void GfxWrite (int x, int y, int fg, int bg, char *str, ...);

#endif // __SIMPCONS_H
