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
#include "csutil/objreg.h"

#include "iutil/eventq.h"

csBaseEventHandler::csBaseEventHandler()
  : scfImplementationType (this)
{
}


csBaseEventHandler::~csBaseEventHandler()
{
  if (queue)
    queue->RemoveListener (this);
}

bool csBaseEventHandler::RegisterQueue (iEventQueue* q, unsigned int trigger)
{
  if (queue)
    queue->RemoveListener (this);
  queue = q;
  if (0 != q)
    q->RegisterListener(this, trigger);
  return true;
}

bool csBaseEventHandler::RegisterQueue (
  iObjectRegistry* registry, unsigned int trigger)
{
  csRef<iEventQueue> q (CS_QUERY_REGISTRY (registry, iEventQueue));
  if (0 == q)
    return false;
  return RegisterQueue (q, trigger);
}

// This is just ugly, but it is the static definition of the pmfnTriggers array
bool (csBaseEventHandler::*csBaseEventHandler::pmfnTriggers[])(iEvent &event) =
  {
    &csBaseEventHandler::OnUnhandledEvent   /*csevNothing*/,
    &csBaseEventHandler::OnKeyboard         /*csevKeyboard*/,
    &csBaseEventHandler::OnMouseMove        /*csevMouseMove*/,
    &csBaseEventHandler::OnMouseDown        /*csevMouseDown*/,
    &csBaseEventHandler::OnMouseUp          /*csevMouseUp*/,
    &csBaseEventHandler::OnMouseClick       /*csevMouseClick*/,
    &csBaseEventHandler::OnMouseDoubleClick /*csevMouseDoubleClick*/,
    &csBaseEventHandler::OnJoystickMove     /*csevJoystickMove*/,
    &csBaseEventHandler::OnJoystickDown     /*csevJoystickDown*/,
    &csBaseEventHandler::OnJoystickUp       /*csevJoystickUp*/,
    &csBaseEventHandler::OnCommand          /*csevCommand*/,
    &csBaseEventHandler::OnBroadcast        /*csevBroadcast*/,
  };

bool csBaseEventHandler::HandleEvent (iEvent &event)
{
  uint8 index = event.Type;
  if (_CSBASEEVENT_MAXARRAYINDEX < index)
    index = 0;
  return (this->*(pmfnTriggers[index]))(event);
}

#define DefaultTrigger(trigger) \
  bool csBaseEventHandler::trigger (iEvent & /*event*/) \
  { \
    return false;\
  }

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
DefaultTrigger ( OnCommand )

bool csBaseEventHandler::OnBroadcast (iEvent &event)
{
  switch (csCommandEventHelper::GetCode(&event))
  {
  case cscmdPreProcess:
    PreProcessFrame ();
    break;

  case cscmdProcess:
    ProcessFrame ();
    break;

  case cscmdPostProcess:
    PostProcessFrame ();
    break;

  case cscmdFinalProcess:
    FinishFrame ();
    break;

  default:
    return OnUnhandledEvent (event);
  }
  return true;
}

void csBaseEventHandler::PreProcessFrame ()
{
}

void csBaseEventHandler::ProcessFrame ()
{
}

void csBaseEventHandler::PostProcessFrame ()
{
}

void csBaseEventHandler::FinishFrame ()
{
}
