/*
  Crystal Space Windowing System: Event manager
  Copyright (C) 1998 by Jorrit Tyberghein
  Written by Andrew Zabolotny <bit@freya.etu.ru>
  Minor fixes added by Olivier Langlois <olanglois@sympatico.ca>

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
#include "cssys/cseventq.h"

csEventQueue::csEventQueue (size_t iLength)
  : EventQueue (NULL), Length (0), SpinLock (0)
{
  evqHead = evqTail = 0;
  Resize (iLength);
}

csEventQueue::~csEventQueue ()
{
  Clear ();
  if (EventQueue)
    delete[] EventQueue;
}

void csEventQueue::Put (iEvent *Event)
{
again:
  Lock ();
  size_t newHead = evqHead + 1;
  if (newHead == Length)
    newHead = 0;

  if (newHead == evqTail)	// Queue full?
  {
    Unlock ();
    Resize (Length * 2);	// Normally queue should not be more than half full
    goto again;
  } /* endif */

  EventQueue [evqHead] = Event;
  evqHead = newHead;
  Unlock ();
}

iEvent *csEventQueue::Get ()
{
  if (IsEmpty ())
    return NULL;
  else
  {
    Lock ();
    size_t oldTail = evqTail++;
    if (evqTail == Length)
      evqTail = 0;
    iEvent *ev = (iEvent *)EventQueue [oldTail];
    Unlock ();
    return ev;
  } /* endif */
}

void csEventQueue::Clear ()
{
  iEvent *ev;
  while ((ev = Get()) != NULL)
    { delete ev; }
}

void csEventQueue::Resize (size_t iLength)
{
  if (iLength <= 0)
    iLength = DEF_EVENT_QUEUE_LENGTH;

  if (iLength == Length)
    return;

  Lock ();
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
