/*
    Copyright (C) 2005 Dan Härdfeldt and Seth Yastrov

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _CS_CEGUI_EVENTHANDLER_H_
#define _CS_CEGUI_EVENTHANDLER_H_

/**\file 
*/
/**
* \addtogroup CEGUI
* @{ */

#include "csutil/csbaseeventh.h"

struct iObjectRegistry;
class csMouseDriver;

class csCEGUIRenderer;

/// Handles resize events and injects mouse and keyboard into CEGUI.
class csCEGUIEventHandler : public csBaseEventHandler
{
public:
  /// Constructor.
  csCEGUIEventHandler (iObjectRegistry*, csCEGUIRenderer*);

  /// Destructor.
  ~csCEGUIEventHandler ();

  /// Initialize the event handler. This sets up the event queue.
  bool Initialize ();

  /// Handle unhandled events (like broadcast messages).
  bool OnUnhandledEvent (iEvent &event);

  /// Handle mouse down events.
  bool OnMouseDown (iEvent &event);

  /// Handle mouse move events.
  bool OnMouseMove (iEvent &event);

  /// Handle mouse up events.
  bool OnMouseUp (iEvent &event);

  /// Handle keyboard events.
  bool OnKeyboard (iEvent &event);

private:
  iObjectRegistry *obj_reg;
  csCEGUIRenderer* renderer;
  csMouseDriver *md;
};
#endif // _CS_CEGUI_EVENTHANDLER_H_
