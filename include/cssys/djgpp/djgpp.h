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

#include "cssys/csinput.h"
#include "cssys/system.h"

/// DJGPP version.
class SysSystemDriver : public csSystemDriver, public iEventPlug
{
public:
  SysSystemDriver ();
  virtual ~SysSystemDriver ();

  virtual void NextFrame ();

  /// Open the system
  virtual bool Open (const char *Title);
  /// Close the system
  virtual void Close ();

  /// Execute a system-dependent extension
  virtual bool SystemExtension (const char *iCommand, ...);

  SCF_DECLARE_IBASE_EXT (csSystemDriver);

  //------------------------- iEventPlug interface ---------------------------//

  virtual unsigned GetPotentiallyConflictingEvents ()
  { return CSEVTYPE_Keyboard | CSEVTYPE_Mouse; }
  virtual unsigned QueryEventPriority (unsigned /*iType*/)
  { return 100; }

private:
  bool KeyboardOpened;
  bool MouseOpened;
  bool MouseExists;
  float SensivityFactor;
  int mouse_sensivity_x;
  int mouse_sensivity_y;
  int mouse_sensivity_threshold;
  iEventOutlet *EventOutlet;
};

#endif // DJGPP_H
