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
#include "iutil/hiercache.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivideo/rendermesh.h"

#include "csplugincommon/shader/shadercachehelper.h"
#include "csutil/base64.h"
#include "csutil/csendian.h"
#include "csutil/cspmeter.h"
#include "csutil/databuf.h"
#include "csutil/documenthelper.h"
#include "csutil/parasiticdatabuffer.h"
#include "csutil/scfarray.h"
#include "csutil/scfstr.h"
#include "csutil/stringquote.h"
#include "csutil/xmltiny.h"
#include "cstool/vfsdirchange.h"

#include "forcedprioshader.h"
#include "foreignnodes.h"
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
    return currentEval->Evaluate (condition);
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
    MyBitArrayMalloc bits (evaluator.GetNumConditions ());
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
    csConditionNode*& falseNode,
    const MyBitArrayTemp& conditionResultsTrue,
    const MyBitArrayTemp& conditionResultsFalse,
    const MyBitArrayTemp& conditionResultsSet)
  {
    if (rootNode == 0)
    {
      // This is the first condition, new node gets root
      CS_ASSERT_MSG ("No root but parent? Weird.", parent == 0);
      parent = GetRoot ();

      parent->condition = condition;

      parent->trueNode = trueNode = NewNode (parent);
      trueNode->variant = GetVariant (trueNode);
      variantConditions.PutUnique (trueNode->variant,
	VariantConditionsBits (conditionResultsTrue, conditionResultsSet));

      parent->falseNode = falseNode = NewNode (parent);
      falseNode->variant = GetVariant (falseNode);
      variantConditions.PutUnique (falseNode->variant,
	VariantConditionsBits (conditionResultsFalse, conditionResultsSet));
    }
    else
    {
      if (parent == 0)
        parent = GetRoot ();

      parent->trueNode = trueNode = NewNode (parent);
      parent->falseNode = falseNode = NewNode (parent);

      CS_ASSERT(parent->variant != csArrayItemNotFound);
      MyBitArrayMalloc bits (evaluator.GetNumConditions ());
      parent->condition = condition;

      trueNode->variant = GetVariant (trueNode);
      variantConditions.PutUnique (trueNode->variant,
	VariantConditionsBits (conditionResultsTrue, conditionResultsSet));

      falseNode->variant = parent->variant;
      falseNode->FillConditionArray (bits);
      variantIDs.PutUnique (bits, falseNode->variant);
      variantConditions.PutUnique (falseNode->variant,
	VariantConditionsBits (conditionResultsFalse, conditionResultsSet));

      parent->variant = csArrayItemNotFound;
    }
  }

  void csShaderConditionResolver::FinishAdding ()
  {
    variantIDs.Empty();
  }

  size_t csShaderConditionResolver::GetVariant ()
  {
    CS_ASSERT(currentEval != 0);

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
        if (currentEval->Evaluate (currentRoot->condition))
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

  void csShaderConditionResolver::SetVariantEval (size_t variant)
  {
    if (rootNode == 0) return;

    csBitArray conditionResults (evaluator.GetNumConditions());
    csBitArray conditionSet (evaluator.GetNumConditions());
    VariantConditionsBits* conditionsPtr = variantConditions.GetElementPointer (
      variant);
    CS_ASSERT(conditionsPtr);

    size_t i = 0;
    for (; i < conditionsPtr->conditionResults.GetSize(); i++)
    {
      conditionResults.Set (i, conditionsPtr->conditionResults[i]);
      conditionSet.Set (i, conditionsPtr->conditionsSet[i]);
    }
    currentEval = evaluator.BeginTicketEvaluation (conditionSet, conditionResults);
  }

  void csShaderConditionResolver::DumpConditionTree (csString& out, bool includeConditions)
  {
    if (rootNode == 0)
      return;

    if (includeConditions)
    {
      SeenConditionsSet seenConds;
      DumpUsedConditions (out, rootNode, seenConds);
    }
    out += "\n";
    DumpConditionNode (out, rootNode, 0);
  }

  void csShaderConditionResolver::DumpUsedConditions (csString& out, csConditionNode* node,
    SeenConditionsSet& seenConds)
  {
    if (node == 0) return;

    if ((node->condition != csCondAlwaysFalse)
       && (node->condition != csCondAlwaysTrue))
     DumpUsedCondition (out, node->condition, seenConds);

    DumpUsedConditions (out, node->trueNode, seenConds);
    DumpUsedConditions (out, node->falseNode, seenConds);
  }

  void csShaderConditionResolver::DumpUsedCondition (csString& out, csConditionID id,
    SeenConditionsSet& seenConds)
  {
    if ((seenConds.GetSize() > id) && (seenConds[id])) return;
    
    if (seenConds.GetSize() <= id) seenConds.SetSize (id+1);
    seenConds.SetBit (id);
	  
    const CondOperation& condOp = evaluator.GetCondition (id);
    if (condOp.left.type == operandOperation)
    {
      DumpUsedCondition (out, condOp.left.operation, seenConds);
    }
    if (condOp.right.type == operandOperation)
    {
      DumpUsedCondition (out, condOp.right.operation, seenConds);
    }
    out.AppendFmt ("condition %zu = '", id);
    out.Append (evaluator.GetConditionString (id));
    out.Append ("'\n");
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

  void csShaderConditionResolver::GetVariantConditions (size_t variant,
    const MyBitArrayMalloc*& conditionResults,
    const MyBitArrayMalloc*& conditionSet)
  {
    VariantConditionsBits* conditionsPtr = variantConditions.GetElementPointer (
      variant);
    CS_ASSERT(conditionsPtr);
    
    conditionResults = &conditionsPtr->conditionResults;
    conditionSet = &conditionsPtr->conditionsSet;
  }

  void csShaderConditionResolver::CollectUsedConditions (csConditionNode* node,
    ConditionsWriter& condWrite)
  {
    if (node == 0) return;
    if (node->condition == csCondAlwaysTrue) return;
    if (node->condition == csCondAlwaysFalse) return;

    csFIFO<csConditionID> condStack;
    condStack.Push (node->condition);
    while (condStack.GetSize() > 0)
    {
      csConditionID cond = condStack.PopTop();
      condWrite.GetDiskID (cond);
      const CondOperation& op = evaluator.GetCondition (cond);
      if (op.left.type == operandOperation) condStack.Push (op.left.operation);
      if (op.right.type == operandOperation) condStack.Push (op.right.operation);
    }
    CollectUsedConditions (node->falseNode, condWrite);
    CollectUsedConditions (node->trueNode, condWrite);
  }

  bool csShaderConditionResolver::ReadFromCache (iFile* cacheFile,
    ConditionsReader& condReader)
  {
    uint32 nextVarLE;
    if (cacheFile->Read ((char*)&nextVarLE, sizeof (nextVarLE))
      != sizeof (nextVarLE))
      return false;
    nextVariant = csLittleEndian::UInt32 (nextVarLE);

    if (!ReadNode (cacheFile, condReader, 0, rootNode))
    {
      delete rootNode; rootNode = 0;
      nextVariant = 0;
      return false;
    }

    return true;
  }

  bool csShaderConditionResolver::WriteToCache (iFile* cacheFile,
    ConditionsWriter& condWriter)
  {
    uint32 nextVarLE = (uint32)nextVariant;
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
      if (!WriteNode (cacheFile, rootNode, condWriter)) return false;
    }

    return true;
  }

  bool csShaderConditionResolver::ReadNode (iFile* cacheFile, 
    const ConditionsReader& condRead, csConditionNode* parent,
    csConditionNode*& node)
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
      node->condition = condRead.GetConditionID (condLE);
      if (node->condition == (csConditionID)~0) return false;
      return ReadNode (cacheFile, condRead, node, node->trueNode)
        && ReadNode (cacheFile, condRead, node, node->falseNode);
    }
  }

  bool csShaderConditionResolver::WriteNode (iFile* cacheFile, csConditionNode* node,
    const ConditionsWriter& condWrite)
  {
    if (node->variant == csArrayItemNotFound)
    {
      CS_ASSERT(node->condition != csCondAlwaysTrue);
      CS_ASSERT(node->condition != csCondAlwaysFalse);

      uint32 condLE = condWrite.GetDiskID (node->condition);
      condLE = csLittleEndian::UInt32 (condLE);
      if (cacheFile->Write ((char*)&condLE, sizeof (condLE)) != sizeof (condLE))
        return false;
      return WriteNode (cacheFile, node->trueNode, condWrite)
        && WriteNode (cacheFile, node->falseNode, condWrite);
    }
    else
    {
      uint32 varLE = (uint32)node->variant;
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
    sharedEvaluator (compiler->sharedEvaluator),
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
    shaderCache = shadermgr->GetShaderCache();
  }

  csXMLShader::csXMLShader (csXMLShaderCompiler* compiler)
    : scfImplementationType (this), techsResolver (0),
    sharedEvaluator (compiler->sharedEvaluator),
    fallbackTried (false)
  {
    InitTokenTable (xmltokens);

    csXMLShader::ldr_context = ldr_context;

    activeTech = 0;
    filename = 0;
    csXMLShader::compiler = compiler;
    g3d = compiler->g3d;
    csXMLShader::forcepriority = -1;
    useFallbackContext = false;

    shadermgr = csQueryRegistry<iShaderManager> (compiler->objectreg);
    CS_ASSERT (shadermgr); // Should be present - loads us, after all

    vfsStartDir = CS::StrDup (compiler->vfs->GetCwd ());
  }

  csXMLShader::~csXMLShader ()
  {
    for (size_t i = 0; i < techniques.GetSize(); i++)
    {
      techniques[i].Free();
    }

    cs_free (filename);
    delete techsResolver;
    cs_free (vfsStartDir);
    cs_free (const_cast<char*> (allShaderMeta.description));
  }

  csPtr<iDocumentNode> csXMLShader::StripShaderRoot (iDocumentNode* shaderRoot)
  {
    csRef<iDocument> doc = compiler->xmlDocSys->CreateDocument();
    csRef<iDocumentNode> docRoot = doc->CreateRoot ();
    csRef<iDocumentNode> topNode =
      docRoot->CreateNodeBefore (shaderRoot->GetType());
    topNode->SetValue (shaderRoot->GetValue());
    CS::DocSystem::CloneAttributes (shaderRoot, topNode);
    
    csRef<iDocumentNodeIterator> shaderNodesIter = shaderRoot->GetNodes();
    while (shaderNodesIter->HasNext())
    {
      csRef<iDocumentNode> child = shaderNodesIter->Next();
      csStringID id = xmltokens.Request (child->GetValue());
      if ((id != csXMLShaderCompiler::XMLTOKEN_SHADERVARS)
          && (id != csXMLShaderCompiler::XMLTOKEN_FALLBACKSHADER))
        continue;
      csRef<iDocumentNode> newNode = topNode->CreateNodeBefore (child->GetType());
      CS::DocSystem::CloneNode (child, newNode);
    }
    return csPtr<iDocumentNode> (topNode);
  }


  /* Magic value for cache file.
  * The most significant byte serves as a "version", increase when the
  * cache file format changes. */
  static const uint32 cacheFileMagic = 0x09737863;

  bool csXMLShader::Load (iDocumentNode* source, bool forPrecache)
  {
    originalShaderDoc = source;
    csRef<iDocumentNode> shaderRoot;
    techsResolver = new csShaderConditionResolver (*sharedEvaluator);

    CS::PluginCommon::ShaderCacheHelper::ShaderDocHasher hasher (
      compiler->objectreg, source);

    csString cacheType = source->GetAttributeValue ("name");
    cacheTag = source->GetAttributeValue ("_cachetag");
    bool haveProvidedCacheTag = !cacheTag.IsEmpty();
    bool forceCacheLoad = source->GetAttributeValueAsBool ("_forceCacheLoad");
    csString cacheID_header;
    {
      csString cacheID_base (source->GetAttributeValue ("_cacheid"));
      if (cacheID_base.IsEmpty())
      {
        CS::Utility::Checksum::MD5::Digest sourceDigest (
	  CS::Utility::Checksum::MD5::Encode (CS::DocSystem::FlattenNode (source)));
        cacheID_base = sourceDigest.HexString();
      }
      cacheID_header.Format ("%sXH", cacheID_base.GetData());
      cacheScope_tech.Format ("%sXT", cacheID_base.GetData());
    }
    if (shaderCache.IsValid ()) shaderCache = shaderCache->GetRootedCache (
      csString().Format ("/%s", cacheType.GetData()));
    bool cacheValid = (shaderCache != 0) && !cacheType.IsEmpty()
      && !cacheID_header.IsEmpty();
    if (!cacheValid) shaderCache.Invalidate();
    bool readFromCache = cacheValid && !forPrecache;

    csRef<iFile> cacheFile;
    if (cacheValid)
    {
      csRef<iDataBuffer> cacheData;
      if (readFromCache)
        cacheData = shaderCache->ReadCache (csString().Format ("/%s", cacheID_header.GetData()));
      if (cacheData.IsValid())
      {
        cacheFile.AttachNew (new csMemFile (cacheData, true));
      }
      else
        readFromCache = false;
      if (cacheFile.IsValid())
      {
        do
        {
          readFromCache = false;

          // Read magic header
          uint32 diskMagic;
          size_t read = cacheFile->Read ((char*)&diskMagic, sizeof (diskMagic));
          if (read != sizeof (diskMagic)) break;
          if (csLittleEndian::UInt32 (diskMagic) != cacheFileMagic) break;

          csString cacheFileTag =
            CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile);
          // Extract hash stream
          csRef<iDataBuffer> hashStream = 
            CS::PluginCommon::ShaderCacheHelper::ReadDataBuffer (cacheFile);
          if (!hashStream.IsValid()) break;

	  /* Note: empty cache tag means none was provided with the source;
	     ignoring a mismatch here is okay: if the input data changed the
	     hash stream will mismatch as well. The cache tag itself is then
	     only used to verify technique data. */
          if (haveProvidedCacheTag)
          {
	    if (cacheFileTag != cacheTag) break;
            readFromCache = true;
          }
          else
          {
	    cacheTag = cacheFileTag;

            readFromCache = hasher.ValidateHashStream (hashStream);
          }
        }
        while (false);
      }
    }

    if (cacheTag.IsEmpty())
    {
      csRef<iDataBuffer> hashStream = hasher.GetHashStream ();
      if (hashStream.IsValid())
      {
	// Hash hash stream (to get a smaller ID)
	CS::Utility::Checksum::MD5::Digest hashDigest (
	  CS::Utility::Checksum::MD5::Encode (hashStream->GetData(),
					      hashStream->GetSize()));
	/* In theory, anything would work as long as (a) it changes when
	some file the shader uses changes (b) the tag is reasonably
	unique (also over multiple program runs).
	E.g. a UUID, recomputed when the shader is 'touched',
	could do as well. */
	
	cacheTag = CS::Utility::EncodeBase64 (&hashDigest, sizeof (hashDigest));
      }
    }

    ConditionsReader* condReader = 0;
    if (readFromCache)
    {
      ForeignNodeReader techniqueNodes (compiler);
      readFromCache = techniqueNodes.StartUse (cacheFile)
        && techniqueNodes.EndUse ();

      if (readFromCache)
      {
        readFromCache = false;
        csRef<iDataBuffer> conditionsBuf =
          CS::PluginCommon::ShaderCacheHelper::ReadDataBuffer (cacheFile);
        if (conditionsBuf.IsValid())
        {
          condReader = new ConditionsReader (*sharedEvaluator, conditionsBuf);
	  readFromCache = condReader->GetStatus();
        }
      }

      if (readFromCache)
      {
        // Read condition tree from cache
        readFromCache = techsResolver->ReadFromCache (cacheFile, *condReader);
        if (readFromCache && compiler->doDumpConds)
        {
          csString tree;
          tree.SetGrowsBy (0);
          techsResolver->DumpConditionTree (tree, true);
          csString filename;
          filename.Format ("/tmp/shader/cond_%s_techs.txt", source->GetAttributeValue ("name"));
          compiler->vfs->WriteFile (filename, tree.GetData(), tree.Length ());
        }
      }
      
      if (readFromCache)
      {
	size_t tvc = techsResolver->GetVariantCount();
	if (tvc == 0) tvc = 1;
	
	for (size_t tvi = 0; (tvi < tvc) && readFromCache; tvi++)
	{
	  csRef<iDataBuffer> bitsBuf =
	    CS::PluginCommon::ShaderCacheHelper::ReadDataBuffer (cacheFile);
	    
	  if (!bitsBuf.IsValid())
	  {
	    readFromCache = false;
	    break;
	  }
	  ShaderTechVariant newVar;
	  newVar.activeTechniques = csBitArray::Unserialize (
	    bitsBuf->GetUint8(), bitsBuf->GetSize());
	  techVariants.Push (newVar);
	}
      }

      size_t numTechniques = 0;
      if (readFromCache)
      {
        uint32 diskTechNum;
        if (cacheFile->Read ((char*)&diskTechNum, sizeof (diskTechNum))
	    != sizeof (diskTechNum))
	  readFromCache = false;
	else
	  numTechniques = csLittleEndian::UInt32 (diskTechNum);
      }
	  
      csRef<iHierarchicalCache> techCache;
      for (size_t t = 0; (t < numTechniques) && readFromCache; t++)
      {
        Technique newTech;
        readFromCache = newTech.ReadFromCache (cacheFile);
	if (!readFromCache) break;
        
        csRef<iDataBuffer> techData =
	  CS::PluginCommon::ShaderCacheHelper::ReadDataBuffer (cacheFile);
        if (!techData.IsValid())
        {
          readFromCache = false;
          break;
        }
        readFromCache = LoadTechniqueFromCache (newTech, techniqueNodes,
          techData, techniques.GetSize());
        // Discard variations data from cache as well
        if (!readFromCache)
        {
	  techCache = shaderCache->GetRootedCache (
	    csString().Format ("/%s/%zu", cacheScope_tech.GetData(),
	      t));
          techCache->ClearCache ("/");
        }
        else
          techniques.Push (newTech);
      }

      if (readFromCache)
      {
	csRef<iDataBuffer> docBuf =
	  CS::PluginCommon::ShaderCacheHelper::ReadDataBuffer (cacheFile);
	if (docBuf.IsValid ())
	{
	  if (compiler->binDocSys.IsValid())
	  {
	    csRef<iDocument> doc = compiler->binDocSys->CreateDocument();
	    if (doc->Parse (docBuf) == 0)
	    {
	      csRef<iDocumentNodeIterator> nodes = doc->GetRoot()->GetNodes();
	      if (nodes->HasNext())
		shaderRootStripped = nodes->Next();
	    }
	  }
	  if (!shaderRootStripped.IsValid())
	  {
	    csRef<iDocument> doc = compiler->xmlDocSys->CreateDocument();
	    if (doc->Parse (docBuf) == 0)
	    {
	      csRef<iDocumentNodeIterator> nodes = doc->GetRoot()->GetNodes();
	      if (nodes->HasNext())
		shaderRootStripped = nodes->Next();
	    }
	  }
	  readFromCache = shaderRootStripped.IsValid();
	}
	else
	 readFromCache = false;
      }
      if (!readFromCache)
      {
        // Make sure shader is pristine
        delete techsResolver;
        techsResolver = new csShaderConditionResolver (*sharedEvaluator);
        techVariants.Empty();
        techniques.Empty();
      }
    }

    if (!readFromCache)
    {
      // shaderweaver wants us to fail loading so it can re-weave
      if (forceCacheLoad) return false;

      // Getting from cache failed, so prep for writing to cache
      cacheFile.AttachNew (new csMemFile ());
      // Write magic header
      uint32 diskMagic = csLittleEndian::UInt32 (cacheFileMagic);
      cacheFile->Write ((char*)&diskMagic, sizeof (diskMagic));
      CS::PluginCommon::ShaderCacheHelper::WriteString (cacheFile, cacheTag);
      if (haveProvidedCacheTag)
      {
        // Write empty hash stream
        if (!CS::PluginCommon::ShaderCacheHelper::WriteDataBuffer (
            cacheFile, 0))
          cacheFile.Invalidate();
      }
      else
      {
        // Write hash stream
        csRef<iDataBuffer> hashStream = hasher.GetHashStream ();
        if (!CS::PluginCommon::ShaderCacheHelper::WriteDataBuffer (
            cacheFile, hashStream))
          cacheFile.Invalidate();
      }

      // Scan techniques on node w/ expanded templates
      {
	csRef<csWrappedDocumentNode> wrappedNode;
        wrappedNode.AttachNew (
          compiler->wrapperFact->CreateWrapperStatic (source, 
          0, 0, wdnfpoExpandTemplates));
        shaderRoot = wrappedNode;
      }
      shaderRootStripped = StripShaderRoot (shaderRoot);
    
      csArray<TechniqueKeeper> techniquesTmp;
      ScanForTechniques (shaderRoot, techniquesTmp, forcepriority);

      /* Find a suitable technique
       * (Note that a wrapper is created for each technique node individually,
       * not the whole shader) */
      csArray<TechniqueKeeper>::Iterator techIt = techniquesTmp.GetIterator ();
      while (techIt.HasNext ())
      {
        const TechniqueKeeper& tk = techIt.Next();
        Technique newTech;
        newTech.priority = tk.priority;
        newTech.minLights = tk.node->GetAttributeValueAsInt ("minlights");
        newTech.ScanMetadata (tk.node);

        techniques.Push (newTech);
      }
    
      csRefArray<iDocumentNode> extraNodes;
      csRef<csWrappedDocumentNode> wrappedNode;
      if (compiler->doDumpConds)
      {
        csString tree;
        tree.SetGrowsBy (0);
        wrappedNode.AttachNew (compiler->wrapperFact->CreateWrapper (shaderRoot, 
          techsResolver, techsResolver->evaluator, extraNodes, &tree, 
          wdnfpoHandleConditions | wdnfpoOnlyOneLevelConditions
          | wdnfpoExpandTemplates));
        techsResolver->DumpConditionTree (tree);
        csString filename;
        filename.Format ("/tmp/shader/cond_%s_techs.txt",
          source->GetAttributeValue ("name"));
        compiler->vfs->WriteFile (filename, tree.GetData(), tree.Length ());
      }
      else
        wrappedNode.AttachNew (compiler->wrapperFact->CreateWrapper (shaderRoot, 
          techsResolver, techsResolver->evaluator, extraNodes, 0,
          wdnfpoHandleConditions | wdnfpoOnlyOneLevelConditions
          | wdnfpoExpandTemplates));
      shaderRoot = wrappedNode;
      
      PrepareTechVars (shaderRoot, techniquesTmp, forcepriority);
      
      ForeignNodeStorage techniqueNodes (compiler);
      if (cacheFile.IsValid() && !techniqueNodes.StartUse (cacheFile))
        cacheFile.Invalidate();

      csRefArray<iDataBuffer> techCacheData;
      for (size_t i = 0; i < techniques.GetSize(); i++)
      {
        csRef<iDataBuffer> cacheData;
        LoadTechnique (techniques[i], techniquesTmp[i].node,
          techniqueNodes, cacheData);
        techCacheData.Push (cacheData);
      }

      if (cacheFile.IsValid() && !techniqueNodes.EndUse())
        cacheFile.Invalidate();
      
      if (cacheValid && cacheFile.IsValid())
      {
        bool cacheState;
        ConditionsWriter condWriter (*sharedEvaluator);
        techsResolver->CollectUsedConditions (condWriter);
        {
          csRef<iDataBuffer> conditionsBuf = condWriter.GetPersistentData();
          cacheState = CS::PluginCommon::ShaderCacheHelper::WriteDataBuffer (
            cacheFile, conditionsBuf);
        }

        if (cacheState)
          cacheState = techsResolver->WriteToCache (cacheFile, condWriter);
	for (size_t i = 0; (i < techVariants.GetSize()) && cacheState; i++)
	{
	  const ShaderTechVariant& tv = techVariants[i];
	  size_t bitsSerSize;
	  uint8* bitsSer = tv.activeTechniques.Serialize (bitsSerSize);
	  CS::DataBuffer<> bitsBuffer ((char*)bitsSer, bitsSerSize);
	  cacheState = CS::PluginCommon::ShaderCacheHelper::WriteDataBuffer (
	    cacheFile, &bitsBuffer);
	}
        if (cacheState)
        {
	  uint32 diskTechNum = csLittleEndian::UInt32 ((uint32)techniques.GetSize());
	  cacheState = cacheFile->Write ((char*)&diskTechNum, sizeof (diskTechNum))
	    == sizeof (diskTechNum);
	    
	  for (size_t t = 0; (t < techniques.GetSize()) && cacheState; t++)
	  {
	    Technique& tech = techniques[t];
	    cacheState = tech.WriteToCache (cacheFile) &&
              CS::PluginCommon::ShaderCacheHelper::WriteDataBuffer (cacheFile,
                techCacheData[t]);
	  }
        }
        if (cacheState)
        {
          csRef<iDocumentSystem> docSys = compiler->binDocSys.IsValid()
            ? compiler->binDocSys : compiler->xmlDocSys;
	  csRef<iDocument> doc = docSys->CreateDocument();
	  csRef<iDocumentNode> docRoot = doc->CreateRoot();
	  csRef<iDocumentNode> topNode = docRoot->CreateNodeBefore (shaderRootStripped->GetType());
	  CS::DocSystem::CloneNode (shaderRootStripped, topNode);
	  csMemFile docDataFile;
	  if (doc->Write (&docDataFile) != 0)
	    cacheState = false;
	  else
	  {
	    csRef<iDataBuffer> docData = docDataFile.GetAllData();
	    cacheState = CS::PluginCommon::ShaderCacheHelper::WriteDataBuffer (
	      cacheFile, docData);
	  }
	}

        if (cacheState)
        {
          csRef<iDataBuffer> allCacheData = cacheFile->GetAllData();
          shaderCache->CacheData (allCacheData->GetData(),
            allCacheData->GetSize(), 
            csString().Format ("/%s", cacheID_header.GetData()));
        }
      }
    }

    allTechVariantCount = 0;
    for (size_t i = 0; i < techniques.GetSize(); i++)
    {
      size_t vc = techniques[i].resolver->GetVariantCount();
      if (vc == 0) vc = 1;
      allTechVariantCount += vc;
    }

    //Load global shadervars block
    csRef<iDocumentNode> varNode = shaderRootStripped->GetNode(
      xmltokens.Request (csXMLShaderCompiler::XMLTOKEN_SHADERVARS));
    if (varNode)
      ParseGlobalSVs (ldr_context, varNode);

    delete condReader;

    return true;
  }

  bool csXMLShader::Precache (iDocumentNode* source, iHierarchicalCache* cacheTo,
                              bool quick)
  {
    shaderCache = cacheTo;
    Load (source, true);
    if (!shaderCache.IsValid()) return false;

    bool result = true;
    csTextProgressMeter* progress = 0;
    if (!quick)
    {
      size_t tvc = techsResolver->GetVariantCount();
      if (tvc == 0) tvc = 1;

      size_t totalTechs = 0;

      for (size_t t = 0; t < techniques.GetSize(); t++)
      {
        Technique& tech = techniques[t];

        size_t vc = tech.resolver->GetVariantCount();
        if (vc == 0) vc = 1;
        
        if (compiler->do_verbose)
	  compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "Shader %s: priority %d: %zu variations",
	  CS::Quote::Single (GetName()), tech.priority, vc);

        totalTechs += vc;
      }

      size_t techsHandled = 0;
      csTicks startTime = csGetTicks();
      for (size_t t = 0; t < techniques.GetSize(); t++)
      {
        Technique& tech = techniques[t];

        csRef<iHierarchicalCache> techCache;
        techCache = shaderCache->GetRootedCache (
	  csString().Format ("/%s/%zu", cacheScope_tech.GetData(), t));

        size_t vc = tech.resolver->GetVariantCount();
        if (vc == 0) vc = 1;
        for (size_t vi = 0; vi < vc; vi++)
        {
	  tech.resolver->SetVariantEval (vi);

	  //size_t ticket = vi * (techniques.GetSize()+1) + (t+1);
	  //((vi*techVar.techniques.GetSize() + t) * (tvc+1) + (tvi+1));
	  size_t ticket = ComputeTicket (t, vi);

	  if (compiler->doDumpXML)
	  {
	    csRef<iDocumentSystem> docsys;
	    docsys.AttachNew (new csTinyDocumentSystem);
	    csRef<iDocument> newdoc = docsys->CreateDocument();
	    CS::DocSystem::CloneNode (tech.techNode, newdoc->CreateRoot());
	    newdoc->Write (compiler->vfs, csString().Format ("/tmp/shader/%s_%zu_%zu.xml",
	      GetName(), t, vi));
	  }

	  csRef<iHierarchicalCache> varCache;
	  varCache.AttachNew (
	    new CS::PluginCommon::ShaderCacheHelper::MicroArchiveCache (
	    techCache, csString().Format ("/%zu", vi)));

	  // So external files are found correctly
	  csVfsDirectoryChanger dirChange (compiler->vfs);
	  dirChange.ChangeTo (vfsStartDir);

	  csXMLShaderTech* xmltech = new csXMLShaderTech (this);
	  bool result = xmltech->Precache (tech.techNode, ticket, varCache);
	  if (!result)
	  {
	    if (compiler->do_verbose)
	    {
	      compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
	        "Shader %s<%zu/%zu>: Technique with priority %d fails. Reason: %s.",
	        CS::Quote::Single (GetName()), vi, tech.priority,
	        xmltech->GetFailReason());
	    }
	    result = false;
	  }
	  delete xmltech;
	  techsHandled++;
	  if (progress)
	    progress->Step (1);
	  else if (csGetTicks() - startTime > 1000)
	  {
	    progress = new csTextProgressMeter (0, (int)totalTechs);
	    progress->SetGranularity (progress->GetTickScale());
	    progress->Step ((uint)techsHandled);
	  }
	  tech.resolver->SetCurrentEval (0);
        }
      }
    }

    /* If the "fallback" node lacks a 'file' attribute it's probably an inline
    * shader, precache as well */
    csRef<iDocumentNodeIterator> fallbackNodes = source->GetNodes ("fallbackshader");
    while (fallbackNodes->HasNext())
    {
      csRef<iDocumentNode> fallbackNode (fallbackNodes->Next());

      // So external files are found correctly
      csVfsDirectoryChanger chdir (compiler->vfs);
      chdir.ChangeTo (vfsStartDir);

      const char* fileStr = fallbackNode->GetAttributeValue ("file");
      if (fileStr == 0)
      {
        const char* type = fallbackNode->GetAttributeValue ("compiler");
        if (type == 0)
          type = fallbackNode->GetAttributeValue ("type");
        if (type != 0)
        {
          csRef<iShaderCompiler> shcom = shadermgr->GetCompiler (type);
          if (shcom.IsValid()) 
          {
            result &= shcom->PrecacheShader (fallbackNode, cacheTo, quick);
          }
        }
      }
    }
    delete progress;
    return result;
  }

  size_t csXMLShader::GetPrioritiesTicket (const CS::Graphics::RenderMeshModes& modes,
    const csShaderVariableStack& stack)
  {
    csRef<csConditionEvaluator::TicketEvaluator> eval (
      sharedEvaluator->BeginTicketEvaluationCaching (modes, &stack));
    techsResolver->SetCurrentEval (eval);
    
    int lightCount = 0;
    if (stack.GetSize() > compiler->stringLightCount)
    {
      csShaderVariable* svLightCount = stack[compiler->stringLightCount];
      if (svLightCount != 0)
        svLightCount->GetValue (lightCount);
    }

    size_t tvc = techsResolver->GetVariantCount();
    if (tvc == 0) tvc = 1;
    size_t tvi = techsResolver->GetVariant ();

    techsResolver->SetCurrentEval (0);
    if (tvi != csArrayItemNotFound)
      return tvi+tvc*lightCount;
    else
      return csArrayItemNotFound;
  }
  
  csPtr<iShaderPriorityList> csXMLShader::GetAvailablePriorities (size_t prioTicket) const
  {
    if (prioTicket == csArrayItemNotFound) return 0;
  
    size_t tvc = techsResolver->GetVariantCount();
    if (tvc == 0) tvc = 1;
    int lightCount = (int)(prioTicket/tvc);
    
    csShaderPriorityList* p = new csShaderPriorityList;
    size_t tvi = prioTicket%tvc;
    
    // Get the techniques variant
    const ShaderTechVariant& techVar = techVariants[tvi];

    for (size_t t = 0; t < techniques.GetSize(); t++)
    {
      if (!techVar.activeTechniques.IsBitSet (t)) continue;
      const Technique& tech = techniques[t];
      if (lightCount < tech.minLights) continue;
      p->priorities.Push (tech.priority);
    }
    return csPtr<iShaderPriorityList> (p);
  }
  
  csPtr<iString> csXMLShader::GetTechniqueMetadata (int priority, const char* dataKey) const
  {
    for (size_t t = 0; t < techniques.GetSize(); t++)
    {
      const Technique& tech = techniques[t];
      if (tech.priority == priority)
      {
        const csString* metaStr = tech.metadata.GetElementPointer (dataKey);
        if (!metaStr || metaStr->IsEmpty())
          return 0;
        else
          return csPtr<iString> (new scfString (*metaStr));
      }
    }
    return 0;
  }

  csPtr<iShader> csXMLShader::ForceTechnique (int priority)
  {
    for (size_t t = 0; t < techniques.GetSize(); t++)
    {
      const Technique& tech = techniques[t];
      if (tech.priority == priority)
      {
        return csPtr<iShader> (new ForcedPriorityShader (this, t));
      }
    }
    return 0;
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

  void csXMLShader::GetFallbackShader (iShader*& shader, iXMLShaderInternal*& xmlshader)
  {
    if (!fallbackTried)
    {
      // So external files are found correctly
      csVfsDirectoryChanger chdir (compiler->vfs);
      chdir.ChangeTo (vfsStartDir);

      csRef<iDocumentNode> fallbackNode = shaderRootStripped->GetNode ("fallbackshader");
      if (fallbackNode.IsValid())
      {
        if (fallbackNode->GetAttribute ("file").IsValid())
          fallbackShader = compiler->synldr->ParseShaderRef (ldr_context,
          fallbackNode);
        else
          fallbackShader = compiler->synldr->ParseShader (ldr_context,
          fallbackNode);
      }
      fallbackShaderXML = scfQueryInterfaceSafe<iXMLShaderInternal> (fallbackShader);

      fallbackTried = true;
    }
    shader = fallbackShader;
    xmlshader = fallbackShaderXML;
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
    csRenderMeshModes modes;
    csRef<csConditionEvaluator::TicketEvaluator> eval (
      sharedEvaluator->BeginTicketEvaluationCaching (modes, &wrapper.svStack));
    techsResolver->SetCurrentEval (eval);
    compiler->LoadSVBlock (ldr_context, node, &wrapper);
    techsResolver->SetCurrentEval (0);
  }

  size_t csXMLShader::GetTicketForTech (const csRenderMeshModes& modes, 
    const csShaderVariableStack& stack, size_t techNum)
  {
    csRef<csConditionEvaluator::TicketEvaluator> eval (
      sharedEvaluator->BeginTicketEvaluationCaching (modes, &stack));
    
    int lightCount = 0;
    if (stack.GetSize() > compiler->stringLightCount)
    {
      csShaderVariable* svLightCount = stack[compiler->stringLightCount];
      if (svLightCount != 0)
        svLightCount->GetValue (lightCount);
    }

    return GetTicketForTech (modes, stack, eval, lightCount, techNum);
  }
    
  size_t csXMLShader::GetTicketForTech (const csRenderMeshModes& modes, 
    const csShaderVariableStack& stack, 
    csConditionEvaluator::TicketEvaluator* eval,
    int lightCount, size_t techNum)
  {
    Technique& tech = techniques[techNum];
    if (lightCount < tech.minLights) return csArrayItemNotFound;

    csRef<iHierarchicalCache> techCache;
    tech.resolver->SetCurrentEval (eval);

    size_t vi = tech.resolver->GetVariant ();
    if (vi != csArrayItemNotFound)
    {
      csXMLShaderTech*& var = tech.variants.GetExtend (vi);
      tech.variantsPrepared.SetSize (csMax (tech.variantsPrepared.GetSize(),
	vi+1));
      size_t ticket = ComputeTicket (techNum, vi);

      csRef<iHierarchicalCache> varCache;
      if (!tech.variantsPrepared[vi])
      {
	if (!techCache.IsValid() && shaderCache.IsValid())
	{
	  techCache = shaderCache->GetRootedCache (
	    csString().Format ("/%s/%zu", cacheScope_tech.GetData(), techNum));
	}

	if (techCache.IsValid())
	{
	  varCache.AttachNew (
	    new CS::PluginCommon::ShaderCacheHelper::MicroArchiveCache (
	    techCache, csString().Format ("/%zu", vi)));
	}

	if (compiler->doDumpXML)
	{
	  csRef<iDocumentSystem> docsys;
	  docsys.AttachNew (new csTinyDocumentSystem);
	  csRef<iDocument> newdoc = docsys->CreateDocument();
	  CS::DocSystem::CloneNode (tech.techNode, newdoc->CreateRoot());
	  newdoc->Write (compiler->vfs, csString().Format ("/tmp/shader/%s_%zu_%zu.xml",
	    GetName(), techNum, vi));
	}

	iShaderProgram::CacheLoadResult loadResult = iShaderProgram::loadFail;
	var = 0;
	if (techCache.IsValid())
	{
	  var = new csXMLShaderTech (this);
	  loadResult = var->LoadFromCache (ldr_context, tech.techNode,
	    varCache, shaderRootStripped, ticket);
	  if (compiler->do_verbose)
	  {
	    switch (loadResult)
	    {
	    case iShaderProgram::loadFail:
	      {
		compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
		  "Shader %s: Technique with priority %d<%zu> fails (from cache). Reason: %s.",
		  CS::Quote::Single (GetName()), tech.priority, vi, var->GetFailReason());
	      }
	      break;
	    case iShaderProgram::loadSuccessShaderInvalid:
	      {
		compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
		  "Shader %s: Technique with priority %d<%zu> succeeds (from cache) but shader is invalid.",
		  CS::Quote::Single (GetName()), tech.priority, vi);
	      }
	      break;
	    case iShaderProgram::loadSuccessShaderValid:
	      {
		compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
		  "Shader %s: Technique with priority %d<%zu> succeeds (from cache).",
		  CS::Quote::Single (GetName()), tech.priority, vi);
	      }
	      break;
	    }
	  }
	  if (loadResult != iShaderProgram::loadSuccessShaderValid)
	  {
	    delete var; var = 0;
	  }
	}

	if ((var == 0)
	  && (loadResult == iShaderProgram::loadFail))
	{
	  // So external files are found correctly
	  csVfsDirectoryChanger dirChange (compiler->vfs);
	  dirChange.ChangeTo (vfsStartDir);

	  var = new csXMLShaderTech (this);
	  bool loadResult = var->Load (ldr_context, tech.techNode, shaderRootStripped, ticket,
	    varCache);
	  if (loadResult)
	  {
	    if (compiler->do_verbose)
	      compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
	      "Shader %s: Technique with priority %d<%zu> succeeds!",
	      CS::Quote::Single (GetName()), tech.priority, vi);
	  }
	  else
	  {
	    if (compiler->do_verbose)
	    {
	      compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
		"Shader %s: Technique with priority %d<%zu> fails. Reason: %s.",
		CS::Quote::Single (GetName()), tech.priority, vi, var->GetFailReason());
	    }
	    delete var; var = 0;
	  }
	}

	tech.variantsPrepared[vi] = true;
      }
      if (var != 0)
      {
	tech.resolver->SetCurrentEval (0);
	return ticket;
      }
    }
    tech.resolver->SetCurrentEval (0);
    
    return csArrayItemNotFound;
  }
  
  size_t csXMLShader::GetTicketForTechVar (const csRenderMeshModes& modes, 
    const csShaderVariableStack& stack, csConditionEvaluator::TicketEvaluator* eval,
    int lightCount, size_t tvi)
  {
    size_t ticket = csArrayItemNotFound;

    // Get the techniques variant
    ShaderTechVariant& techVar = techVariants.GetExtend (tvi);

    csXMLShaderTech* usedTech = 0;
    for (size_t t = 0; t < techniques.GetSize(); t++)
    {
      if (!techVar.activeTechniques.IsBitSet (t)) continue;
      ticket = GetTicketForTech (modes, stack, eval, lightCount, t);
      if (ticket != csArrayItemNotFound)
      {
        usedTech = TechForTicket (ticket);
        break;
      }
    }

    if (usedTech == 0)
    {
      iShader* fallback;
      iXMLShaderInternal* fallbackXML;
      bool useShortcut = true;
      if (!fallbackTried)
      {
	/* If we're going to load a shader release the lock as loading the new
	   shader may need it. */
	eval->EndEvaluation();
	useShortcut = false;
      }
      GetFallbackShader (fallback, fallbackXML);
      if (fallback)
      {
	if (compiler->do_verbose && !techVar.shownError)
	{
	  compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
	    "No technique validated for shader %s TV %zu: using fallback", 
	    CS::Quote::Single (GetName()), tvi);
	}
	size_t fbticket = (size_t)~0;
	if (useShortcut && (fallbackXML != 0))
	  fbticket = fallbackXML->GetTicketNoSetup (modes, stack, eval,
            lightCount);
	if (fbticket == (size_t)~0)
	{
	  eval->EndEvaluation();
	  fbticket = fallback->GetTicket (modes, stack);
	}
	ticket = ComputeTicketForFallback (fbticket);
      }
      else
      {
	ticket = csArrayItemNotFound;
	if (!techVar.shownError && compiler->do_verbose)
	  compiler->Report (CS_REPORTER_SEVERITY_WARNING,
	    "No technique validated for shader %s TV %zu",
	    CS::Quote::Single (GetName()), tvi);
      }
    }
    techVar.shownError = true;

    return ticket;
  }

  size_t csXMLShader::GetTicketNoSetupInternal (const csRenderMeshModes& modes, 
    const csShaderVariableStack& stack,
    csConditionEvaluator::TicketEvaluator* eval, int lightCount)
  {
    size_t ticket = csArrayItemNotFound;
    techsResolver->SetCurrentEval (eval);

    size_t tvc = techsResolver->GetVariantCount();
    if (tvc == 0) tvc = 1;

    size_t tvi = techsResolver->GetVariant ();
    if (tvi != csArrayItemNotFound)
    {
      ticket = GetTicketForTechVar (modes, stack, eval, lightCount, tvi);
    }

    techsResolver->SetCurrentEval (0);

    return ticket;
  }

  size_t csXMLShader::GetTicket (const csRenderMeshModes& modes, 
    const csShaderVariableStack& stack)
  {
    csRef<csConditionEvaluator::TicketEvaluator> eval (
      sharedEvaluator->BeginTicketEvaluationCaching (modes, &stack));

    int lightCount = 0;
    if (stack.GetSize() > compiler->stringLightCount)
    {
      csShaderVariable* svLightCount = stack[compiler->stringLightCount];
      if (svLightCount != 0)
        svLightCount->GetValue (lightCount);
    }

    return GetTicketNoSetupInternal (modes, stack, eval, lightCount);
  }

  size_t csXMLShader::GetTicketNoSetup (const csRenderMeshModes& modes, 
    const csShaderVariableStack& stack, void* eval, int lightCount)
  {
    return GetTicketNoSetupInternal (modes, stack,
      reinterpret_cast<csConditionEvaluator::TicketEvaluator*> (eval),
      lightCount);
  }

  void csXMLShader::PrepareTechVars (iDocumentNode* shaderRoot,
				     const csArray<TechniqueKeeper>& allTechniques,
				     int forcepriority)
  {
    size_t tvc = techsResolver->GetVariantCount();
    if (tvc == 0) tvc = 1;
    
    for (size_t tvi = 0; tvi < tvc; tvi++)
    {
      /* Back up the evaluation state so forcing the variant does not 'taint'
       * the eval cache for normal evaluation */
      csRef<csConditionEvaluator::TicketEvaluator> oldeval (
	techsResolver->GetCurrentEval());
      {
	techsResolver->SetVariantEval (tvi);
	ShaderTechVariant& techVar = techVariants.GetExtend (tvi);
        
	techVar.activeTechniques.SetSize (techniques.GetSize ());
      
	csArray<TechniqueKeeper> techniquesTmp;
	ScanForTechniques (shaderRoot, techniquesTmp, forcepriority);

	csArray<TechniqueKeeper>::Iterator techIt = techniquesTmp.GetIterator ();
	while (techIt.HasNext ())
	{
	  const TechniqueKeeper& tk = techIt.Next();
	  csWrappedDocumentNode* wrapperNode =
	    static_cast<csWrappedDocumentNode*> ((iDocumentNode*)tk.node);
          
	  for (size_t t = 0; t < techniques.GetSize(); t++)
	  {
	    csWrappedDocumentNode* srcWrapperNode =
	      static_cast<csWrappedDocumentNode*> ((iDocumentNode*)allTechniques[t].node);
	    if (srcWrapperNode->Equals (wrapperNode->GetWrappedNode()))
	    {
	      techVar.activeTechniques.SetBit (t);
	      break;
	    }
	  }
	}
      }
      techsResolver->SetCurrentEval (oldeval);
    }
  }
  
  void csXMLShader::ComputeTechniquesConditionsResults (size_t techIndex,
    MyBitArrayTemp& condResults)
  {
    size_t tvc = techsResolver->GetVariantCount();
    if (tvc == 0) return;
    
    condResults.SetSize (sharedEvaluator->GetNumConditions ()*2);
    
    for (size_t tvi = 0; tvi < tvc; tvi++)
    {
      const ShaderTechVariant& techVar = techVariants[tvi];
      if (techVar.activeTechniques.IsBitSet (techIndex))
      {
	const MyBitArrayMalloc* conditionResults;
	const MyBitArrayMalloc* conditionSet;
	techsResolver->GetVariantConditions (tvi, conditionResults,
	  conditionSet);
	for (size_t c = 0; c < conditionSet->GetSize(); c++)
	{
	  if ((*conditionSet)[c])
          {
	    condResults.SetBit (2*c + ((*conditionResults)[c] ? 0 : 1));
          }
          else
          {
            /* The condition isn't set in the given tech variant. Means it's
               result has to be assumed "undefined", so set the result bits
               accordingly. */
	    condResults.SetBit (2*c);
	    condResults.SetBit (2*c+1);
          }
	}
      }
    }
  }

  bool csXMLShader::LoadTechniqueFromCache (Technique& tech,
    ForeignNodeReader& foreignNodes, iDataBuffer* cacheData,
    size_t techIndex)
  {
    csRef<iFile> cacheFile;
    cacheFile.AttachNew (new csMemFile (cacheData, true));

    uint32 diskMagic;
    size_t read = cacheFile->Read ((char*)&diskMagic, sizeof (diskMagic));
    if (read != sizeof (diskMagic)) return false;
    if (csLittleEndian::UInt32 (diskMagic) != cacheFileMagic) return false;

    csString cachedTag =
      CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile);
    if (cachedTag != cacheTag) return false;

    csRef<iDataBuffer> conditionsBuf =
      CS::PluginCommon::ShaderCacheHelper::ReadDataBuffer (cacheFile);
    if (!conditionsBuf.IsValid()) return false;

    ConditionsReader condReader (*sharedEvaluator, conditionsBuf);

    tech.resolver = new csShaderConditionResolver (*sharedEvaluator);
    if (!tech.resolver->ReadFromCache (cacheFile, condReader))
    {
      delete tech.resolver;
      return false;
    }

    csRef<csWrappedDocumentNode> wrappedNode;
    if (compiler->doDumpConds)
    {
      csString tree;
      tree.SetGrowsBy (0);

      wrappedNode.AttachNew (compiler->wrapperFact->CreateWrapperFromCache (cacheFile,
	tech.resolver, *sharedEvaluator, foreignNodes, condReader, &tree));

      tech.resolver->DumpConditionTree (tree);
      csString filename;
      filename.Format ("/tmp/shader/cond_%s_%zu.txt",
        originalShaderDoc->GetAttributeValue ("name"),
        techIndex);
      compiler->vfs->WriteFile (filename, tree.GetData(), tree.Length ());
    }
    else
      wrappedNode.AttachNew (compiler->wrapperFact->CreateWrapperFromCache (cacheFile,
	tech.resolver, *sharedEvaluator, foreignNodes, condReader, 0));
    if (!wrappedNode.IsValid()) return false;
    tech.techNode = wrappedNode;

    return true;
  }

  void csXMLShader::LoadTechnique (Technique& tech, iDocumentNode* srcNode,
    ForeignNodeStorage& foreignNodes, csRef<iDataBuffer>& cacheData,
    bool forPrecache)
  {
    size_t techIndex = techniques.GetIndex (&tech);
    tech.resolver = new csShaderConditionResolver (*sharedEvaluator);

    csRefArray<iDocumentNode> extraNodes;
    static const char* const extraNodeNames[] = { "vp", "fp", "vproc", 0 };
    csRef<iDocumentNodeIterator> passNodes = srcNode->GetNodes ("pass");
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

    // So external files are found correctly
    csVfsDirectoryChanger dirChange (compiler->vfs);
    dirChange.ChangeTo (vfsStartDir);

    csRef<csWrappedDocumentNode> wrappedNode;
    
    /* Some condition results are fixed from the tech determination;
     * pass these to the technique node wrapping */
    MyBitArrayTemp condResults;
    ComputeTechniquesConditionsResults (techIndex, condResults);

    if (compiler->doDumpConds)
    {
      csString tree;
      tree.SetGrowsBy (0);

      wrappedNode.AttachNew (compiler->wrapperFact->CreateWrapper (srcNode, 
        tech.resolver, tech.resolver->evaluator, extraNodes, &tree,
        wdnfpoHandleConditions, &condResults));

      tech.resolver->DumpConditionTree (tree);
      csString filename;
      filename.Format ("/tmp/shader/cond_%s_%zu.txt",
        shaderRootStripped->GetAttributeValue ("name"),
        techIndex);
      compiler->vfs->WriteFile (filename, tree.GetData(), tree.Length ());
    }
    else
      wrappedNode.AttachNew (compiler->wrapperFact->CreateWrapper (srcNode, 
        tech.resolver, tech.resolver->evaluator, extraNodes, 0,
        wdnfpoHandleConditions, &condResults));

    tech.techNode = wrappedNode;

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

      if (cacheFile.IsValid())
      {
        ConditionsWriter condWriter (*sharedEvaluator);
        tech.resolver->CollectUsedConditions (condWriter);
        wrappedNode->CollectUsedConditions (condWriter);
        {
          csRef<iDataBuffer> conditionsBuf = condWriter.GetPersistentData();
          if (!CS::PluginCommon::ShaderCacheHelper::WriteDataBuffer (
            cacheFile, conditionsBuf))
            cacheFile.Invalidate();
        }

        if (cacheFile.IsValid() && !tech.resolver->WriteToCache (cacheFile,
          condWriter))
          cacheFile.Invalidate();
        if (cacheFile.IsValid() && !wrappedNode->StoreToCache (cacheFile,
            foreignNodes, condWriter))
          cacheFile.Invalidate();
        if (cacheFile.IsValid())
        {
          cacheData = cacheFile->GetAllData();
        }
      }
    }
  }

  bool csXMLShader::ActivatePass (size_t ticket, size_t number)
  { 
    if (IsFallbackTicket (ticket))
    {
      useFallbackContext = true;
      iShader* fallback;
      iXMLShaderInternal* fallbackXML;
      GetFallbackShader (fallback, fallbackXML);
      return fallback->ActivatePass (GetFallbackTicket (ticket), number);
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
      iShader* fallback;
      iXMLShaderInternal* fallbackXML;
      GetFallbackShader (fallback, fallbackXML);
      return fallback->DeactivatePass (GetFallbackTicket (ticket));
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
        "Unable to open shader program file %s", CS::Quote::Single (filename));
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
        "Unable to parse shader program file %s: %s",
	CS::Quote::Single (filename), err);
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

    const Technique* technique = TechniqueForTicket (variant);
    if (technique == 0) return 0;
    csShaderConditionResolver* resolver = technique->resolver;

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

  //-------------------------------------------------------------------------
  
  bool csXMLShader::Technique::ReadFromCache (iFile* cacheFile)
  {
    int32 diskPriority;
    if (cacheFile->Read ((char*)&diskPriority, sizeof (diskPriority))
	!= sizeof (diskPriority))
      return false;
    priority = csLittleEndian::Int32 (diskPriority);
      
    int32 diskMinLights;
    if (cacheFile->Read ((char*)&diskMinLights, sizeof (diskMinLights))
	!= sizeof (diskMinLights))
      return false;
    minLights = csLittleEndian::Int32 (diskMinLights);

    uint32 diskMetaNum;
    if (cacheFile->Read ((char*)&diskMetaNum, sizeof (diskMetaNum))
        != sizeof (diskMetaNum))
      return false;
    diskMetaNum = csLittleEndian::UInt32 (diskMetaNum);
    for (uint i = 0; i < diskMetaNum; i++)
    {
      csString key =
        CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile);
      csString value =
        CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile);
      metadata.Put (key, value);
    }
      
    return true;
  }

  bool csXMLShader::Technique::WriteToCache (iFile* cacheFile)
  {
    int32 diskPriority = csLittleEndian::Int32 (priority);
    if (cacheFile->Write ((char*)&diskPriority, sizeof (diskPriority))
          != sizeof (diskPriority))
      return false;
      
    int32 diskMinLights = csLittleEndian::Int32 (minLights);
    if (cacheFile->Write ((char*)&diskMinLights, sizeof (diskMinLights))
        != sizeof (diskMinLights))
      return false;
      
    uint32 diskMetaNum = csLittleEndian::UInt32 ((uint32)metadata.GetSize());
    if (cacheFile->Write ((char*)&diskMetaNum, sizeof (diskMetaNum))
        != sizeof (diskMetaNum))
      return false;
    MetadataHash::GlobalIterator metaIt = metadata.GetIterator();
    while (metaIt.HasNext())
    {
      csString key;
      const csString& value = metaIt.Next (key);
      
      if (!CS::PluginCommon::ShaderCacheHelper::WriteString (cacheFile, key))
        return false;
      if (!CS::PluginCommon::ShaderCacheHelper::WriteString (cacheFile, value))
        return false;
    }
      
    return true;
  }
  
  void csXMLShader::Technique::ScanMetadata (iDocumentNode* node)
  {
    csRef<iDocumentNodeIterator> metaNodeIt = node->GetNodes ("meta");
    while (metaNodeIt->HasNext())
    {
      csRef<iDocumentNode> metaNode = metaNodeIt->Next ();
      if (metaNode->GetType() != CS_NODE_ELEMENT) continue;
      
      const char* metaKey = metaNode->GetAttributeValue ("name");
      if (metaKey == 0) continue;
      const char* metaValue = metaNode->GetContentsValue ();
      metadata.Put (metaKey, metaValue);
    }
  }
}
CS_PLUGIN_NAMESPACE_END(XMLShader)
