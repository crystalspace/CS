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

#ifndef CSBE_H
#define CSBE_H

#include "cscom/com.h"
#include "def.h"
#include "csinput/csinput.h"
#include "cssys/common/system.h"
#include "cssys/be/beitf.h"
#include "igraph2d.h"

/// BeLIB version.
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
  
public:
  SysSystemDriver ();

  /// Check for system-specific INI entries
  void SetSystemDefaults ();
  // Main event loop
  virtual void Loop ();
  // Parse an unknown argument on command-line
  virtual bool ParseArg (int argc, char* argv [], int& i);
  // Display system-specific help
  virtual void Help ();

  long LoopThread();

  /// Implementation of IBeLibSystemDriver
  class XBeLibSystemDriver : public IBeLibSystemDriver
  {
    DECLARE_IUNKNOWN()
    /// Get user settings
    STDMETHOD (GetSettings) (int &SimDepth, bool &UseSHM, bool &HardwareCursor);
    /// Get Unix-specific keyboard event handler routine
    STDMETHOD (GetKeyboardHandler) (BeKeyboardHandler &Handler, void *&Parm);
    /// Get Unix-specific mouse event handler routine
    STDMETHOD (GetMouseHandler) (BeMouseHandler &Handler, void *&Parm);
    /// Get Unix-specific focus change event handler routine
    STDMETHOD (GetFocusHandler) (BeFocusHandler &Handler, void *&Parm);
    /// Set a callback that gets called from inside the main event loop
    STDMETHOD (SetLoopCallback) (LoopCallback Callback, void *Param);
  };
  // COM stuff
  DECLARE_IUNKNOWN ()
  DECLARE_INTERFACE_TABLE (SysSystemDriver)
  DECLARE_COMPOSITE_INTERFACE_EMBEDDED (BeLibSystemDriver);
  
private:
  /// Called when CrystalSpace window changes its "focused" state
  static void SysFocusChange (void *Self, int Enable);
};

/// BeLIB version.
class SysKeyboardDriver : public csKeyboardDriver
{
//  friend class csSystemDriver;
public:
  SysKeyboardDriver();

  static void Handler (void *param, int Key, bool Down);
//  virtual bool Open (csEventQueue *EvQueue);
//  virtual void Close ();
};

/// BeLIB version.
class SysMouseDriver : public csMouseDriver
{
//  friend class csSystemDriver;
public:
  SysMouseDriver ();

  static void Handler (void *param, int Button, int Down, int x, int y,
    int ShiftFlags);
//  virtual bool Open (csEventQueue *EvQueue);
//  virtual void Close ();
};

#endif // CSBE_H
