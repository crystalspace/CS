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
#include "csinput/csinput.h"
#include "cssys/system.h"

/// DJGPP version.
class SysSystemDriver : public csSystemDriver, public iDosSystemDriver
{
public:
  DECLARE_IBASE;

  SysSystemDriver ();

  virtual void Loop ();

  /// Implementation of iDosSystemDriver

  /// Enable or disable text-mode CsPrintf
  virtual void EnablePrintf (bool Enable);
  /// Set mouse position since mouse driver is part of system driver
  virtual bool SetMousePosition (int x, int y);
};

/// DJGPP version.
class SysKeyboardDriver : public csKeyboardDriver
{
private:
  bool KeyboardOpened;
public:
  SysKeyboardDriver ();
  virtual ~SysKeyboardDriver ();

  virtual bool Open (csEventQueue*);
  virtual void Close ();
};

/// DJGPP version.
class SysMouseDriver : public csMouseDriver
{
private:
  bool MouseOpened;
  bool MouseExists;
  float SensivityFactor;
  int mouse_sensivity_x;
  int mouse_sensivity_y;
  int mouse_sensivity_threshold;

public:
  SysMouseDriver();
  virtual ~SysMouseDriver ();

  virtual bool Open (iSystem *System, csEventQueue*);
  virtual void Close ();
  bool SetMousePosition (int x, int y);
};

#endif // DJGPP_H
