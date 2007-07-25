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

/**\file
 * Text progress meter
 */

#include "csextern.h"
#include "csutil/scf_implementation.h"
#include "ivaria/pmeter.h"

struct iConsoleOutput;

/**
 * The csTextProgressMeter class displays a simple percentage-style textual
 * progress meter.  By default, the meter is presented to the user by passing
 * CS_MSG_INITIALIZATION to the system print function.  This setting may be
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
class CS_CRYSTALSPACE_EXPORT csTextProgressMeter : 
  public scfImplementation1<csTextProgressMeter, iProgressMeter>
{
private:
  iConsoleOutput* console;
  int granularity;
  int tick_scale;
  int total;
  int current;
  int anchor;

public:
  /// Constructs a new progress meter.
  csTextProgressMeter (iConsoleOutput* console, int total = 100);
  /// Destroys the progress meter.
  virtual ~csTextProgressMeter ();

  /**
   * Set the tick scale.  Valid values are 1-100, inclusive.  Default is 2.  A
   * value of 1 means that each printed tick represents one unit, thus a total
   * of 100 ticks will be printed.  A value of 2 means that each tick
   * represents two units, thus a total of 50 ticks will be printed, etc.
   */
  void SetTickScale (int);
  /// Get the tick scale.
  int GetTickScale () const { return tick_scale; }

  /**
   * Set the id and description of what we are currently monitoring.
   * An id can be something like "crystalspace.engine.lighting.calculation".
   * \sa \ref FormatterNotes
   */
  virtual void CS_GNUC_PRINTF (3, 4)
      SetProgressDescription (const char*, const char*, ...) { }
  virtual void CS_GNUC_PRINTF (3, 0)
      SetProgressDescriptionV (const char*, const char*, va_list) { }

  /// Increment the meter by n units (default 1) and print a tick mark.
  virtual void Step (unsigned int n = 1);
  /// Reset the meter to 0%.
  virtual void Reset () { current = 0; anchor = 0; }
  /// Reset the meter and print the initial tick mark ("0%").
  virtual void Restart ();
  /// Abort the meter.
  virtual void Abort ();
  /// Finalize the meter (i.e. we completed the task sooner than expected).
  virtual void Finalize ();

  /// Set the total element count represented by the meter and perform a reset.
  virtual void SetTotal (int n) { total = n; Reset(); }
  /// Get the total element count represented by the meter.
  virtual int GetTotal () const { return total; }
  /// Get the current value of the meter (<= total).
  virtual int GetCurrent () const { return current; }

  /**
   * Set the refresh granularity.  Valid values are 1-100, inclusive.  Default
   * is 10.  The meter is only refreshed after each "granularity" * number of
   * units have passed.  For instance, if granularity is 20, then * the meter
   * will only be updated at most 5 times, or every 20%.
   */
  virtual void SetGranularity (int);
  /// Get the refresh granularity.
  virtual int GetGranularity () const { return granularity; }
};

#endif // __CS_CSPMETER_H__
