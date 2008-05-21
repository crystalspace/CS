/*
  Copyright (C) 2003-2006 by Marten Svanfeldt
		2005-2006 by Frank Richter

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
#include <ctype.h>

#include "imap/services.h"
#include "iutil/cache.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivideo/rendermesh.h"

#include "csplugincommon/shader/shadercachehelper.h"
#include "csutil/csendian.h"
#include "csutil/documenthelper.h"
#include "csutil/parasiticdatabuffer.h"
#include "csutil/scfarray.h"
#include "csutil/xmltiny.h"
#include "cstool/vfsdirchange.h"

#include "shader.h"
#include "xmlshader.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

CS_LEAKGUARD_IMPLEMENT (csXMLShader);

//---------------------------------------------------------------------------

csShaderConditionResolver::csShaderConditionResolver (
  csConditionEvaluator& evaluator)
  : rootNode (0), nextVariant (0), evaluator (evaluator)
{
  SetEvalParams (0, 0);
}

csShaderConditionResolver::~csShaderConditionResolver ()
{
  delete rootNode;
}

const char* csShaderConditionResolver::SetLastError (const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  lastError.FormatV (msg, args);
  va_end (args);
  return lastError;
}

const char* csShaderConditionResolver::ParseCondition (const char* str, 
						       size_t len, 
						       csConditionID& result)
{
  CondOperation op;
  const char* err = ParseCondition (str, len, op);
  if (err) return err;
  result = evaluator.FindOptimizedCondition (op);
  return 0;
}

const char* csShaderConditionResolver::ParseCondition (const char* str, 
						       size_t len, 
						       CondOperation& result)
{
  csExpressionTokenList tokens;
  const char* err = tokenizer.Tokenize (str, len, tokens);
  if (err)
    return SetLastError ("Tokenization: %s", err);
  csExpression* newExpression;
  err = parser.Parse (tokens, newExpression);
  if (err)
  {
    delete newExpression;
    return SetLastError ("Parsing: %s", err);
  }

  err = evaluator.ProcessExpression (newExpression, result);
  delete newExpression;
  if (err)
    return SetLastError ("Processing: %s", err);

  return 0;
}

bool csShaderConditionResolver::Evaluate (csConditionID condition)
{
  const csRenderMeshModes* modes = csShaderConditionResolver::modes;
  const csShaderVariableStack* stack = csShaderConditionResolver::stack;

  return evaluator.Evaluate (condition, modes ? *modes : csRenderMeshModes(),
    stack);
}

csConditionNode* csShaderConditionResolver::NewNode (csConditionNode* parent)
{
  csConditionNode* newNode = new csConditionNode (parent);
  return newNode;
}

csConditionNode* csShaderConditionResolver::GetRoot ()
{
  if (rootNode == 0)
    rootNode = NewNode (0);
  return rootNode;
}

size_t csShaderConditionResolver::GetVariant (csConditionNode* node)
{
  MyBitArrayTemp bits (evaluator.GetNumConditions ());
  node->FillConditionArray (bits);
  size_t* var = variantIDs.GetElementPointer (bits);
  if (var)
    return *var;
  else
  {
    variantIDs.Put (bits, nextVariant);
    return nextVariant++;
  }
}

void csShaderConditionResolver::AddNode (csConditionNode* parent,
					 csConditionID condition, 
					 csConditionNode*& trueNode,
					 csConditionNode*& falseNode)
{
  if (rootNode == 0)
  {
    // This is the first condition, new node gets root
    CS_ASSERT_MSG ("No root but parent? Weird.", parent == 0);
    parent = GetRoot ();

    parent->condition = condition;

    parent->trueNode = trueNode = NewNode (parent);
    trueNode->variant = GetVariant (trueNode);

    parent->falseNode = falseNode = NewNode (parent);
    falseNode->variant = GetVariant (falseNode);
  }
  else
  {
    if (parent == 0)
      parent = GetRoot ();

    parent->trueNode = trueNode = NewNode (parent);
    parent->falseNode = falseNode = NewNode (parent);

    CS_ASSERT(parent->variant != csArrayItemNotFound);
    MyBitArrayTemp bits (evaluator.GetNumConditions ());
    parent->condition = condition;

    trueNode->variant = GetVariant (trueNode);

    falseNode->variant = parent->variant;
    falseNode->FillConditionArray (bits);
    variantIDs.PutUnique (bits, parent->variant);

    parent->variant = csArrayItemNotFound;
  }
}

void csShaderConditionResolver::FinishAdding ()
{
  variantIDs.Empty();
}

void csShaderConditionResolver::SetEvalParams (const csRenderMeshModes* modes,
					       const csShaderVariableStack* stack)
{
  csShaderConditionResolver::modes = modes;
  csShaderConditionResolver::stack = stack;
}

size_t csShaderConditionResolver::GetVariant ()
{
  const csRenderMeshModes& modes = *csShaderConditionResolver::modes;
  const csShaderVariableStack* stack = csShaderConditionResolver::stack;

  if (rootNode == 0)
  {
    return 0;
  }
  else
  {
    csConditionNode* currentRoot = 0;
    csConditionNode* nextRoot = rootNode;

    while (nextRoot != 0)
    {
      currentRoot = nextRoot;
      if (evaluator.Evaluate (currentRoot->condition, modes, stack))
      {
	nextRoot = currentRoot->trueNode;
      }
      else
      {
	nextRoot = currentRoot->falseNode;
      }
    }
    CS_ASSERT (currentRoot != 0);
    return currentRoot->variant;
  }
}

void csShaderConditionResolver::DumpConditionTree (csString& out)
{
  if (rootNode == 0)
    return;

  out += "\n";
  DumpConditionNode (out, rootNode, 0);
}

static void Indent (csString& out, int n)
{
  while (n-- > 0)
    out += "| ";
}

void csShaderConditionResolver::DumpConditionNode (csString& out,
						   csConditionNode* node, 
						   int level)
{
  if (node == 0)
  {
    Indent (out, level);
    out.Append ("<none>\n");
  }
  else
  {
    if (node->variant != csArrayItemNotFound)
    {
      out.AppendFmt ("variant: %zu", node->variant);
    }
    else
    {
      out.Append ("\n");
      Indent (out, level);
      out.AppendFmt ("condition %zu = true: ", node->condition);
      DumpConditionNode (out, node->trueNode, level + 1);
      out.Append ("\n");
      Indent (out, level);
      out.AppendFmt ("condition %zu = false: ", node->condition);
      DumpConditionNode (out, node->falseNode, level + 1);
    }
  }
}
  
bool csShaderConditionResolver::ReadFromCache (iFile* cacheFile)
{
  uint32 reqCondNumLE;
  if (cacheFile->Read ((char*)&reqCondNumLE, sizeof (reqCondNumLE))
      != sizeof (reqCondNumLE))
    return false;
  if (csLittleEndian::UInt32 (reqCondNumLE)
      > evaluator.GetNumConditions())
    return false;

  uint32 nextVarLE;
  if (cacheFile->Read ((char*)&nextVarLE, sizeof (nextVarLE))
      != sizeof (nextVarLE))
    return false;
  nextVariant = csLittleEndian::UInt32 (nextVarLE);

  if (!ReadNode (cacheFile, 0, rootNode))
  {
    delete rootNode; rootNode = 0;
    nextVariant = 0;
    return false;
  }
  
  return true;
}

bool csShaderConditionResolver::WriteToCache (iFile* cacheFile)
{
  uint32 reqCondNumLE =
    csLittleEndian::UInt32 (evaluator.GetNumConditions());
  if (cacheFile->Write ((char*)&reqCondNumLE, sizeof (reqCondNumLE))
      != sizeof (reqCondNumLE))
    return false;
    
  uint32 nextVarLE = nextVariant;
  nextVarLE = csLittleEndian::UInt32 (nextVarLE);
  if (cacheFile->Write ((char*)&nextVarLE, sizeof (nextVarLE))
      != sizeof (nextVarLE))
    return false;

  if (rootNode == 0)
  {
    // Special case
    uint32 varLE = 0xffffffff;
    if (cacheFile->Write ((char*)&varLE, sizeof (varLE)) != sizeof (varLE))
      return false;
  }
  else
  {
    if (!WriteNode (cacheFile, rootNode)) return false;
  }
  
  return true;
}
  
bool csShaderConditionResolver::ReadNode (iFile* cacheFile, 
  csConditionNode* parent, csConditionNode*& node)
{
  uint32 condLE;
  if (cacheFile->Read ((char*)&condLE, sizeof (condLE)) != sizeof (condLE))
    return false;
  if (condLE == 0xffffffff)
  {
    // Special case 'no root'
    node = 0;
    return true;
  }
  node = NewNode (parent);
  condLE = csLittleEndian::UInt32 (condLE);
  if (condLE & 0x80000000)
  {
    // Leaf, ie variant
    node->variant = condLE & 0x7fffffff;
    return true;
  }
  else
  {
    // Node, ie condition
    node->condition = condLE;
    return ReadNode (cacheFile, node, node->trueNode)
      && ReadNode (cacheFile, node, node->falseNode);
  }
}

bool csShaderConditionResolver::WriteNode (iFile* cacheFile, csConditionNode* node)
{
  if (node->variant == csArrayItemNotFound)
  {
    CS_ASSERT(node->condition != csCondAlwaysTrue);
    CS_ASSERT(node->condition != csCondAlwaysFalse);
    
    uint32 condLE = node->condition;
    condLE = csLittleEndian::UInt32 (condLE);
    if (cacheFile->Write ((char*)&condLE, sizeof (condLE)) != sizeof (condLE))
      return false;
    return WriteNode (cacheFile, node->trueNode)
      && WriteNode (cacheFile, node->falseNode);
  }
  else
  {
    uint32 varLE = node->variant;
    varLE |= 0x80000000;
    varLE = csLittleEndian::UInt32 (varLE);
    return cacheFile->Write ((char*)&varLE, sizeof (varLE)) == sizeof (varLE);
  }
}

//---------------------------------------------------------------------------

csXMLShader::csXMLShader (csXMLShaderCompiler* compiler, 
    			  iLoaderContext* ldr_context,
			  iDocumentNode* source,
			  int forcepriority)
  : scfImplementationType (this), techsResolver (0),
    sharedEvaluator (new csConditionEvaluator (compiler->stringsSvName,
      compiler->condConstants)),
    cachedEvaluatorConditionNum ((size_t)-1),
    fallbackTried (false)
{
  InitTokenTable (xmltokens);

  csXMLShader::ldr_context = ldr_context;

  activeTech = 0;
  filename = 0;
  csXMLShader::compiler = compiler;
  g3d = compiler->g3d;
  csXMLShader::forcepriority = forcepriority;
  useFallbackContext = false;
  
  allShaderMeta.numberOfLights = source->GetAttributeValueAsInt ("lights");

  shadermgr = csQueryRegistry<iShaderManager> (compiler->objectreg);
  CS_ASSERT (shadermgr); // Should be present - loads us, after all
  
  vfsStartDir = CS::StrDup (compiler->vfs->GetCwd ());
  Load (source);
}

csXMLShader::~csXMLShader ()
{
  for (size_t i = 0; i < techVariants.GetSize (); i++)
  {
    techVariants[i].Free();
  }

  cs_free (filename);
  delete techsResolver;
  cs_free (vfsStartDir);
  cs_free (const_cast<char*> (allShaderMeta.description));
}

/* Magic value for cache file.
 * The most significant byte serves as a "version", increase when the
 * cache file format changes. */
static const uint32 cacheFileMagic = 0x04737863;

void csXMLShader::Load (iDocumentNode* source)
{
  techsResolver = new csShaderConditionResolver (*sharedEvaluator);
  
  CS::PluginCommon::ShaderCacheHelper::ShaderDocHasher hasher (
    compiler->objectreg, source);
  
  shaderCache = shadermgr->GetShaderCache();
  cacheType = source->GetAttributeValue ("name");
  cacheTag = source->GetAttributeValue ("_cachetag");
  csString cacheID_header;
  {
    csString cacheID_base (source->GetAttributeValue ("_cacheid"));
    if (cacheID_base.IsEmpty())
    {
      csMD5::Digest sourceDigest (csMD5::Encode (CS::DocSystem::FlattenNode (source)));
      cacheID_base = sourceDigest.HexString();
    }
    if (cacheTag.IsEmpty()) cacheTag = cacheID_base;
    cacheID_header.Format ("%sXH", cacheID_base.GetData());
    cacheScope_evaluator.Format ("%sXE", cacheID_base.GetData());
    cacheScope_tech.Format ("%sXT", cacheID_base.GetData());
  }
  bool cacheValid = (shaderCache != 0) && !cacheType.IsEmpty()
    && !cacheID_header.IsEmpty();
  bool useShaderCache = cacheValid;
  
  csRef<iFile> cacheFile;
  if (useShaderCache)
  {
    useShaderCache = false;
    csRef<iDataBuffer> cacheData;
    cacheData = shaderCache->ReadCache (cacheType, cacheID_header, ~0);
    if (cacheData.IsValid())
    {
      cacheFile.AttachNew (new csMemFile (cacheData, true));
    }
    if (cacheFile.IsValid())
    {
      do
      {
	// Read magic header
	uint32 diskMagic;
	size_t read = cacheFile->Read ((char*)&diskMagic, sizeof (diskMagic));
	if (read != sizeof (diskMagic)) break;
	if (csLittleEndian::UInt32 (diskMagic) != cacheFileMagic) break;
	
	csString cacheFileTag =
	  CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile);
	if (cacheFileTag != cacheTag) break;
	
	// Extract hash stream
	csRef<iDataBuffer> hashStream = 
	  CS::PluginCommon::ShaderCacheHelper::ReadDataBuffer (cacheFile);
	if (!hashStream.IsValid()) break;
	
	useShaderCache = hasher.ValidateHashStream (hashStream);
      }
      while (false);
    }
    if (!useShaderCache)
    {
      // Getting from cache failed, so prep for writing to cache
      cacheFile.AttachNew (new csMemFile ());
      // Write magic header
      uint32 diskMagic = csLittleEndian::UInt32 (cacheFileMagic);
      cacheFile->Write ((char*)&diskMagic, sizeof (diskMagic));
      CS::PluginCommon::ShaderCacheHelper::WriteString (cacheFile, cacheTag);
      // Write hash stream
      csRef<iDataBuffer> hashStream = hasher.GetHashStream ();
      if (!CS::PluginCommon::ShaderCacheHelper::WriteDataBuffer (
	  cacheFile, hashStream))
	cacheFile.Invalidate();
    }
  }
  
  if (useShaderCache)
  {
    useShaderCache = false;
    do
    {
      csRef<iDataBuffer> evalData;
      evalData = shaderCache->ReadCache (cacheType, cacheScope_evaluator, ~0);
      if (!evalData.IsValid()) break;
      csMemFile evalFile (evalData, true);
      uint32 diskMagic;
      size_t read = evalFile.Read ((char*)&diskMagic, sizeof (diskMagic));
      if (read != sizeof (diskMagic)) break;
      if (csLittleEndian::UInt32 (diskMagic) != cacheFileMagic) break;
      
      useShaderCache = sharedEvaluator->ReadFromCache (&evalFile, cacheTag);
      if (useShaderCache)
        cachedEvaluatorConditionNum = sharedEvaluator->GetNumConditions();
      else
      {
        delete sharedEvaluator;
        sharedEvaluator = new csConditionEvaluator (compiler->stringsSvName,
          compiler->condConstants);
      }
      csPrintf ("shader = %s\nsharedEvaluator->GetNumConditions() = %zu\n", cacheType.GetData(),
        sharedEvaluator->GetNumConditions());
    }
    while (false);
    
    if (useShaderCache)
    {
      // Read condition tree from cache
      useShaderCache = techsResolver->ReadFromCache (cacheFile);
    }
    if (useShaderCache)
    {
      csRef<iDocumentNode> wrappedNode;
      wrappedNode.AttachNew (compiler->wrapperFact->CreateWrapperFromCache (
	cacheFile, techsResolver, techsResolver->evaluator));
      useShaderCache = wrappedNode.IsValid ();
      shaderRoot = wrappedNode;
      
      if (compiler->doDumpConds)
      {
	csString tree;
	tree.SetGrowsBy (0);
	techsResolver->DumpConditionTree (tree);
	csString filename;
	filename.Format ("/tmp/shader/cond_%s_techs.txt", source->GetAttributeValue ("name"));
	compiler->vfs->WriteFile (filename, tree.GetData(), tree.Length ());
      }
    }
    else
    {
      // Make sure resolver is pristine
      delete techsResolver;
      techsResolver = new csShaderConditionResolver (*sharedEvaluator);
    }
  }
  
  if (!useShaderCache)
  {
    csRefArray<iDocumentNode> extraNodes;
    csRef<csWrappedDocumentNode> wrappedNode;
    if (compiler->doDumpConds)
    {
      csString tree;
      tree.SetGrowsBy (0);
      wrappedNode.AttachNew (compiler->wrapperFact->CreateWrapper (source, 
	techsResolver, techsResolver->evaluator, extraNodes, &tree, true));
      techsResolver->DumpConditionTree (tree);
      csString filename;
      filename.Format ("/tmp/shader/cond_%s_techs.txt",
        source->GetAttributeValue ("name"));
      compiler->vfs->WriteFile (filename, tree.GetData(), tree.Length ());
    }
    else
      wrappedNode.AttachNew (compiler->wrapperFact->CreateWrapper (source, 
        techsResolver, techsResolver->evaluator, extraNodes, 0, true));
    shaderRoot = wrappedNode;
    
    if (cacheValid)
    {
      bool cacheState = techsResolver->WriteToCache (cacheFile);
      if (cacheState)
        cacheState = wrappedNode->StoreToCache (cacheFile);
      
      if (cacheState)
      {
        csRef<iDataBuffer> allCacheData = cacheFile->GetAllData();
	shaderCache->CacheData (allCacheData->GetData(),
	  allCacheData->GetSize(), cacheType, cacheID_header, ~0);
      }
    
      if (cachedEvaluatorConditionNum != sharedEvaluator->GetNumConditions())
      {
	csMemFile cacheFile;
	uint32 diskMagic = csLittleEndian::UInt32 (cacheFileMagic);
	cacheFile.Write ((char*)&diskMagic, sizeof (diskMagic));
	if (sharedEvaluator->WriteToCache (&cacheFile, cacheTag))
	{
	  csRef<iDataBuffer> cacheData = cacheFile.GetAllData();
	  shaderCache->CacheData (cacheData->GetData(), cacheData->GetSize(),
	    cacheType, cacheScope_evaluator, ~0);
	}
	cachedEvaluatorConditionNum = sharedEvaluator->GetNumConditions();
      }
    }
  }
  readFromCache = useShaderCache;
  
  //Load global shadervars block
  csRef<iDocumentNode> varNode = shaderRoot->GetNode(
    xmltokens.Request (csXMLShaderCompiler::XMLTOKEN_SHADERVARS));
 
  if (varNode)
    ParseGlobalSVs (ldr_context, varNode);
}

void csXMLShader::SelfDestruct ()
{
  if (shadermgr)
    shadermgr->UnregisterShader ((iShader*)this);
}

int csXMLShader::CompareTechniqueKeeper (
  TechniqueKeeper const& t1, TechniqueKeeper const& t2)
{
  int v = t2.priority - t1.priority;
  if (v == 0) v = t2.tagPriority - t1.tagPriority;
  return v;
}

void csXMLShader::ScanForTechniques (iDocumentNode* templ,
		csArray<TechniqueKeeper>& techniquesTmp,
		int forcepriority)
{
  csRef<iDocumentNodeIterator> it = templ->GetNodes();

  // Read in the techniques.
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () == CS_NODE_ELEMENT &&
      xmltokens.Request (child->GetValue ()) == XMLTOKEN_TECHNIQUE)
    {
      //save it
      int p = child->GetAttributeValueAsInt ("priority");
      if ((forcepriority != -1) && (p != forcepriority)) continue;
      TechniqueKeeper keeper (child, p);
      // Compute the tag's priorities.
      csRef<iDocumentNodeIterator> tagIt = child->GetNodes ("tag");
      while (tagIt->HasNext ())
      {
	csRef<iDocumentNode> tag = tagIt->Next ();
	csStringID tagID = compiler->strings->Request (tag->GetContentsValue ());

	csShaderTagPresence presence;
	int priority;
	shadermgr->GetTagOptions (tagID, presence, priority);
	if (presence == TagNeutral)
	{
	  keeper.tagPriority += priority;
	}
      }
      techniquesTmp.Push (keeper);
    }
  }

  techniquesTmp.Sort (&CompareTechniqueKeeper);
}

iShader* csXMLShader::GetFallbackShader()
{
  if (!fallbackTried)
  {
    // So external files are found correctly
    csVfsDirectoryChanger chdir (compiler->vfs);
    chdir.ChangeTo (vfsStartDir);
  
    csRef<iDocumentNode> fallbackNode = shaderRoot->GetNode ("fallbackshader");
    if (fallbackNode.IsValid())
    {
      if (fallbackNode->GetAttribute ("file").IsValid())
	fallbackShader = compiler->synldr->ParseShaderRef (ldr_context,
	  fallbackNode);
      else
	fallbackShader = compiler->synldr->ParseShader (ldr_context,
	  fallbackNode);
    }
  
    fallbackTried = true;
  }
  return fallbackShader;
}

/**
 * A wrapped class to have a csShaderVarStack synced to an
 * iShaderVariableContext*.
 */
class SVCWrapper : public scfImplementation1<SVCWrapper, 
					     iShaderVariableContext>
{
  csShaderVariableContext& wrappedSVC;
public:
  csShaderVariableStack svStack;

  SVCWrapper (csShaderVariableContext& wrappedSVC, size_t maxSVs) : 
    scfImplementationType (this), wrappedSVC (wrappedSVC)
  {     
    svStack.Setup (maxSVs);
    wrappedSVC.PushVariables (svStack);
  }
  virtual ~SVCWrapper () { }
  virtual void AddVariable (csShaderVariable *variable)
  {
    wrappedSVC.AddVariable (variable);
  }
  virtual csShaderVariable* GetVariable (CS::ShaderVarStringID name) const
  { return wrappedSVC.GetVariable (name); }
  virtual const csRefArray<csShaderVariable>& GetShaderVariables () const
  { return wrappedSVC.GetShaderVariables (); }  
  virtual void PushVariables (csShaderVariableStack& stacks) const
  { wrappedSVC.PushVariables (stacks); }
  virtual bool IsEmpty() const
  { return wrappedSVC.IsEmpty(); }
  void ReplaceVariable (csShaderVariable *variable)
  { wrappedSVC.ReplaceVariable (variable); }
  void Clear ()
  { wrappedSVC.Clear(); }
  bool RemoveVariable (csShaderVariable *variable)
  { return wrappedSVC.RemoveVariable (variable); }
  bool RemoveVariable (CS::ShaderVarStringID name)
  { return wrappedSVC.RemoveVariable (name); }
};

void csXMLShader::ParseGlobalSVs (iLoaderContext* ldr_context,
    iDocumentNode* node)
{
  SVCWrapper wrapper (globalSVContext, shadermgr->GetSVNameStringset ()->GetSize ());
  sharedEvaluator->ResetEvaluationCache();
  techsResolver->SetEvalParams (0, &wrapper.svStack);
  compiler->LoadSVBlock (ldr_context, node, &wrapper);
  techsResolver->SetEvalParams (0, 0);
}

size_t csXMLShader::GetTicket (const csRenderMeshModes& modes, 
			       const csShaderVariableStack& stack)
{
  size_t ticket = csArrayItemNotFound;
  sharedEvaluator->ResetEvaluationCache();
  techsResolver->SetEvalParams (&modes, &stack);
  
  // So external files are found correctly
  compiler->vfs->PushDir ();
  compiler->vfs->ChDir (vfsStartDir);

  size_t tvc = techsResolver->GetVariantCount();
  if (tvc == 0) tvc = 1;
  
  size_t tvi = techsResolver->GetVariant ();
  if (tvi != csArrayItemNotFound)
  {
    // Get the techniques variant
    ShaderTechVariant& techVar = techVariants.GetExtend (tvi);
    
    if (!techVar.prepared)
    {
      csArray<TechniqueKeeper> techniquesTmp;
      ScanForTechniques (shaderRoot, techniquesTmp, forcepriority);

      /* Find a suitable technique
       * (Note that a wrapper is created for each technique node individually,
       * not the whole shader) */
      csArray<TechniqueKeeper>::Iterator techIt = techniquesTmp.GetIterator ();
      while (techIt.HasNext ())
      {
	const TechniqueKeeper& tk = techIt.Next();
	ShaderTechVariant::Technique newTech;
	newTech.priority = tk.priority;
	newTech.srcNode = tk.node;
	
	techVar.techniques.Push (newTech);
      }
    }
    
    csXMLShaderTech* usedTech = 0;
    for (size_t t = 0; t < techVar.techniques.GetSize(); t++)
    {
      ShaderTechVariant::Technique& tech = techVar.techniques[t];
      
      if (tech.resolver == 0)
      {
        tech.resolver = new csShaderConditionResolver (*sharedEvaluator);
        
        bool useCache = false;
        uint cacheID = (t * (tvc)) + tvi;
        csRef<iFile> cacheFile;
        if (readFromCache && shaderCache.IsValid())
        {
          csRef<iDataBuffer> cacheData (shaderCache->ReadCache (cacheType, 
            cacheScope_tech, cacheID));
          if (cacheData.IsValid())
            cacheFile.AttachNew (new csMemFile (cacheData, true));
        }
        if (cacheFile.IsValid())
        {
          do
          {
	    uint32 diskMagic;
	    size_t read = cacheFile->Read ((char*)&diskMagic, sizeof (diskMagic));
	    if (read != sizeof (diskMagic)) break;
	    if (csLittleEndian::UInt32 (diskMagic) != cacheFileMagic) break;
	    
	    csString cachedTag =
	      CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile);
	    if (cachedTag != cacheTag) break;
	    
	    if (!tech.resolver->ReadFromCache (cacheFile))
	    {
	      delete tech.resolver;
	      tech.resolver = new csShaderConditionResolver (*sharedEvaluator);
	      break;
	    }
	    
	    csRef<csWrappedDocumentNode> wrappedNode;
	    wrappedNode.AttachNew (compiler->wrapperFact->CreateWrapperFromCache (cacheFile,
	      tech.resolver, *sharedEvaluator));
	    if (!wrappedNode.IsValid()) break;
	    tech.techNode = wrappedNode;
	    tech.srcNode.Invalidate();
	    
	    useCache = true;
	  }
	  while (false);
        }
      
        if (!useCache)
        {
	  csRefArray<iDocumentNode> extraNodes;
	  static const char* const extraNodeNames[] = { "vp", "fp", "vproc", 0 };
	  csRef<iDocumentNodeIterator> passNodes = tech.srcNode->GetNodes ("pass");
	  while (passNodes->HasNext())
	  {
	    csRef<iDocumentNode> passNode = passNodes->Next();
	    const char* const* extraName = extraNodeNames;
	    while (*extraName)
	    {
	      csRef<iDocumentNode> node = passNode->GetNode (*extraName);
	      if (node.IsValid())
	      {
		const char* filename = node->GetAttributeValue ("file");
		if (filename != 0)
		{
		  csRef<iDocumentNode> extraNode = OpenDocFile (filename);
		  if (extraNode.IsValid()) extraNodes.Push (extraNode);
		}
	      }
	      extraName++;
	    }
	  }
	  
	  /* @@@ TODO: Some SV values are fixed from the tech determination;
	  * treat them as constant in the technique */
	  csRef<csWrappedDocumentNode> wrappedNode;
	  wrappedNode.AttachNew (compiler->wrapperFact->CreateWrapper (tech.srcNode, 
	    tech.resolver, tech.resolver->evaluator, extraNodes, 0));
	  tech.techNode = wrappedNode;
	  tech.srcNode.Invalidate();
	  
	  if (shaderCache.IsValid())
	  {
	    csRef<csMemFile> cacheFile;
	    cacheFile.AttachNew (new csMemFile);
	    uint32 diskMagic = csLittleEndian::UInt32 (cacheFileMagic);
	    if (cacheFile->Write ((char*)&diskMagic, sizeof (diskMagic))
	        != sizeof (diskMagic))
	      cacheFile.Invalidate();
	    
	    if (cacheFile.IsValid()
	        && !CS::PluginCommon::ShaderCacheHelper::WriteString (
	          cacheFile, cacheTag))
	      cacheFile.Invalidate();
	      
	    if (cacheFile.IsValid() && !tech.resolver->WriteToCache (cacheFile))
	      cacheFile.Invalidate();
	    if (cacheFile.IsValid() && !wrappedNode->StoreToCache (cacheFile))
	      cacheFile.Invalidate();
	    if (cacheFile.IsValid())
	    {
	      csRef<iDataBuffer> cacheData (cacheFile->GetAllData());
	      shaderCache->CacheData (cacheData->GetData(), cacheData->GetSize(),
	        cacheType, cacheScope_tech, cacheID);
	    }
	  }
	}
	
	if (compiler->do_verbose)
	  compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
	    "Shader '%s'<%zu>: priority %d: %zu variations",
	    GetName(), tvi, tech.priority, tech.resolver->GetVariantCount());
      }
      
      tech.resolver->SetEvalParams (&modes, &stack);
  
      //csPrintf ("shader = %s\nsharedEvaluator->GetNumConditions() = %zu\n", cacheType.GetData(),
        //sharedEvaluator->GetNumConditions());
      size_t vi = tech.resolver->GetVariant ();
      if (vi != csArrayItemNotFound)
      {
	ShaderVariant& var = tech.variants.GetExtend (vi);
	ticket = ((vi*techVar.techniques.GetSize() + t) * (tvc+1) + (tvi+1));
    
	if (!var.prepared)
	{
	  if (compiler->doDumpXML)
	  {
	    csRef<iDocumentSystem> docsys;
	    docsys.AttachNew (new csTinyDocumentSystem);
	    csRef<iDocument> newdoc = docsys->CreateDocument();
	    CS::DocSystem::CloneNode (tech.techNode, newdoc->CreateRoot());
	    newdoc->Write (compiler->vfs, csString().Format ("/tmp/shader/%s_%zu_%zu.xml",
	      GetName(), tvi, vi));
	  }
    
	  var.tech = new csXMLShaderTech (this);
	  if (var.tech->Load (ldr_context, tech.techNode, shaderRoot, ticket))
	  {
	    if (compiler->do_verbose)
	      compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
		"Shader '%s'<%zu/%zu>: Technique with priority %d succeeds!",
		GetName(), tvi, vi, tech.priority);
	  }
	  else
	  {
	    if (compiler->do_verbose)
	    {
	      compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
		"Shader '%s'<%zu/%zu>: Technique with priority %d fails. Reason: %s.",
		GetName(), tvi, vi, tech.priority, var.tech->GetFailReason());
	    }
	    delete var.tech; var.tech = 0;
	  }
	  
	  var.prepared = true;
	}
	if (var.tech != 0)
	{
          tech.resolver->SetEvalParams (0, 0);
	  usedTech = var.tech;
	  break;
	}
      }
      tech.resolver->SetEvalParams (0, 0);
    }
    
    if (usedTech == 0)
    {
      if (GetFallbackShader ())
      {
	if (compiler->do_verbose && !techVar.prepared)
	{
	  compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
	    "No technique validated for shader '%s'<%zu>: using fallback", 
	    GetName(), tvi);
	}
	size_t fallbackTicket = GetFallbackShader()->GetTicket (modes, stack);
	if (fallbackTicket != csArrayItemNotFound)
	{
	  ticket = fallbackTicket * (tvc+1);
	}
	else
	  ticket = csArrayItemNotFound;
      }
      else if (!techVar.prepared && compiler->do_verbose)
	compiler->Report (CS_REPORTER_SEVERITY_WARNING,
	  "No technique validated for shader '%s'<%zu>", GetName(), tvi);
    }
    techVar.prepared = true;
  }
  
  compiler->vfs->PopDir ();

  techsResolver->SetEvalParams (0, 0);
  
  if (shaderCache.IsValid()
      && (cachedEvaluatorConditionNum != sharedEvaluator->GetNumConditions()))
  {
    csMemFile cacheFile;
    uint32 diskMagic = csLittleEndian::UInt32 (cacheFileMagic);
    cacheFile.Write ((char*)&diskMagic, sizeof (diskMagic));
    if (sharedEvaluator->WriteToCache (&cacheFile, cacheTag))
    {
      csRef<iDataBuffer> cacheData = cacheFile.GetAllData();
      shaderCache->CacheData (cacheData->GetData(), cacheData->GetSize(),
        cacheType, cacheScope_evaluator, ~0);
    }
    cachedEvaluatorConditionNum = sharedEvaluator->GetNumConditions();
  }
  
  return ticket;
}

bool csXMLShader::ActivatePass (size_t ticket, size_t number)
{ 
  if (IsFallbackTicket (ticket))
  {
    useFallbackContext = true;
    return GetFallbackShader()->ActivatePass (GetFallbackTicket (ticket), number);
  }

  CS_ASSERT_MSG ("ActivatePass() has already been called.",
    activeTech == 0);
  activeTech = (ticket != csArrayItemNotFound) ? TechForTicket (ticket) : 0;
  return activeTech ? activeTech->ActivatePass (number) : false;
}

bool csXMLShader::DeactivatePass (size_t ticket)
{ 
  if (IsFallbackTicket (ticket))
  {
    useFallbackContext = false;
    return GetFallbackShader()->DeactivatePass (GetFallbackTicket (ticket));
  }

  bool ret = activeTech ? activeTech->DeactivatePass() : false; 
  activeTech = 0;
  return ret;
}

void csXMLShader::DumpStats (csString& str)
{
  if (techsResolver->GetVariantCount () == 0)
    str.Replace ("unvarying techs");
  else
    str.Format ("%zu tech variations", techsResolver->GetVariantCount ());
}

csRef<iDocumentNode> csXMLShader::OpenDocFile (const char* filename)
{
  csRef<iVFS> VFS =  
    csQueryRegistry<iVFS> (compiler->objectreg);
  csRef<iFile> file = VFS->Open (filename, VFS_FILE_READ);
  if (!file)
  {
    compiler->Report (CS_REPORTER_SEVERITY_ERROR,
      "Unable to open shader program file '%s'", filename);
    return 0;
  }
  csRef<iDocumentSystem> docsys (
    csQueryRegistry<iDocumentSystem> (compiler->objectreg));
  if (docsys == 0)
    docsys.AttachNew (new csTinyDocumentSystem ());

  csRef<iDocument> doc = docsys->CreateDocument ();
  const char* err = doc->Parse (file, true);
  if (err != 0)
  {
    compiler->Report (CS_REPORTER_SEVERITY_ERROR,
      "Unable to parse shader program file '%s': %s", filename, err);
    return 0;
  }

  return doc->GetRoot ();
}

csRef<iDocumentNode> csXMLShader::LoadProgramFile (const char* filename, 
                                                   size_t variant)
{
  csRef<iDocumentNode> programRoot = OpenDocFile (filename);
  if (!programRoot.IsValid()) return 0;

  csString dumpFN;
  if (compiler->doDumpConds || compiler->doDumpXML)
  {
    csString filenameClean (filename);
    for (size_t p = 0; p < filenameClean.Length (); p++)
    {
      if (!isalnum (filenameClean[p])) filenameClean[p] = '_';
    }
    dumpFN.Format ("%s_%s",
      GetName(), filenameClean.GetData());
  }
  
  size_t tvc = techsResolver->GetVariantCount();
  if (tvc == 0) tvc = 1;
  size_t techVar = (variant % (tvc+1))-1;
  size_t techAndVar = variant / (tvc+1);
  const csArray<ShaderTechVariant::Technique>& techniques =
    techVariants[techVar].techniques;
  csShaderConditionResolver* resolver =
    techniques[techAndVar % techniques.GetSize()].resolver;

  csRef<iDocumentNode> programNode;
  if (compiler->doDumpConds)
  {
    csString tree;
    programNode.AttachNew (compiler->wrapperFact->CreateWrapperStatic (
      programRoot, resolver, &tree));
    resolver->DumpConditionTree (tree);
    compiler->vfs->WriteFile (csString().Format ("/tmp/shader/%s_%zu.txt",
      dumpFN.GetData(), variant), tree.GetData(), tree.Length ());
  }
  else
    programNode.AttachNew (compiler->wrapperFact->CreateWrapperStatic (
      programRoot, resolver, 0));

  if (compiler->doDumpXML)
  {
    csRef<iDocumentSystem> docsys;
    docsys.AttachNew (new csTinyDocumentSystem);
    csRef<iDocument> newdoc = docsys->CreateDocument();
    CS::DocSystem::CloneNode (programNode, newdoc->CreateRoot());
    newdoc->Write (compiler->vfs, csString().Format ("/tmp/shader/%s_%zu.xml",
      dumpFN.GetData(), variant));
  }
  return programNode;
}

}
CS_PLUGIN_NAMESPACE_END(XMLShader)
