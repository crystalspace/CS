/*
    Copyright (C) 2001 by Jorrit Tyberghein

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __IUTIL_EVENTH_H__
#define __IUTIL_EVENTH_H__

#include "csutil/scf.h"

struct iEvent;

SCF_VERSION (iEventHandler, 0, 0, 1);

/**
 * This interface describes an entity that can receive events.
 */
struct iEventHandler : public iBase
{
  /**
   * This is the basic event handler function. Component should register first
   * with an event queue, using iEventQueue::RegisterListener() method,
   * before he'll receive any events. The handler should return true
   * if the event has been handled indeed (and thus to not pass it further).
   * The default implementation of HandleEvent does nothing.
   * NOTE: do NOT return true unless you really handled the event
   * and want the event to not be passed further for processing by
   * other plugins.
   */
  virtual bool HandleEvent (iEvent &/*Event*/) = 0;
};

#endif // __IUTIL_EVENTH_H__

