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
#include "iutil/event.h"
#include "iutil/evdefs.h"
#include "csutil/hashmap.h"
#include "csutil/binder.h"

csHashKey csHashComputeEvent (iEvent *ev)
{
  switch (ev->Type)
  {
    case csevKeyDown:
    case csevKeyUp:
      return (csHashKey)
        (CSMASK_Keyboard + ev->Key.Char + ev->Key.Code);

    case csevMouseMove:
      return (csHashKey) CSMASK_MouseMove;

    case csevMouseDown:
    case csevMouseUp:
      return (csHashKey)
        ((CSMASK_MouseDown | CSMASK_MouseUp) + ev->Mouse.Button);

    case csevJoystickMove:
      return (csHashKey) CSMASK_JoystickMove;

    case csevJoystickDown:
    case csevJoystickUp:
      return (csHashKey)
        (CSMASK_JoystickDown | CSMASK_JoystickUp);

    default:
      return 0;
  }
}

SCF_IMPLEMENT_IBASE (csInputBinder)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

csInputBinder::csInputBinder (int size = 127)
{
  SCF_CONSTRUCT_IBASE (NULL);
  Hash = new csHashMap (size);
}

csInputBinder::~csInputBinder ()
{
  UnbindAll ();
  delete Hash;
}

bool csInputBinder::HandleEvent (iEvent &ev)
{
  csEvBind *pair = (csEvBind *)Hash->Get (csHashComputeEvent (&ev));
  if (pair) switch (ev.Type)
  {
    case csevKeyDown:
    case csevMouseDown:
    case csevJoystickDown:
      if (pair->x) *(bool *)(pair->x) = true;
      return true;

    case csevKeyUp:
    case csevMouseUp:
    case csevJoystickUp:
      if (pair->x) *(bool *)(pair->x) = false;
      return true;

    case csevMouseMove:
      if (pair->x) *(int *)(pair->x) = ev.Mouse.x;
      if (pair->y) *(int *)(pair->y) = ev.Mouse.y;
      return true;

    case csevJoystickMove:
      if (pair->x) *(int *)(pair->x) = ev.Joystick.x;
      if (pair->y) *(int *)(pair->y) = ev.Joystick.y;
      return true;

    default:
      break;
  }
  return false;
}

void csInputBinder::Bind (iEvent *ev, int *xvar = NULL, int *yvar = NULL)
{
  if (! yvar) switch (ev->Type) {
    case csevMouseMove:
      if (ev->Mouse.y > ev->Mouse.x) { yvar = xvar; xvar = NULL; }
      break;

    case csevJoystickMove:
      if (ev->Joystick.y > ev->Joystick.x) { yvar = xvar; xvar = NULL; }
      break;

    default:
      break;
  }
  csHashKey key = csHashComputeEvent (ev);
  csEvBind *pair = (csEvBind *)Hash->Get (key);
  if (! pair) pair = new csEvBind;
  if (xvar) pair->x = xvar;
  if (yvar) pair->y = yvar;
  Hash->DeleteAll (key);
  Hash->Put (key, (csHashObject)pair);
}

void csInputBinder::Bind (csEvent &ev, int *xvar = NULL, int *yvar = NULL)
{
  Bind (&ev, xvar, yvar);
}

bool csInputBinder::Unbind (iEvent *ev)
{
  csHashIterator iter (Hash, csHashComputeEvent (ev));
  if (! iter.HasNext ()) return false;
  while (iter.HasNext ())
  {
    csEvBind *pair = (csEvBind *)iter.Next ();
    delete pair;
  }
  return true;
}

bool csInputBinder::Unbind (csEvent &ev)
{
  return Unbind (&ev);
}

bool csInputBinder::UnbindAll ()
{
  csHashIterator iter (Hash);
  if (! iter.HasNext ()) return false;
  while (iter.HasNext ())
  {
    csEvBind *pair = (csEvBind *)iter.Next ();
    delete pair;
  }
  return true;
}

