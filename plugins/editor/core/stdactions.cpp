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

#include <iengine/mesh.h>
#include <iengine/sector.h>

#include <wx/bitmap.h>

#include "stdactions.h"

#include "ieditor/editorobject.h"

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

SetObjectNameAction::SetObjectNameAction (iEditorObject* obj, const char* name)
  : scfImplementationType (this), obj (obj), name (name)
{
}

csPtr<iAction> SetObjectNameAction::Do ()
{
  csString oldName = obj->GetName ();
  obj->SetName (name);

  return csPtr<iAction> (new SetObjectNameAction (obj, oldName));
}

// ----------------------------------------------------------------------------

SetObjectParentAction::SetObjectParentAction (iEditorObject* obj, iBase* parent)
  : scfImplementationType (this), obj (obj), parent (parent)
{
}

csPtr<iAction> SetObjectParentAction::Do ()
{
  iBase* oldParent = obj->GetParent ();
  obj->SetParent (parent);

  return csPtr<iAction> (new SetObjectParentAction (obj, oldParent));
}

}
CS_PLUGIN_NAMESPACE_END(CSE)
