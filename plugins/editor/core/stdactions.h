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

#ifndef __CORE_STDACTIONS_H__
#define __CORE_STDACTIONS_H__

#include <iengine/engine.h>

#include <csutil/csstring.h>

#include "ieditor/action.h"
#include "ieditor/editorobject.h"
#include "ieditor/objectlist.h"

using namespace CS::EditorApp;

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

class SetObjectNameAction : public scfImplementation1<SetObjectNameAction,
    iAction>
{
public:
  SetObjectNameAction (iEditorObject* obj, const char* name);
  
  virtual csPtr<iAction> Do ();
  virtual const wxChar* GetDescription () const { return wxT("Rename object"); }

private:
  csRef<iEditorObject> obj;
  csString name;
};

class SetObjectParentAction : public scfImplementation1<SetObjectParentAction,iAction>
{
public:
  SetObjectParentAction (iEditorObject* obj, iBase* parent);
  
  virtual csPtr<iAction> Do ();
  virtual const wxChar* GetDescription () const { return wxT("Reparent object"); }

private:
  csRef<iEditorObject> obj;
  csRef<iBase> parent;
};

}
CS_PLUGIN_NAMESPACE_END(CSE)

#endif
