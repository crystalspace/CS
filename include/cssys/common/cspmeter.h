/*
    Copyright (C) 1999 by Eric Sunshine <sunshine@sunshineco.com>
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

#ifndef __CSPMETER_H__
#define __CSPMETER_H__

#include "csutil/csbase.h"

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
 * reset.  SetTotal() implicitly performs a reset.  The meter does not print a
 * newline after 100% has been reached, on the assumption that the client may
 * wish to print some text on the same line on which the meter appeared.  If
 * the client needs a newline printed after 100% has been reached, then it is
 * the client's responsibility to print it.
 */
class csProgressMeter : public csBase
{
private:
  int type;	// One of MSG_INITIALIZATION, MSG_CONSOLE, MSG_STDOUT, etc.
  int total;
  int current;
  int anchor;

public:
  csProgressMeter(int total = 100);
  ~csProgressMeter() {}

  void SetMessageType(int n) { type = n; }
  int GetMessageType() const { return type; }

  void Step();
  void Reset() { current = 0; anchor = 0; }

  void SetTotal(int n) { total = n; Reset(); }
  int GetTotal() const { return total; }
  int GetCurrent() const { return current; }

};

#endif // __CSPMETER_H__
