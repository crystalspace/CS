/*
  OS/2 support for Crystal Space 3D library
  Copyright (C) 1998 by Jorrit Tyberghein
  Written by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CSOS2_H__
#define __CSOS2_H__

#include "csinput/csinput.h"
#include "cssys/system.h"
#include "icsos2.h"

/**
 * This is the System driver for OS/2. It implements all required
 * functionality for standard csSystemDriver class.
 */
class SysSystemDriver : public csSystemDriver
{
  /// Window position in percents
  int WindowX, WindowY;
  /// Window width and height
  int WindowWidth, WindowHeight;
  /// Use system cursor if true; otherwise use builtin CSWS software cursors
  bool HardwareCursor;

public:
  /// Initialize system-dependent data
  SysSystemDriver ();

  /// Check if configuration files requests 16 bits per pixel
  virtual void SetSystemDefaults (csIniFile *config);

  /**
   * This is a function that prints the commandline help text.
   * If system has system-dependent switches, it should override
   * this method and type its own text (possibly invoking
   * csSystemDriver::Help() first).
   */
  virtual void Help ();

  /**
   * System loop. This should be called last since it returns
   * only on program exit
   */
  virtual void Loop ();

  /// The system is idle: we can sleep for a while
  virtual void Sleep (int SleepTime);

  /// Implementation of IOS2SystemDriver
  class XOS2SystemDriver : public IOS2SystemDriver
  {
    DECLARE_IUNKNOWN ()
    /// Get user settings
    STDMETHOD (GetSettings) (int &WindowX, int &WindowY,
      int &WindowWidth, int &WindowHeight, bool &HardwareCursor);
    /// Put a keyboard event into event queue
    STDMETHOD (KeyboardEvent) (int ScanCode, bool Down);
    /// Put a mouse event into event queue
    STDMETHOD (MouseEvent) (int Button, int Down, int x, int y, int ShiftFlags);
    /// Put a focus change event into event queue
    STDMETHOD (FocusEvent) (bool Enable);
    /// Handle application termination event
    STDMETHOD (TerminateEvent) ();
  };

  // COM stuff
  DECLARE_IUNKNOWN ()
  DECLARE_INTERFACE_TABLE (SysSystemDriver)
  DECLARE_COMPOSITE_INTERFACE_EMBEDDED (OS2SystemDriver)

protected:
  /**
   * This is a system-dependent function which eats a single
   * command-line option (like -help, ...). If system-dependent
   * part does not recognize the option, it should pass it to
   * its parent class method.
   */
  virtual bool ParseArg (int argc, char* argv[], int& i);
  /// Called when CrystalSpace window changes its "focused" state
  void FocusHandler (bool Enable);
  /// Called when window is closed by user
  void TerminateHandler ();
};

/**
 * This is the Keyboard class for OS/2. All of its functionality
 * is included in the 2D graphics driver; this class is just a
 * container for the handler function.
 */
class SysKeyboardDriver : public csKeyboardDriver
{
public:
  SysKeyboardDriver ();

  void Handler (unsigned char ScanCode, bool Down);
};

/**
 * This is the Mouse class for OS/2. All of its functionality
 * is included in the 2D graphics driver; this class is just a
 * container for the handler function.
 */
class SysMouseDriver : public csMouseDriver
{
public:
  SysMouseDriver ();

  void Handler (int Button, int Down, int x, int y, int ShiftFlags);
};

#endif // __CSOS2_H__
