/*
    Crystal Space Windowing System: keyboard accelerator class
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

#ifndef __CSKEYACC_H__
#define __CSKEYACC_H__

#include "csutil/csvector.h"
#include "cscomp.h"

///
class csKeyboardAccelerator : public csComponent
{
  class csAccVector : public csVector
  {
  public:
    /// Initialize object
    csAccVector ();
    /// Destroy the object
    virtual ~csAccVector ();
    /// Virtual function which frees a vector element
    virtual bool FreeItem (csSome Item);
  };

  /// The table that contains keyboard event->generated event conversion table
  csAccVector Accelerators;

public:
  /// Create keyboard accelerator object
  csKeyboardAccelerator (csComponent *iParent);
  /// Destroy keyboard accelerator object
  virtual ~csKeyboardAccelerator ();

  /// Insert a key->event conversion table element
  void Event (int iKey, int iShifts, csEvent &iEvent);
  /// Insert a key->command event conversion table element
  void Command (int iKey, int iShifts, int iCommand, void *iInfo = NULL);
  /// Insert a key->broadcast event conversion table element
  void Broadcast (int iKey, int iShifts, int iCommand, void *iInfo = NULL);

  /// The "core" function
  virtual bool PostHandleEvent (csEvent &Event);
};

#endif // __CSKEYACC_H__
