/*
    Copyright (C) 2003 by Mathew Sutcliffe <oktal@gmx.co.uk>

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
#include "csutil/csunicode.h"
#include "csutil/binder.h"
#include "iutil/event.h"
#include "iutil/evdefs.h"
#include "csutil/csevent.h"
#include "csutil/event.h"

struct csEvBind
{
  bool pos;
  bool tgl;
  bool up;
  union
  {
    iInputBinderPosition *p;
    iInputBinderBoolean *b;
  };
  csEvBind (iInputBinderPosition *pp)
  {
    p = pp;
    if (p) p->IncRef ();
    pos = true;
    tgl = false;
    up = false;
  }
  csEvBind (iInputBinderBoolean *bb, bool toggle)
  {
    b = bb;
    if (b) b->IncRef ();
    pos = false;
    tgl = toggle;
    up = true;
  }
  ~csEvBind ()
  {
    if (pos)
      { if (p) p->DecRef (); }
    else
      { if (b) b->DecRef (); }
  }
};

csHashKey csHashComputeEvent (iEvent* const ev)
{
  switch (ev->Type)
  {
    case csevKeyboard:
      {
	utf32_char codeRaw = 0;
	ev->Retrieve ("keyCodeRaw", codeRaw);
	return CSMASK_Keyboard | (codeRaw << 8);
      }

    case csevMouseMove:
      return CSMASK_MouseMove
        | ((abs (ev->Mouse.x) > abs (ev->Mouse.y) ? 0 : 1) << 16);

    case csevMouseDown:
    case csevMouseUp:
      return CSMASK_MouseDown
        | (ev->Mouse.Button << 16);

    case csevJoystickMove:
      return CSMASK_JoystickMove
        | ((abs (ev->Joystick.x) > abs (ev->Joystick.y) ? 0 : 1) << 16);

    case csevJoystickDown:
    case csevJoystickUp:
      return CSMASK_JoystickDown
        | (ev->Joystick.Button << 16);

    default:
      return 0;
  }
}

SCF_IMPLEMENT_IBASE (csInputBinder)
  SCF_IMPLEMENTS_INTERFACE (iInputBinder)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csInputBinder::eiEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csInputBinder::csInputBinder (iBase *parent, int size)
  : Hash (size)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiEventHandler);
}

csInputBinder::~csInputBinder ()
{
  UnbindAll ();
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiEventHandler);
  SCF_DESTRUCT_IBASE ();
}

inline bool csInputBinder::HandleEvent (iEvent &ev)
{
  switch (ev.Type)
  {
    case csevKeyboard:
    case csevMouseDown:
    case csevJoystickDown:
    case csevMouseUp:
    case csevJoystickUp:
    {
      bool up = (ev.Type == csevMouseUp) || (ev.Type == csevJoystickUp) ||
	((ev.Type == csevKeyboard) && 
	(csKeyEventHelper::GetEventType (&ev) == csKeyEventTypeUp));
      csEvBind *bind = (csEvBind *) Hash.Get (csHashComputeEvent (&ev));

      if (bind && !bind->pos && bind->b)
      {
	if (up)
	{
	  if (bind->tgl)
	    bind->up = true;
	  else
	    bind->b->Set (false);
	}
	else
	{
	  if (bind->tgl)
	  {
	    if (bind->up)
	    {
	      bind->up = false;
	      bind->b->Set (! bind->b->Get ());
	    }
	  }
	  else
	    bind->b->Set (true);
	}
        return true;
      }
      else
        return false;
    }

    case csevMouseMove:
    {
      csEvent evX (0, csevMouseMove, 1, 0, 0, 0);
      csEvent evY (0, csevMouseMove, 0, 1, 0, 0);
      csEvBind *bindx = (csEvBind *) Hash.Get (csHashComputeEvent (
        &evX));
      csEvBind *bindy = (csEvBind *) Hash.Get (csHashComputeEvent (
        &evY));

      bool ok = false;
      if (bindx && bindx->pos)
      {
        ok = true;
        bindx->p->Set (ev.Mouse.x);
      }
      if (bindy && bindy->pos)
      {
        ok = true;
        bindy->p->Set (ev.Mouse.y);
      }
      return ok;
    }

    case csevJoystickMove:
    {
      csEvent evX (0, csevJoystickMove, ev.Joystick.number, 1, 0, 0, 0);
      csEvent evY (0, csevJoystickMove, ev.Joystick.number, 0, 1, 0, 0);
      csEvBind *bindx = (csEvBind *) Hash.Get (csHashComputeEvent (
        &evX));
      csEvBind *bindy = (csEvBind *) Hash.Get (csHashComputeEvent (
        &evY));

      bool ok = false;
      if (bindx && bindx->pos)
      {
        ok = true;
        bindx->p->Set (ev.Mouse.x);
      }
      if (bindy && bindy->pos)
      {
        ok = true;
        bindy->p->Set (ev.Mouse.y);
      }
      return ok;
    }

    default:
      break;
  }
  return false;
}

void csInputBinder::Bind (iEvent &ev, iInputBinderBoolean *var, bool toggle)
{
  Hash.Put (csHashComputeEvent (&ev),
    (csHashObject) new csEvBind (var, toggle));
}

void csInputBinder::Bind (iEvent &ev, iInputBinderPosition *var)
{
  Hash.Put (csHashComputeEvent (&ev),
    (csHashObject) new csEvBind (var));
}

bool csInputBinder::Unbind (iEvent &ev)
{
  csHashIterator iter (& Hash, csHashComputeEvent (&ev));
  if (! iter.HasNext ()) return false;
  while (iter.HasNext ())
    delete (csEvBind *) iter.Next ();
  Hash.DeleteAll (csHashComputeEvent (&ev));
  return true;
}

bool csInputBinder::UnbindAll ()
{
  csGlobalHashIterator iter (& Hash);
  if (! iter.HasNext ()) return false;
  while (iter.HasNext ())
    delete (csEvBind *) iter.Next ();
  Hash.DeleteAll ();
  return true;
}

SCF_IMPLEMENT_IBASE (csInputBinderPosition)
  SCF_IMPLEMENTS_INTERFACE (iInputBinderPosition)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csInputBinderBoolean)
  SCF_IMPLEMENTS_INTERFACE (iInputBinderBoolean)
SCF_IMPLEMENT_IBASE_END

//CS_IMPLEMENT_STATIC_VARIABLE_CLEANUP
