/*
    Crystal Space Windowing System: keyboard accelerator class
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_CSKEYACC_H__
#define __CS_CSKEYACC_H__

/**\file
 * Crystal Space Windowing System: keyboard accelerator class
 */

/**
 * \addtogroup csws_comps_keyacc
 * @{ */
 
#include "csextern.h"
 
#include "csutil/parray.h"
#include "cscomp.h"

class csEvent;
struct csAccElement;

/**
 * A keyboard accelerator is a invisible component which monitors
 * all events and if it sees a keyboard event which matches one of
 * the predefined combinations it emmits a corresponding event.
 */
class CS_CSWS_EXPORT csKeyboardAccelerator : public csComponent
{
  /// The table that contains keyboard event->generated event conversion table
  csPDelArray<csAccElement> Accelerators;

public:
  /// Create keyboard accelerator object
  csKeyboardAccelerator (csComponent *iParent);
  /// Destroy keyboard accelerator object
  virtual ~csKeyboardAccelerator ();

  /// Insert a key->event conversion table element
  void Event (int iKey, int iShifts, csEvent &iEv);
  /// Insert a key->command event conversion table element
  void Command (int iKey, int iShifts, int iCommand, intptr_t iInfo = 0);
  /// Insert a key->broadcast event conversion table element
  void Broadcast (int iKey, int iShifts, int iCommand, intptr_t iInfo = 0);

  /// The "core" function
  virtual bool PostHandleEvent (iEvent &Event);
};

/** @} */

#endif // __CS_CSKEYACC_H__
