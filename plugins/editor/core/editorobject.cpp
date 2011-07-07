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

#include "ieditor/interfacewrappermanager.h"
#include "ieditor/interfacewrapper.h"
#include "ieditor/editor.h"

#include "editorobject.h"

namespace CS {
namespace EditorApp {

EditorObject::EditorObject (iObjectRegistry* obj_reg, iBase* object, wxBitmap* icon)
  : scfImplementationType (this), object (object), type (EditorObjectTypeUnknown), icon (icon)
{
  csRef<iInterfaceWrapperManager> interfaceManager (csQueryRegistry<iInterfaceWrapperManager> (obj_reg));
  
  // For each interface that the object implements
  scfInterfaceMetadataList* metadataList = object->GetInterfaceMetadata ();
  for (size_t i = 0; i < metadataList->metadataCount; i++)
  {
    scfInterfaceMetadata metadata = metadataList->metadata[i];

    // TODO: Check that version is compatible
    // Get the interface wrapper factory for this interface
    csRef<iInterfaceWrapperFactory> fact = interfaceManager->GetFactory (metadata.interfaceID);
    if (fact.IsValid ())
    {
      csRef<iInterfaceWrapper> inst (fact->CreateInstance (object));
      interfaces.Push (inst);

      if (!nameAttribInterface && fact->HasNameAttribute ())
        nameAttribInterface = inst;

      if (!parentAttribInterface && fact->HasParentAttribute ())
        parentAttribInterface = inst;

      if (type == EditorObjectTypeUnknown)
        type = fact->GetInterfaceType ();

      hasInterfaceHash.Put (metadata.interfaceID, true);
    }
  }
}

EditorObject::~EditorObject ()
{
}

const char* EditorObject::GetName () const
{
  if (!nameAttribInterface)
    return 0;
  
  return nameAttribInterface->GetObjectName ();
}

void EditorObject::SetName (const char* name)
{
  if (!nameAttribInterface)
    return;
  
  nameAttribInterface->SetObjectName (name);
  NotifyListeners ();
}

iBase* EditorObject::GetParent ()
{
  if (!parentAttribInterface)
    return 0;

  return parentAttribInterface->GetObjectParent ();
}

void EditorObject::SetParent (iBase* parent)
{
  if (!parentAttribInterface)
    return;

  parentAttribInterface->SetObjectParent (parent);
  NotifyListeners ();
}

csPtr<iBaseIterator> EditorObject::GetIterator ()
{
  if (!parentAttribInterface)
    return 0;
  
  return parentAttribInterface->GetIterator ();
}

EditorObjectType EditorObject::GetType () const
{
  return type;
}

wxBitmap* EditorObject::GetIcon () const
{
  return icon;
}

iBase* EditorObject::GetIBase ()
{
  return object;
}

bool EditorObject::HasInterface (scfInterfaceID id)
{
  return hasInterfaceHash.Get (id, false);
}

void EditorObject::AddListener (iEditorObjectChangeListener* listener)
{
  listeners.Push (listener);
}

void EditorObject::RemoveListener (iEditorObjectChangeListener* listener)
{
  listeners.Delete (listener);
}

void EditorObject::NotifyListeners ()
{
  csRefArray<iEditorObjectChangeListener>::Iterator it = listeners.GetIterator ();
  while (it.HasNext ())
  {
    it.Next ()->OnObjectChanged (this);
  }
}

} // namespace EditorApp
} // namespace CS
