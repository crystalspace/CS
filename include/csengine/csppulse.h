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

#ifndef __CS_CSPPULSE_H__
#define __CS_CSPPULSE_H__

#include "csutil/csbase.h"
#include "cstypes.h"
#include "isys/system.h"

/**
 * The csProgressPulse class provides a simple twirling textual cursor built
 * out of the characters '-', '\', '|', and '/'.  This type of functionality
 * is generally used as a sort of pulse beat during indeterminately lengthy
 * computational operations in order to let the user know that progress is
 * being made and that the program is not hanging.  By default, the pulse beat
 * is presented to the user by passing MSG_INITIALIZATION to the system print
 * function.  This setting may be changed with the SetMessageType() method.
 * To animate the pulse object, call the Step() method each time a unit of
 * work has been completed.  At each step a backspace (\b) followed by one of
 * the pulse characters (-, \, |, or /) is printed, except for the very first
 * step, in which case the backspace is omitted.  Erase() clears the pulse, if
 * necessary, by printing a backspace, followed by a space (' '), followed by
 * one more backspace.  Reset() erases the pulse and then resets the state.
 * Erase() is called automatically by the destructor.
 */
class csProgressPulse : public csBase
{
private:
  iSystem* sys;
  int type;	// One of MSG_INITIALIZATION, MSG_CONSOLE, MSG_STDOUT, etc.
  int state;
  bool drawn;

public:
  /// Constructs a new progress pulse.
  csProgressPulse(iSystem*);
  /// Destroys the progress pulse.
  ~csProgressPulse();

  /// Set the message type for iSystem::Printf().
  void SetMessageType(int n) { type = n; }
  /// Get the message type used for iSystem::Printf().
  int GetMessageType() const { return type; }

  /// Increment the progress by one pulse.
  void Step();
  /// Erase the pulse indicator.
  void Erase();
  /// Reset the state and erase the indicator.
  void Reset();
};

#endif // __CS_CSPPULSE_H__
