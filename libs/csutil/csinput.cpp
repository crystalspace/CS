/*
    Crystal Space input library
    Copyright (C) 1998,2000 by Jorrit Tyberghein
    Copyright (C) 2001 by Eric Sunshine <sunshine@sunshineco.com>
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
#include "csutil/cfgacc.h"
#include "csutil/csevent.h"
#include "csutil/csinput.h"
#include "cssys/sysfunc.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"

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

//--//--//--//--//--//--//--//--//--//--//--//--//--/> Input driver <--//--//--

SCF_IMPLEMENT_IBASE (csInputDriver::FocusListener)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

csInputDriver::csInputDriver(iObjectRegistry* r) : Registry(r), Queue(0)
{
  Listener.Parent = this;
}

csInputDriver::~csInputDriver()
{
  // Force a refetch of Queue in order to double check its validity since, at
  // shutdown time, the event queue might already have been destroyed.
  Queue = NULL;
  GetEventQueue();
  StopListening();
}

void csInputDriver::SetSCFParent(iBase* p)
{
  Listener.SetSCFParent(p);
  StartListening();
}

iEventQueue* csInputDriver::GetEventQueue()
{
  if (Queue == 0)
  {
    Queue = CS_QUERY_REGISTRY(Registry, iEventQueue);
    if (Queue) Queue->DecRef ();
  }
  return Queue;
}

void csInputDriver::StartListening()
{
  if (Queue == 0 && GetEventQueue() != 0) // Not already registered.
    Queue->RegisterListener(&Listener, CSMASK_Command);
}

void csInputDriver::StopListening()
{
  if (Queue != 0) // Already registered.
    Queue->RemoveListener(&Listener);
}

void csInputDriver::Post(iEvent* e)
{
  StartListening(); // If this failed at construction, try again.
  if (Queue != 0)
    Queue->Post(e);
  else
    e->DecRef();
}

bool csInputDriver::FocusListener::HandleEvent(iEvent& e)
{
  bool const mine = (e.Type == csevBroadcast && 
    e.Command.Code == cscmdFocusChanged && !e.Command.Info);
  if (mine) // Application lost focus.
    Parent->LostFocus();
  return mine;
}

//--//--//--//--//--//--//--//--//--//--//--//--//--/> Keyboard driver <--//--/

SCF_IMPLEMENT_IBASE(csKeyboardDriver)
  SCF_IMPLEMENTS_INTERFACE(iKeyboardDriver)
SCF_IMPLEMENT_IBASE_END

csKeyboardDriver::csKeyboardDriver (iObjectRegistry* r) :
  csInputDriver(r), KeyState (256 + (CSKEY_LAST - CSKEY_FIRST + 1))
{
  SCF_CONSTRUCT_IBASE(0);
  SetSCFParent(this);
  KeyState.Reset();
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
        |  (GetKeyState (CSKEY_CTRL ) ? CSMASK_CTRL  : 0)
        |  (GetKeyState (CSKEY_ALT  ) ? CSMASK_ALT   : 0);

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

  Post(new csEvent (csGetTicks (),
    iDown ? csevKeyDown : csevKeyUp, iKey, iChar, smask));
}

void csKeyboardDriver::SetKeyState (int iKey, bool iDown)
{
  int idx = (iKey < 256) ? iKey : (256 + iKey - CSKEY_FIRST);
  if (iDown)
    KeyState.Set (idx);
  else
    KeyState.Reset (idx);
}

bool csKeyboardDriver::GetKeyState (int iKey)
{
  int idx = (iKey < 256) ? iKey : (256 + iKey - CSKEY_FIRST);
  return KeyState [idx];
}

//--//--//--//--//--//--//--//--//--//--//--//--//--//--> Mouse driver <--//--/

SCF_IMPLEMENT_IBASE(csMouseDriver)
  SCF_IMPLEMENTS_INTERFACE(iMouseDriver)
SCF_IMPLEMENT_IBASE_END

csMouseDriver::csMouseDriver (iObjectRegistry* r) :
  csInputDriver(r), Keyboard(0)
{
  SCF_CONSTRUCT_IBASE(0);
  SetSCFParent(this);

  LastX = LastY = 0;
  memset (&Button, 0, sizeof (Button));
  Reset();

  csConfigAccess cfg;
  cfg.AddConfig(Registry, "/config/mouse.cfg");
  SetDoubleClickTime (
    cfg->GetInt ("MouseDriver.DoubleClickTime", 300),
    cfg->GetInt ("MouseDriver.DoubleClickDist", 2));
}

csMouseDriver::~csMouseDriver ()
{
}

void csMouseDriver::Reset ()
{
  for (int i = 0; i < CS_MAX_MOUSE_BUTTONS; i++)
    if (Button[i])
      DoButton (i + 1, false, LastX, LastY);
  LastClickButton = -1;
}

iKeyboardDriver* csMouseDriver::GetKeyboardDriver()
{
  if (Keyboard == 0)
  {
    Keyboard = CS_QUERY_REGISTRY(Registry, iKeyboardDriver);
    if (Keyboard) Keyboard->DecRef ();
  }
  return Keyboard;
}

void csMouseDriver::DoButton (int button, bool down, int x, int y)
{
  if (x != LastX || y != LastY)
    DoMotion (x, y);

  if (button <= 0 || button >= CS_MAX_MOUSE_BUTTONS)
    return;

  iKeyboardDriver* k = GetKeyboardDriver();
  int smask = (k->GetKeyState (CSKEY_SHIFT) ? CSMASK_SHIFT : 0)
            | (k->GetKeyState (CSKEY_ALT  ) ? CSMASK_ALT   : 0)
            | (k->GetKeyState (CSKEY_CTRL ) ? CSMASK_CTRL  : 0);

  Button [button - 1] = down;

  csTicks evtime = csGetTicks ();
  Post(new csEvent
    (evtime, down ? csevMouseDown : csevMouseUp, x, y, button, smask));

  if ((button == LastClickButton)
   && (evtime - LastClickTime <= DoubleClickTime)
   && (unsigned (ABS (x - LastClickX)) <= DoubleClickDist)
   && (unsigned (ABS (y - LastClickY)) <= DoubleClickDist))
  {
    Post(new csEvent(evtime,
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
    iKeyboardDriver* k = GetKeyboardDriver();
    int smask = (k->GetKeyState (CSKEY_SHIFT) ? CSMASK_SHIFT : 0)
              | (k->GetKeyState (CSKEY_ALT  ) ? CSMASK_ALT   : 0)
              | (k->GetKeyState (CSKEY_CTRL ) ? CSMASK_CTRL  : 0);
    LastX = x;
    LastY = y;
    Post(new csEvent (csGetTicks (), csevMouseMove, x, y, 0, smask));
  }
}

void csMouseDriver::SetDoubleClickTime (int iTime, size_t iDist)
{
  DoubleClickTime = iTime;
  DoubleClickDist = iDist;
}

//--//--//--//--//--//--//--//--//--//--//--//--//--/> Joystick driver <--//--/

SCF_IMPLEMENT_IBASE(csJoystickDriver)
  SCF_IMPLEMENTS_INTERFACE(iJoystickDriver)
SCF_IMPLEMENT_IBASE_END

csJoystickDriver::csJoystickDriver (iObjectRegistry* r) :
  csInputDriver(r), Keyboard(0)
{
  SCF_CONSTRUCT_IBASE(0);
  SetSCFParent(this);
  memset (&Button, 0, sizeof (Button));
  memset (&LastX, sizeof (LastX), 0);
  memset (&LastY, sizeof (LastY), 0);
}

void csJoystickDriver::Reset ()
{
  for (int i = 0; i < CS_MAX_JOYSTICK_COUNT; i++)
    for (int j = 0; j < CS_MAX_JOYSTICK_BUTTONS; j++)
      if (Button [i][j])
        DoButton (i + 1, j + 1, false, LastX [i], LastY [i]);
}

iKeyboardDriver* csJoystickDriver::GetKeyboardDriver()
{
  if (Keyboard == 0)
  {
    Keyboard = CS_QUERY_REGISTRY(Registry, iKeyboardDriver);
    if (Keyboard) Keyboard->DecRef ();
  }
  return Keyboard;
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

  iKeyboardDriver* k = GetKeyboardDriver();
  int smask = (k->GetKeyState (CSKEY_SHIFT) ? CSMASK_SHIFT : 0)
            | (k->GetKeyState (CSKEY_ALT)   ? CSMASK_ALT   : 0)
            | (k->GetKeyState (CSKEY_CTRL)  ? CSMASK_CTRL  : 0);

  Button [number - 1] [button - 1] = down;
  Post(new csEvent (csGetTicks (),
    down ? csevJoystickDown : csevJoystickUp, number, x, y, button, smask));
}

void csJoystickDriver::DoMotion (int number, int x, int y)
{
  if (number <= 0 && number > CS_MAX_JOYSTICK_COUNT)
    return;

  if (x != LastX [number - 1] || y != LastY [number - 1])
  {
    iKeyboardDriver* k = GetKeyboardDriver();
    int smask = (k->GetKeyState (CSKEY_SHIFT) ? CSMASK_SHIFT : 0)
              | (k->GetKeyState (CSKEY_ALT)   ? CSMASK_ALT   : 0)
              | (k->GetKeyState (CSKEY_CTRL)  ? CSMASK_CTRL  : 0);
    LastX [number - 1] = x;
    LastY [number - 1] = y;
    Post(new csEvent(csGetTicks(), csevJoystickMove, number, x, y, 0, smask));
  }
}
