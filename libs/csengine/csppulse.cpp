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

#include "sysdef.h"
#include "csengine/csppulse.h"
#include "csengine/sysitf.h"

static char const ANIMATION[] = "-\\|/";
int const ANIMATION_COUNT = sizeof(ANIMATION) / sizeof(ANIMATION[0]) - 1;

static int GLOBAL_STATE = 0;
int const NO_STATE = -1;

csProgressPulse::csProgressPulse(bool inherit) :
  type(MSG_INITIALIZATION), state(NO_STATE), inherit_global_state(inherit)
{
}

csProgressPulse::~csProgressPulse()
{
  if (state != NO_STATE)
  {
    CsPrintf (type, "\b \b");
    GLOBAL_STATE = state;
  }
}

void csProgressPulse::Step()
{
  char const* prefix = "\b";
  if (state == NO_STATE)
  {
    prefix = "";
    state = (inherit_global_state ? GLOBAL_STATE : 0);
  }

  CsPrintf (type, "%s%c", prefix, ANIMATION[state]);
  if (++state >= ANIMATION_COUNT)
    state = 0;
}
