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

  /// The system is idle: we can sleep for a while
  virtual void Sleep (int SleepTime);

  /// Perform extension function
  bool PerformExtension (const char *iCommand, ...);
};

#endif // __CSOS2_H__
