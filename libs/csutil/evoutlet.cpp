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
#include "csutil/evoutlet.h"
#include "csutil/csevent.h"
#include "csutil/cseventq.h"
#include "csutil/csevcord.h"
#include "csutil/sysfunc.h"
#include "iutil/csinput.h"
#include "iutil/objreg.h"

SCF_IMPLEMENT_IBASE (csEventOutlet)
  SCF_IMPLEMENTS_INTERFACE (iEventOutlet)
SCF_IMPLEMENT_IBASE_END

csEventOutlet::csEventOutlet(iEventPlug* p,csEventQueue* q,iObjectRegistry* r):
  EnableMask((unsigned int)(-1)), Plug(p), Queue(q), Registry(r)
{
  SCF_CONSTRUCT_IBASE (0);
}

csEventOutlet::~csEventOutlet ()
{
  size_t idx = Queue->EventOutlets.Find (this);
  if (idx != (size_t)-1)
  {
    Queue->EventOutlets [idx] = 0;
    Queue->EventOutlets.DeleteIndex (idx);
  }
  SCF_DESTRUCT_IBASE ();
}

#define DRIVER_GETTER(X) \
i##X##Driver* csEventOutlet::Get##X##Driver() \
{ \
  if (X##Driver == 0) \
  { \
    X##Driver = CS_QUERY_REGISTRY(Registry, i##X##Driver); \
  } \
  return X##Driver; \
}
DRIVER_GETTER(Keyboard)
DRIVER_GETTER(Mouse)
DRIVER_GETTER(Joystick)
#undef DRIVER_GETTER

csPtr<iEvent> csEventOutlet::CreateEvent ()
{
  return Queue->CreateEvent(0);
}

void csEventOutlet::Post (iEvent *Event)
{
  if ((1 << Event->Type) & EnableMask)
  {
    // Check for a pertinent event cord
    csEventCord *cord = (csEventCord*)
      Queue->GetEventCord (Event->Category, Event->SubCategory);
    // If the cord does not handle the event, then place it in the queue.
    if (!cord || !cord->Post (Event))
      Queue->Post (Event);
  }
}

void csEventOutlet::Key (utf32_char codeRaw, utf32_char codeCooked, bool iDown)
{
  if ((codeRaw || codeCooked) && (EnableMask & CSEVTYPE_Keyboard))
  {
    iKeyboardDriver* k = GetKeyboardDriver();
    if (k != 0)
      k->DoKey (codeRaw, codeCooked, iDown, false);
  }
}

void csEventOutlet::Mouse (int iButton, bool iDown, int x, int y)
{
  if (EnableMask & CSEVTYPE_Mouse)
  {
    iMouseDriver* m = GetMouseDriver();
    if (m != 0)
    {
      if (iButton == 0)
        m->DoMotion (x, y);
      else
        m->DoButton (iButton, iDown, x, y);
    }
  }
}

void csEventOutlet::Joystick (int iNumber, int iButton,
  bool iDown, int x, int y)
{
  if (EnableMask & CSEVTYPE_Joystick)
  {
    iJoystickDriver* j = GetJoystickDriver();
    if (j != 0)
    {
      if (iButton == 0)
        j->DoMotion (iNumber, x, y);
      else
        j->DoButton (iNumber, iButton, iDown, x, y);
    }
  }
}

void csEventOutlet::Broadcast (int iCode, intptr_t iInfo)
{
  Queue->Post (csRef<iEvent> (csPtr<iEvent>
    (new csEvent (csGetTicks (), csevBroadcast, iCode, iInfo))));
}

void csEventOutlet::ImmediateBroadcast (int iCode, intptr_t iInfo)
{
  csEvent Event (csGetTicks (), csevBroadcast, iCode, iInfo);
  Queue->Dispatch (Event);
}
