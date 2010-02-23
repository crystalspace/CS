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

#include "../xmlshader/iinternal.h"

CS_PLUGIN_NAMESPACE_BEGIN(ShaderWeaver)
{

class WeaverCompiler;

class WeaverShader : public scfImplementationExt4<WeaverShader,
						  csObject,
						  iShader,
						  iSelfDestruct,
						  iXMLShader,
						  iXMLShaderInternal>
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
  csRef<iXMLShaderInternal> realShaderXML;
  csString filename;

protected:
  void InternalRemove() { SelfDestruct(); }
  
  bool GeneratePasses (iDocumentNode* passgenNode,
    const FileAliases& aliases, 
    Synthesizer::DocNodeArray& nonPassNodes,
    csArray<Synthesizer::DocNodeArray>& prePassNodes,
    csPDelArray<Snippet>& passSnippets);

  /// Common information, used by all methods writing to/reading from shader cache
  struct CacheInfo;
  /// Try to load a shader document to pass to xmlshader from cache
  csRef<iDocument> TryLoadShader (CacheInfo& ci, iDocumentNode* source,
    iHierarchicalCache* cacheTo);
  /// Helper to validate combiner codes, stored in cacheFile at current position
  bool ValidateCombinerCodes (iFile* cacheFile, csString& cacheFailReason);
  /// Synthersize a shader and store result in cacheTo
  csRef<iDocument> SynthesizeShaderAndCache (CacheInfo& ci, iDocumentNode* source,
    iHierarchicalCache* cacheTo, int forcepriority);
  /// Synthesize a shader
  csRef<iDocument> SynthesizeShader (const csArray<TechniqueKeeper>& techniques,
				     const FileAliases& aliases,
				     iDocumentNode* docSource,
				     const char* cacheID, 
				     const char* _cacheTag,
                                     csString& cacheTag,
                                     CombinerLoaderSet& combiners);
  /// Helper to store combiner codes in cacheFile at current position
  bool WriteCombinerCodes (iFile* cacheFile, const CombinerLoaderSet& combiners);

  /// Helper to dump weaving result
  void DumpWeaved (CacheInfo& ci, iDocument* synthShader);

  /// Set up the fallback shader consisting of all techniques after the first
  void MakeFallbackShader (iDocumentNode* targetNode,
    iDocumentNode* docSource,
    const csArray<TechniqueKeeper>& techniques);
  csRef<iDocumentNode> GetNodeOrFromFile (iDocumentNode* node);
public:
  CS_LEAKGUARD_DECLARE (WeaverShader);

  WeaverShader (WeaverCompiler* compiler);
  virtual ~WeaverShader();
  
  bool Load (iLoaderContext* ldr_context, iDocumentNode* source,
    int forcepriority);
  bool Precache (iDocumentNode* source, iHierarchicalCache* cacheTo,
    bool quick);

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

  virtual const csShaderMetadata& GetMetadata () const
  {
    return realShader->GetMetadata ();
  }

  virtual void GetUsedShaderVars (size_t ticket, csBitArray& bits, 
				  uint userFlags) const
  {
    realShader->GetUsedShaderVars (ticket, bits, userFlags);
  }
  
  void PushShaderVariables (csShaderVariableStack& s, size_t t) const
  {
    realShader->PushShaderVariables (s, t);
  }
  
  size_t GetPrioritiesTicket (const CS::Graphics::RenderMeshModes& modes,
    const csShaderVariableStack& stack)
  {
    return realShader->GetPrioritiesTicket (modes, stack);
  }
  
  csPtr<iShaderPriorityList> GetAvailablePriorities (size_t ticket) const
  {
    return realShader->GetAvailablePriorities (ticket);
  }
  
  csPtr<iString> GetTechniqueMetadata (int priority, const char* dataKey) const
  {
    return realShader->GetTechniqueMetadata (priority, dataKey);
  }
  
  csPtr<iShader> ForceTechnique (int priority)
  {
    return realShader->ForceTechnique (priority);
  }

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

  /**\name iXMLShaderInternal implementation
   * @{ */
  virtual size_t GetTicketNoSetup (const csRenderMeshModes& modes, 
    const csShaderVariableStack& stack, void* eval, int lightCount)
  {
    return realShaderXML.IsValid()
      ? realShaderXML->GetTicketNoSetup (modes, stack, eval, lightCount)
      : (size_t)~0;
  }
  /** @} */
public:
  csStringHash& xmltokens;
};

}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)

#endif // __CS_SHADER_H__
