/*
    Crystal Space Windowing System: timer class
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CSTIMER_H__
#define __CSTIMER_H__

#include "cscomp.h"

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

/**
 * Timer is a pseudo-component class which counts time and generates a
 * cscmdTimerPulse command to its parent each time timer crosses
 * bound between two periods. Timer period can be set either at run time
 * or during object creation. This component is always invisible.
 */
class csTimer : public csComponent
{
  /// Timer period in milliseconds
  unsigned timeout;
  /// Period start
  unsigned start;
  /// Pause time before counting begins
  unsigned pause;
  /// Timer is stopped?
  bool Stopped;

public:
  /// Create timer object: the timer is created in running state
  csTimer (csComponent *iParent, unsigned iPeriod);

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

#endif // __CSTIMER_H__
