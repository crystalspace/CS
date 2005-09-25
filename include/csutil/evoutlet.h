/*
    Crystal Space event outlet class
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

#ifndef __CS_UTIL_EVOUTLET_H__
#define __CS_UTIL_EVOUTLET_H__

/**\file
 * Implementation for iEventOutlet
 */

#include "csextern.h"
#include "csutil/csunicode.h"
#include "iutil/event.h"
#include "iutil/csinput.h"

struct iObjectRegistry;
class csEventQueue;

/**
 * A class which implements the iEventOutlet interface.
 */
class CS_CRYSTALSPACE_EXPORT csEventOutlet : public iEventOutlet
{
private:
  /// The mask of events to allow from this plug
  unsigned EnableMask;
  /// The event plug object
  iEventPlug *Plug;
  /// The owning event queue
  csEventQueue *Queue;
  /// The shared-object registry.
  iObjectRegistry* Registry;
  /// The shared keyboard driver.
  csRef<iKeyboardDriver> KeyboardDriver;
  /// The shared mouse driver.
  csRef<iMouseDriver> MouseDriver;
  /// The shared joystick driver.
  csRef<iJoystickDriver> JoystickDriver;

  iKeyboardDriver* GetKeyboardDriver();
  iMouseDriver*    GetMouseDriver();
  iJoystickDriver* GetJoystickDriver();

public:
  SCF_DECLARE_IBASE;

  /// Initialize the outlet
  csEventOutlet (iEventPlug*, csEventQueue*, iObjectRegistry*);
  /// Destroy the outlet
  virtual ~csEventOutlet ();

  /// Create a event object on behalf of the system driver.
  virtual csPtr<iEvent> CreateEvent ();
  /// Put a previously created event into system event queue.
  virtual void Post (iEvent*);
  /// Put a keyboard event into event queue.
  virtual void Key (utf32_char codeRaw, utf32_char codeCooked, bool iDown);
  /// Put a mouse event into event queue (old interface)
  virtual void Mouse (uint iButton, bool iDown, int x, int y);
  /// Put a mouse event into event queue (new interface)
  virtual void Mouse (uint iNumber, uint iButton, bool iDown, 
		      const int32 *axes, uint numAxes);
  /// Put a joystick event into event queue.
  virtual void Joystick (uint iNumber, uint iButton, bool iDown, 
    const int32 *axes, uint numAxes);
  /// Put a broadcast event into event queue.
  virtual void Broadcast (uint iCode, intptr_t iInfo);
  /// Broadcast a event to all plugins
  virtual void ImmediateBroadcast (uint iCode, intptr_t iInfo);
};

#endif // __CS_UTIL_EVOUTLET_H__
