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

#ifndef __CS_IVARIA_CONIN_H__
#define __CS_IVARIA_CONIN_H__

/**\file
 * Graphical console input
 */

#include "csutil/scf.h"

struct iEvent;
struct iConsoleOutput;


SCF_VERSION (iConsoleExecCallback, 0, 0, 1);

/**
 * Command execution callback.
 */
struct iConsoleExecCallback : public iBase
{
  /// Execute.
  virtual void Execute (const char* cmd) = 0;
};

SCF_VERSION (iConsoleInput, 1, 0, 1);

/**
 * This is a plugin that can handle keyboard input and display
 * it on an associated console. The plugin has a command history
 * and when user presses 'Enter' can call some callback function
 * to execute the entered command.
 * 
 * <b>WARNING</b> Do NOT use the event handler
 * that may (or may not) be implemented by the console and register
 * that to the event queue. This doesn't work properly. Instead register
 * your own event handler and call HandleEvent() from that.
 * 
 * Main creators of instances implementing this interface:
 * - Standard input console plugin (crystalspace.console.input.standard)
 * 
 * Main ways to get pointers to this interface:
 * - csQueryRegistry()
 */
struct iConsoleInput : public iBase
{
  /**
   * Bind to an output console.
   * \param console
   */
  virtual void Bind (iConsoleOutput* console) = 0;

  /**
   * Set the command execution callback.
   * \param callback will be called whenever the user presses enter
   * so the application can then perform the command that was entered.
   */
  virtual void SetExecuteCallback (iConsoleExecCallback* callback) = 0;
  /// Get the command execution callback.
  virtual iConsoleExecCallback* GetExecuteCallback () = 0;

  /**
   * Return a line from the input buffer.
   * \param line is the line number to get or -1 (default) for current line.
   */
  virtual const char *GetText (int line = -1) const = 0;

  /// Return the current input line number.
  virtual int GetCurLine () const = 0;

  /// Retrieve the size of the history buffer.
  virtual int GetBufferSize () const = 0;

  /**
   * Set the size of the history buffer.
   * \param size is the size in lines of the history buffer.
   */
  virtual void SetBufferSize (int size) = 0;

  /// Clear the history buffer.
  virtual void Clear () = 0;

  /// Set the prompt string.
  virtual void SetPrompt (const char *prompt) = 0;

  /**
   * Handle a console-related event. Do NOT use the event handler
   * that may (or may not) be implemented by the console and register
   * that to the event queue. This doesn't work properly. Instead register
   * your own event handler and call HandleEvent() from that.
   */
  virtual bool HandleEvent (iEvent&) = 0;
};

#endif // __CS_IVARIA_CONIN_H__
