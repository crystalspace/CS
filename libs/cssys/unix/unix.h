/*
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

#ifndef __UNIX_H__
#define __UNIX_H__

#include "cscom/com.h"
#include "csinput/csinput.h"
#include "cssys/common/system.h"
#include "iunix.h"
#include "igraph2d.h"

/// Unix version.
class SysSystemDriver : public csSystemDriver
{
  /// Use system cursor if true; otherwise use builtin CSWS software cursors
  bool HardwareCursor;
  /// Use shared-memory extension? (ONLY FOR X2D DRIVER)
  bool UseSHM;
  /// Simulated depth (ONLY FOR X2D DRIVER)
  int SimDepth;
  /// The main loop callback
  LoopCallback Callback;
  /// The loop callback parameter
  void *CallbackParam;

  /// Application focus change event handler
  void FocusHandler (int Enable);
  
public:
  // Constructor
  SysSystemDriver ();

  /// Check for system-specific INI entries
  virtual void SetSystemDefaults (csIniFile *config);
  // Main event loop
  virtual void Loop ();
  // Parse an unknown argument on command-line
  virtual bool ParseArg (int argc, char* argv [], int& i);
  // Display system-specific help
  virtual void Help ();
  // Sleep for given number of 1/1000 seconds
  virtual void Sleep (int SleepTime);

  /// Implementation of IUnixSystemDriver
  class XUnixSystemDriver : public IUnixSystemDriver
  {
    DECLARE_IUNKNOWN()
    /// Get user settings
    STDMETHOD (GetSettings) (int &SimDepth, bool &UseSHM, bool &HardwareCursor);
    /// Set a callback that gets called from inside the main event loop
    STDMETHOD (SetLoopCallback) (LoopCallback Callback, void *Param);
    /// Put a keyboard event into event queue
    STDMETHOD (KeyboardEvent) (int Key, bool Down);
    /// Put a mouse event into event queue
    STDMETHOD (MouseEvent) (int Button, int Down, int x, int y, int ShiftFlags);
    /// Put a focus change event into event queue
    STDMETHOD (FocusEvent) (int Enable);
  };
  // COM stuff
  DECLARE_IUNKNOWN ()
  DECLARE_INTERFACE_TABLE (SysSystemDriver)
  DECLARE_COMPOSITE_INTERFACE_EMBEDDED (UnixSystemDriver);
};

/// Unix version.
class SysKeyboardDriver : public csKeyboardDriver
{
public:
  SysKeyboardDriver ();

  void Handler (int Key, bool Down);
};

/// Unix version.
class SysMouseDriver : public csMouseDriver
{
public:
  SysMouseDriver ();

  void Handler (int Button, int Down, int x, int y, int ShiftFlags);
};

#endif // __UNIX_H__
