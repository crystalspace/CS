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

#ifndef PHYZTEST_H
#define PHYZTEST_H

#include <stdarg.h>
#include "cssys/sysdriv.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"

class csSector;
class csView;
class csWorld;
class csDynLight;

class Phyztest : public SysSystemDriver
{
  typedef SysSystemDriver superclass;
public:
  csSector* room;
  csView* view;
  csWorld* world;
  csDynLight* dynlight;
  float angle;
  int motion_flags;

public:
  Phyztest ();
  virtual ~Phyztest ();

  virtual bool Initialize (int argc, char *argv[], const char *iConfigName);
  virtual void NextFrame (time_t elapsed_time, time_t current_time);
  virtual bool HandleEvent (csEvent &Event);
};

#endif // PHYZTEST_H

