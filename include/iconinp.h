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

#ifndef __CS_ICONSOLEINPUT_H__
#define __CS_ICONSOLEINPUT_H__

#include "iplugin.h"
#include "csutil/scf.h"

SCF_VERSION(iConsoleInput, 0, 0, 1);
struct iConsoleInput : public iPlugIn
{
  /// Initialize the plugin, and return success status
  virtual bool Initialize(iSystem *iSys) = 0;

  /// Handle a keyboard event
  virtual bool HandleEvent(csEvent &Event) = 0;

  /// Return a line from the input buffer (-1 = current line)
  virtual csString *GetInput(int line = -1) = 0;

  /// Return the current input line number
  virtual int GetCurLine() const = 0;

  /// Save the current line in history and create a new input line
  virtual void NewLine() = 0;

};

#endif // ! __CS_ICONSOLEINPUT_H__
