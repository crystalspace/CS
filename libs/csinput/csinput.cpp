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
#include "cssys/common/system.h"
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
  memset (&Key, 0, sizeof (Key));
  memset (aKey, 0, sizeof (aKey));
}

void csKeyboardDriver::do_keypress (time_t evtime, int key)
{
  int smask = (Key.shift ? CSMASK_SHIFT : 0)
            | (Key.ctrl ? CSMASK_CTRL : 0)
            | (Key.alt ? CSMASK_ALT : 0);
  if (key < 256)
  {
    if (!aKey [key])
      smask |= CSMASK_FIRST;
    aKey [key] = true;
  }
  if ((key >= 32) && (key < 128))
    if (Key.shift) key = ShiftedKey [key - 32];
  if (EventQueue)
  {
    CHK (EventQueue->Put (new csEvent (evtime, csevKeyDown, key, smask)));
  }
  SetKey (key, true);
}

void csKeyboardDriver::do_keyrelease (time_t evtime, int key)
{
  if (key < 256)
    aKey [key] = false;
  if ((key >= 32) && (key < 128))
    if (Key.shift) key = ShiftedKey [key - 32];
  if (EventQueue)
  {
    CHK (EventQueue->Put (new csEvent (evtime, csevKeyUp, key, 0)));
  }
  SetKey (key, false);
}

void csKeyboardDriver::SetKey (int key, bool state)
{
  switch (key)
  {
    case CSKEY_ESC:       Key.esc = state;       break;
    case CSKEY_ENTER:     Key.enter = state;     break;
    case CSKEY_TAB:       Key.tab = state;       break;
    case CSKEY_BACKSPACE: Key.backspace = state; break;
    case CSKEY_UP:        Key.up = state;        break;
    case CSKEY_DOWN:      Key.down = state;      break;
    case CSKEY_LEFT:      Key.left = state;      break;
    case CSKEY_RIGHT:     Key.right = state;     break;
    case CSKEY_PGUP:      Key.pgup = state;      break;
    case CSKEY_PGDN:      Key.pgdn = state;      break;
    case CSKEY_HOME:      Key.home = state;      break;
    case CSKEY_END:       Key.end = state;       break;
    case CSKEY_INS:       Key.ins = state;       break;
    case CSKEY_DEL:       Key.del = state;       break;
    case CSKEY_CTRL:      Key.ctrl = state;      break;
    case CSKEY_ALT:       Key.alt = state;       break;
    case CSKEY_SHIFT:     Key.shift = state;     break;
  } /* endswitch */
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

bool csMouseDriver::Open (ISystem* System, csEventQueue *EvQueue)
{
  EventQueue = EvQueue;
  Reset ();
  int val;
  System->ConfigGetInt ("MouseDriver", "DoubleClockTime", val, 300);
  DoubleClickTime = val;
  System->ConfigGetInt ("MouseDriver", "DoubleClickDist", val, 2);
  DoubleClickDist = val;
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
