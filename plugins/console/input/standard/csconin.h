/*
    Standard Console Input
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

#ifndef __CSCONIN_H__
#define __CSCONIN_H__

#include "ivaria/conin.h"
#include "ivaria/conout.h"
#include "isys/plugin.h"
#include "csutil/csstrvec.h"

/**
 * This is the standard command-line handler with history and
 * a connection to a iConsoleOutput for output.
 */
class csConsoleInput : iConsoleInput
{
  // The command history
  csStrVector History;
  int HistoryPos;
  // Max lines in history
  int MaxLines;
  // The callback
  csConsoleExecCallback Callback;
  void *CallbackData;
  // The console
  iConsoleOutput *Console;
  // The prompt
  char *Prompt;
  int PromptLen;
  // Current line
  char *line;
  int linemax;
  // Insert mode
  bool InsertMode;
  // Cursor position
  int CursorPos;

  // Refresh the console input on associated console
  void Refresh ();

public:
  SCF_DECLARE_IBASE;

  /// Construct the object
  csConsoleInput (iBase *iParent);
  /// Destroy the object
  virtual ~csConsoleInput ();

  /// Initialize the plugin, and return success status
  virtual bool Initialize (iSystem *iSys);

  /// Handle an input event
  virtual bool HandleEvent (iEvent &Event);

  /// Bind to a console
  virtual void Bind (iConsoleOutput *iCon);

  /// Set the command execution callback
  virtual void ExecuteCallback (csConsoleExecCallback iCallback,
    void *iCallbackData) { Callback=iCallback; CallbackData=iCallbackData; }

  /// Return a line from the input buffer (-1 = current line)
  virtual const char *GetText (int iLine = -1) const;

  /// Return the current input line number
  virtual int GetCurLine () const;

  /// Retrieve the size of the history buffer
  virtual int GetBufferSize () const
  { return MaxLines; }

  /// Set the size of the history buffer;
  virtual void SetBufferSize (int iSize);

  /// Clear the history buffer
  virtual void Clear ();

  /// Set the prompt string
  virtual void SetPrompt (const char *iPrompt);

  // Implement iPlugin interface.
  struct eiPlugIn : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csConsoleInput);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize(p); }
    virtual bool HandleEvent (iEvent& e) { return scfParent->HandleEvent(e); }
  } scfiPlugin;

  // Implement iConsoleWatcher interface.
  struct eiConsoleWatcher : public iConsoleWatcher
  {
    SCF_DECLARE_EMBEDDED_IBASE(csConsoleInput);
    virtual void ConsoleVisibilityChanged(iConsoleOutput*, bool visible);
  } scfiConsoleWatcher;
  friend struct eiConsoleWatcher;
};

#endif // __CSCONIN_H__
