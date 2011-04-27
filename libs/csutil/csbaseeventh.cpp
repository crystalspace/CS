/*
    Copyright (C) 2003 by Odes B. Boatwright.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include "cssysdef.h"
#include "csutil/csbaseeventh.h"
#include "csutil/event.h"
#include "csutil/csevent.h"
#include "csutil/objreg.h"

#include "iutil/eventq.h"

csBaseEventHandler::EventHandlerImpl::EventHandlerImpl (
  csBaseEventHandler* parent) : scfImplementationType (this),
  parent (parent)
{
}

csBaseEventHandler::csBaseEventHandler() : 
  object_registry (0),
  self (CS_EVENT_INVALID)
{
  FrameEvent = CS_EVENT_INVALID;
  eventh.AttachNew (new EventHandlerImpl (this));
}

void csBaseEventHandler::Initialize (iObjectRegistry *r)
{
  object_registry = r;
  self = csEventHandlerRegistry::RegisterID (r, eventh);
  FrameEvent = csevFrame (r);
}

csBaseEventHandler::~csBaseEventHandler()
{
  if (object_registry)
    csEventHandlerRegistry::ReleaseID (object_registry, eventh);
  if (queue)
    queue->RemoveListener (eventh);
  eventh->parent = 0;
}

bool csBaseEventHandler::RegisterQueue (iEventQueue* q, csEventID event)
{
  if (queue)
    queue->RemoveListener (eventh);
  queue = q;
  if (0 != q)
    q->RegisterListener (eventh, event);
  return true;
}

bool csBaseEventHandler::RegisterQueue (iEventQueue* q, csEventID events[])
{
  if (queue)
    queue->RemoveListener (eventh);
  queue = q;
  if (q != 0)
    q->RegisterListener (eventh, events);
  return true;
}

bool csBaseEventHandler::RegisterQueue (iObjectRegistry* registry,
	csEventID event)
{
  csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (registry));
  if (0 == q)
    return false;
  return RegisterQueue (q, event);
}

bool csBaseEventHandler::RegisterQueue (iObjectRegistry* registry,
	csEventID events[])
{
  csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (registry));
  if (0 == q)
    return false;
  return RegisterQueue (q, events);
}

void csBaseEventHandler::UnregisterQueue ()
{
  if (queue)
    queue->RemoveListener (eventh);
  queue = 0;
}

bool csBaseEventHandler::HandleEvent (iEvent &event)
{
  CS_ASSERT_MSG("You need to call Initialize() with a valid object registry "
    "before you can use csBaseEventHandler", object_registry != 0);
  if (event.Name == FrameEvent)
  {
    Frame();
    return true;
  }
  else if (CS_IS_KEYBOARD_EVENT(object_registry, event))
    return OnKeyboard(event);
  else if (CS_IS_MOUSE_EVENT(object_registry, event))
  {
    switch(csMouseEventHelper::GetEventType(&event))
    {
    case csMouseEventTypeMove:
      return OnMouseMove(event);
    case csMouseEventTypeUp:
      return OnMouseUp(event);
    case csMouseEventTypeDown:
      return OnMouseDown(event);
    case csMouseEventTypeClick:
      return OnMouseClick(event);
    case csMouseEventTypeDoubleClick:
      return OnMouseDoubleClick(event);
    }
  }
  else if (CS_IS_JOYSTICK_EVENT(object_registry, event))
  {
    if (csJoystickEventHelper::GetButton(&event))
    {
      if (csJoystickEventHelper::GetButtonState(&event))
	return OnJoystickDown(event);
      else
	return OnJoystickUp(event);
    }
    else
    {
      return OnJoystickMove(event);
    }
  }
  return  OnUnhandledEvent(event);
}

#define DefaultTrigger(trigger)			   \
  bool csBaseEventHandler::trigger (iEvent &)      \
  { return false; }

DefaultTrigger ( OnUnhandledEvent )
DefaultTrigger ( OnKeyboard )
DefaultTrigger ( OnMouseMove )
DefaultTrigger ( OnMouseDown )
DefaultTrigger ( OnMouseUp )
DefaultTrigger ( OnMouseClick )
DefaultTrigger ( OnMouseDoubleClick )
DefaultTrigger ( OnJoystickMove )
DefaultTrigger ( OnJoystickDown )
DefaultTrigger ( OnJoystickUp )

#include "csutil/deprecated_warn_off.h"

void csBaseEventHandler::Frame ()
{
  // Support for 'backwards' event handling methods
  PreProcessFrame ();
  ProcessFrame ();
  PostProcessFrame ();
  /* This is not entirely correct ... but a practical approximation that lets
     things work */
  FinishFrame ();
}

#include "csutil/deprecated_warn_on.h"
