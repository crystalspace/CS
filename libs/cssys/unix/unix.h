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

#include "csutil/scf.h"
#include "csinput/csinput.h"
#include "cssys/system.h"
#include "iunix.h"
#include "igraph2d.h"

/// Unix version.
class SysSystemDriver : public csSystemDriver, public iUnixSystemDriver
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

public:
  DECLARE_IBASE;

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

  /// Implementation of iUnixSystemDriver

  /// Get user settings
  virtual void GetExtSettings (int &SimDepth, bool &UseSHM, bool &HardwareCursor);
  /// Set a callback that gets called from inside the main event loop
  virtual void SetLoopCallback (LoopCallback Callback, void *Param);
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
