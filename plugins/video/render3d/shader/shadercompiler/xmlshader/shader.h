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
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"
#include "ivideo/shader/xmlshader.h"
#include "imap/ldrctxt.h"

#include "csutil/bitarray.h"
#include "csutil/csobject.h"
#include "csutil/dirtyaccessarray.h"

#include "cpi/condition.h"
#include "cpi/docwrap.h"
#include "iinternal.h"
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
  void FillConditionArray (MyBitArrayMalloc& array)
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
class csShaderConditionResolver :
  public CS::Memory::CustomAllocatedDerived<iConditionResolver>
{
  csExpressionTokenizer tokenizer;
  csExpressionParser parser;
  
  csConditionNode* rootNode;
  size_t nextVariant;
  csHash<size_t, MyBitArrayMalloc> variantIDs;
  struct VariantConditionsBits
  {
    MyBitArrayMalloc conditionResults;
    MyBitArrayMalloc conditionsSet;
    
    VariantConditionsBits (const MyBitArrayMalloc& r, const MyBitArrayMalloc& s)
     : conditionResults (r), conditionsSet (s) {}
  };
  csHash<VariantConditionsBits, size_t> variantConditions;

  csRef<csConditionEvaluator::TicketEvaluator> currentEval;

  csString lastError;
  const char* SetLastError (const char* msg, ...) CS_GNUC_PRINTF (2, 3);
  csConditionNode* NewNode (csConditionNode* parent);
  csConditionNode* GetRoot ();
  
  typedef csBitArray SeenConditionsSet;
  void DumpUsedConditions (csString& out, csConditionNode* node,
    SeenConditionsSet& seenConds);
  void DumpUsedCondition (csString& out, csConditionID id,
    SeenConditionsSet& seenConds);
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

  csShaderConditionResolver (csConditionEvaluator& evaluator);
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
    const MyBitArrayTemp& conditionResultsFalse,
    const MyBitArrayTemp& conditionResultsSet);
  virtual void FinishAdding ();

  size_t GetVariant ();
  size_t GetVariantCount () const
  { return nextVariant; }
  void SetVariantEval (size_t variant);
  void DumpConditionTree (csString& out, bool includeConditions = false);
  
  void GetVariantConditions (size_t variant,
    const MyBitArrayMalloc*& conditionResults,
    const MyBitArrayMalloc*& conditionSet);

  csConditionEvaluator::TicketEvaluator* GetCurrentEval ()
  { return currentEval; }
  void SetCurrentEval (csConditionEvaluator::TicketEvaluator* eval)
  { currentEval = eval; }
  
  bool ReadFromCache (iFile* cacheFile, ConditionsReader& condReader);
  bool WriteToCache (iFile* cacheFile, ConditionsWriter& condWriter);
  
  void CollectUsedConditions (ConditionsWriter& condWrite)
  { CollectUsedConditions (rootNode, condWrite); }
};

class ForcedPriorityShader;

class csXMLShader : public scfImplementationExt4<csXMLShader,
						 csObject,
						 iShader,
						 iSelfDestruct,
						 iXMLShader,
						 iXMLShaderInternal>
{
  friend class csShaderConditionResolver;
  friend class ForcedPriorityShader;

  csRef<iDocumentNode> originalShaderDoc;
  csRef<iDocumentNode> shaderRootStripped;
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
  /* Technique variants
     These are 'lightweight' and only store which techniques (from
     'techniques' array) are used in a variant. */
  struct ShaderTechVariant
  {
    csBitArray activeTechniques;
    bool shownError;
    
    ShaderTechVariant() : shownError (false) {}
  };
  csArray<ShaderTechVariant> techVariants;
  
  // Actual techniques
  struct Technique
  {
    int priority;
    int minLights;

    csShaderConditionResolver* resolver;
    csRef<iDocumentNode> srcNode;
    csRef<csWrappedDocumentNode> techNode;
    
    csBitArray variantsPrepared;
    csArray<csXMLShaderTech*> variants;
    
    typedef csHash<csString, csString> MetadataHash;
    MetadataHash metadata;
  
    Technique() : resolver (0) {}
    void Free ()
    {
      for (size_t i = 0; i < variants.GetSize(); i++)
      {
	delete variants[i];
      }
      delete resolver;
    }
    
    bool ReadFromCache (iFile* cacheFile);
    bool WriteToCache (iFile* cacheFile);
    void ScanMetadata (iDocumentNode* node);
  };
  csArray<Technique> techniques;
  size_t allTechVariantCount;
  
  csRef<csConditionEvaluator> sharedEvaluator;
  csRef<iHierarchicalCache> shaderCache;
  csString cacheTag;
  csString cacheScope_tech;

  /// Shader we fall back to if none of the techs validate
  csRef<iShader> fallbackShader;
  csRef<iXMLShaderInternal> fallbackShaderXML;
  bool fallbackTried;
  void GetFallbackShader (iShader*& shader, iXMLShaderInternal*& xmlshader);
  
  /// Identify whether a ticker refers to the fallback shader
  bool IsFallbackTicket (size_t ticket) const
  { 
    return ticket >= allTechVariantCount;
  }
  /// Extract the fallback's ticket number
  size_t GetFallbackTicket (size_t ticket) const
  { 
    return ticket - allTechVariantCount;
  }
  bool useFallbackContext;
  
  const Technique* TechniqueForTicket (size_t ticket, size_t* remainder = 0) const
  {
    for (size_t t = 0; t < techniques.GetSize(); t++)
    {
      size_t vc = techniques[t].resolver->GetVariantCount();
      if (vc == 0) vc = 1;
      if (ticket < vc)
      {
	if (remainder) *remainder = ticket;
	return &(techniques[t]);
      }
      ticket -= vc;
    }
    return 0;
  }

  csXMLShaderTech* TechForTicket (size_t ticket) const
  {
    size_t techVar;
    const Technique* tech = TechniqueForTicket (ticket, &techVar);
    if (tech == 0) return 0;
    if (techVar >= tech->variants.GetSize()) return 0;
    return tech->variants[techVar];
  }
  size_t ComputeTicket (size_t technique, size_t techVar) const
  {
    size_t ticket = 0;
    for (size_t t = 0; t < technique; t++)
    {
      size_t vc = techniques[t].resolver->GetVariantCount();
      if (vc == 0) vc = 1;
      ticket += vc;
    }
    ticket += techVar;
    return ticket;
  }
  size_t ComputeTicketForFallback (size_t nextTicket) const
  {
    if (nextTicket == csArrayItemNotFound) return csArrayItemNotFound;
    return nextTicket + allTechVariantCount;
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

  csPtr<iDocumentNode> StripShaderRoot (iDocumentNode* shaderRoot);
  bool Load (iDocumentNode* source, bool forPrecache);
    
  void PrepareTechVars (iDocumentNode* shaderRoot,
    const csArray<TechniqueKeeper>& allTechniques, int forcepriority);
  /**
   * Computes the values of conditions for a given technique.
   * Each condition takes up _two_ bits in the result array. 10 means the
   * condition is true, 01 means it's false. Otherwise the value of the
   * condition is indefinite.
   */
  void ComputeTechniquesConditionsResults (size_t techIndex,
    MyBitArrayTemp& condResults);
  
  bool LoadTechniqueFromCache (Technique& tech,
    ForeignNodeReader& foreignNodes, iDataBuffer* cacheData, size_t techIndex);
  void LoadTechnique (Technique& tech, iDocumentNode* srcNode,
    ForeignNodeStorage& foreignNodes, csRef<iDataBuffer>& cacheData,
    bool forPrecache = true);
public:
  CS_LEAKGUARD_DECLARE (csXMLShader);

  csXMLShader (csXMLShaderCompiler* compiler,
      iLoaderContext* ldr_context, iDocumentNode* source,
      int forcepriority);
  csXMLShader (csXMLShaderCompiler* compiler);
  virtual ~csXMLShader();
  
  bool Load (iDocumentNode* source) { return Load (source, false); }
  bool Precache (iDocumentNode* source, iHierarchicalCache* cacheTo,
    bool quick);

  virtual iObject* QueryObject () 
  { return (iObject*)(csObject*)this; }

  /// Get name of the File where it was loaded from.
  const char* GetFileName ()
  { return filename; }

  /// Set name of the File where it was loaded from.
  void SetFileName (const char* filename)
  { this->filename = CS::StrDup(filename); }

  size_t GetTicketForTech (const csRenderMeshModes& modes, 
    const csShaderVariableStack& stack, size_t techNum);
  size_t GetTicketForTech (const csRenderMeshModes& modes, 
    const csShaderVariableStack& stack, csConditionEvaluator::TicketEvaluator* eval,
    int lightCount, size_t techNum);
  size_t GetTicketForTechVar (const csRenderMeshModes& modes, 
    const csShaderVariableStack& stack, csConditionEvaluator::TicketEvaluator* eval,
    int lightCount, size_t tvi);
  size_t GetTicketNoSetupInternal (const csRenderMeshModes& modes, 
    const csShaderVariableStack& stack,
    csConditionEvaluator::TicketEvaluator* eval, int lightCount);
  virtual size_t GetTicket (const CS::Graphics::RenderMeshModes& modes,
      const csShaderVariableStack& stack);

  /// Get number of passes this shader have
  virtual size_t GetNumberOfPasses (size_t ticket)
  {
    if (ticket == csArrayItemNotFound) return 0;
    if (IsFallbackTicket (ticket))
    {
      iShader* fallback;
      iXMLShaderInternal* fallbackXML;
      GetFallbackShader (fallback, fallbackXML);
      return fallback->GetNumberOfPasses (GetFallbackTicket (ticket));
    }
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
    {
      iShader* fallback;
      iXMLShaderInternal* fallbackXML;
      GetFallbackShader (fallback, fallbackXML);
      return fallback->SetupPass (GetFallbackTicket (ticket),
	mesh, modes, stack);
    }

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
    {
      iShader* fallback;
      iXMLShaderInternal* fallbackXML;
      GetFallbackShader (fallback, fallbackXML);
      return fallback->TeardownPass (GetFallbackTicket (ticket));
    }

    CS_ASSERT_MSG ("A pass must be activated prior calling TeardownPass()",
      activeTech);
    return activeTech->TeardownPass(); 
  }

  /// Completely deactivate a pass
  virtual bool DeactivatePass (size_t ticket);	

  /// Get shader metadata
  virtual const csShaderMetadata& GetMetadata () const
  {
    return allShaderMeta;
  }

  virtual void GetUsedShaderVars (size_t ticket, csBitArray& bits,
				  uint userFlags) const
  {
    if (ticket == csArrayItemNotFound) return;
    
    if (IsFallbackTicket (ticket))
    {
      fallbackShader->GetUsedShaderVars (GetFallbackTicket (ticket),
        bits, userFlags);
      return;
    }

    csXMLShaderTech* tech = TechForTicket (ticket);
    if (tech != 0) tech->GetUsedShaderVars (bits, userFlags);
  }
  
  size_t GetPrioritiesTicket (const CS::Graphics::RenderMeshModes& modes,
    const csShaderVariableStack& stack);
  csPtr<iShaderPriorityList> GetAvailablePriorities (size_t prioTicket) const;
  csPtr<iString> GetTechniqueMetadata (int priority, const char* dataKey) const;
  csPtr<iShader> ForceTechnique (int priority);

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
  virtual iDocumentNode* GetShaderSource () { return originalShaderDoc; }
  /** @} */
  
  /**\name iXMLShaderInternal implementation
   * @{ */
  virtual size_t GetTicketNoSetup (const csRenderMeshModes& modes,
    const csShaderVariableStack& stack, void* eval, int lightCount);
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
