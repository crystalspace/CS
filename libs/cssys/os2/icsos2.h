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

#ifndef __ICSOS2_H__
#define __ICSOS2_H__

#include "csutil/scf.h"

SCF_VERSION (iOS2SystemDriver, 0, 0, 1);

struct iOS2SystemDriver : public iBase
{
  /// Get user settings
  virtual void GetExtSettings (int &oWindowX, int &oWindowY,
    int &oWindowWidth, int &oWindowHeight, bool &oHardwareCursor) = 0;
  /// Put a keyboard event into event queue
  virtual void KeyboardEvent (int ScanCode, bool Down) = 0;
};

#endif // __ICSOS2_H__
