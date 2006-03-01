/*
  Copyright (C) 2003-2006 by Marten Svanfeldt
		2004-2006 by Frank Richter

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

#include "csutil/bitarray.h"
#include "csutil/csobject.h"
#include "csutil/dirtyaccessarray.h"

#include "condition.h"
#include "docwrap.h"
#include "shadertech.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

class csXMLShaderCompiler;
class csXMLShader;

/**
 * A node in the actual binary condition tree.
 */
struct csConditionNode
{
  csConditionID condition;
  size_t variant;

  csConditionNode* parent;
  csConditionNode* trueNode;
  csConditionNode* falseNode;

  csConditionNode (csConditionNode* parent) : parent (parent),
    trueNode (0), falseNode (0)
  {
    condition = csCondAlwaysTrue;
    variant = csArrayItemNotFound;
  }
  ~csConditionNode ()
  {
    delete trueNode;
    delete falseNode;
  }
  void FillConditionArray (csBitArray& array)
  {
    if (!parent) return;
    const csConditionID cond = parent->condition;
    if ((cond != csCondAlwaysFalse) && (cond != csCondAlwaysTrue))
      array.Set (parent->condition, this == parent->trueNode);
    parent->FillConditionArray (array);
  }
};

/**
 * An implementation of the callback used by csWrappedDocumentNode.
 */
class csShaderConditionResolver : public iConditionResolver
{
  csExpressionTokenizer tokenizer;
  csExpressionParser parser;
  
  csConditionNode* rootNode;
  size_t nextVariant;
  csHash<size_t, csBitArray> variantIDs;

  const csRenderMeshModes* modes;
  const csShaderVarStack* stacks;

  csString lastError;
  const char* SetLastError (const char* msg, ...) CS_GNUC_PRINTF (2, 3);
  csConditionNode* NewNode (csConditionNode* parent);
  csConditionNode* GetRoot ();
  
  void DumpConditionNode (csString& out, csConditionNode* node, int level);
  size_t GetVariant (csConditionNode* node);
public:
  csConditionEvaluator evaluator;

  csShaderConditionResolver (csXMLShaderCompiler* compiler);
  virtual ~csShaderConditionResolver ();

  virtual const char* ParseCondition (const char* str, size_t len, 
    CondOperation& operation);
  virtual const char* ParseCondition (const char* str, size_t len, 
    csConditionID& result);
  virtual bool Evaluate (csConditionID condition);
  virtual void AddNode (csConditionNode* parent,
    csConditionID condition, csConditionNode*& trueNode, 
    csConditionNode*& falseNode);
  void ResetEvaluationCache() { evaluator.ResetEvaluationCache(); }

  void SetEvalParams (const csRenderMeshModes* modes,
    const csShaderVarStack* stacks);
  size_t GetVariant ();
  size_t GetVariantCount () const
  { return nextVariant; }
  void DumpConditionTree (csString& out);
};

class csXMLShader : public scfImplementationExt2<csXMLShader,
						 csObject,
						 iShader,
						 iSelfDestruct>
{
  friend class csShaderConditionResolver;

  csRef<iDocumentNode> shaderSource;
  char* vfsStartDir;
  int forcepriority;
  csHash<csRef<iDocumentNode>, csString> programSources;

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

  csXMLShaderTech* activeTech;
  csShaderConditionResolver* resolver;
  struct ShaderVariant
  {
    csXMLShaderTech* tech;
    bool prepared;

    ShaderVariant() 
    {
      tech = 0;
      prepared = false;
    }
  };
  csArray<ShaderVariant> variants;

  /// Shader we fall back to if none of the techs validate
  csRef<iShader> fallbackShader;
  /// Identify whether a ticker refers to the fallback shader
  bool IsFallbackTicket (size_t ticket) const
  { 
    size_t vc = resolver->GetVariantCount();
    if (vc == 0) vc = 1;
    return ticket >= vc;
  }
  /// Extract the fallback's ticker number
  size_t GetFallbackTicket (size_t ticket) const
  { 
    size_t vc = resolver->GetVariantCount();
    if (vc == 0) vc = 1;
    return ticket - vc;
  }
  bool useFallbackContext;

  csShaderVariableContext globalSVContext;
  void ParseGlobalSVs (iDocumentNode* node);

  csShaderVariableContext& GetUsedSVContext ()
  {
    return activeTech ? activeTech->svcontext : globalSVContext;
  }
  const csShaderVariableContext& GetUsedSVContext () const
  {
    return activeTech ? activeTech->svcontext : globalSVContext;
  }
public:
  CS_LEAKGUARD_DECLARE (csXMLShader);

  csXMLShader (csXMLShaderCompiler* compiler, iDocumentNode* source,
    int forcepriority);
  virtual ~csXMLShader();

  virtual iObject* QueryObject () 
  { return (iObject*)(csObject*)this; }

  /// Get name of the File where it was loaded from.
  const char* GetFileName ()
  { return filename; }

  /// Set name of the File where it was loaded from.
  void SetFileName (const char* filename)
  { this->filename = csStrNew(filename); }

  virtual size_t GetTicket (const csRenderMeshModes& modes,
    const csShaderVarStack& stacks);

  /// Get number of passes this shader have
  virtual size_t GetNumberOfPasses (size_t ticket)
  {
    if (IsFallbackTicket (ticket))
      return fallbackShader->GetNumberOfPasses (GetFallbackTicket (ticket));
    csXMLShaderTech* tech = (ticket != csArrayItemNotFound) ? 
      variants[ticket].tech : 0;
    return tech ? tech->GetNumberOfPasses () : 0;
  }

  /// Activate a pass for rendering
  virtual bool ActivatePass (size_t ticket, size_t number);

  /// Setup a pass.
  virtual bool SetupPass (size_t ticket, const csRenderMesh *mesh,
    csRenderMeshModes& modes,
    const csShaderVarStack &stacks)
  { 
    if (IsFallbackTicket (ticket))
      return fallbackShader->SetupPass (GetFallbackTicket (ticket),
	mesh, modes, stacks);

    CS_ASSERT_MSG ("A pass must be activated prior calling SetupPass()",
      activeTech);
    return activeTech->SetupPass (mesh, modes, stacks); 
  }

  /**
   * Tear down current state, and prepare for a new mesh 
   * (for which SetupPass is called)
   */
  virtual bool TeardownPass (size_t ticket)
  { 
    if (IsFallbackTicket (ticket))
      return fallbackShader->TeardownPass (GetFallbackTicket (ticket));

    CS_ASSERT_MSG ("A pass must be activated prior calling TeardownPass()",
      activeTech);
    return activeTech->TeardownPass(); 
  }

  /// Completely deactivate a pass
  virtual bool DeactivatePass (size_t ticket);	

  /// Get shader metadata
  virtual const csShaderMetadata& GetMetadata (size_t ticket) const
  {
    if (IsFallbackTicket (ticket))
      return fallbackShader->GetMetadata (GetFallbackTicket (ticket));

    csXMLShaderTech* tech;
    if ((ticket != csArrayItemNotFound)
      && ((tech = variants[ticket].tech) != 0))
      return tech->metadata;
    else
      return allShaderMeta;
  }

  friend class csXMLShaderCompiler;

  //--------------------- iSelfDestruct implementation -------------------//

  virtual void SelfDestruct ();

  //=================== iShaderVariableContext ================//

  /// Add a variable to this context
  void AddVariable (csShaderVariable *variable)
  { 
    if (useFallbackContext)
    {
      fallbackShader->AddVariable (variable);
      return;
    }
    GetUsedSVContext().AddVariable (variable); 
  }

  /// Get a named variable from this context
  csShaderVariable* GetVariable (csStringID name) const
  { 
    if (useFallbackContext)
      return fallbackShader->GetVariable (name);
    return GetUsedSVContext().GetVariable (name); 
  }

  /// Get Array of all ShaderVariables
  const csRefArray<csShaderVariable>& GetShaderVariables () const
  { 
    if (useFallbackContext)
      return fallbackShader->GetShaderVariables();
    return GetUsedSVContext().GetShaderVariables(); 
  }

  /**
   * Push the variables of this context onto the variable stacks
   * supplied in the "stacks" argument
   */
  void PushVariables (csShaderVarStack &stacks) const
  { 
    if (useFallbackContext)
    {
      fallbackShader->PushVariables (stacks);
      return;
    }
    GetUsedSVContext().PushVariables (stacks); 
  }

  bool IsEmpty() const
  {
    if (useFallbackContext)
      return fallbackShader->IsEmpty();
    return GetUsedSVContext().IsEmpty();
  }

  void ReplaceVariable (csShaderVariable *variable)
  { 
    if (useFallbackContext)
      fallbackShader->ReplaceVariable (variable);
    GetUsedSVContext().ReplaceVariable (variable);
  }
  void Clear ()
  { 
    if (useFallbackContext)
      fallbackShader->Clear();
    GetUsedSVContext().Clear();
  }

  /// Set object description
  void SetDescription (const char *desc)
  {
    delete [] allShaderMeta.description;
    allShaderMeta.description = csStrNew (desc);
  }

  /// Return some info on this shader
  void DumpStats (csString& str);
  csRef<iDocumentNode> LoadProgramFile (const char* filename);
public:
  //Holders
  csXMLShaderCompiler* compiler;
  csWeakRef<iGraphics3D> g3d;
  csWeakRef<iShaderManager> shadermgr;
  char* filename;

  csShaderMetadata allShaderMeta;

  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE \
  "plugins/video/render3d/shader/shadercompiler/xmlshader/xmlshader.tok"
#include "cstool/tokenlist.h"
};

}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#endif // __CS_SHADER_H__
