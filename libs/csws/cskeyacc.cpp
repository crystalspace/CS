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

#include "cssysdef.h"
#include <stdio.h>
#include "csws/cscomp.h"
#include "csws/cskeyacc.h"
#include "csws/csapp.h"
#include "csutil/event.h"

struct csAccElement
{
  utf32_char Key;
  unsigned int Shifts;
  csEvent Event;
};

csKeyboardAccelerator::csKeyboardAccelerator (csComponent *iParent)
  : csComponent (iParent)
{
}

csKeyboardAccelerator::~csKeyboardAccelerator ()
{
}

void csKeyboardAccelerator::Event (int iKey, int iShifts, csEvent &iEv)
{
  csAccElement *ae = new csAccElement;
  ae->Key = iKey;
  ae->Shifts = iShifts;
  ae->Event = iEv;
  Accelerators.Push (ae);
}

void csKeyboardAccelerator::Command (int iKey, int iShifts, int iCommand,
  intptr_t iInfo)
{
  csEvent ev (0, csevCommand, iCommand, iInfo);
  Event (iKey, iShifts, ev);
}

void csKeyboardAccelerator::Broadcast (int iKey, int iShifts, int iCommand,
  intptr_t iInfo)
{
  csEvent ev (0, csevBroadcast, iCommand, iInfo);
  Event (iKey, iShifts, ev);
}

bool csKeyboardAccelerator::PostHandleEvent (iEvent &Event)
{
  if (csComponent::PostHandleEvent (Event))
    return true;

  if ((Event.Type == csevKeyboard) &&
    (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown)
   && (app->FocusOwner == 0)
   && (app->KeyboardOwner == 0))
  {
    size_t i;
    for (i = Accelerators.Length (); i-- > 0;)
    {
      csAccElement *ae = (csAccElement *)Accelerators [i];
      if ((ae->Key == csKeyEventHelper::GetRawCode (&Event))
       && (ae->Shifts == (csKeyEventHelper::GetModifiersBits (&Event) & 
       CSMASK_ALLSHIFTS)))
      {
        app->Post (new csEvent (ae->Event));
        return true;
      } /* endif */
    } /* endfor */
  } /* endif */
  return false;
}
