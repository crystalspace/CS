/*
  Crystal Space Windowing System: Miscelaneous CSWS utilites
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

#ifndef __CSWSSYS_H__
#define __CSWSSYS_H__

#include "libs/cssys/common/sysdriv.h"
#include "libs/cssys/common/system.h"
#include "csapp.h"

// A special variation of SysSystemDriver for csApp
class appSystemDriver : public SysSystemDriver
{
  friend void cleanup ();

  int curline, maxlines, maxwidth;
  char **textline;
  int *linecolor;
  int textcolor;
  csApp *application;
  csWorld *world;

public:
  /// Initialize the application's system driver given csApp and csWorld
  appSystemDriver (csApp *ParentApp, csWorld *ParentWorld);
  /// Destroy the system driver object
  virtual ~appSystemDriver ();
  /// Initialize system driver and debug console
  virtual bool Initialize (int argc, char *argv[], IConfig *config);
  /// Close debug console
  void CloseConsole ();
  /// Call application to process queued events
  virtual void NextFrame (long elapsed_time, long current_time);
  /// Replace DemoWrite() for output to debug console
  virtual void DemoWrite (char* buf);
  /// Display an alert message
  virtual void Alert (char* msg);
  /// Display a warning message
  virtual void Warn (char* msg);
};

#endif // __CSWSSYS_H__
