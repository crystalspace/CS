/*
    DOS support for Crystal Space 3D library
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by David N. Arnold <derek_arnold@fuse.net>
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

#ifndef DJGPP_H
#define DJGPP_H

#include "idjgpp.h"
#include "cssys/csinput.h"
#include "cssys/system.h"

/// DJGPP version.
class SysSystemDriver : public csSystemDriver, public iDosSystemDriver
{
public:
  SysSystemDriver ();

  virtual void Loop ();

  DECLARE_IBASE_EXT (csSystemDriver);

  virtual bool Open (const char *Title);
  virtual void Close ();

  /// Implementation of iDosSystemDriver

  /// Set mouse position since mouse driver is part of system driver
  virtual bool SetMousePosition (int x, int y);

private:
  bool KeyboardOpened;
  bool MouseOpened;
  bool MouseExists;
  float SensivityFactor;
  int mouse_sensivity_x;
  int mouse_sensivity_y;
  int mouse_sensivity_threshold;
};

#endif // DJGPP_H
