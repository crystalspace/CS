/*
  Crystal Space Windowing System: Event manager
  Copyright (C) 1998 by Jorrit Tyberghein
  Copyright (C) 2001 by Eric Sunshine <sunshine@sunshineco.com>
  Partially written by Andrew Zabolotny <bit@freya.etu.ru>

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

#include <stddef.h>
#include "cssysdef.h"
#include "csutil/cseventq.h"
#include "iutil/eventh.h"
#include "cssys/sysfunc.h"

SCF_IMPLEMENT_IBASE (csEventQueue)
  SCF_IMPLEMENTS_INTERFACE (iEventQueue)
SCF_IMPLEMENT_IBASE_END

csEventQueue::csEventQueue (iObjectRegistry* r, size_t iLength) :
  Registry(r), EventQueue(0), evqHead(0), evqTail(0), Length(0), SpinLock(0),
  busy_looping (0), delete_occured (false)
{
  SCF_CONSTRUCT_IBASE (0);
  Resize (iLength);
  // Create the default event outlet.
  EventOutlets.Push (new csEventOutlet (0, this, Registry));
}

csEventQueue::~csEventQueue ()
{
  // We don't allow deleting the event queue from within an event handler.
  CS_ASSERT (busy_looping <= 0);
  Clear();
  if (EventQueue)
    delete[] EventQueue;
  for (int i = Listeners.Length() - 1; i >= 0; i--)
    Listeners[i].object->DecRef();
  EventOutlets.Get(0)->DecRef(); // The default event outlet which we created.
}

void csEventQueue::Post (iEvent *Event)
{
again:
  Lock ();
  size_t newHead = evqHead + 1;
  if (newHead == Length)
    newHead = 0;

  if (newHead == evqTail) // Queue full?
  {
    Unlock ();
    Resize (Length * 2); // Normally queue should not be more than half full.
    goto again;
  } /* endif */

  EventQueue [evqHead] = Event;
  evqHead = newHead;
  Unlock ();
}

iEvent *csEventQueue::Get ()
{
  iEvent* ev = 0;
  if (!IsEmpty ())
  {
    Lock ();
    size_t oldTail = evqTail++;
    if (evqTail == Length)
      evqTail = 0;
    ev = (iEvent*)EventQueue [oldTail];
    Unlock ();
  } /* endif */
  return ev;
}

void csEventQueue::Clear ()
{
  iEvent* ev;
  while ((ev = Get()) != NULL)
    ev->DecRef();
}

void csEventQueue::Resize (size_t iLength)
{
  if (iLength <= 0)
    iLength = DEF_EVENT_QUEUE_LENGTH;

  Lock ();
  if (iLength == Length)
  {
    Unlock ();
    return;
  }

  // Remember old event queue and allocate a new one
  volatile iEvent **oldEventQueue = EventQueue;
  EventQueue = (volatile iEvent**) new iEvent *[iLength];
  // Remember old values of head & tail and set both to 0
  size_t oldHead = evqHead, oldTail = evqTail;
  evqHead = evqTail = 0;
  // Remember old queue length and set new one
  size_t oldLength = Length;
  Length = iLength;

  // Copy old events into the new queue until its full
  if (oldEventQueue)
  {
    while ((oldTail != oldHead) && (evqHead < Length - 1))
    {
      EventQueue [evqHead++] = oldEventQueue [oldTail++];
      if (oldTail == oldLength)
        oldTail = 0;
    } /* endwhile */
  } /* endif */

  delete[] oldEventQueue;
  Unlock ();
}

void csEventQueue::StartLoop ()
{
  busy_looping++;
}

void csEventQueue::EndLoop ()
{
  busy_looping--;
  if (busy_looping <= 0 && delete_occured)
  {
    // We are no longer in a loop and a delete occured so here
    // we really clean up the entries in the listener array.
    delete_occured = false;
    for (int i = Listeners.Length() - 1; i >= 0; i--)
    {
      Listener const& listener = Listeners[i];
      if (!listener.object)
        Listeners.Delete (i);
    }
  }
}

void csEventQueue::Notify (iEvent& e)
{
  int i;
  StartLoop ();
  for (i = Listeners.Length() - 1; i >= 0; i--)
  {
    Listener const& listener = Listeners[i];
    if (listener.object && (listener.trigger & CSMASK_Nothing) != 0)
      listener.object->HandleEvent (e);
  }
  EndLoop ();
}

void csEventQueue::Process ()
{
  csEvent notification (csGetTicks(), csevBroadcast, cscmdPreProcess);
  Notify (notification);

  iEvent* ev;
  while ((ev = Get ()) != 0)
  {
    Dispatch (*ev);
    ev->DecRef ();
  }

  notification.Command.Code = cscmdProcess;
  Notify (notification);

  while ((ev = Get ()) != 0)
  {
    Dispatch (*ev);
    ev->DecRef ();
  }

  notification.Command.Code = cscmdPostProcess;
  Notify (notification);

  while ((ev = Get ()) != 0)
  {
    Dispatch (*ev);
    ev->DecRef ();
  }

  notification.Command.Code = cscmdFinalProcess;
  Notify (notification);
}

void csEventQueue::Dispatch (iEvent& e)
{
  int const evmask = 1 << e.Type;
  bool const canstop = ((e.Flags & CSEF_BROADCAST) == 0);
  StartLoop ();
  int i, n;
  for (i = 0, n = Listeners.Length(); i < n; i++)
  {
    Listener const& l = Listeners[i];
    if ((l.trigger & evmask) != 0 && l.object && l.object->HandleEvent(e)
    	&& canstop)
      break;
  }
  EndLoop ();
}

int csEventQueue::FindListener (iEventHandler* listener) const
{
  int i;
  for (i = Listeners.Length() - 1; i >= 0; i--)
  {
    Listener const& l = Listeners[i];
    if (l.object == listener)
      return i;
  }
  return -1;
}

void csEventQueue::RegisterListener (iEventHandler* listener,
	unsigned int trigger)
{
  int const n = FindListener (listener);
  if (n >= 0)
    Listeners[n].trigger = trigger;
  else
  {
    Listener l = { listener, trigger };
    Listeners.Push (l);
    listener->IncRef ();
  }
}

void csEventQueue::RemoveListener (iEventHandler* listener)
{
  int const n = FindListener(listener);
  if (n >= 0)
  {
    Listeners[n].object->DecRef();
    // Only delete the entry in the vector if we're not in a loop.
    if (busy_looping <= 0)
      Listeners.Delete(n);
    else
    {
      Listeners[n].object = NULL;
      delete_occured = true;
    }
  }
}

void csEventQueue::ChangeListenerTrigger (iEventHandler* l,
	unsigned int trigger)
{
  int const n = FindListener(l);
  if (n >= 0)
    Listeners[n].trigger = trigger;
}

iEventOutlet* csEventQueue::CreateEventOutlet (iEventPlug* plug)
{
  csEventOutlet* outlet = 0;
  if (plug != 0)
  {
    outlet = new csEventOutlet(plug, this, Registry);
    EventOutlets.Push (outlet);
  }
  return outlet;
}

iEventOutlet* csEventQueue::GetEventOutlet()
{
  return EventOutlets.Get(0);
}

iEventCord* csEventQueue::GetEventCord (int cat, int subcat)
{
  csEventCord* cord;
  int const n = EventCords.Find (cat, subcat);
  if (n >= 0) 
    cord = EventCords.Get(n);
  else
  {
    cord = new csEventCord (cat, subcat);
    EventCords.Push (cord);
  }
  return cord;
}

int csEventQueue::EventCordsVector::Find (int cat, int subcat)
{
  int i;
  for (i = Length() - 1; i >= 0; i--)
  {
    csEventCord *cord = Get(i);
    if (cat == cord->GetCategory() && subcat == cord->GetSubcategory())
      return i;
  }
  return -1;
}
