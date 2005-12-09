 /*
  Crystal Space - Event Queue Implementation
  Copyright (C) 1998 by Jorrit Tyberghein
  Copyright (C) 2001 by Eric Sunshine <sunshine@sunshineco.com>
  Partially written by Andrew Zabolotny <bit@freya.etu.ru>
  Copyright (C) 2005 by Adam D. Bradley <artdodge@cs.bu.edu>

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

  TODO: make this reentrant again
*/

#include "cssysdef.h"
#include <stddef.h>
#include "csutil/ref.h"
#include "csutil/csevcord.h"
#include "csutil/csevent.h"
#include "csutil/cseventq.h"
#include "csutil/evoutlet.h"
#include "csutil/sysfunc.h"
#include "iutil/eventh.h"
#include "csutil/eventnames.h"
#include "csutil/eventhandlers.h"


csEventQueue::csEventQueue (iObjectRegistry* r, size_t iLength) : 
  scfImplementationType (this),
  Registry(r), 
  NameRegistry(csEventNameRegistry::GetRegistry(r)),
  HandlerRegistry(csEventHandlerRegistry::GetRegistry(r)),
  EventQueue(0), evqHead(0), evqTail(0), Length(0),
  EventTree(0), EventPool(0)
{
  Mutex = csMutex::Create();
  Resize (iLength);
  // Create the default event outlet.
  EventOutlets.Push (new csEventOutlet (0, this, Registry));
  EventTree = csEventTree::CreateRootNode(HandlerRegistry, NameRegistry, this);

  Frame = csevFrame (NameRegistry);
  PreProcess = csevPreProcess (NameRegistry);
  ProcessEvent = csevProcess (NameRegistry);
  PostProcess = csevPostProcess (NameRegistry);
  FinalProcess = csevFinalProcess (NameRegistry);

  csRef<iTypedFrameEventDispatcher> PreProcessEventDispatcher;
  csRef<iTypedFrameEventDispatcher> ProcessEventDispatcher;
  csRef<iTypedFrameEventDispatcher> PostProcessEventDispatcher;
  csRef<iTypedFrameEventDispatcher> FinalProcessEventDispatcher;
  PreProcessEventDispatcher.AttachNew (
    new PreProcessFrameEventDispatcher (this));
  ProcessEventDispatcher.AttachNew (
    new ProcessFrameEventDispatcher (this));
  PostProcessEventDispatcher.AttachNew (
    new PostProcessFrameEventDispatcher (this));
  FinalProcessEventDispatcher.AttachNew (
    new FinalProcessFrameEventDispatcher (this));

  if (!Subscribe (PreProcessEventDispatcher, Frame) ||
      !Subscribe (ProcessEventDispatcher, Frame) ||
      !Subscribe (PostProcessEventDispatcher, Frame) ||
      !Subscribe (FinalProcessEventDispatcher, Frame)) 
  {
    CS_ASSERT(0);
  }
}

csEventQueue::~csEventQueue ()
{
  // We don't allow deleting the event queue from within an event handler.
  Clear();
  if (EventQueue)
    delete[] EventQueue;
  EventOutlets.Get(0)->DecRef(); // The default event outlet which we created.
  while (EventPool) 
  {
    csPoolEvent *e = EventPool->next;
    EventPool->Free();
    EventPool = e;
  }
  csEventTree::DeleteRootNode (EventTree); // Magic!
  EventTree = 0;
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

iEvent *csEventQueue::CreateRawEvent ()
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
  return e;
}

csPtr<iEvent> csEventQueue::CreateEvent ()
{
  iEvent *e = CreateRawEvent ();
  e->Name = 0;
  e->Broadcast = 0;
  e->Time = csGetTicks();
  return csPtr<iEvent>((iEvent*)e);
}

csPtr<iEvent> csEventQueue::CreateEvent (const csEventID &name, bool broadcast) 
{
  iEvent *e = CreateRawEvent ();
  e->Name = name;
  e->Broadcast = broadcast;
  e->Time = csGetTicks();
  return csPtr<iEvent>((iEvent*)e);
}

csPtr<iEvent> csEventQueue::CreateEvent (const csEventID &name)
{ 
  return CreateEvent (name, false);
}

csPtr<iEvent> csEventQueue::CreateEvent (const char *name) 
{ 
  return CreateEvent (NameRegistry->GetID(name), false); 
}

csPtr<iEvent> csEventQueue::CreateBroadcastEvent (const csEventID &name) 
{ 
  return CreateEvent (name, true); 
}

csPtr<iEvent> csEventQueue::CreateBroadcastEvent (const char *name)
{
  return CreateEvent (NameRegistry->GetID(name), true); 
}

void csEventQueue::Post (iEvent *Event)
{
#ifdef ADB_DEBUG
  std::cerr << "Queuing up event " 
	    << NameRegistry->GetString(Event->Name) 
	    << " (" << Event->Time << ")"
	    << std::endl;
#endif
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
#ifdef ADB_DEBUG
  if (ev != 0)
    std::cerr << "Returning head of queue " 
	      << NameRegistry->GetString(ev->Name)
	      << " (" << ev->Time << ")"
	      << std::endl;
  else
    std::cerr << "Returning head of queue (null)" << std::endl;
#endif
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

void csEventQueue::Notify (const csEventID &name)
{
#ifdef ADB_DEBUG
  std::cerr << "Doing notify (immediate broadcast) to " << NameRegistry->GetString(name) << std::endl;
#endif

  csEventTree *epoint = EventHash.Get(name, NULL);
  if (!epoint)
  {
    /* Expensive exception, build the tree out to include "name". */
    epoint = EventTree->FindNode(name, this);
  }
  CS_ASSERT(epoint);
  epoint->Notify();
}

void csEventQueue::Process ()
{
  csRef<iEvent> ev;
  for (ev = Get(); ev.IsValid(); ev = Get())
  {
    iEvent& e = *ev;
    Dispatch (e);
  }

#ifdef ADB_DEBUG
  std::cerr << "[Sending Frame event]" << std::endl;
#endif

  Notify (Frame);

#ifdef ADB_DEBUG
  std::cerr << "[Finished with Frame event]" << std::endl;
#endif
}

void csEventQueue::Dispatch (iEvent& e)
{
#ifdef ADB_DEBUG
  std::cerr << "Dispatching event " 
	    << NameRegistry->GetString(e.Name) 
	    << " into event tree"
	    << std::endl;
#endif
  csEventTree *epoint = EventHash.Get(e.Name, NULL);
  if (!epoint)
  {
    epoint = EventTree->FindNode(e.Name, this);
  }
  CS_ASSERT(epoint);

  epoint->Dispatch(e);
}

csHandlerID csEventQueue::RegisterListener (iEventHandler * listener)
{
#ifdef ADB_DEBUG
  std::cerr << "Registering listener (no events) " << listener->GenericName() << std::endl;
#endif
  return HandlerRegistry->GetID(listener);
}

bool csEventQueue::Subscribe (iEventHandler *listener, const csEventID &ename)
{
#ifdef ADB_DEBUG
  std::cerr << "Registering listener " << listener->GenericName()
	    << " to " << NameRegistry->GetString(ename) <<  std::endl;
#endif
  bool ret = EventTree->Subscribe (HandlerRegistry->GetID(listener), 
				   ename, this);
#ifdef ADB_DEBUG
  EventTree->Dump();
#endif
  return ret;
}

bool csEventQueue::Subscribe (iEventHandler *listener, const csEventID ename[])
{
#ifdef ADB_DEBUG
  std::cerr << "Registering listener " << listener->GenericName() << " to:";
#endif
  csHandlerID handler = HandlerRegistry->GetID(listener);
  for (int ecount=0 ; ename[ecount]!=CS_EVENTLIST_END ; ecount++)
  {
#ifdef ADB_DEBUG
    std::cerr << " " << NameRegistry->GetString(ename[ecount]);
#endif
    if (!EventTree->Subscribe(handler, ename[ecount], this))
    {
      for (int i=0 ; i<ecount ; i++)
      {
#ifdef ADB_DEBUG
	std::cerr << " (UNDOING: " 
		  << NameRegistry->GetString(ename[i])
		  << ")";
#endif
	EventTree->Unsubscribe(handler, ename[i], this);
      }
      return false;
    }
  }
#ifdef ADB_DEBUG
  std::cerr << std::endl;
  EventTree->Dump();
#endif
  return true;
}

void csEventQueue::Unsubscribe (iEventHandler *listener, const csEventID ename[])
{
#ifdef ADB_DEBUG
  std::cerr << "Unregistering listener " 
	    << listener->GenericName() 
	    << " from:";
#endif
  for (int iter=0 ; ename[iter] != CS_EVENTLIST_END ; iter++)
  {
#ifdef ADB_DEBUG
    std::cerr << " " << NameRegistry->GetString(ename[iter]);
#endif
    EventTree->Unsubscribe(HandlerRegistry->GetID(listener), 
			   ename[iter], this);
  }
#ifdef ADB_DEBUG
  std::cerr << std::endl;
#endif
}

void csEventQueue::Unsubscribe (iEventHandler *listener, const csEventID &ename)
{
#ifdef ADB_DEBUG
  std::cerr << "Unregistering listener " << listener->GenericName() 
	    << " from " << NameRegistry->GetString(ename) << std::endl;
#endif
  EventTree->Unsubscribe (HandlerRegistry->GetID(listener), ename, this);
}

void csEventQueue::RemoveListener (iEventHandler* listener)
{
#ifdef ADB_DEBUG
  std::cerr << "Unregistering listener " << listener->GenericName()
	    << " (all events)" << std::endl;
#endif
  EventTree->Unsubscribe(HandlerRegistry->GetID(listener), CS_EVENT_INVALID, this);
}

void csEventQueue::RemoveAllListeners ()
{
#ifdef ADB_DEBUG
  std::cerr << "Unregistering all listeners" << std::endl;
#endif
  CS_ASSERT(EventTree != 0);
  csEventTree::DeleteRootNode(EventTree); // Magic!
  EventTree = csEventTree::CreateRootNode(HandlerRegistry, NameRegistry, this);
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

iEventCord* csEventQueue::GetEventCord (const csEventID &name)
{
  csEventCord *cord = EventCords.Get (name, NULL);
  if (!cord)
  {
    cord = new csEventCord (name);
    EventCords.PutUnique(name, cord);
    cord->DecRef ();
  }
  return cord;
}

