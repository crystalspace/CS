/*
    Copyright (C) 2007 by Seth Yastrov

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

#include <cssysdef.h>
#include "csutil/scf.h"

#include <csutil/objreg.h>
#include <iutil/object.h>
#include <iengine/camera.h>

#include "context.h"

#include "ieditor/action.h"

namespace CS {
namespace EditorApp {

Context::Context (iObjectRegistry* obj_reg)
  : scfImplementationType (this), object_reg (obj_reg)
{
  object_reg->Register (this, "iContext");
}

Context::~Context ()
{
  object_reg->Unregister (this, "iContext");
}

iObject* Context::GetActiveObject ()
{
  return active;
}

const csWeakRefArray<iObject>& Context::GetSelectedObjects ()
{
  return selection;
}

void Context::AddSelectedObject (iObject* obj)
{
  if (selection.Find(obj) == csArrayItemNotFound)
  {
    selection.Push(obj);
  }
  active = obj;
}

void Context::RemoveSelectedObject (iObject* obj)
{
  selection.Delete(obj);
  active = obj;
}

void Context::ClearSelectedObjects ()
{
  selection.DeleteAll();
}

iCamera* Context::GetCamera ()
{
  return camera;
}

void Context::SetCamera (iCamera* cam)
{
  camera = cam;
  NotifyListeners ();
}

void Context::AddListener (iContextListener* listener)
{
  listeners.Push (listener);
}

void Context::RemoveListener (iContextListener* listener)
{
  listeners.Delete (listener);
}

void Context::NotifyListeners ()
{
  csRefArray<iContextListener>::Iterator it = listeners.GetIterator ();
  while (it.HasNext ())
  {
    it.Next ()->OnChanged (this);
  }
}

} // namespace EditorApp
} // namespace CS
