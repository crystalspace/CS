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

#include "shader.h"
#include "xmlshader.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

CS_LEAKGUARD_IMPLEMENT (csXMLShader);

//---------------------------------------------------------------------------

csShaderConditionResolver::csShaderConditionResolver (
  csXMLShaderCompiler* compiler) : rootNode (0), nextVariant (0),
  evaluator (compiler->stringsSvName, compiler->condConstants)
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
  uint32 nextVarLE;
  if (cacheFile->Read ((char*)&nextVarLE, sizeof (nextVarLE))
      != sizeof (nextVarLE))
    return false;
  nextVariant = csLittleEndian::UInt32 (nextVarLE);
    
  if (!ReadNode (cacheFile, 0, rootNode)
    || !evaluator.ReadFromCache (cacheFile))
  {
    delete rootNode; rootNode = 0;
    nextVariant = 0;
    return false;
  }
  
  return true;
}

bool csShaderConditionResolver::WriteToCache (iFile* cacheFile)
{
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
  
  return evaluator.WriteToCache (cacheFile);
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
  : scfImplementationType (this), resolver (0)
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
  for (size_t i = 0; i < variants.GetSize (); i++)
  {
    delete variants[i].tech;
  }

  cs_free (filename);
  delete resolver;
  cs_free (vfsStartDir);
  cs_free (const_cast<char*> (allShaderMeta.description));
}

/* Magic value for cache file.
 * The most significant byte serves as a "version", increase when the
 * cache file format changes. */
static const uint32 cacheFileMagic = 0x00737863;

void csXMLShader::Load (iDocumentNode* source)
{
  resolver = new csShaderConditionResolver (compiler);
  
  CS::PluginCommon::ShaderCacheHelper::ShaderDocHasher hasher (
    compiler->objectreg, source);
  
  iCacheManager* shaderCache = shadermgr->GetShaderCache();
  csString shaderName (source->GetAttributeValue ("name"));
  csString cacheID_header;
  {
    csMD5::Digest sourceDigest (csMD5::Encode (CS::DocSystem::FlattenNode (source)));
    csString digestStr (sourceDigest.HexString());
    cacheID_header.Format ("%sXH", digestStr.GetData());
  }
  bool cacheValid = (shaderCache != 0) && !shaderName.IsEmpty()
    && !cacheID_header.IsEmpty();
  bool useShaderCache = cacheValid;
  
  csRef<iFile> cacheFile;
  if (useShaderCache)
  {
    useShaderCache = false;
    csRef<iDataBuffer> cacheData;
    cacheData = shaderCache->ReadCache (shaderName, cacheID_header, ~0);
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
      // Write hash stream
      csRef<iDataBuffer> hashStream = hasher.GetHashStream ();
      if (!CS::PluginCommon::ShaderCacheHelper::WriteDataBuffer (
	  cacheFile, hashStream))
	cacheFile.Invalidate();
    }
  }
  
  if (useShaderCache)
  {
    {
    // Read condition tree from cache
    useShaderCache = resolver->ReadFromCache (cacheFile);
    }
    if (useShaderCache)
    {
      csRef<iDocumentNode> wrappedNode;
      wrappedNode.AttachNew (compiler->wrapperFact->CreateWrapperFromCache (
	cacheFile, resolver, resolver->evaluator));
      useShaderCache = wrappedNode.IsValid ();
      shaderSource = wrappedNode;
      
      if (compiler->doDumpConds)
      {
	csString tree;
	tree.SetGrowsBy (0);
	resolver->DumpConditionTree (tree);
	csString filename;
	filename.Format ("/tmp/shader/cond_%s.txt", source->GetAttributeValue ("name"));
	compiler->vfs->WriteFile (filename, tree.GetData(), tree.Length ());
      }
    }
    else
    {
      // Make sure resolver is pristine
      delete resolver;
      resolver = new csShaderConditionResolver (compiler);
    }
  }
  
  if (!useShaderCache)
  {
    csRefArray<iDocumentNode> extraNodes;
    {
      static const char* const extraNodeNames[] = { "vp", "fp", "vproc", 0 };
      csRef<iDocumentNodeIterator> techNodes = source->GetNodes ("technique");
      while (techNodes->HasNext())
      {
	csRef<iDocumentNode> techNode = techNodes->Next();
	csRef<iDocumentNodeIterator> passNodes = techNode->GetNodes ("pass");
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
      }
    }
  
    csRef<csWrappedDocumentNode> wrappedNode;
    if (compiler->doDumpConds)
    {
      csString tree;
      tree.SetGrowsBy (0);
      wrappedNode.AttachNew (compiler->wrapperFact->CreateWrapper (source, 
	resolver, resolver->evaluator, extraNodes, &tree));
      resolver->DumpConditionTree (tree);
      csString filename;
      filename.Format ("/tmp/shader/cond_%s.txt", source->GetAttributeValue ("name"));
      compiler->vfs->WriteFile (filename, tree.GetData(), tree.Length ());
    }
    else
      wrappedNode.AttachNew (compiler->wrapperFact->CreateWrapper (source, 
        resolver, resolver->evaluator, extraNodes, 0));
    shaderSource = wrappedNode;
    
    if (cacheValid)
    {
      bool cacheState = resolver->WriteToCache (cacheFile);
      if (cacheState)
        cacheState = wrappedNode->StoreToCache (cacheFile);
      
      if (cacheState)
      {
        csRef<iDataBuffer> allCacheData = cacheFile->GetAllData();
	shaderCache->CacheData (allCacheData->GetData(),
	  allCacheData->GetSize(), shaderName, cacheID_header, ~0);
      }
    }
  }
  
  //Load global shadervars block
  csRef<iDocumentNode> varNode = shaderSource->GetNode(
    xmltokens.Request (csXMLShaderCompiler::XMLTOKEN_SHADERVARS));
 
  if (varNode)
    ParseGlobalSVs (ldr_context, varNode);
    
  csRef<iDocumentNode> fallbackNode = shaderSource->GetNode ("fallbackshader");
  if (fallbackNode.IsValid())
  {
    if (fallbackNode->GetAttribute ("file").IsValid())
      fallbackShader = compiler->synldr->ParseShaderRef (ldr_context,
	fallbackNode);
    else
      fallbackShader = compiler->synldr->ParseShader (ldr_context,
	fallbackNode);
  }
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
  bool RemoveVariable (csStringID name)
  { return wrappedSVC.RemoveVariable (name); }
};

void csXMLShader::ParseGlobalSVs (iLoaderContext* ldr_context,
    iDocumentNode* node)
{
  SVCWrapper wrapper (globalSVContext, shadermgr->GetSVNameStringset ()->GetSize ());
  resolver->ResetEvaluationCache();
  resolver->SetEvalParams (0, &wrapper.svStack);
  compiler->LoadSVBlock (ldr_context, node, &wrapper);
  resolver->SetEvalParams (0, 0);
}

size_t csXMLShader::GetTicket (const csRenderMeshModes& modes, 
			       const csShaderVariableStack& stack)
{
  resolver->ResetEvaluationCache();
  resolver->SetEvalParams (&modes, &stack);
  size_t vi = resolver->GetVariant ();

  if (vi != csArrayItemNotFound)
  {
    ShaderVariant& var = variants.GetExtend (vi);

    if (!var.prepared)
    {
      if (compiler->doDumpXML)
      {
	csRef<iDocumentSystem> docsys;
	docsys.AttachNew (new csTinyDocumentSystem);
	csRef<iDocument> newdoc = docsys->CreateDocument();
	CS::DocSystem::CloneNode (shaderSource, newdoc->CreateRoot());
	newdoc->Write (compiler->vfs, csString().Format ("/tmp/shader/%s_%zu.xml",
	  GetName(), vi));
      }

      // So external files are found correctly
      compiler->vfs->PushDir ();
      compiler->vfs->ChDir (vfsStartDir);

      csArray<TechniqueKeeper> techniquesTmp;
      ScanForTechniques (shaderSource, techniquesTmp, forcepriority);
		      
      csArray<TechniqueKeeper>::Iterator techIt = techniquesTmp.GetIterator ();
      while (techIt.HasNext ())
      {
	TechniqueKeeper tk = techIt.Next();
	csXMLShaderTech* tech = new csXMLShaderTech (this);
	if (tech->Load (ldr_context, tk.node, shaderSource, vi))
	{
	  if (compiler->do_verbose)
	    compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
	      "Shader '%s'<%zu>: Technique with priority %u succeeds!",
	      GetName(), vi, tk.priority);
	  var.tech = tech;
	  break;
	}
	else
	{
	  if (compiler->do_verbose)
	  {
	    compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
	      "Shader '%s'<%zu>: Technique with priority %u fails. Reason: %s.",
	      GetName(), vi, tk.priority, tech->GetFailReason());
	  }
	  delete tech;
	}
      }
      compiler->vfs->PopDir ();

      var.prepared = var.tech != 0;
    }
    if (var.tech == 0)
    {
      if (fallbackShader.IsValid())
      {
	if (compiler->do_verbose && !var.prepared)
	{
	  compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
	    "No technique validated for shader '%s'<%zu>: using fallback", 
	    GetName(), vi);
	}
	size_t fallbackTicket = fallbackShader->GetTicket (modes, stack);
	if (fallbackTicket != csArrayItemNotFound)
	{
	  size_t vc = resolver->GetVariantCount();
	  if (vc == 0) vc = 1;
	  vi = fallbackTicket + vc;
	}
	else
	  vi = csArrayItemNotFound;
      }
      else if (!var.prepared && compiler->do_verbose)
	compiler->Report (CS_REPORTER_SEVERITY_WARNING,
	  "No technique validated for shader '%s'<%zu>", GetName(), vi);
      var.prepared = true;
    }
  }
  resolver->SetEvalParams (0, 0);
  return vi;
}

bool csXMLShader::ActivatePass (size_t ticket, size_t number)
{ 
  if (IsFallbackTicket (ticket))
  {
    useFallbackContext = true;
    return fallbackShader->ActivatePass (GetFallbackTicket (ticket), number);
  }

  CS_ASSERT_MSG ("ActivatePass() has already been called.",
    activeTech == 0);
  activeTech = (ticket != csArrayItemNotFound) ? variants[ticket].tech :
    0;
  return activeTech ? activeTech->ActivatePass (number) : false;
}

bool csXMLShader::DeactivatePass (size_t ticket)
{ 
  if (IsFallbackTicket (ticket))
  {
    useFallbackContext = false;
    return fallbackShader->DeactivatePass (GetFallbackTicket (ticket));
  }

  bool ret = activeTech ? activeTech->DeactivatePass() : false; 
  activeTech = 0;
  return ret;
}

void csXMLShader::DumpStats (csString& str)
{
  if (resolver->GetVariantCount () == 0)
    str.Replace ("unvarying");
  else
    str.Format ("%zu variations", resolver->GetVariantCount ());
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
