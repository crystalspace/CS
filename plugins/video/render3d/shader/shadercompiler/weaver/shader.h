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
#include "ivideo/shader/xmlshader.h"
#include "imap/ldrctxt.h"

#include "csutil/bitarray.h"
#include "csutil/csobject.h"
#include "csutil/dirtyaccessarray.h"

#include "snippet.h"
#include "synth.h"

CS_PLUGIN_NAMESPACE_BEGIN(ShaderWeaver)
{

class WeaverCompiler;

class WeaverShader : public scfImplementationExt3<WeaverShader,
						  csObject,
						  iShader,
						  iSelfDestruct,
						  iXMLShader>
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
  void Parse (iDocumentNode* templ,
    csArray<TechniqueKeeper>& techniquesTmp, int forcepriority,
    FileAliases& aliases);
  
  static int CompareTechniqueKeeper (TechniqueKeeper const&,
				     TechniqueKeeper const&);

  /// Shader we actually use
  csRef<iShader> realShader;
  csString filename;

protected:
  void InternalRemove() { SelfDestruct(); }

  bool GeneratePasses (iDocumentNode* passgenNode,
    const FileAliases& aliases, 
    Synthesizer::DocNodeArray& nonPassNodes,
    csArray<Synthesizer::DocNodeArray>& prePassNodes,
    csPDelArray<Snippet>& passSnippets);

  csRef<iDocument> LoadTechsFromDoc (const csArray<TechniqueKeeper>& techniques,
    const FileAliases& aliases, iDocumentNode* docSource,
    const char* cacheID, const char* cacheTag, iFile* cacheFile,
    bool& cacheState);
  csRef<iDocument> LoadTechsFromCache (iFile* cacheFile,
    const char* cacheFailReason);
  
  csRef<iDocument> DoSynthesis (iDocumentNode* source,
    iHierarchicalCache* cacheTo, int forcepriority);
public:
  CS_LEAKGUARD_DECLARE (WeaverShader);

  WeaverShader (WeaverCompiler* compiler);
  virtual ~WeaverShader();
  
  bool Load (iLoaderContext* ldr_context, iDocumentNode* source,
    int forcepriority);
  bool Precache (iDocumentNode* source, iHierarchicalCache* cacheTo);

  virtual iObject* QueryObject () 
  { return static_cast<iObject*> (static_cast<csObject*> (this)); }

  virtual const char* GetFileName() { return filename; }
  virtual void SetFileName (const char* fn) { filename = fn; }

  virtual size_t GetTicket (const CS::Graphics::RenderMeshModes& modes,
      const csShaderVariableStack& stack)
  {
    return realShader->GetTicket (modes, stack);
  }

  virtual size_t GetNumberOfPasses (size_t ticket)
  {
    return realShader->GetNumberOfPasses (ticket);
  }

  virtual bool ActivatePass (size_t ticket, size_t number)
  {
    return realShader->ActivatePass (ticket, number);
  }

  virtual bool SetupPass (size_t ticket, const CS::Graphics::RenderMesh *mesh,
    CS::Graphics::RenderMeshModes& modes,
    const csShaderVariableStack& stack)
  { 
    return realShader->SetupPass (ticket, mesh, modes, stack);
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

  virtual const csShaderMetadata& GetMetadata () const
  {
    return realShader->GetMetadata ();
  }

  virtual void GetUsedShaderVars (size_t ticket, csBitArray& bits) const
  {
    realShader->GetUsedShaderVars (ticket, bits);
  }
  
  void PushShaderVariables (csShaderVariableStack& s, size_t t) const
  {
    realShader->PushShaderVariables (s, t);
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

  csShaderVariable* GetVariable (CS::ShaderVarStringID name) const
  { 
    return realShader->GetVariable (name);
  }

  const csRefArray<csShaderVariable>& GetShaderVariables () const
  { 
    return realShader->GetShaderVariables ();
  }

  void PushVariables (csShaderVariableStack& stack) const
  { 
    realShader->PushVariables (stack);
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
  bool RemoveVariable (CS::ShaderVarStringID name)
  {
    return realShader->RemoveVariable (name);
  }
  /** @} */

  /**\name iXMLShader implementation
   * @{ */
  iDocumentNode* GetShaderSource ()
  { 
    csRef<iXMLShader> wrappedShader = scfQueryInterfaceSafe<iXMLShader> (
      realShader);
    if (wrappedShader.IsValid())
      return wrappedShader->GetShaderSource();
    return 0; 
  }
  /** @} */

public:
  csStringHash& xmltokens;
};

}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)

#endif // __CS_SHADER_H__
