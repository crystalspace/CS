/*
    Crystal Space Windowing System: timer class
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>
    Copyright (C) 2002 by Mat Sutcliffe <oktal@gmx.co.uk>

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

#ifndef __CS_CSTIMER_H__
#define __CS_CSTIMER_H__

#include "cscomp.h"

/**\file
 * Crystal Space Windowing System: timer class
 */

/**
 * \addtogroup csws_comps_timer
 * @{ */
 
#include "csextern.h"
 
/// csTimer class messages
enum
{
  /**
   * The TimerPulse event is generated each time timer crosses
   * the bound between two periods. This command is sent to csTimer
   * object owner.
   * <pre>
   * IN: (csTimer *)source
   * </pre>
   */
  cscmdTimerPulse = 0x00000300
};

#include "iutil/event.h"
#include "csutil/csevent.h"
struct iEventHandler;
struct iEventQueue;
struct iEventOutlet;

/**
 * Timer is a pseudo-component class which counts time and generates a
 * cscmdTimerPulse command to its parent each time timer crosses
 * bound between two periods. Timer period can be set either at run time
 * or during object creation. This component is always invisible.
 */
class CS_CRYSTALSPACE_EXPORT csTimer : public csComponent
{
  /// Timer period in milliseconds
  unsigned timeout;
  /// Period start
  unsigned start;
  /// Pause time before counting begins
  unsigned pausetime;
  /// Timer is stopped?
  bool Stopped;

  iEventHandler *eventh;
  iEventOutlet *evento;
  void Init (unsigned iPeriod);
  struct csTimerEvent : public csEvent
  {
    SCF_DECLARE_IBASE;
    csTimerEvent () { SCF_CONSTRUCT_IBASE(0); }
    virtual ~csTimerEvent() { SCF_DESTRUCT_IBASE(); }
  } TimerEvent;
  
public:
  /// Create timer object: the timer is created in running state
  csTimer (csComponent *iParent, unsigned iPeriod);
  csTimer (iEventHandler *iParent, unsigned iPeriod, intptr_t iInfo = 0);
  csTimer (iEventQueue *iParent, unsigned iPeriod, intptr_t iInfo = 0);

  /// Handle external events and generate timeouts
  virtual bool HandleEvent (iEvent &Event);

  /// Stop timer
  void Stop ();

  /// Restart timer from period start point
  void Restart ();

  /// Pause for specified time
  void Pause (unsigned iPause);

  /// Check if timer is running
  bool Running () { return !Stopped; }
};

/** @} */

#endif // __CS_CSTIMER_H__
