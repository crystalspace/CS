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
#include "ivideo/shader/xmlshader.h"
#include "imap/ldrctxt.h"

#include "csutil/bitarray.h"
#include "csutil/csobject.h"
#include "csutil/dirtyaccessarray.h"

#include "cpi/condition.h"
#include "cpi/docwrap.h"
#include "shadertech.h"
#include "cpi/mybitarray.h"

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
  void FillConditionArray (MyBitArrayTemp& array)
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
  csHash<size_t, MyBitArrayTemp, TempHeapAlloc> variantIDs;
  bool keepVariantToConditionsMap;
  csHash<MyBitArrayTemp, size_t, TempHeapAlloc> variantConditions;

  const CS::Graphics::RenderMeshModes* modes;
  const csShaderVariableStack* stack;

  csString lastError;
  const char* SetLastError (const char* msg, ...) CS_GNUC_PRINTF (2, 3);
  csConditionNode* NewNode (csConditionNode* parent);
  csConditionNode* GetRoot ();
  
  void DumpConditionNode (csString& out, csConditionNode* node, int level);
  size_t GetVariant (csConditionNode* node);
  
  void CollectUsedConditions (csConditionNode* node,
    ConditionsWriter& condWrite);

  bool ReadNode (iFile* cacheFile, const ConditionsReader& condRead,
    csConditionNode* parent, csConditionNode*& node);
  bool WriteNode (iFile* cacheFile, csConditionNode* node,
    const ConditionsWriter& condWrite);
public:
  csConditionEvaluator& evaluator;

  csShaderConditionResolver (csConditionEvaluator& evaluator,
    bool keepVariantToConditionsMap);
  virtual ~csShaderConditionResolver ();

  virtual const char* ParseCondition (const char* str, size_t len, 
    CondOperation& operation);
  virtual const char* ParseCondition (const char* str, size_t len, 
    csConditionID& result);

  virtual bool Evaluate (csConditionID condition);

  virtual void AddNode (csConditionNode* parent,
    csConditionID condition, csConditionNode*& trueNode, 
    csConditionNode*& falseNode,
    const MyBitArrayTemp& conditionResultsTrue,
    const MyBitArrayTemp& conditionResultsFalse);
  virtual void FinishAdding ();

  void SetEvalParams (const CS::Graphics::RenderMeshModes* modes,
    const csShaderVariableStack* stack);
  size_t GetVariant ();
  size_t GetVariantCount () const
  { return nextVariant; }
  void SetVariant (size_t variant);
  void DumpConditionTree (csString& out);
  
  bool ReadFromCache (iFile* cacheFile, ConditionsReader& condReader);
  bool WriteToCache (iFile* cacheFile, ConditionsWriter& condWriter);
  
  void CollectUsedConditions (ConditionsWriter& condWrite)
  { CollectUsedConditions (rootNode, condWrite); }
};

class csXMLShader : public scfImplementationExt3<csXMLShader,
						 csObject,
						 iShader,
						 iSelfDestruct,
						 iXMLShader>
{
  friend class csShaderConditionResolver;

  csRef<iDocumentNode> shaderRoot;
  char* vfsStartDir;
  int forcepriority;

  // We need a reference to the loader context for delayed loading.
  csRef<iLoaderContext> ldr_context;

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
  csShaderConditionResolver* techsResolver;
  struct ShaderVariant
  {
    csXMLShaderTech* tech;
    bool prepared;

    ShaderVariant() : tech (0), prepared (false) { }
  };
  struct ShaderTechVariant
  {
    struct Technique
    {
      int priority;
      int minLights;
      csRef<csWrappedDocumentNode> srcNode;
      
      csShaderConditionResolver* resolver;
      csRef<iDocumentNode> techNode;
      csArray<ShaderVariant> variants;
    
      Technique() : resolver (0) {}
      void Free ()
      {
	for (size_t i = 0; i < variants.GetSize(); i++)
	{
	  delete variants[i].tech;
	}
        delete resolver;
      }
    };
  
    bool prepared;
    csArray<Technique> techniques;
    
    ShaderTechVariant() : /*resolver (0)*/prepared(false) {}
    void Free ()
    {
      for (size_t i = 0; i < techniques.GetSize(); i++)
      {
        techniques[i].Free();
      }
    }
  };
  csArray<ShaderTechVariant> techVariants;
  csRef<csConditionEvaluator> sharedEvaluator;
  csRef<iHierarchicalCache> shaderCache;
  csString cacheTag;
  csString cacheScope_tech;

  /// Shader we fall back to if none of the techs validate
  csRef<iShader> fallbackShader;
  bool fallbackTried;
  iShader* GetFallbackShader();
  
  /// Identify whether a ticker refers to the fallback shader
  bool IsFallbackTicket (size_t ticket) const
  { 
    size_t tvc = techsResolver->GetVariantCount();
    if (tvc == 0) tvc = 1;
    return (ticket % (tvc+1)) == 0;
  }
  /// Extract the fallback's ticket number
  size_t GetFallbackTicket (size_t ticket) const
  { 
    size_t tvc = techsResolver->GetVariantCount();
    if (tvc == 0) tvc = 1;
    return (ticket / (tvc+1));
  }
  bool useFallbackContext;
  
  csXMLShaderTech* TechForTicket (size_t ticket) const
  {
    size_t tvc = techsResolver->GetVariantCount();
    if (tvc == 0) tvc = 1;
    size_t techVar = (ticket % (tvc+1))-1;
    size_t techAndVar = ticket / (tvc+1);
    if (techVar >= techVariants.GetSize()) return 0;
    const csArray<ShaderTechVariant::Technique>& techniques =
      techVariants[techVar].techniques;
    if (techniques.GetSize() == 0) return 0;
    return techniques[techAndVar % techniques.GetSize()]
      .variants[techAndVar / techniques.GetSize()].tech;
  }

  csShaderVariableContext globalSVContext;
  void ParseGlobalSVs (iLoaderContext* ldr_context, iDocumentNode* node);

  csShaderVariableContext& GetUsedSVContext ()
  {
    return activeTech ? activeTech->svcontext : globalSVContext;
  }
  const csShaderVariableContext& GetUsedSVContext () const
  {
    return activeTech ? activeTech->svcontext : globalSVContext;
  }

protected:
  void InternalRemove() { SelfDestruct(); }

  void Load (iDocumentNode* source, bool forPrecache);
    
  void PrepareTechVar (ShaderTechVariant& techVar,
    int forcepriority);
  
  bool LoadTechniqueFromCache (ShaderTechVariant::Technique& tech,
    iHierarchicalCache* cache);
  void LoadTechnique (ShaderTechVariant::Technique& tech,
    iHierarchicalCache* cacheTo, size_t dbgTechNum,
    bool forPrecache = true);
public:
  CS_LEAKGUARD_DECLARE (csXMLShader);

  csXMLShader (csXMLShaderCompiler* compiler,
      iLoaderContext* ldr_context, iDocumentNode* source,
      int forcepriority);
  csXMLShader (csXMLShaderCompiler* compiler);
  virtual ~csXMLShader();
  
  bool Precache (iDocumentNode* source, iHierarchicalCache* cacheTo);

  virtual iObject* QueryObject () 
  { return (iObject*)(csObject*)this; }

  /// Get name of the File where it was loaded from.
  const char* GetFileName ()
  { return filename; }

  /// Set name of the File where it was loaded from.
  void SetFileName (const char* filename)
  { this->filename = CS::StrDup(filename); }

  virtual size_t GetTicket (const CS::Graphics::RenderMeshModes& modes,
      const csShaderVariableStack& stack);

  /// Get number of passes this shader have
  virtual size_t GetNumberOfPasses (size_t ticket)
  {
    if (ticket == csArrayItemNotFound) return 0;
    if (IsFallbackTicket (ticket))
      return GetFallbackShader()->GetNumberOfPasses (GetFallbackTicket (ticket));
    csXMLShaderTech* tech = TechForTicket (ticket);
    return tech ? tech->GetNumberOfPasses () : 0;
  }

  /// Activate a pass for rendering
  virtual bool ActivatePass (size_t ticket, size_t number);

  /// Setup a pass.
  virtual bool SetupPass (size_t ticket, const CS::Graphics::RenderMesh *mesh,
    CS::Graphics::RenderMeshModes& modes,
    const csShaderVariableStack& stack)
  { 
    if (IsFallbackTicket (ticket))
      return GetFallbackShader()->SetupPass (GetFallbackTicket (ticket),
	mesh, modes, stack);

    CS_ASSERT_MSG ("A pass must be activated prior calling SetupPass()",
      activeTech);
    return activeTech->SetupPass (mesh, modes, stack); 
  }

  /**
   * Tear down current state, and prepare for a new mesh 
   * (for which SetupPass is called)
   */
  virtual bool TeardownPass (size_t ticket)
  { 
    if (IsFallbackTicket (ticket))
      return GetFallbackShader()->TeardownPass (GetFallbackTicket (ticket));

    CS_ASSERT_MSG ("A pass must be activated prior calling TeardownPass()",
      activeTech);
    return activeTech->TeardownPass(); 
  }

  /// Completely deactivate a pass
  virtual bool DeactivatePass (size_t ticket);	

  /// Get shader metadata
  virtual const csShaderMetadata& GetMetadata (size_t ticket) const
  {
    return GetMetadata();
  }

  virtual const csShaderMetadata& GetMetadata () const
  {
    return allShaderMeta;
  }

  virtual void GetUsedShaderVars (size_t ticket, csBitArray& bits) const
  {
    if (ticket == csArrayItemNotFound) return;
    
    if (IsFallbackTicket (ticket))
    {
      fallbackShader->GetUsedShaderVars (GetFallbackTicket (ticket),
        bits);
      return;
    }

    csXMLShaderTech* tech = TechForTicket (ticket);
    if (tech != 0) tech->GetUsedShaderVars (bits);
  }

  friend class csXMLShaderCompiler;

  /**\name iSelfDestruct implementation
   * @{ */
  virtual void SelfDestruct ();
  /** @} */

  /**\name iShaderVariableContext implementation
   * @{ */
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
  csShaderVariable* GetVariable (CS::ShaderVarStringID name) const
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

  void PushShaderVariables (csShaderVariableStack& s, size_t t) const
  {
    if (IsFallbackTicket (t))
    {
      if (fallbackShader) fallbackShader->PushVariables (s);
      return;
    }
    GetUsedSVContext().PushVariables (s);
  }

  /**
   * Push the variables of this context onto the variable stacks
   * supplied in the "stacks" argument
   */
  void PushVariables (csShaderVariableStack& stack) const
  { 
    if (useFallbackContext)
    {
      fallbackShader->PushVariables (stack);
      return;
    }
    GetUsedSVContext().PushVariables (stack); 
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
    {
      fallbackShader->ReplaceVariable (variable);
      return;
    }
    GetUsedSVContext().ReplaceVariable (variable);
  }
  void Clear ()
  { 
    if (useFallbackContext)
    {
      fallbackShader->Clear();
      return;
    }
    GetUsedSVContext().Clear();
  }
  bool RemoveVariable (csShaderVariable* variable)
  {
    if (useFallbackContext)
      return fallbackShader->RemoveVariable (variable);
    return GetUsedSVContext().RemoveVariable (variable);
  }
  bool RemoveVariable (CS::ShaderVarStringID name)
  {
    if (useFallbackContext)
      return fallbackShader->RemoveVariable (name);
    return GetUsedSVContext().RemoveVariable (name);
  }
  /** @} */

  /**\name iXMLShader implementation
   * @{ */
  virtual iDocumentNode* GetShaderSource () { return shaderRoot; }
  /** @} */

  /// Set object description
  void SetDescription (const char *desc)
  {
    cs_free (const_cast<char*> (allShaderMeta.description));
    allShaderMeta.description = CS::StrDup (desc);
  }

  /// Return some info on this shader
  void DumpStats (csString& str);
  csRef<iDocumentNode> OpenDocFile (const char* filename);
  csRef<iDocumentNode> LoadProgramFile (const char* filename, size_t variant);
public:
  //Holders
  csRef<csXMLShaderCompiler> compiler;
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
