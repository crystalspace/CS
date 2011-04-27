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

#include "ieditor/editorobject.h"
#include "objectlist.h"

namespace CS {
namespace EditorApp {

ObjectList::ObjectList ()
  : scfImplementationType (this)
{
}

ObjectList::~ObjectList ()
{
}


void ObjectList::Add (iEditorObject* obj)
{
  objects.PutUnique (obj->GetIBase (), obj);

  csRefArray<iObjectListListener>::Iterator it = listeners.GetIterator ();
  while (it.HasNext ())
  {
    it.Next ()->OnObjectAdded (this, obj);
  }
}

void ObjectList::Remove (iEditorObject* obj)
{
  csRefArray<iObjectListListener>::Iterator it = listeners.GetIterator ();
  while (it.HasNext ())
  {
    it.Next ()->OnObjectRemoved (this, obj);
  }
  
  objects.Delete (obj->GetIBase (), obj);
}

void ObjectList::Clear ()
{
  csRefArray<iObjectListListener>::Iterator it = listeners.GetIterator ();
  while (it.HasNext ())
  {
    it.Next ()->OnObjectsCleared (this);
  }
  
  objects.DeleteAll ();
}

csPtr<iEditorObjectIterator> ObjectList::GetIterator ()
{
  return csPtr<iEditorObjectIterator> (new ObjectList::ObjectListIterator (objects));
}

void ObjectList::AddListener (iObjectListListener* listener)
{
  listeners.Push (listener);
}

void ObjectList::RemoveListener (iObjectListListener* listener)
{
  listeners.Delete (listener);
}

iEditorObject* ObjectList::FindObject (iBase* obj)
{
  return objects.Get (obj, 0);
}

} // namespace EditorApp
} // namespace CS

