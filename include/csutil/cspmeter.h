/*
    Copyright (C) 1999,2000 by Eric Sunshine <sunshine@sunshineco.com>
    Writen by Eric Sunshine <sunshine@sunshineco.com>

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

#ifndef __CS_CSPMETER_H__
#define __CS_CSPMETER_H__

#include "isys/system.h"

/**
 * The csProgressMeter class displays a simple percentage-style textual
 * progress meter.  By default, the meter is presented to the user by passing
 * MSG_INITIALIZATION to the system print function.  This setting may be
 * changed with the SetMessageType() method.  After constructing a progress
 * meter, call SetTotal() to set the total number of steps represented by the
 * meter.  The default is 100.  To animate the meter, call the Step() method
 * each time a unit of work has been completed.  At most Step() should be
 * called 'total' times.  Calling Step() more times than this will not break
 * anything, but if you do so, then the meter will not accurately reflect the
 * progress being made.  Calling Reset() will reset the meter to zero, but
 * will not update the display.  Reset() is provided so that the meter can be
 * re-used, but it is the client's responsibility to ensure that the display
 * is in a meaningful state.  For instance, the client should probably ensure
 * that a newline '\n' has been printed before re-using a meter which has been
 * reset.  The complementary method Restart() both resets the meter and prints
 * the initial tick mark ("0%").  The meter does not print a newline after
 * 100% has been reached, on the assumption that the client may wish to print
 * some text on the same line on which the meter appeared.  If the client
 * needs a newline printed after 100% has been reached, then it is the
 * client's responsibility to print it.
 */
class csProgressMeter
{
private:
  iSystem* sys;
  int type;	// One of MSG_INITIALIZATION, MSG_CONSOLE, MSG_STDOUT, etc.
  int granularity;
  int tick_scale;
  int total;
  int current;
  int anchor;

public:
  /// Constructs a new progress meter.
  csProgressMeter(iSystem*, int total = 100, int type = MSG_INITIALIZATION);
  /// Destroys the progress meter.
  ~csProgressMeter() {}

  /// Set the message type for iSystem::Printf().
  void SetMessageType(int n) { type = n; }
  /// Get the message type used for iSystem::Printf().
  int GetMessageType() const { return type; }

  /// Increment the meter by one unit and print a tick mark.
  void Step();
  /// Reset the meter to 0%.
  void Reset() { current = 0; anchor = 0; }
  /// Reset the meter and print the initial tick mark ("0%").
  void Restart();

  /// Set the total element count represented by the meter and perform a reset.
  void SetTotal(int n) { total = n; Reset(); }
  /// Get the total element count represented by the meter.
  int GetTotal() const { return total; }
  /// Get the current value of the meter (<= total).
  int GetCurrent() const { return current; }

  /**
   * Set the refresh granularity.  Valid values are 1-100, inclusive.  Default
   * is 10.  The meter is only refreshed after each "granularity" * number of
   * units have passed.  For instance, if granularity is 20, then * the meter
   * will only be updated at most 5 times, or every 20%.
   */
  void SetGranularity(int);
  /// Get the refresh granularity.
  int GetGranularity() const { return granularity; }

  /**
   * Set the tick scale.  Valid values are 1-100, inclusive.  Default is 2.  A
   * value of 1 means that each printed tick represents one unit, thus a total
   * of 100 ticks will be printed.  A value of 2 means that each tick
   * represents two units, thus a total of 50 ticks will be printed, etc.
   */
  void SetTickScale(int);
  /// Get the tick scale.
  int GetTickScale() const { return tick_scale; }
};

#endif // __CS_CSPMETER_H__
