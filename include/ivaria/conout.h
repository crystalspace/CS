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

#ifndef __CS_IVARIA_CONOUT_H__
#define __CS_IVARIA_CONOUT_H__

#include "csutil/scf.h"


class csRect;
struct iConsoleOutput;
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

SCF_VERSION (iConsoleWatcher, 0, 0, 1);

/**
 * This interface is implemented by objects interested in knowing when the
 * console's visibility status has changed.
 */
struct iConsoleWatcher : public iBase
{
  /// Called when the watched console's visibility status changes.
  virtual void ConsoleVisibilityChanged(iConsoleOutput*, bool visible) = 0;
};


SCF_VERSION (iConsoleOutput, 2, 1, 0);

/**
 * This is the Crystal Space Console interface.  It is an output only system.
 * It can be used in conjunction with the iConsoleInput interface to form an
 * interactive console.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>Simple output console plugin (crystalspace.console.output.simple)
 *   <li>Standard output console plugin (crystalspace.console.output.standard)
 *   <li>Fancy output console plugin (crystalspace.console.output.fancy)
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>CS_QUERY_REGISTRY()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iConsoleInput
 *   <li>iStandardReporterListener
 *   </ul>
 */
struct iConsoleOutput : public iBase
{
  /**
   * Put some text to the console. Console acts like a simple
   * TTY and should interpret basical symbols like '@\n' and '@\b'.
   * The '@\r' character has a special meaning: it sets a flag that
   * tells console to clear the current line before next character
   * is output. That is, you can emmit non-persistent messages
   * this way: PutText ("some text@\r"); This message will disappear
   * as soon as any other message will be sent to console.
   *
   * Remember that if you are not using the printf-style functionality, and
   * the string may include '%' signs, you should do PutText("%s", str)
   * instead of PutText(str).
   * \sa \ref FormatterNotes
   */
  virtual void PutText (const char *text, ...) CS_GNUC_PRINTF (2, 3) = 0;

  /**
   * Var_args version of PutText.
   */
  virtual void PutTextV (const char *text, va_list args)
      CS_GNUC_PRINTF (2, 0) = 0;

  /**
   * Return a line from the buffer.
   * \param line is the line to get or -1 (default) for the current line.
   */
  virtual const char *GetLine (int line = -1) const = 0;

  /**
   * Display the console and return the dirty rectangle.
   * The graphics driver should be in 2D draw mode.
   * Only call this function from the same thread that maintains
   * the graphics subsystem!
   * \param oRect will be set to the dirty rectangle so you can
   * choose to update only that part of the screen. You can
   * also supply 0 (default) if you are not interested in that.
   */
  virtual void Draw2D (csRect *oRect = 0) = 0;

  /**
   * Update the 3D part of the console on the window.
   * The graphics driver should be in 3D draw mode.
   * Only call this function from the same thread that maintains
   * the graphics subsystem!
   * \param oRect will be set to the dirty rectangle so you can
   * choose to update only that part of the screen. You can
   * also supply 0 (default) if you are not interested in that.
   */
  virtual void Draw3D (csRect *oRect = 0) = 0;

  /**
   * Clear console.
   * \param wipe If true then the buffer is completely cleared. Else (default)
   * the top line is moved to the current line.
   */
  virtual void Clear (bool wipe = false) = 0;

  /// Set the buffer size in lines.
  virtual void SetBufferSize (int maxLines) = 0;

  /// Retrieve the transparency setting.
  virtual bool GetTransparency () const = 0;
  /// Set transparency.
  virtual void SetTransparency (bool transp) = 0;

  /// Gets the current font.
  virtual iFont *GetFont () const = 0;
  /// Sets the font.
  virtual void SetFont (iFont *font) = 0;

  /// Get the current top line being displayed.
  virtual int GetTopLine () const = 0;
  /**
   * Set the current top line, or use of the constants above for scrolling.
   * \param topLine is the new top line.
   * \param snap If true the console returns to the very bottom of the display
   * when a new line is printed.
   */
  virtual void ScrollTo (int topLine, bool snap = true) = 0;

  /// Retrieve the cursor style.
  virtual int GetCursorStyle () const = 0;
  /// Assign the cursor style.
  virtual void SetCursorStyle (int style) = 0;

  /**
   * Show/hide the console. In 'hidden' state console should not display
   * anything at all (when Draw() is called) or draw some minimal information
   */
  virtual void SetVisible (bool show) = 0;
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
   * <p>
   * Note that some implementations of the output consoles may be thread-safe.
   * But in that case you MUST use AutoUpdate(false) because access to
   * graphics is not thread-safe.
   */
  virtual void AutoUpdate (bool autoUpdate) = 0;

  /// Set cursor horizontal position (-1 == follow output)
  virtual void SetCursorPos (int charNo) = 0;

  /// Query maximal line width in characters
  virtual int GetMaxLineWidth () = 0;

  /**
   * Tell console that this object should be notified when console
   * visibility status changes.
   */
  virtual void RegisterWatcher (iConsoleWatcher*) = 0;

  /// Implement simple extension commands.
  virtual bool PerformExtension (const char *command, ...) = 0;

  /// Implement simple extension commands.
  virtual bool PerformExtensionV (const char *command, va_list) = 0;
};

#endif // __CS_IVARIA_CONOUT_H__
