/*
  Copyright (C) 2009 by Frank Richter

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"

#include "forcedprioshader.h"

#include "iutil/string.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{
  ForcedPriorityShader::ForcedPriorityShader (csXMLShader* parent, size_t techNum)
   : scfImplementationType (this), parentShader (parent), techNum (techNum)
  {
  }

  void ForcedPriorityShader::AddVariable (csShaderVariable *variable)
  {
    parentShader->AddVariable (variable);
  }
  
  csShaderVariable* ForcedPriorityShader::GetVariable (CS::ShaderVarStringID name) const
  {
    return parentShader->GetVariable (name);
  }
  
  const csRefArray<csShaderVariable>& ForcedPriorityShader::GetShaderVariables () const
  {
    return parentShader->GetShaderVariables();
  }
  
  void ForcedPriorityShader::PushVariables (csShaderVariableStack& stack) const
  {
    parentShader->PushVariables (stack);
  }
  
  bool ForcedPriorityShader::IsEmpty() const
  {
    return parentShader->IsEmpty();
  }
  
  void ForcedPriorityShader::ReplaceVariable (csShaderVariable *variable)
  {
    parentShader->ReplaceVariable (variable);
  }
  
  void ForcedPriorityShader::Clear ()
  {
    parentShader->Clear ();
  }
  
  bool ForcedPriorityShader::RemoveVariable (csShaderVariable* variable)
  {
    return parentShader->RemoveVariable (variable);
  }
  
  bool ForcedPriorityShader::RemoveVariable (CS::ShaderVarStringID name)
  {
    return parentShader->RemoveVariable (name);
  }

  /*
     @@@ NOTE Activating etc. _really_ changes the state of the parent shader
     Not nice. OTOH, there isn't really supposed to be more than 1 shader activated
     at once, anyway ...
   */
  size_t ForcedPriorityShader::GetTicket (const CS::Graphics::RenderMeshModes& modes,
    const csShaderVariableStack& stack)
  {
    return parentShader->GetTicketForTech (modes, stack, techNum);
  }
  
  size_t ForcedPriorityShader::GetNumberOfPasses (size_t ticket)
  {
    return parentShader->GetNumberOfPasses (ticket);
  }
  
  bool ForcedPriorityShader::ActivatePass (size_t ticket, size_t number)
  {
    return parentShader->ActivatePass (ticket, number);
  }
  
  bool ForcedPriorityShader::SetupPass (size_t ticket, const CS::Graphics::RenderMesh *mesh,
    CS::Graphics::RenderMeshModes& modes,
    const csShaderVariableStack& stack)
  {
    return parentShader->SetupPass (ticket, mesh, modes,stack);
  }
  
  bool ForcedPriorityShader::TeardownPass (size_t ticket)
  {
    return parentShader->TeardownPass (ticket);
  }
  
  bool ForcedPriorityShader::DeactivatePass (size_t ticket)
  {
    return parentShader->DeactivatePass (ticket);
  }

  void ForcedPriorityShader::GetUsedShaderVars (size_t ticket, csBitArray& bits, 
						uint userFlags) const
  {
    parentShader->GetUsedShaderVars (ticket, bits, userFlags);
  }
  
  const csShaderMetadata& ForcedPriorityShader::GetMetadata () const
  {
    return parentShader->GetMetadata();
  }
  
  void ForcedPriorityShader::PushShaderVariables (csShaderVariableStack& stack,
    size_t ticket) const
  {
    parentShader->PushShaderVariables (stack, ticket);
  }
    
  size_t ForcedPriorityShader::GetPrioritiesTicket (const CS::Graphics::RenderMeshModes& modes,
    const csShaderVariableStack& stack)
  { return csArrayItemNotFound; }
  csPtr<iShaderPriorityList> ForcedPriorityShader::GetAvailablePriorities (size_t prioTicket) const
  { return 0; }
  csPtr<iString> ForcedPriorityShader::GetTechniqueMetadata (int priority, const char* dataKey) const
  { return 0; }
  csPtr<iShader> ForcedPriorityShader::ForceTechnique (int priority)
  { return 0; }
}
CS_PLUGIN_NAMESPACE_END(XMLShader)
