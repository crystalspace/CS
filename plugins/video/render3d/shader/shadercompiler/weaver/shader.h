/*
  Copyright (C) 2003-2007 by Marten Svanfeldt
		2004-2007 by Frank Richter

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

#ifndef __CS_SHADER_H__
#define __CS_SHADER_H__

#include "iutil/selfdestruct.h"
#include "ivideo/shader/shader.h"
#include "imap/ldrctxt.h"

#include "csutil/bitarray.h"
#include "csutil/csobject.h"
#include "csutil/dirtyaccessarray.h"

#include "snippet.h"

CS_PLUGIN_NAMESPACE_BEGIN(ShaderWeaver)
{

class WeaverCompiler;

class WeaverShader : public scfImplementationExt2<WeaverShader,
						  csObject,
						  iShader,
						  iSelfDestruct>
{
  csRef<WeaverCompiler> compiler;
  csRef<iShaderManager> shadermgr;
  
  // struct to hold all techniques, until we decide which to use
  struct TechniqueKeeper
  {
    TechniqueKeeper(iDocumentNode *n, unsigned int p) : 
      node(n), priority(p), tagPriority(0)
    {}
    csRef<iDocumentNode> node;
    unsigned int priority;
    int tagPriority;
  };

  // Scan all techniques in the document.
  void ScanForTechniques (iDocumentNode* templ,
    csArray<TechniqueKeeper>& techniquesTmp, int forcepriority);
  
  static int CompareTechniqueKeeper (TechniqueKeeper const&,
				     TechniqueKeeper const&);

  /// Shader we actually use
  csRef<iShader> realShader;
  csString filename;
public:
  CS_LEAKGUARD_DECLARE (WeaverShader);

  WeaverShader (WeaverCompiler* compiler);
  virtual ~WeaverShader();
  
  bool Load (iLoaderContext* ldr_context, iDocumentNode* source,
      int forcepriority);

  virtual iObject* QueryObject () 
  { return static_cast<iObject*> (static_cast<csObject*> (this)); }

  virtual const char* GetFileName() { return filename; }
  virtual void SetFileName (const char* fn) { filename = fn; }

  virtual size_t GetTicket (const csRenderMeshModes& modes,
      const iShaderVarStack* stacks)
  {
    return realShader->GetTicket (modes, stacks);
  }

  virtual size_t GetNumberOfPasses (size_t ticket)
  {
    return realShader->GetNumberOfPasses (ticket);
  }

  virtual bool ActivatePass (size_t ticket, size_t number)
  {
    return realShader->ActivatePass (ticket, number);
  }

  virtual bool SetupPass (size_t ticket, const csRenderMesh *mesh,
    csRenderMeshModes& modes,
    const iShaderVarStack* stacks)
  { 
    return realShader->SetupPass (ticket, mesh, modes, stacks);
  }

  virtual bool TeardownPass (size_t ticket)
  { 
    return realShader->TeardownPass (ticket);
  }

  virtual bool DeactivatePass (size_t ticket)
  {
    return realShader->DeactivatePass (ticket);
  }

  virtual const csShaderMetadata& GetMetadata (size_t ticket) const
  {
    return realShader->GetMetadata (ticket);
  }

  friend class csXMLShaderCompiler;

  /**\name iSelfDestruct implementation
   * @{ */
  virtual void SelfDestruct ();
  /** @} */

  /**\name iShaderVariableContext implementation
   * @{ */
  void AddVariable (csShaderVariable *variable)
  { 
    realShader->AddVariable (variable);
  }

  csShaderVariable* GetVariable (csStringID name) const
  { 
    return realShader->GetVariable (name);
  }

  const csRefArray<csShaderVariable>& GetShaderVariables () const
  { 
    return realShader->GetShaderVariables ();
  }

  void PushVariables (iShaderVarStack* stacks) const
  { 
    realShader->PushVariables (stacks);
  }

  bool IsEmpty() const
  {
    return realShader->IsEmpty();
  }

  void ReplaceVariable (csShaderVariable *variable)
  { 
    realShader->ReplaceVariable (variable);
  }
  void Clear ()
  { 
    realShader->Clear();
  }
  bool RemoveVariable (csShaderVariable* variable)
  {
    return realShader->RemoveVariable (variable);
  }
  /** @} */

public:
  csStringHash& xmltokens;
};

}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)

#endif // __CS_SHADER_H__
