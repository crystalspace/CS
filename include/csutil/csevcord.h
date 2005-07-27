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

#ifndef __CS_CSEVCORD_H__
#define __CS_CSEVCORD_H__

#include "csextern.h"
#include "csutil/scf.h"
#include "iutil/event.h"

struct iEventHandler;

class csEventOutlet;

/**
 * Event cord.
 */
class CS_CRYSTALSPACE_EXPORT csEventCord : public iEventCord
{
protected:
  /// Pass events to the system queue?
  volatile bool pass;

  /// The category and subcategory of this events on this cord
  int category, subcategory;

  /** \internal
   * Linked list of plugins
   */
  struct PluginData
  {
    iEventHandler *plugin;
    int priority;
    PluginData *next;
  };

  /// The cord itself
  PluginData *plugins;

  /// Protection against multiple threads accessing the same cord
  volatile int SpinLock;

  /// Lock the queue for modifications: NESTED CALLS TO LOCK/UNLOCK NOT ALLOWED!
  inline void Lock() { while (SpinLock) {} SpinLock++; }
  /// Unlock the queue
  inline void Unlock() { SpinLock--; }

  /// iEventOutlet places events into cords.
  friend class csEventOutlet;
  bool Post(iEvent*);

public:
  SCF_DECLARE_IBASE;

  /// Create an event cord for a given category/subcategory
  csEventCord(int category, int subcategory);

  /// Destructor.
  virtual ~csEventCord();

  /// Insert an event handler into the event cord
  virtual int Insert(iEventHandler*, int priority);

  /// Remove an event handler from the event cord
  virtual void Remove(iEventHandler*);

  /// Get whether events are passed to the system event queue
  virtual bool GetPass() const { return pass; }

  /// Set whether events are passed to the system event queue
  virtual void SetPass(bool flag) { pass = flag; }

  /// Get the category of this cord.
  virtual int GetCategory() const { return category; }

  /// Get the subcategory of this cord.
  virtual int GetSubcategory() const { return subcategory; }
};

#endif // __CS_CSEVCORD_H__
