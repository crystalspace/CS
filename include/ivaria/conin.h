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

#ifndef __IVARIA_CONIN_H__
#define __IVARIA_CONIN_H__

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
 * <p>
 * <b>WARNING</b> Do NOT use the event handler
 * that may (or may not) be implemented by the console and register
 * that to the event queue. This doesn't work properly. Instead register
 * your own event handler and call HandleEvent() from that.
 */
struct iConsoleInput : public iBase
{
  /// Bind to a console.
  virtual void Bind (iConsoleOutput*) = 0;

  /// Set the command execution callback.
  virtual void SetExecuteCallback (iConsoleExecCallback* iCallback) = 0;
  /// Get the command execution callback.
  virtual iConsoleExecCallback* GetExecuteCallback () = 0;

  /// Return a line from the input buffer (-1 = current line).
  virtual const char *GetText (int iLine = -1) const = 0;

  /// Return the current input line number.
  virtual int GetCurLine () const = 0;

  /// Retrieve the size of the history buffer.
  virtual int GetBufferSize () const = 0;

  /// Set the size of the history buffer.
  virtual void SetBufferSize (int iSize) = 0;

  /// Clear the history buffer.
  virtual void Clear () = 0;

  /// Set the prompt string.
  virtual void SetPrompt (const char *iPrompt) = 0;

  /**
   * Handle a console-related event. Do NOT use the event handler
   * that may (or may not) be implemented by the console and register
   * that to the event queue. This doesn't work properly. Instead register
   * your own event handler and call HandleEvent() from that.
   */
  virtual bool HandleEvent (iEvent&) = 0;
};

#endif // __IVARIA_CONIN_H__
