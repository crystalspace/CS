/*
    Copyright (C) 2002 by Mathew Sutcliffe <oktal@gmx.co.uk>

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
#include "csutil/binder.h"
#include "iutil/event.h"
#include "iutil/evdefs.h"

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
      if (p) p->DecRef ();
    else
      if (b) b->DecRef ();
  }
};

csHashKey csHashComputeEvent (iEvent &ev)
{
  switch (ev.Type)
  {
    case csevKeyDown:
    case csevKeyUp:
      return CSMASK_Keyboard
        | ((ev.Key.Char < 256 ? ev.Key.Char : ev.Key.Code) << 16);

    case csevMouseMove:
      return CSMASK_MouseMove
        | ((abs (ev.Mouse.x) > abs (ev.Mouse.y) ? 0 : 1) << 16);

    case csevMouseDown:
    case csevMouseUp:
      return CSMASK_MouseDown
        | (ev.Mouse.Button << 16);

    case csevJoystickMove:
      return CSMASK_JoystickMove
        | ((abs (ev.Joystick.x) > abs (ev.Joystick.y) ? 0 : 1) << 16);

    case csevJoystickDown:
    case csevJoystickUp:
      return CSMASK_JoystickDown
        | (ev.Joystick.Button << 16);

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
}

inline bool csInputBinder::HandleEvent (iEvent &ev)
{
  csEvBind *pair = (csEvBind *) Hash.Get (csHashComputeEvent (ev));
  if (pair) switch (ev.Type)
  {
    case csevKeyDown:
    case csevMouseDown:
    case csevJoystickDown:
      if (pair->b)
      {
        if (pair->tgl)
        {
          if (pair->up)
          {
            pair->up = false;
            pair->b->Set (! pair->b->Get ());
          }
        }
        else
          pair->b->Set (true);
      }
      return true;

    case csevKeyUp:
    case csevMouseUp:
    case csevJoystickUp:
      if (pair->b)
      {
        if (pair->tgl)
          pair->up = true;
        else
          pair->b->Set (false);
      }
      return true;

    case csevMouseMove:
      //TODO
      return true;

    case csevJoystickMove:
      //TODO
      return true;

    default:
      break;
  }
  return false;
}

void csInputBinder::Bind (iEvent &ev, iInputBinderBoolean *var, bool toggle)
{
  Hash.Put (csHashComputeEvent (ev),
    (csHashObject) new csEvBind (var, toggle));
}

void csInputBinder::Bind (iEvent &ev, iInputBinderPosition *var)
{
  Hash.Put (csHashComputeEvent (ev),
    (csHashObject) new csEvBind (var));
}

bool csInputBinder::Unbind (iEvent &ev)
{
  csHashIterator iter (& Hash, csHashComputeEvent (ev));
  if (! iter.HasNext ()) return false;
  while (iter.HasNext ())
    delete (csEvBind *) iter.Next ();
  Hash.DeleteAll (csHashComputeEvent (ev));
  return true;
}

bool csInputBinder::UnbindAll ()
{
  csHashIterator iter (& Hash);
  if (! iter.HasNext ()) return false;
  while (iter.HasNext ())
    delete (csEvBind *) iter.Next ();
  Hash.DeleteAll ();
  return true;
}

