/*
    Crystal Space 3D engine: Event Queue interface
    Copyright (C) 1998 by Andrew Zabolotny <bit@freya.etu.ru>
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

#ifndef __CSEVENTQ_H__
#define __CSEVENTQ_H__

#include "cssys/csevent.h"

// Default event queue size: the queue will automatically grow
// when the queue will overflow.
#define DEF_EVENT_QUEUE_LENGTH  256

/**
 * This class represents a general system event queue.
 * The system driver contains an object of this class.
 * <p>
 * The implemented event queue is limited thread-safe. There are some
 * primitive spinlocks acquired/released in critical sections.
 */
class csEventQueue
{
  /// The queue itself
  volatile iEvent **EventQueue;
  /// Queue head and tail pointers
  volatile size_t evqHead, evqTail;
  /// The maximum queue length
  volatile size_t Length;
  /// Protection against multiple threads accessing same event queue
  volatile int SpinLock;

public:
  /// Initializes the event queue
  csEventQueue (size_t iLength = DEF_EVENT_QUEUE_LENGTH);
  /// Destroy an event queue object
  virtual ~csEventQueue ();

  /// Put a event into queue
  void Put (iEvent *Event);
  /// Get next event from queue or NULL
  iEvent *Get ();
  /// Clear event queue
  void Clear ();
  /// Query if queue is empty
  bool IsEmpty () { return evqHead == evqTail; }

private:
  /// Enlarge the queue size.
  void Resize (size_t iLength);
  /// Lock the queue for modifications: NESTED CALLS TO LOCK/UNLOCK NOT ALLOWED!
  inline void Lock ()
  { while (SpinLock) {} SpinLock++; }
  /// Unlock the queue
  inline void Unlock ()
  { SpinLock--; }
};

#endif // __CSEVENTQ_H__
