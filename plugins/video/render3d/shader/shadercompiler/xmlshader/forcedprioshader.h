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

#ifndef __FORCEDPRIOSHADER_H__
#define __FORCEDPRIOSHADER_H__

#include "shader.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{
  class ForcedPriorityShader :
    public scfImplementation1<ForcedPriorityShader,
			      iShader>
  {
    csRef<csXMLShader> parentShader;
    size_t techNum;
  public:
    ForcedPriorityShader (csXMLShader* parent, size_t techNum);
  
    void AddVariable (csShaderVariable *variable);
    csShaderVariable* GetVariable (CS::ShaderVarStringID name) const;
    const csRefArray<csShaderVariable>& GetShaderVariables () const;
    void PushVariables (csShaderVariableStack& stack) const;
    bool IsEmpty() const;
    void ReplaceVariable (csShaderVariable *variable);
    void Clear ();
    bool RemoveVariable (csShaderVariable* variable);
    bool RemoveVariable (CS::ShaderVarStringID name);
  
    iObject* QueryObject () { return parentShader->QueryObject(); }
    const char* GetFileName () { return parentShader->GetFileName(); }
    void SetFileName (const char* filename) { parentShader->SetFileName (filename); }
    
    size_t GetTicket (const CS::Graphics::RenderMeshModes& modes,
      const csShaderVariableStack& stack);
    
    size_t GetNumberOfPasses (size_t ticket);
    bool ActivatePass (size_t ticket, size_t number);
    bool SetupPass (size_t ticket, const CS::Graphics::RenderMesh *mesh,
      CS::Graphics::RenderMeshModes& modes,
      const csShaderVariableStack& stack);
    bool TeardownPass (size_t ticket);
    bool DeactivatePass (size_t ticket);
  
    void GetUsedShaderVars (size_t ticket, csBitArray& bits, uint userFlags) const;
    
    const csShaderMetadata& GetMetadata () const;
    void PushShaderVariables (csShaderVariableStack& stack,
      size_t ticket) const;
      
    size_t GetPrioritiesTicket (const CS::Graphics::RenderMeshModes& modes,
      const csShaderVariableStack& stack);
    csPtr<iShaderPriorityList> GetAvailablePriorities (size_t prioTicket) const;
    csPtr<iString> GetTechniqueMetadata (int priority, const char* dataKey) const;
    csPtr<iShader> ForceTechnique (int priority);
  };
}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#endif // __FORCEDPRIOSHADER_H__
