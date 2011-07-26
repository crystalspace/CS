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

#include <csutil/objreg.h>
#include <iutil/plugin.h>

#include <csutil/csevent.h>

#include "operatormanager.h"

#include "mainframe.h"

#include "ieditor/context.h"

#include <wx/colordlg.h>
#include <wx/msgdlg.h>
#include <wx/button.h>
#include <wx/textdlg.h>
#include <wx/stattext.h>


namespace CS {
namespace EditorApp {

OperatorManager::OperatorManager (iObjectRegistry* obj_reg)
  : scfImplementationType (this), object_reg (obj_reg)
{
  object_reg->Register (this, "iOperatorManager");
  
  eventQueue = csQueryRegistry<iEventQueue> (object_reg);
  if (!eventQueue) return;

  nameRegistry = csEventNameRegistry::GetRegistry(object_reg);
  if (!nameRegistry) return;

  eventQueue->RegisterListener(this);

  //Register for the input events, for Handle().
  eventQueue->RegisterListener (this, nameRegistry->GetID("crystalspace.input"));
}

OperatorManager::~OperatorManager ()
{
  object_reg->Unregister (this, "iOperatorManager");
}

bool OperatorManager::HandleEvent(iEvent& ev)
{
  if (modalOperator)
  {
    csRef<iContext> context = csQueryRegistry<iContext> (object_reg);
    if (modalOperator->Modal(context, &ev) != OperatorRunningModal)
    {
      printf("OperatorManager::HandleEvent MODAL finished\n");
      modalOperator.Invalidate();
    }
    else
      return true;
  }
  return false;
} 

iOperator* OperatorManager::Execute (const char* identifier)
{
  printf("OperatorManager::Execute %s\n", identifier);
  csRef<iOperatorFactory> oper = GetOperator(identifier);
  if (!oper) return 0;
  csRef<iContext> context = csQueryRegistry<iContext> (object_reg);
  csRef<iOperator> op = oper->Create();
  if (op->Poll(context))
  {
    if (op->Execute(context) == OperatorRunningModal)
    {
      printf("OperatorManager::Execute MODAL %s\n", identifier);
      modalOperator = op;
    }
  }
  else
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,"crystalspace.managers.operator", "Poll failed for '%s'", identifier);
  }
  return op;
}

iOperator* OperatorManager::Invoke (const char* identifier, iEvent* ev)
{
  printf("OperatorManager::Invoke %s\n", identifier);
  csRef<iOperatorFactory> oper = GetOperator(identifier);
  if (!oper) return 0;
  csRef<iContext> context = csQueryRegistry<iContext> (object_reg);
  csRef<iOperator> op = oper->Create();
  if (op->Poll(context))
  {
    if (op->Invoke(context, ev) == OperatorRunningModal)
    {
      modalOperator = op;
    }
  }
  else
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,"crystalspace.managers.operator", "Poll failed for '%s'", identifier);
  }
  return op;
}

const char* OperatorManager::GetLabel (const char* identifier)
{
  csRef<iOperatorFactory> oper = GetOperator(identifier);
  if (!oper) return "??? ???";
  return oper->GetLabel();
}

const char* OperatorManager::GetDescription (const char* identifier)
{
  csRef<iOperatorFactory> oper = GetOperator(identifier);
  if (!oper) return "??? ???";
  return oper->GetDescription();
}

void OperatorManager::Uninitialize ()
{
}

void OperatorManager::Initialize ()
{
}

void OperatorManager::Register(const char* identifier, iOperatorFactory* oper)
{
  printf("OperatorManager::Register %s\n", identifier);
  operatorFactories.PutUnique(identifier, oper);
}

csRef<iOperatorFactory> OperatorManager::GetOperator (const char* identifier)
{
  csRef<iOperatorFactory> oper = operatorFactories.Get(identifier, csRef<iOperatorFactory>());
  if (!oper)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.editor", "Attempt to fetch operator '%s' failed!",
        identifier);
  }

  return oper;
}

} // namespace EditorApp
} // namespace CS
