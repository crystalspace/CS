/*
    Crystal Space Input Interface for PicoGUI
    (C) 2003 Mat Sutcliffe <oktal@gmx.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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
#include "iutil/event.h"
#include "iutil/evdefs.h"

extern "C"
{
  #include <picogui/types.h>
  #include <pgserver/common.h>
  #include <pgserver/types.h>
  #include <pgserver/input.h>
}

#include "inputdrv.h"
#include "keyconv.h"

SCF_IMPLEMENT_IBASE (csPGInputHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

csRef<iEventHandler> csPGInputDriver::EvH;
csRef<iEventQueue> csPGInputDriver::EvQ;
inlib *csPGInputDriver::Inlib;
cursor *csPGInputDriver::Cursor;

bool csPGInputDriver::Construct (iEventQueue *eq)
{
  csPGKeyConverter::Construct ();
  EvQ = eq;
  g_error err = load_inlib (RegFunc, & Inlib);
  return err == 0;
}

g_error csPGInputDriver::RegFunc (inlib *i)
{
  i->init		= Init;
  i->close		= Close;
  i->fd_init		= FDInit;
  i->fd_activate	= 0;
  i->poll		= 0;
  i->ispending		= 0;
  i->message		= 0;
  return 0;
}

g_error csPGInputDriver::Init ()
{
  g_error err = cursor_new (& Cursor, 0, 1);
  if (err) return err;

  EvH = csPtr<iEventHandler> (new csPGInputHandler);
  EvQ->RegisterListener (EvH, CSMASK_Input);

  return 0;
}

void csPGInputDriver::FDInit (int *n, fd_set *readfds, timeval *timeout)
{
  timeout->tv_sec = 0;
  timeout->tv_usec = 0;
}

void csPGInputDriver::Close ()
{
  EvQ->RemoveListener (EvH);

  pointer_free (1, & Cursor);

  EvH = 0;
  EvQ = 0;
}

bool csPGInputHandler::HandleEvent (iEvent &ev)
{
  static int mbstate;
  switch (ev.Type)
  {
    case csevKeyDown:
    infilter_send_key (PG_TRIGGER_KEYDOWN,
      csPGKeyConverter::CS2PG (ev), csPGKeyConverter::CS2PGMod (ev));
    return true;

    case csevKeyUp:
    infilter_send_key (PG_TRIGGER_KEYUP,
      csPGKeyConverter::CS2PG (ev), csPGKeyConverter::CS2PGMod (ev));
    return true;

    case csevMouseDown:
    mbstate |= 1<<(ev.Mouse.Button-1);
    infilter_send_pointing (PG_TRIGGER_DOWN, ev.Mouse.x, ev.Mouse.y,
      mbstate, csPGInputDriver::GetCursor ());
    return true;

    case csevMouseUp:
    mbstate &= ~(1<<(ev.Mouse.Button-1));
    infilter_send_pointing (PG_TRIGGER_UP, ev.Mouse.x, ev.Mouse.y,
      mbstate, csPGInputDriver::GetCursor ());
    return true;

    case csevMouseMove:
    infilter_send_pointing (PG_TRIGGER_MOVE, ev.Mouse.x, ev.Mouse.y,
      mbstate, csPGInputDriver::GetCursor ());
    return true;

    default:
    return false;
  }
}
