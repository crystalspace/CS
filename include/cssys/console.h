/*
    This module outlines basical console functionality
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

#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "csutil/csbase.h"
#include "types.h" // for bool.

class csRect;

/**
 * The csConsole class is just an outline of how CrystalSpace console
 * should work. There is no implementation for any of its methods, so
 * you cannot create instances of this class. You should derive your own
 * class from this and assign a instance of it to System->Console variable.
 */
class csConsole : public csBase
{
public:
  /// Show the console
  virtual void Show () = 0;
  /// Hide the console
  virtual void Hide () = 0;

  /// Print a string to the console
  virtual void PutText (char *format, ...) = 0;
  /// Print (if console is active) and execute a command
  virtual void ExecuteCommand (char *command) = 0;
  /**
   * Update the console on the window.
   * If not NULL the given area will be updated to reflect
   * the area that was changed during this Print.
   */
  virtual void Print (csRect* area) = 0;

  /// Return true if console is active
  virtual bool IsActive () = 0;

  /// Clear console
  virtual void Clear () = 0;
};

#endif //  __CONSOLE_H__
