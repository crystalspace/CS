/*
    Crystal Space input library
    Copyright (C) 1998,2000 by Jorrit Tyberghein
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

#include "cssysdef.h"
#include "cssys/system.h"
#include "cssys/csinput.h"
#include "isys/system.h"

// This array defines first 32..128 character codes with SHIFT key applied
char ShiftedKey [128-32] =
{
' ', '!', '"', '#', '$', '%', '&', '"', '(', ')', '*', '+', '<', '_', '>', '?',
')', '!', '@', '#', '$', '%', '^', '&', '*', '(', ':', ':', '<', '+', '>', '?',
'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '^', '_',
'~', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '~', 127
};

//--//--//--//--//--//--//--//--//--//--//--//--//--/> Keyboard driver <--//--/

csKeyboardDriver::csKeyboardDriver (csSystemDriver *system) :
  KeyState (256 + (CSKEY_LAST - CSKEY_FIRST + 1))
{
  System = system;
  KeyState.Reset ();
}

void csKeyboardDriver::Reset ()
{
  for (size_t i = 0; i < KeyState.GetBitCount (); i++)
    if (KeyState [i])
      DoKey (i < 256 ? i : i - 256 + CSKEY_FIRST, 0, false);
}

void csKeyboardDriver::DoKey (int iKey, int iChar, bool iDown)
{
  int smask = (iDown && !GetKeyState (iKey)) ? CSMASK_FIRST : 0;

  SetKeyState (iKey, iDown);

  smask |= (GetKeyState (CSKEY_SHIFT) ? CSMASK_SHIFT : 0)
         | (GetKeyState (CSKEY_CTRL) ? CSMASK_CTRL : 0)
         | (GetKeyState (CSKEY_ALT) ? CSMASK_ALT : 0);

  if (iChar < 0)
  {
    if (smask & CSMASK_ALT)
      iChar = 0;
    else if (smask & CSMASK_CTRL)
      iChar = (iKey > 96 && iKey < 128) ? iKey - 96 : 0;
    else if (smask & CSMASK_SHIFT)
      iChar = (iKey >= 32 && iKey <= 127) ? ShiftedKey [iKey - 32] : 0;
    else
      iChar = (iKey >= 32 && iKey <= 255) ? iKey : 0;
  }

  System->EventQueue.Put (new csEvent (csGetTicks (),
    iDown ? csevKeyDown : csevKeyUp, iKey, iChar, smask));
}

void csKeyboardDriver::SetKeyState (int iKey, bool iDown)
{
  int idx = (iKey < 256) ? iKey : (256 + iKey - CSKEY_FIRST);
  if (iDown) KeyState.Set (idx); else KeyState.Reset (idx);
}

bool csKeyboardDriver::GetKeyState (int iKey)
{
  int idx = (iKey < 256) ? iKey : (256 + iKey - CSKEY_FIRST);
  return KeyState [idx];
}

//--//--//--//--//--//--//--//--//--//--//--//--//--//--> Mouse driver <--//--/

csTicks csMouseDriver::DoubleClickTime;
size_t csMouseDriver::DoubleClickDist;

csMouseDriver::csMouseDriver (csSystemDriver *system)
{
  System = system;
  LastX = LastY = 0;
  memset (&Button, 0, sizeof (Button));
}

void csMouseDriver::Reset ()
{
  int i;
  for (i = 0; i < CS_MAX_MOUSE_BUTTONS; i++)
    if (Button[i])
      DoButton (i + 1, false, LastX, LastY);
  LastClickButton = -1;
}

void csMouseDriver::DoButton (int button, bool down, int x, int y)
{
  if (x != LastX || y != LastY)
    DoMotion (x, y);

  if (button <= 0 || button >= CS_MAX_MOUSE_BUTTONS)
    return;

  int smask = (System->GetKeyState (CSKEY_SHIFT) ? CSMASK_SHIFT : 0)
            | (System->GetKeyState (CSKEY_ALT)   ? CSMASK_ALT   : 0)
            | (System->GetKeyState (CSKEY_CTRL)  ? CSMASK_CTRL  : 0);

  Button [button - 1] = down;

  csTicks evtime = csGetTicks ();
  System->EventQueue.Put (new csEvent (evtime,
    down ? csevMouseDown : csevMouseUp, x, y, button, smask));

  if ((button == LastClickButton)
   && (evtime - LastClickTime <= DoubleClickTime)
   && (unsigned (ABS (x - LastClickX)) <= DoubleClickDist)
   && (unsigned (ABS (y - LastClickY)) <= DoubleClickDist))
  {
    System->EventQueue.Put (new csEvent (evtime,
      down ? csevMouseDoubleClick : csevMouseClick, x, y, button, smask));
    // Don't allow for sequential double click events
    if (down)
      LastClickButton = -1;
  }
  else if (down)
  {
    // Remember the coordinates/button/position of last mousedown event
    LastClickButton = button;
    LastClickTime = evtime;
    LastClickX = x;
    LastClickY = y;
  }
}

void csMouseDriver::DoMotion (int x, int y)
{
  if (x != LastX || y != LastY)
  {
    int smask = (System->GetKeyState (CSKEY_SHIFT) ? CSMASK_SHIFT : 0)
              | (System->GetKeyState (CSKEY_ALT)   ? CSMASK_ALT   : 0)
              | (System->GetKeyState (CSKEY_CTRL)  ? CSMASK_CTRL  : 0);

    LastX = x;
    LastY = y;

    System->EventQueue.Put (new csEvent (csGetTicks (), csevMouseMove,
      x, y, 0, smask));
  }
}

void csMouseDriver::SetDoubleClickTime (int iTime, size_t iDist)
{
  DoubleClickTime = iTime;
  DoubleClickDist = iDist;
}

//--//--//--//--//--//--//--//--//--//--//--//--//--/> Joystick driver <--//--/

csJoystickDriver::csJoystickDriver (csSystemDriver *system)
{
  System = system;
  memset (&Button, 0, sizeof (Button));
  memset (&LastX, sizeof (LastX), 0);
  memset (&LastY, sizeof (LastY), 0);
}

void csJoystickDriver::Reset ()
{
  int i, j;
  for (i = 0; i < CS_MAX_JOYSTICK_COUNT; i++)
    for (j = 0; j < CS_MAX_JOYSTICK_BUTTONS; j++)
      if (Button [i][j])
        DoButton (i + 1, j + 1, false, LastX [i], LastY [i]);
}

void csJoystickDriver::DoButton (int number, int button, bool down,
  int x, int y)
{
  if (number <= 0 && number > CS_MAX_JOYSTICK_COUNT)
    return;

  if (x != LastX [number - 1] || y != LastY [number - 1])
    DoMotion (number, x, y);

  if (button <= 0 || button > CS_MAX_JOYSTICK_BUTTONS)
    return;

  int smask = (System->GetKeyState (CSKEY_SHIFT) ? CSMASK_SHIFT : 0)
            | (System->GetKeyState (CSKEY_ALT)   ? CSMASK_ALT   : 0)
            | (System->GetKeyState (CSKEY_CTRL)  ? CSMASK_CTRL  : 0);

  Button [number - 1] [button - 1] = down;
  System->EventQueue.Put (new csEvent (csGetTicks (),
    down ? csevJoystickDown : csevJoystickUp, number, x, y, button, smask));
}

void csJoystickDriver::DoMotion (int number, int x, int y)
{
  if (number <= 0 && number > CS_MAX_JOYSTICK_COUNT)
    return;

  if (x != LastX [number - 1] || y != LastY [number - 1])
  {
    int smask = (System->GetKeyState (CSKEY_SHIFT) ? CSMASK_SHIFT : 0)
              | (System->GetKeyState (CSKEY_ALT)   ? CSMASK_ALT   : 0)
              | (System->GetKeyState (CSKEY_CTRL)  ? CSMASK_CTRL  : 0);

    LastX [number - 1] = x;
    LastY [number - 1] = y;

    System->EventQueue.Put (new csEvent (csGetTicks (), csevJoystickMove,
      number, x, y, 0, smask));
  }
}
