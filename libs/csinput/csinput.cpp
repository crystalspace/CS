/*
    Crystal Space input library
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Andrew Zabolotny <bit@eltech.ru>

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
#include "cssys/system.h"
#include "csinput/csinput.h"
#include "csutil/inifile.h"
#include "isystem.h"

// This array defines first 32..128 character codes with SHIFT key applied
static char ShiftedKey [128-32] =
{
 ' ', '!', '"', '#', '$', '%', '&', '"', '(', ')', '*', '+', '<', '_', '>', '?',
 ')', '!', '@', '#', '$', '%', '^', '&', '*', '(', ':', ':', '<', '+', '>', '?',
 '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '^', '_',
 '~', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '~', 127
};

//--//--//--//--//--//--//--//--//--//--//--//--//--/> Keyboard driver <--//--//

csKeyboardDriver::csKeyboardDriver ()
{
  EventQueue = NULL;
  Reset ();
}

csKeyboardDriver::~csKeyboardDriver ()
{
  Close ();
}

bool csKeyboardDriver::Open (csEventQueue *EvQueue)
{
  EventQueue = EvQueue;
  Reset ();
  return (EvQueue != NULL);
}

void csKeyboardDriver::Close ()
{
  // no-op
}

void csKeyboardDriver::Reset ()
{
  memset (KeyState, 0, sizeof (KeyState));
}

void csKeyboardDriver::do_keypress (time_t evtime, int key)
{
  int smask = (GetKeyState (CSKEY_SHIFT) ? CSMASK_SHIFT : 0)
            | (GetKeyState (CSKEY_CTRL) ? CSMASK_CTRL : 0)
            | (GetKeyState (CSKEY_ALT) ? CSMASK_ALT : 0);
  if (!GetKeyState (key))
    smask |= CSMASK_FIRST;
  if ((key >= 32) && (key < 128))
    if (GetKeyState (CSKEY_SHIFT))
      key = ShiftedKey [key - 32];
  if (EventQueue)
    CHKB (EventQueue->Put (new csEvent (evtime, csevKeyDown, key, smask)));
  SetKeyState (key, true);
}

void csKeyboardDriver::do_keyrelease (time_t evtime, int key)
{
  if ((key >= 32) && (key < 128))
    if (GetKeyState (CSKEY_SHIFT))
      key = ShiftedKey [key - 32];
  if (EventQueue)
    CHKB (EventQueue->Put (new csEvent (evtime, csevKeyUp, key, 0)));
  SetKeyState (key, false);
}

void csKeyboardDriver::SetKeyState (int key, bool state)
{
  int idx = (key < 256) ? key : (256 + key - CSKEY_FIRST);
  KeyState [idx] = state;
}

bool csKeyboardDriver::GetKeyState (int key)
{
  int idx = (key < 256) ? key : (256 + key - CSKEY_FIRST);
  return KeyState [idx];
}

//--//--//--//--//--//--//--//--//--//--//--//--//--//--> Mouse driver <--//--//

time_t csMouseDriver::DoubleClickTime;
size_t csMouseDriver::DoubleClickDist;

csMouseDriver::csMouseDriver ()
{
  EventQueue = NULL;
  Reset ();
}

csMouseDriver::~csMouseDriver ()
{
  Close ();
}

bool csMouseDriver::Open (iSystem* System, csEventQueue *EvQueue)
{
  EventQueue = EvQueue;
  Reset ();
  DoubleClickTime = System->ConfigGetInt ("MouseDriver", "DoubleClockTime", 300);
  DoubleClickDist = System->ConfigGetInt ("MouseDriver", "DoubleClickDist", 2);
  LastMouseX = LastMouseY = -99999;
  return (EvQueue != NULL);
}

void csMouseDriver::Close ()
{
  // no-op
}

void csMouseDriver::Reset ()
{
  if (EventQueue)
  {
    for (int i = 1; i < 10; i++)
      if (Button[i])
        do_buttonrelease (SysGetTime (), i, 0, 0);
  }
  else
    memset (Button, 0, sizeof (Button));
  LastClickButton = -1;
}

void csMouseDriver::do_buttonpress (time_t evtime, int button, int x, int y, bool shift,
  bool alt, bool ctrl)
{
  int smask = (shift ? CSMASK_SHIFT : 0)
            | (alt ? CSMASK_ALT : 0)
            | (ctrl ? CSMASK_CTRL : 0);
  Button [button] = true;

  int ev = csevMouseDown;

  if ((button == LastClickButton)
   && (evtime - LastClickTime <= DoubleClickTime)
   && (unsigned (ABS (x - LastClickX)) <= DoubleClickDist)
   && (unsigned (ABS (y - LastClickY)) <= DoubleClickDist))
    ev = csevMouseDoubleClick;
  LastClickButton = button;
  LastClickTime = evtime;
  LastMouseX = LastClickX = x;
  LastMouseY = LastClickY = y;

  if (EventQueue)
  {
    CHK (EventQueue->Put (new csEvent (evtime, ev, x, y, button, smask)));
  }
}

void csMouseDriver::do_buttonrelease (time_t evtime, int button, int x, int y)
{
  Button [button] = false;
  if (EventQueue)
  {
    CHK (EventQueue->Put (new csEvent (evtime, csevMouseUp, x, y, button, 0)));
  }
}

void csMouseDriver::do_mousemotion (time_t evtime, int x, int y)
{
  if ((x != LastMouseX)
   || (y != LastMouseY))
  {
    LastMouseX = x;
    LastMouseY = y;

    if (EventQueue)
    {
      CHK (EventQueue->Put (new csEvent (evtime, csevMouseMove, x, y, 0, 0)));
    }
  }
}
