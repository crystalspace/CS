/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Written by Michael Dale Long.

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

#ifndef __CS_EVCORD__
#define __CS_EVCORD__

#include "csutil/scf.h"
#include "isys/event.h"

struct iPlugin;

class csEventCord : public iEventCord
{
public:
  SCF_DECLARE_IBASE;

  /// Create an event cord for a given category/subcategory
  csEventCord(int category, int subcategory);

  /// Insert a plugin into the event cord
  virtual int Insert(iPlugin *plugin, int priority);

  /// Remove a plugin from the event cord
  virtual void Remove(iPlugin *plugin);

  /// Get whether events are passed to the system event queue
  virtual bool GetPass () const;

  /// Set whether events are passed to the system event queue
  virtual void SetPass (bool pass);

  /**
   * For the system driver to post events to the cord.
   * Should not be used directly.
   */
  bool PutEvent (iEvent *event);

  /// The category and subcategory of this events on this cord
  const int category, subcategory;

protected:
  /// Pass events to the system queue?
  volatile bool pass;

  /// Linked list of plugins
  struct PluginData
  {
    iPlugin *plugin;
    int priority;
    PluginData *next;
  };

  /// The cord itself
  PluginData *plugins;

  /// Protection against multiple threads accessing the same cord
  volatile int SpinLock;

  /// Lock the queue for modifications: NESTED CALLS TO LOCK/UNLOCK NOT ALLOWED!
  inline void Lock ()
  { while (SpinLock) ; SpinLock++; }
  /// Unlock the queue
  inline void Unlock ()
  { SpinLock--; }
};

#endif // ! __CS_EVCORD__
