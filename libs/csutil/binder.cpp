/*
    Copyright (C) 2003, 04 by Mathew Sutcliffe <oktal@gmx.co.uk>

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
#include "csutil/event.h"
#include "csutil/csevent.h"
#include "iutil/cfgfile.h"

SCF_IMPLEMENT_IBASE (csInputBinder)
  SCF_IMPLEMENTS_INTERFACE (iInputBinder)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csInputBinder::eiEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csInputBinder::csInputBinder (iBase *parent, int btnSize, int axisSize)
: axisHash (axisSize), axisArray (axisSize),
  btnHash (btnSize), btnArray (btnSize)
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

bool csInputBinder::HandleEvent (iEvent &ev)
{
  switch (ev.Type)
  {
    case csevKeyboard:
    case csevMouseDown:
    case csevJoystickDown:
    case csevMouseUp:
    case csevJoystickUp:
    {
      bool down = (ev.Type == csevMouseDown) || (ev.Type == csevJoystickDown) 
	|| ((ev.Type == csevKeyboard) 
	    && (csKeyEventHelper::GetEventType (&ev) == csKeyEventTypeDown));

      BtnCmd *bind = btnHash.Get
        (csInputDefinition (& ev, CSMASK_ALLMODIFIERS, false), 0);
      if (! bind) return false;

      if (bind->toggle)
      {
        if (down) bind->down = ! bind->down;
      }
      else bind->down = down;

      return true;
    }

    case csevMouseMove:
    {
      for (uint axis = 0; axis <= csMouseEventHelper::GetNumAxes(&ev); axis++)
      {
        AxisCmd *bind = axisHash.Get
          (csInputDefinition (& ev, (uint8)axis), 0);

        if (bind) bind->val = 
	  csMouseEventHelper::GetAxis(&ev, axis);
      }

      return true;
    }

    case csevJoystickMove:
    {
      for (uint axis = 0; axis < csJoystickEventHelper::GetNumAxes(&ev); axis++)
      {
        AxisCmd *bind = axisHash.Get
          (csInputDefinition (& ev, (uint8)axis), 0);

        if (bind) bind->val = 
	  csJoystickEventHelper::GetAxis(&ev, axis);
      }

      return true;
    }

    default: return false;
  }
}

void csInputBinder::BindAxis (const csInputDefinition &def, unsigned cmd,
  int sens)
{
  AxisCmd *bind = new AxisCmd (cmd, sens);
  axisArray.GetExtend (cmd) = bind;
  axisHash.Put (def, bind);
}

void csInputBinder::BindButton (const csInputDefinition &def, unsigned cmd,
  bool toggle)
{
  BtnCmd *bind = new BtnCmd (cmd, toggle);
  btnArray.GetExtend (cmd) = bind;
  btnHash.Put (def, bind);
}

int csInputBinder::Axis (unsigned cmd)
{
  if (axisArray.Length () > cmd)
  {
    AxisCmd *bind = axisArray[cmd];
    if (bind) return bind->val * bind->sens;
  }
  return 0;
}

bool csInputBinder::Button (unsigned cmd)
{
  if (btnArray.Length () > cmd)
  {
    BtnCmd *bind = btnArray[cmd];
    if (bind) return bind->down;
  }
  return false;
}

bool csInputBinder::UnbindAxis (unsigned cmd)
{
  if (axisArray.Length () <= cmd) return false;
  AxisCmd *bind = axisArray[cmd];
  if (! bind) return false;

  axisArray[cmd] = 0;
  delete bind;

  csInputDefinition key;
  AxisHash::GlobalIterator iter (axisHash.GetIterator ());
  while (iter.HasNext ())
  {
    if (bind == iter.NextNoAdvance (key)) break;
    iter.Advance ();
  }
  if (iter.HasNext ()) axisHash.Delete (key, bind);
  return true;
}

bool csInputBinder::UnbindButton (unsigned cmd)
{
  if (btnArray.Length () <= cmd) return false;
  BtnCmd *bind = btnArray[cmd];
  if (! bind) return false;

  btnArray[cmd] = 0;
  delete bind;

  csInputDefinition key;
  BtnHash::GlobalIterator iter (btnHash.GetIterator ());
  while (iter.HasNext ())
  {
    if (bind == iter.NextNoAdvance (key)) break;
    iter.Advance ();
  }
  if (iter.HasNext ()) btnHash.Delete (key, bind);
  return true;
}

void csInputBinder::UnbindAll ()
{
  size_t i;
  for (i = 0; i < axisArray.Length (); i++)
  {
    delete axisArray[i];
    axisArray[i] = 0;
  }
  for (i = 0; i < btnArray.Length (); i++)
  {
    delete btnArray[i];
    btnArray[i] = 0;
  }

  axisHash.DeleteAll ();
  btnHash.DeleteAll ();
}

void csInputBinder::LoadConfig (iConfigFile *cfg, const char *subsection)
{
#if 0
  csRef<iConfigIterator> iter = cfg->Enumerate (subsection);
  while (iter->Next ())
  {
    const char *key = iter->GetKey (true);
    int val = iter->GetInt ();

    csInputDefinition def (key);
    if (! def.IsValid ()) continue;

    //TODO: BindButton or BindAxis (def, val) depending on def's type
  }
#endif
}

void csInputBinder::SaveConfig (iConfigFile *cfg, const char *subsection)
{
#if 0
  AxisHash::GlobalIterator iter (axisHash.GetIterator ());
  while (iter.HasNext ())
  {
    csString key (subsection ? subsection : "")
    csInputDefinition def;
    AxisCmd *cmd = iter.Next (def);
    key.Append (def.AsString ());

    int val = (int) axisArray.Find (cmd);

    cfg->PutInt (key, val);
  }
#endif
}
