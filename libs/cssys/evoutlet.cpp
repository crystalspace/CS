/*
    Event outlet object implementation
    Copyright (C) 2000 by Andrew Zabolotny

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
#include "cssys/system.h"
#include "cssys/csevent.h"
#include "cssys/csevcord.h"

SCF_IMPLEMENT_IBASE (csSystemDriver::csEventOutlet)
  SCF_IMPLEMENTS_INTERFACE (iEventOutlet)
SCF_IMPLEMENT_IBASE_END

csSystemDriver::csEventOutlet::csEventOutlet (iEventPlug *iPlug, csSystemDriver *iSys)
{
  SCF_CONSTRUCT_IBASE (NULL);
  Plug = iPlug;
  EnableMask = unsigned (-1);
  System = iSys;
}

csSystemDriver::csEventOutlet::~csEventOutlet ()
{
  int idx = System->EventOutlets.Find (this);
  if (idx >= 0)
  {
    System->EventOutlets [idx] = NULL;
    System->EventOutlets.Delete (idx);
  }
}

iEvent *csSystemDriver::csEventOutlet::CreateEvent ()
{
  return new csEvent ();
}

void csSystemDriver::csEventOutlet::PutEvent (iEvent *Event)
{
  if ((1 << Event->Type) & EnableMask)
  {
    // Check for a pertinent event cord
    csEventCord *cord = (csEventCord *)System->GetEventCord (Event->Category, Event->SubCategory);

    // Return if the event is not to be handled by the system queue
    if (cord && cord->PutEvent (Event))
      return;
    System->EventQueue.Put (Event);
  }
  else
    delete Event;
}

void csSystemDriver::csEventOutlet::Key (int iKey, int iChar, bool iDown)
{
  if ((iKey || iChar) && (EnableMask & CSEVTYPE_Keyboard))
    System->Keyboard.DoKey (iKey, iChar, iDown);
}

void csSystemDriver::csEventOutlet::Mouse (int iButton, bool iDown, int x, int y)
{
  if (EnableMask & CSEVTYPE_Mouse)
    if (iButton == 0)
      System->Mouse.DoMotion (x, y);
    else
      System->Mouse.DoButton (iButton, iDown, x, y);
}

void csSystemDriver::csEventOutlet::Joystick (int iNumber, int iButton,
  bool iDown, int x, int y)
{
  if (EnableMask & CSEVTYPE_Joystick)
    if (iButton == 0)
      System->Joystick.DoMotion (iNumber, x, y);
    else
      System->Joystick.DoButton (iNumber, iButton, iDown, x, y);
}

void csSystemDriver::csEventOutlet::Broadcast (int iCode, void *iInfo)
{
  System->EventQueue.Put (new csEvent (csGetClicks (), csevBroadcast,
    iCode, iInfo));
}

void csSystemDriver::csEventOutlet::ImmediateBroadcast (int iCode, void *iInfo)
{
  csEvent Event (csGetClicks (), csevBroadcast, iCode, iInfo);
  System->HandleEvent (Event);
}
