/*
  Copyright (C) 2003 by Marten Svanfeldt

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

#ifndef __XMLSHADER_H__
#define __XMLSHADER_H__

#include "csutil/array.h"
#include "csutil/bitarray.h"
#include "csutil/csobject.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/leakguard.h"
#include "csutil/weakref.h"
#include "ivideo/material.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"
#include "iutil/document.h"
#include "iutil/strset.h"
#include "iutil/objreg.h"
#include "csplugincommon/shader/shaderplugin.h"

#include "expparser.h"
#include "docwrap.h"
#include "condeval.h"

class csXMLShaderCompiler;
class csXMLShader;

class csXMLShaderTech
{
private:
  friend class csXMLShader;

  struct shaderPass
  {
    //mix and alpha mode
    uint mixMode;
    csAlphaMode alphaMode;
    csZBufMode zMode;
    bool overrideZmode;

    shaderPass () 
    { 
      mixMode = CS_FX_MESH;
      overrideZmode = false;
      //setup default mappings
      for (unsigned int i=0; i < STREAMMAX; i++)
        defaultMappings[i] = CS_BUFFER_NONE;

      defaultMappings[CS_VATTRIB_POSITION] = CS_BUFFER_POSITION;
      defaultMappings[CS_VATTRIB_COLOR] = CS_BUFFER_COLOR;
      defaultMappings[CS_VATTRIB_TEXCOORD] = CS_BUFFER_TEXCOORD0;
    }

    enum
    {
      STREAMMAX = 16,
      TEXTUREMAX = 16
    };

    //buffer mappings
    //default mapping, index is csVertexAttrib (16 first), value is csRenderBufferName
    csRenderBufferName defaultMappings[STREAMMAX];
    csArray<csStringID> custommapping_id;
    csDirtyAccessArray<csVertexAttrib> custommaping_attrib;
    csArray<bool> custommapping_generic;
    csDirtyAccessArray<csRef<csShaderVariable> > custommapping_variables;

    //texture mappings
    csStringID textureID[TEXTUREMAX];
    csRef<csShaderVariable> textureRef[TEXTUREMAX];
    csRef<csShaderVariable> autoAlphaTexRef;
    int textureCount;

    //programs
    csRef<iShaderProgram> vp;
    csRef<iShaderProgram> fp;
    csRef<iShaderProgram> vproc;

    //writemasks
    bool wmRed, wmGreen, wmBlue, wmAlpha;

    //variable context
    csShaderVariableContext svcontext;

    csXMLShaderTech* owner;
  };

  //variable context
  csShaderVariableContext svcontext;

  //optimization stuff
  static iRenderBuffer* last_buffers[shaderPass::STREAMMAX*2];
  static iRenderBuffer* clear_buffers[shaderPass::STREAMMAX*2];
  //static csVertexAttrib vertexattributes[shaderPass::STREAMMAX*2];
  static size_t lastBufferCount;

  static iTextureHandle* last_textures[shaderPass::TEXTUREMAX];
  static iTextureHandle* clear_textures[shaderPass::TEXTUREMAX];
  static int textureUnits[shaderPass::TEXTUREMAX];
  static size_t lastTexturesCount;

  //keep this so we can reset in deactivate
  bool orig_wmRed, orig_wmGreen, orig_wmBlue, orig_wmAlpha;
  csZBufMode oldZmode;

  //Array of passes
  shaderPass* passes;
  size_t passesCount;

  size_t currentPass;

  csXMLShader* parent;
  const csStringHash& xmltokens;
  bool do_verbose;
  csString fail_reason;

  // metadata
  csShaderMetadata metadata;

  // load one pass, return false if it fails
  bool LoadPass (iDocumentNode *node, shaderPass *pass);
  // load a shaderdefinition block
  //bool LoadSVBlock (iDocumentNode *node, iShaderVariableContext *context);
  // load a shaderprogram
  csPtr<iShaderProgram> LoadProgram (iShaderTUResolver* tuResolve, iDocumentNode *node, 
    shaderPass *pass);
  // Set reason for failure.
  void SetFailReason (const char* reason, ...) CS_GNUC_PRINTF (2, 3);

  int GetPassNumber (shaderPass* pass);
public:
  CS_LEAKGUARD_DECLARE (csXMLShaderTech);

  csXMLShaderTech (csXMLShader* parent);
  ~csXMLShaderTech();

  size_t GetNumberOfPasses()
  { return passesCount; }
  bool ActivatePass (size_t number);
  bool SetupPass  (const csRenderMesh *mesh,
    csRenderMeshModes& modes,
    const csShaderVarStack &stacks);
  bool TeardownPass();
  bool DeactivatePass();

  bool Load (iDocumentNode* node, iDocumentNode* parentSV);

  const char* GetFailReason()
  { return fail_reason.GetData(); }
};

/**
 * A node in the actual binary condition tree.
 */
struct csRealConditionNode : public csRefCount
{
  csConditionID condition;
  size_t variant;

  csRealConditionNode* parent;
  csRef<csRealConditionNode> trueNode;
  csRef<csRealConditionNode> falseNode;

  csRealConditionNode (csRealConditionNode* parent)
  {
    this->parent = parent;
    condition = csCondAlwaysTrue;
    variant = csArrayItemNotFound;
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
 * A node in the condition tree.
 * 'Clients' of the condition tree only see single nodes, although
 * it may be backed by multiple nodes in the actual tree. (Happens
 * if nodes need to be inserted into multiple locations in the actual
 * tree, when multiple conditions are on the same level - the tree
 * is just binary, after all.)
 */
struct csConditionNode
{
  csRefArray<csRealConditionNode> nodes;
};

/**
 * An implementation of the callback used by csWrappedDocumentNode.
 */
class csShaderConditionResolver : public iConditionResolver
{
  csExpressionTokenizer tokenizer;
  csExpressionParser parser;
  csConditionEvaluator evaluator;
  
  csPDelArray<csConditionNode> condNodes;
  csConditionNode* rootNode;
  size_t nextVariant;
  csHash<size_t, csBitArray> variantIDs;

  const csRenderMeshModes* modes;
  const csShaderVarStack* stacks;

  csString lastError;
  const char* SetLastError (const char* msg, ...) CS_GNUC_PRINTF (2, 3);
  csConditionNode* NewNode ();
  csConditionNode* GetRoot ();
  
  void AddToRealNode (csRealConditionNode* node, csConditionID condition, 
    csConditionNode* trueNode, csConditionNode* falseNode);
  void DumpConditionNode (csRealConditionNode* node, int level);
  size_t GetVariant (csRealConditionNode* node);
public:
  csShaderConditionResolver (csXMLShaderCompiler* compiler);
  virtual ~csShaderConditionResolver ();

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
  void DumpConditionTree ();
};

class csXMLShader : public iShader, public csObject
{
  friend class csShaderConditionResolver;

  csRef<iDocumentNode> shaderSource;
  char* vfsStartDir;
  int forcepriority;

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
    size_t ticketOverride;

    ShaderVariant() 
    {
      tech = 0;
      prepared = false;
      ticketOverride = (size_t)~0;
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
  void ParseGlobalSVs ();

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

  SCF_DECLARE_IBASE_EXT (csObject);

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

  /// Completly deactivate a pass
  virtual bool DeactivatePass (size_t ticket);	

  /// Get shader metadata
  virtual const csShaderMetadata& GetMetadata (size_t ticket) const
  {
    if (IsFallbackTicket (ticket))
      return fallbackShader->GetMetadata (GetFallbackTicket (ticket));

    if (ticket == csArrayItemNotFound)
      return allShaderMeta;
    else
      return variants[ticket].tech->metadata;
  }

  friend class csXMLShaderCompiler;

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

class csXMLShaderCompiler : public iShaderCompiler, public iComponent
{
public:
  CS_LEAKGUARD_DECLARE (csXMLShaderCompiler);

  SCF_DECLARE_IBASE;
  csXMLShaderCompiler(iBase* parent);

  virtual ~csXMLShaderCompiler();

  virtual bool Initialize (iObjectRegistry* object_reg);

  /// Get a name identifying this compiler
  virtual const char* GetName() 
  { return "XMLShader"; }

  /// Compile a template into a shader. Will return 0 if it fails
  virtual csPtr<iShader> CompileShader (iDocumentNode *templ,
		  int forcepriority = -1);

  /// Validate if a template is a valid shader to this compiler
  virtual bool ValidateTemplate (iDocumentNode *templ);

  /// Check if template is parsable by this compiler
  virtual bool IsTemplateToCompiler (iDocumentNode *templ);

  /// Get a list of priorities for a given shader.
  virtual csPtr<iShaderPriorityList> GetPriorities (
		  iDocumentNode* templ);

  void Report (int severity, const char* msg, ...);

  bool LoadSVBlock (iDocumentNode *node, iShaderVariableContext *context);
public:
  bool do_verbose;
  /// XML Token and management
  csStringHash xmltokens;

  //Standard vars
  iObjectRegistry* objectreg;
  csRef<iStringSet> strings;
  csWeakRef<iGraphics3D> g3d;
  csRef<iSyntaxService> synldr;
  csRef<iVFS> vfs;
  csWrappedDocumentNodeFactory* wrapperFact;

#define CS_TOKEN_ITEM_FILE \
  "plugins/video/render3d/shader/shadercompiler/xmlshader/xmlshader.tok"
#include "cstool/tokenlist.h"
};

#endif
