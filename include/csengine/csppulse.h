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

#ifndef __CSPPULSE_H__
#define __CSPPULSE_H__

#include "csutil/csbase.h"

/**
 * The csProgressPulse class provides a simple twirling textual cursor built
 * out of the characters '-', '\', '|', and '/'.  This type of functionality
 * is generally used as a sort of pulse beat during indeterminately lengthy
 * computational operations in order to let the user know that progress is
 * being made and that the program is not hanging.  By default, the pulse beat
 * is presented to the user by passing MSG_INITIALIZATION to the system print
 * function.  This setting may be changed with the SetMessageType() method.
 * When you construct a pulse object, you can specify whether its state is
 * initialized to a default value or whether it inherits from the global
 * state.  Global state allows one pulse object to pick up where another left
 * off in its animation sequence.  This allows you to create unrelated pulse
 * objects in different parts of a program and have them appear to the user as
 * representing a single computational operation, without having to manually
 * pass around state information.  To animate the pulse object, call the
 * Step() method each time a unit of work has been completed.  At each step a
 * backspace (\b) followed by one of the pulse characters (-, \, |, or /) is
 * printed, except for the very first step, in which case the backspace is
 * omitted.  If Step() was called at any time during the object's life time,
 * then when it is destroyed, it it prints a backspace, followed by a space
 * (' '), followed by one more backspace.
 */
class csProgressPulse : public csBase
{
private:
  int type;	// One of MSG_INITIALIZATION, MSG_CONSOLE, MSG_STDOUT, etc.
  int state;
  bool inherit_global_state;

public:
  csProgressPulse(bool inherit_global_state = false);
  ~csProgressPulse();

  void SetMessageType(int n) { type = n; }
  int GetMessageType() const { return type; }

  void Step();
};

#endif // __CSPPULSE_H__
