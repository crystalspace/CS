/*
    OS/2 support for Crystal Space 3D library
    Copyright (C) 1998 by Andrew Zabolotny <bit@eltech.ru>

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

#include "cssys/csinput.h"
#include "cssys/system.h"
#include "icsos2.h"

/**
 * This is the System driver for OS/2. It implements all required
 * functionality for standard csSystemDriver class.
 */
class SysSystemDriver : public csSystemDriver, public iOS2SystemDriver
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

  DECLARE_IBASE_EXT (csSystemDriver)

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

  /// Implementation of iOS2SystemDriver

  /// Get user settings
  virtual void GetExtSettings (int &oWindowX, int &oWindowY,
    int &oWindowWidth, int &oWindowHeight, bool &oHardwareCursor);
  /// Put a keyboard event into event queue
  virtual void KeyboardEvent (int ScanCode, bool Down);
};

#endif // __CSOS2_H__
