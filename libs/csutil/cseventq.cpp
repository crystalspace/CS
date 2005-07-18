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

#include "cssysdef.h"
#include <stddef.h>
#include "csutil/csevent.h"
#include "csutil/cseventq.h"
#include "iutil/eventh.h"
#include "csutil/sysfunc.h"

SCF_IMPLEMENT_IBASE (csEventQueue)
  SCF_IMPLEMENTS_INTERFACE (iEventQueue)
SCF_IMPLEMENT_IBASE_END

csEventQueue::csEventQueue (iObjectRegistry* r, size_t iLength) :
  Registry(r), EventQueue(0), evqHead(0), evqTail(0), Length(0),
  busy_looping (0), delete_occured (false), EventPool(0)
{
  SCF_CONSTRUCT_IBASE (0);
  Mutex = csMutex::Create();
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
  RemoveAllListeners ();
  EventOutlets.Get(0)->DecRef(); // The default event outlet which we created.
  while (EventPool) 
  {
    csPoolEvent *e = EventPool->next;
    EventPool->Free();
    EventPool = e;
  }
  SCF_DESTRUCT_IBASE ();
}

uint32 csEventQueue::CountPool()
{
  if (!EventPool) return 0;
  csPoolEvent *e = EventPool;
  uint32 count = 0;
  while (e)
  {
    count++;
    e = e->next;
  }
  return count;
}

csPtr<iEvent> csEventQueue::CreateEvent(uint8 type) 
{
  csPoolEvent *e;
  if (EventPool) 
  {
    e = EventPool;
    EventPool = e->next;
  }
  else 
  {
    e = new csPoolEvent(this);
  }
  e->Type = type;
  e->Time = csGetTicks();
  e->Category = 0;
  e->SubCategory = 0;

  //@@@ Set here the right event's flag. This seems to me an ugly hack
  //more than a bug fixed. I think that something should be redesigned.
  //For example, we could remove the 'Flags' field
  //from iEvent since it seems pretty unused (ie the only possible
  //flag used is CSEF_BROADCAST). When we would like to detect if the event
  //is a "broadcasted" event, instead of detecting the presence
  //of that flag, we could simply test for the iEvent::Type being equal to
  //csevBroadcast.     Luca
  if (type == csevBroadcast)
    e->Flags = CSEF_BROADCAST;
  else
    e->Flags = 0;

  return csPtr<iEvent>((iEvent*)e);
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
  }

  EventQueue [evqHead] = Event;
  Event->IncRef ();
  evqHead = newHead;
  Unlock ();
}

csPtr<iEvent> csEventQueue::Get ()
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
  }
  return ev;
}

void csEventQueue::Clear ()
{
  csRef<iEvent> ev;
  for (ev = Get(); ev.IsValid(); ev = Get()) /* empty */;
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
    }
  }

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
    for (size_t i = Listeners.Length(); i > 0; i--)
    {
      Listener const& listener = Listeners[i - 1];
      if (!listener.object)
        Listeners.DeleteIndex (i - 1);
    }
  }
}

void csEventQueue::Notify (unsigned int pseudo_event)
{
  csRef<iEvent> e(CreateEvent(csevBroadcast));
  e->Add("cmdCode", (uint32)pseudo_event);
  e->Add("cmdInfo", (uint32)0);

  StartLoop ();
  for (size_t i = Listeners.Length(); i > 0; i--)
  {
    Listener const& listener = Listeners[i - 1];
    if (listener.object && (listener.trigger & CSMASK_Nothing) != 0)
      listener.object->HandleEvent (*e);
  }
  EndLoop ();
}

void csEventQueue::Process ()
{
  Notify (cscmdPreProcess);

  csRef<iEvent> ev;
  for (ev = Get(); ev.IsValid(); ev = Get())
  {
    iEvent& e = *ev;
    Dispatch (e);
  }

  Notify (cscmdProcess);
  Notify (cscmdPostProcess);
  Notify (cscmdFinalProcess);
}

void csEventQueue::Dispatch (iEvent& e)
{
  int const evmask = 1 << e.Type;
  bool const canstop = ((e.Flags & CSEF_BROADCAST) == 0);
  StartLoop ();
  size_t i, n;
  for (i = 0, n = Listeners.Length(); i < n; i++)
  {
    Listener const& l = Listeners[i];
    if ((l.trigger & evmask) != 0 && l.object && l.object->HandleEvent(e)
      && canstop)
      break;
  }
  EndLoop ();
}

size_t csEventQueue::FindListener (iEventHandler* listener) const
{
  size_t i;
  for (i = Listeners.Length(); i > 0; i--)
  {
    const size_t idx = i - 1;
    Listener const& l = Listeners[idx];
    if (l.object == listener)
      return idx;
  }
  return (size_t)-1;
}

void csEventQueue::RegisterListener (iEventHandler* listener,
  unsigned int trigger)
{
  size_t const n = FindListener (listener);
  if (n != (size_t)-1)
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
  size_t const n = FindListener(listener);
  if (n != (size_t)-1)
  {
    iBase* listener = Listeners[n].object;
    // Only delete the entry in the vector if we're not in a loop.
    if (busy_looping <= 0)
      Listeners.DeleteIndex (n);
    else
    {
      Listeners[n].object = 0;
      delete_occured = true;
    }
    listener->DecRef();
  }
}

void csEventQueue::RemoveAllListeners ()
{
  for (size_t i = Listeners.Length(); i > 0; i--)
  {
    const size_t idx = i - 1;
    iBase* listener = Listeners[idx].object;
    if (busy_looping <= 0)
    {
      Listeners.DeleteIndex (idx);
    }
    else
    {
      Listeners[idx].object = 0;
      delete_occured = true;
    }
    listener->DecRef();
  }
}


void csEventQueue::ChangeListenerTrigger (iEventHandler* l,
  unsigned int trigger)
{
  size_t const n = FindListener(l);
  if (n != (size_t)-1)
    Listeners[n].trigger = trigger;
}

csPtr<iEventOutlet> csEventQueue::CreateEventOutlet (iEventPlug* plug)
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
  size_t const n = EventCordsFind (cat, subcat);
  if (n != (size_t)-1)
    cord = EventCords.Get(n);
  else
  {
    cord = new csEventCord (cat, subcat);
    EventCords.Push (cord);
    cord->DecRef ();
  }
  return cord;
}

size_t csEventQueue::EventCordsFind (int cat, int subcat)
{
  size_t i = EventCords.Length();
 
  while (i > 0)
  {
    i--;
    csEventCord *cord = EventCords[i];
    if (cat == cord->GetCategory() && subcat == cord->GetSubcategory())
      return i;
  }
  return (size_t)-1;
}
