/*
    Copyright (C) 2011 by Jelle Hellemans

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
#include "csutil/scf.h"
#include "csutil/sysfunc.h"

#include "iutil/objreg.h"
#include "iutil/plugin.h"

#include "cstool/enginetools.h"
#include "iengine/camera.h"
#include "csgeom/vector2.h"
#include <iengine/mesh.h>
#include <iutil/object.h>
#include <iutil/csinput.h>

#include "csutil/event.h"
#include "csutil/eventnames.h"

#include "ieditor/action.h"
#include "ieditor/actionmanager.h"
#include "ieditor/context.h"

#include "selectoperator.h"



CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{
  
SCF_IMPLEMENT_FACTORY (SelectOperator)

SelectOperator::SelectOperator (iBase* parent)
 : scfImplementationType (this, parent)
{
}

bool SelectOperator::Initialize (iObjectRegistry* obj_reg, const char* identifier, const char* label, const char* desc)
{
  this->object_reg = obj_reg;
  this->identifier = identifier;
  this->label = label;
  this->desc = desc;
  return true;
}

const char* SelectOperator::GetIdentifier () { return identifier; }
const char* SelectOperator::GetLabel () { return label; }
const char* SelectOperator::GetDescription () { return desc; }

SelectOperator::~SelectOperator ()
{
}

bool SelectOperator::Poll (iContext* ctx)
{
  return ctx != 0 && ctx->GetCamera();
}

OperatorState SelectOperator::Execute (iContext*)
{
  return OperatorFinished;
}

OperatorState SelectOperator::Invoke (iContext* ctx, iEvent* ev)
{
  if (ctx->GetCamera()->GetSector ())
  {
    int mouse_x = csMouseEventHelper::GetX(ev);
    int mouse_y = csMouseEventHelper::GetY(ev);
    csScreenTargetResult result = csEngineTools::FindScreenTarget (csVector2 (mouse_x, mouse_y), 100000.0f, ctx->GetCamera());
    if (result.mesh)
    {
      printf("SelectOperator::Invoke select\n");
      csRef<iKeyboardDriver> kbd = csQueryRegistry<iKeyboardDriver> (object_reg);
      csRef<iActionManager> actionManager = csQueryRegistry<iActionManager> (object_reg);
      csRef<iAction> action;
      action.AttachNew(new SelectObjectAction(object_reg, result.mesh->QueryObject(), kbd->GetKeyState (CSKEY_SHIFT)));
      actionManager->Do(action);
    }
  }
  return OperatorFinished;
}

OperatorState SelectOperator::Modal (iContext*, iEvent* ev)
{
  return OperatorFinished;
}


SelectObjectAction::SelectObjectAction (iObjectRegistry* object_reg, iObject* obj, bool multi) 
  : scfImplementationType (this), object_reg(object_reg), object(obj), multiple(multi)
{
}

bool SelectObjectAction::Do () 
{ 
  if (object)
  {
    csRef<iContext> context = csQueryRegistry<iContext> (object_reg);
    if (!multiple) context->ClearSelectedObjects();
    context->AddSelectedObject(object);
    printf("SelectObjectAction::Do AddSelectedObject %s\n", multiple?"multi":"single");
    return true; 
  }
  return false; 
}

bool SelectObjectAction::Undo () 
{ 
  if (object)
  {
    csRef<iContext> context = csQueryRegistry<iContext> (object_reg);
    context->RemoveSelectedObject(object);
    printf("SelectObjectAction::Undo RemoveSelectedObject\n");
    return true; 
  }
  return false; 
}

}
CS_PLUGIN_NAMESPACE_END(CSE)
