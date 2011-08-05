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
#include <iutil/document.h>

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

csPtr<iOperator> OperatorManager::Create (const char* identifier)
{
  csRef<OperatorMeta> meta = operatorMeta.Get(identifier, csRef<OperatorMeta>());
  if (!meta)
  {
    csRef<iDocumentNode> klass = iSCF::SCF->GetPluginMetadataNode(identifier);
    if (klass)
    {
      meta.AttachNew (new OperatorMeta());
      csRef<iDocumentNode> label = klass->GetNode("label");
      if (label) meta->label = label->GetContentsValue ();
      csRef<iDocumentNode> description = klass->GetNode("description");
      if (description) meta->description = description->GetContentsValue ();
      operatorMeta.PutUnique(identifier, meta);
    }
  }
  if (!meta)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.managers.operator", "Failed to load metadata for operator '%s'", identifier);
    return 0;
  }
  csRef<iBase> base = iSCF::SCF->CreateInstance(identifier);
  if (!base)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.managers.operator", "Failed to instantiate operator '%s'", identifier);
    return 0;
  }
  csRef<iOperator> ref = scfQueryInterface<iOperator> (base);
  if (!ref)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.managers.operator", "Not of type iOperator: '%s'", identifier);
    return 0;
  }
  ref->Initialize(object_reg, identifier, meta->label, meta->description);

  return csPtr<iOperator> (ref);
}

iOperator* OperatorManager::Execute (iOperator* op)
{
  printf("OperatorManager::Execute %s\n", op->GetIdentifier ());
  csRef<iContext> context = csQueryRegistry<iContext> (object_reg);
  if (op->Poll(context))
  {
    if (op->Execute(context) == OperatorRunningModal)
    {
      printf("OperatorManager::Execute MODAL %s\n", op->GetIdentifier ());
      modalOperator = op;
    }
  }
  else
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,"crystalspace.managers.operator", "Poll failed for '%s'", op->GetIdentifier ());
  }
  return op;
}

iOperator* OperatorManager::Invoke (iOperator* op, iEvent* ev)
{
  printf("OperatorManager::Invoke %s %s %s\n",  op->GetIdentifier (), op->GetLabel (), op->GetDescription ());
  csRef<iContext> context = csQueryRegistry<iContext> (object_reg);
  if (op->Poll(context))
  {
    if (op->Invoke(context, ev) == OperatorRunningModal)
    {
      modalOperator = op;
    }
  }
  else
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,"crystalspace.managers.operator", "Poll failed for '%s'", op->GetIdentifier ());
  }
  return op;
}

void OperatorManager::Uninitialize ()
{
}

void OperatorManager::Initialize ()
{
}


} // namespace EditorApp
} // namespace CS
