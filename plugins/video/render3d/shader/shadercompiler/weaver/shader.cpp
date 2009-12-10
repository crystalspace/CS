/*
  Copyright (C) 2003-2007 by Marten Svanfeldt
            (C) 2004-2007 by Frank Richter

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
#include "imap/services.h"
#include "iutil/databuff.h"
#include "iutil/document.h"
#include "iutil/hiercache.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"

#include "csplugincommon/shader/shadercachehelper.h"
#include "csutil/base64.h"
#include "csutil/csendian.h"
#include "csutil/cspmeter.h"
#include "csutil/documenthelper.h"
#include "csutil/memfile.h"
#include "csutil/objreg.h"
#include "csutil/scfstr.h"
#include "csutil/parasiticdatabuffer.h"
#include "csutil/xmltiny.h"

#include "shader.h"
#include "weaver.h"
#include "synth.h"

CS_PLUGIN_NAMESPACE_BEGIN(ShaderWeaver)
{
  namespace WeaverCommon = CS::PluginCommon::ShaderWeaver;

  CS_LEAKGUARD_IMPLEMENT (WeaverShader);

  /* Magic value for cache file.
  * The most significant byte serves as a "version", increase when the
  * cache file format changes. */
  static const uint32 cacheFileMagic = 0x03727677;

  WeaverShader::WeaverShader (WeaverCompiler* compiler) : 
    scfImplementationType (this), compiler (compiler), 
    xmltokens (compiler->xmltokens)
  {
    shadermgr =  csQueryRegistry<iShaderManager> (compiler->objectreg);
    CS_ASSERT (shadermgr);
  }

  WeaverShader::~WeaverShader()
  {
  }

  bool WeaverShader::GeneratePasses (iDocumentNode* passgenNode, 
                                     const FileAliases& aliases,
                                     Synthesizer::DocNodeArray& nonPassNodes, 
                                     csArray<Synthesizer::DocNodeArray>& prePassNodes,
                                     csPDelArray<Snippet>& passSnippets)
  {
    const char* counterParam = passgenNode->GetAttributeValue ("param");
    if (!counterParam || !*counterParam)
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING,
        passgenNode, "<passgen> node needs 'param' attribute");
      return false;
    }
    const char* sequence = passgenNode->GetAttributeValue ("sequence");
    if (!sequence || !*sequence)
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING,
        passgenNode, "<passgen> node needs 'sequence' attribute");
      return false;
    }
    const char* compareSV = passgenNode->GetAttributeValue ("comparesv");
    
    csStringArray seqSplit;
    seqSplit.SplitString (sequence, ",");
    for (size_t i = 0; i < seqSplit.GetSize(); i++)
    {
      const char* seqStr = seqSplit[i];
      if (!seqStr || !*seqStr) continue;
      
      if (compareSV != 0)
      {
        csRef<iDocumentNode> compNode = compiler->CreateAutoNode (CS_NODE_UNKNOWN);
        compNode->SetValue (csString().Format ("?if vars.\"%s\".int >= %s ?",
          compareSV, seqStr));
        
        nonPassNodes.Push (compNode);
      }
      
      csRef<iDocumentNode> passNode = compiler->CreateAutoNode (CS_NODE_ELEMENT);
      passNode->SetValue ("pass");
      
      {
        bool hasParamNode = false;
	csRef<iDocumentNodeIterator> childNodes = passgenNode->GetNodes ();
	while (childNodes->HasNext ())
	{
	  csRef<iDocumentNode> node = childNodes->Next ();
	  csRef<iDocumentNode> newNode = 
	    passNode->CreateNodeBefore (node->GetType ());
	  CS::DocSystem::CloneNode (node, newNode);
	  
	  if (!hasParamNode
	     && (node->GetType() == CS_NODE_ELEMENT)
	     && (strcmp (node->GetValue(), "combiner") == 0))
	  {
	    csRef<iDocumentNode> paramNode =
	      passNode->CreateNodeBefore (CS_NODE_ELEMENT);
	    paramNode->SetValue ("parameter");
	    paramNode->SetAttribute ("id", counterParam);
	    paramNode->SetAttribute ("type", "int");
	    csRef<iDocumentNode> paramValueNode =
	      paramNode->CreateNodeBefore (CS_NODE_TEXT);
	    paramValueNode->SetValue (seqStr);
	    
	    hasParamNode = true;
	  }
	}
      }
      passSnippets.Push (new Snippet (compiler, passNode, 0, aliases, 0));
      prePassNodes.Push (nonPassNodes);
      nonPassNodes.Empty();
      
      if (compareSV != 0)
      {
        csRef<iDocumentNode> compNode = compiler->CreateAutoNode (CS_NODE_UNKNOWN);
        compNode->SetValue ("?endif?");
        nonPassNodes.Push (compNode);
      }
    }
    
    return true;
  }

  csRef<iDocument> WeaverShader::LoadTechsFromDoc (const csArray<TechniqueKeeper>& techniques,
						   const FileAliases& aliases,
						   iDocumentNode* docSource,
						   const char* cacheID, 
						   const char* _cacheTag,
						   iFile* cacheFile,
						   csRef<iString>& cachingError)
  {
    realShader.Invalidate();
    
    csRef<iDocumentSystem> docsys;
    docsys.AttachNew (new csTinyDocumentSystem);
    csRef<iDocument> synthesizedDoc = docsys->CreateDocument ();
    
    csRef<iDocumentNode> rootNode = synthesizedDoc->CreateRoot();
    csRef<iDocumentNode> shaderNode = rootNode->CreateNodeBefore (CS_NODE_ELEMENT);
    shaderNode->SetValue ("shader");
    CS::DocSystem::CloneAttributes (docSource, shaderNode);
    shaderNode->SetAttribute ("compiler", "xmlshader");
    if (cacheID != 0) shaderNode->SetAttribute ("_cacheid", cacheID);
    csString cacheTag (_cacheTag);

    csRef<iDocumentNode> shadervarsNode =
      shaderNode->CreateNodeBefore (CS_NODE_ELEMENT);
    shadervarsNode->SetValue ("shadervars");
    ShaderVarNodesHelper shaderVarNodesHelper (shadervarsNode);
	  
    csRef<iDocumentNodeIterator> shaderVarNodes = 
      docSource->GetNodes ("shadervar");
    while (shaderVarNodes->HasNext ())
    {
      csRef<iDocumentNode> svNode = shaderVarNodes->Next ();
      csRef<iDocumentNode> newNode = 
	shadervarsNode->CreateNodeBefore (svNode->GetType ());
      CS::DocSystem::CloneNode (svNode, newNode);
    }

    CombinerLoaderSet combiners;
    csRefArray<iDocumentNode> techniqueNodes;
    //for (size_t t = 0; t < techniques.GetSize(); t++)
    for (size_t t = 0; t < 1; t++)
    {
      iDocumentNode* techSource = techniques[t].node;
      
      csRef<iDocumentNodeIterator> it = techSource->GetNodes();
    
      // Read in the passes.
      Synthesizer::DocNodeArray nonPassNodes;
      csArray<Synthesizer::DocNodeArray> prePassNodes;
      csPDelArray<Snippet> passSnippets;
      while (it->HasNext ())
      {
	csRef<iDocumentNode> child = it->Next ();
	bool handled = false;
	if (child->GetType () == CS_NODE_ELEMENT)
	{
	  switch (xmltokens.Request (child->GetValue ()))
	  {
	    case WeaverCompiler::XMLTOKEN_PASS:
	      {
	        csRef<iDocumentNode> passNode (GetNodeOrFromFile (child));
	        passSnippets.Push (new Snippet (compiler, passNode, 0, aliases, 0));
	        prePassNodes.Push (nonPassNodes);
	        nonPassNodes.Empty();
	        handled = true;
	      }
	      break;
	    case WeaverCompiler::XMLTOKEN_PASSGEN:
	      GeneratePasses (child, aliases, nonPassNodes, prePassNodes, passSnippets);
	      handled = true;
	      break;
	  }
	}
	if (!handled)
	  nonPassNodes.Push (child);
      }

      const char* shaderName = docSource->GetAttributeValue ("name");
      Synthesizer synth (compiler, shaderName, prePassNodes, passSnippets, nonPassNodes);
    
      csTextProgressMeter pmeter (0);
      csPrintf ("shader %s: ", shaderName);
      synth.Synthesize (shaderNode, shaderVarNodesHelper, techniqueNodes,
        techSource, combiners, &pmeter);
    }
    if (techniques.GetSize() > 1)
    {
      csRef<iDocumentNode> newFallback = shaderNode->CreateNodeBefore (CS_NODE_ELEMENT);
      newFallback->SetValue ("fallbackshader");
      CS::DocSystem::CloneAttributes (docSource, newFallback);
      
      csString shaderNameDecorated (docSource->GetAttributeValue ("name"));
      size_t atat = shaderNameDecorated.FindFirst ("@@");
      if (atat != (size_t)-1)
        shaderNameDecorated.DeleteAt (atat, shaderNameDecorated.Length()-atat);
      shaderNameDecorated.AppendFmt ("@@%d", techniques[1].priority);
      newFallback->SetAttribute ("name", shaderNameDecorated);
      
      csRef<iDocumentNodeIterator> docNodes = docSource->GetNodes ();
      while (docNodes->HasNext())
      {
        csRef<iDocumentNode> orgNode = docNodes->Next();
        if (orgNode->Equals (techniques[0].node)) continue;
        
        csRef<iDocumentNode> newNode = newFallback->CreateNodeBefore (
          orgNode->GetType());
        CS::DocSystem::CloneNode (orgNode, newNode);
      }
    }
    else
    {
      csRef<iDocumentNode> fallback = docSource->GetNode ("fallbackshader");
      if (fallback.IsValid())
      {
        csRef<iDocumentNode> newFallback = shaderNode->CreateNodeBefore (CS_NODE_ELEMENT);
        CS::DocSystem::CloneNode (fallback, newFallback);
      }
    }
    
    for (size_t t = 0; t < techniqueNodes.GetSize(); t++)
      techniqueNodes[t]->SetAttributeAsInt ("priority",
        int (techniqueNodes.GetSize()-t));
    
    /* Include combiner code into cache tag to trigger updating the
       generated shader */
    {
      csStringArray cacheTagExtra;
      CombinerLoaderSet::GlobalIterator it (combiners.UnlockedGetIterator());
      while (it.HasNext())
      {
        WeaverCommon::iCombinerLoader* combinerLoader = it.Next();
        cacheTagExtra.Push (combinerLoader->GetCodeString());
      }
      cacheTagExtra.Sort ();
      for (size_t i = 0; i < cacheTagExtra.GetSize(); i++)
      {
	cacheTag.Append (";");
	cacheTag.Append (cacheTagExtra[i]);
      }
    }
    shaderNode->SetAttribute ("_cachetag", cacheTag);

    if (cacheFile)
    {
      // Write magic header
      uint32 diskMagic = csLittleEndian::UInt32 (cacheFileMagic);
      cacheFile->Write ((char*)&diskMagic, sizeof (diskMagic));

      uint32 diskCombinerNum = csLittleEndian::UInt32 (combiners.UnlockedGetSize());
      cacheFile->Write ((char*)&diskCombinerNum, sizeof (diskCombinerNum));
      {
	CombinerLoaderSet::GlobalIterator it (combiners.UnlockedGetIterator());
	while (it.HasNext())
	{
	  WeaverCommon::iCombinerLoader* combinerLoader = it.Next();
	  csRef<iFactory> scfFactory = scfQueryInterface<iFactory> (combinerLoader);
	  CS::PluginCommon::ShaderCacheHelper::WriteString (cacheFile, scfFactory->QueryClassID());
	  CS::PluginCommon::ShaderCacheHelper::WriteString (cacheFile, combinerLoader->GetCodeString());
	}
      }
      
      iDocumentSystem* cacheDocSys = compiler->binDocSys.IsValid()
	? compiler->binDocSys : compiler->xmlDocSys;
	
      csRef<iDocument> cacheDoc = cacheDocSys->CreateDocument();
      csRef<iDocumentNode> cacheDocRoot = cacheDoc->CreateRoot();
      csRef<iDocumentNode> cacheShaderNode = cacheDocRoot->CreateNodeBefore (
	CS_NODE_ELEMENT);
      CS::DocSystem::CloneNode (shaderNode, cacheShaderNode);
      
      csMemFile cachedDocFile;
      const char* writeErr = cacheDoc->Write (&cachedDocFile);
      if (writeErr != 0)
	cachingError.AttachNew (new scfString (
	  csString ("failed to write cache doc: ") + writeErr));
      else
      {
	csRef<iDataBuffer> cachedDocBuf = cachedDocFile.GetAllData ();
	if (!CS::PluginCommon::ShaderCacheHelper::WriteDataBuffer (
	  cacheFile, cachedDocBuf))
	{
	  cachingError.AttachNew (new scfString ("failed to write cache doc buffer"));
	}
      }
    }
      
    return synthesizedDoc;
  }
  
  csRef<iDocument> WeaverShader::LoadTechsFromCache (iFile* cacheFile,
                                                     const char*& cacheFailReason)
  {
    size_t read;
  
    csRef<iDataBuffer> cacheData = cacheFile->GetAllData();
    // Read magic header
    uint32 diskMagic;
    read = cacheFile->Read ((char*)&diskMagic, sizeof (diskMagic));
    if (read != sizeof (diskMagic))
    {
      cacheFailReason = "Read error";
      return 0;
    }
    if (csLittleEndian::UInt32 (diskMagic) != cacheFileMagic)
    {
      cacheFailReason = "Out of date (magic)";
      return 0;
    }

    uint32 diskCombinerNum;
    read = cacheFile->Read ((char*)&diskCombinerNum, sizeof (diskCombinerNum));
    if (read != sizeof (diskCombinerNum))
    {
      cacheFailReason = "Read error";
      return 0;
    }
    for (uint i = 0; i < csLittleEndian::UInt32 (diskCombinerNum); i++)
    {
      csString combinerID =
	CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile);
      csString combinerCode =
	CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile);

      csRef<WeaverCommon::iCombinerLoader> loader = 
	csLoadPluginCheck<WeaverCommon::iCombinerLoader> (compiler->objectreg,
	  combinerID);
      if (!loader.IsValid())
      {
	cacheFailReason = "Failed to load combiner";
	return 0;
      }
      if (combinerCode != loader->GetCodeString())
      {
	cacheFailReason = "Out of date (combiner code)";
	return 0;
      }
    }
    
    csRef<iDataBuffer> cachedDocData = 
      CS::PluginCommon::ShaderCacheHelper::ReadDataBuffer (cacheFile);
    if (!cachedDocData.IsValid())
    {
      cacheFailReason = "Failed to read cached doc";
      return 0;
    }
    
    csRef<iDocument> cacheDoc;
    cacheDoc = compiler->binDocSys->CreateDocument();
    if (cacheDoc->Parse (cachedDocData) != 0)
    {
      cacheDoc = compiler->xmlDocSys->CreateDocument();
      if (cacheDoc->Parse (cachedDocData) != 0)
      {
	cacheFailReason = "Failed to parse cached doc";
	return 0;
      }
    }
    
    return cacheDoc;
  }
  
  csRef<iDocument> WeaverShader::DoSynthesis (iDocumentNode* source,
                                              iHierarchicalCache* shaderCache,
                                              int forcepriority)
  {
    FileAliases aliases;
    csArray<TechniqueKeeper> techniques;
    Parse (source, techniques, forcepriority, aliases);
    
    CS::PluginCommon::ShaderCacheHelper::ShaderDocHasher hasher (
      compiler->objectreg, source);
    
    csString shaderName (source->GetAttributeValue ("name"));
    csString cacheID_base;
    csString cacheID_header;
    csString cacheID_tech;
    {
      csMD5::Digest sourceDigest (csMD5::Encode (CS::DocSystem::FlattenNode (source)));
      cacheID_base = sourceDigest.HexString();
      cacheID_header.Format ("%sWH", cacheID_base.GetData());
      cacheID_tech.Format ("%sWT", cacheID_base.GetData());
    }
    bool cacheValid = (shaderCache != 0) && !shaderName.IsEmpty()
      && !cacheID_header.IsEmpty() && !cacheID_tech.IsEmpty();
    bool useShaderCache = cacheValid;
    csString cacheFailReason;
      
    if (useShaderCache)
    {
      useShaderCache = false;
      csString cacheFileName;
      cacheFileName.Format ("/%s/%s",
	shaderName.GetData(), cacheID_header.GetData());
      csRef<iFile> cacheFile;
      {
	csRef<iDataBuffer> cacheData;
	cacheData = shaderCache->ReadCache (cacheFileName);
	if (cacheData.IsValid())
	{
	  cacheFile.AttachNew (new csMemFile (cacheData, true));
	}
      }
      if (cacheFile.IsValid())
      {
	do
	{
	  // Read magic header
	  uint32 diskMagic;
	  size_t read = cacheFile->Read ((char*)&diskMagic, sizeof (diskMagic));
	  if (read != sizeof (diskMagic))
	  {
	    cacheFailReason = "Read error";
	    break;
	  }
	  if (csLittleEndian::UInt32 (diskMagic) != cacheFileMagic)
	  {
	    cacheFailReason = "Out of date (magic)";
	    break;
	  }
	  
	  // Extract hash stream
	  csRef<iDataBuffer> hashStream = 
	    CS::PluginCommon::ShaderCacheHelper::ReadDataBuffer (cacheFile);
	  if (!hashStream.IsValid())
	  {
	    cacheFailReason = "Read error";
	    break;
	  }
	  
	  useShaderCache = hasher.ValidateHashStream (hashStream);
	  if (!useShaderCache)
	  {
	    cacheFailReason = "Out of date (hash)";
	    break;
	  }
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
	if (CS::PluginCommon::ShaderCacheHelper::WriteDataBuffer (
	  cacheFile, hashStream))
	{
	  csRef<iDataBuffer> allCacheData = cacheFile->GetAllData();
	  if (!shaderCache->CacheData (allCacheData->GetData(),
	      allCacheData->GetSize(), cacheFileName)
	    && compiler->do_verbose)
	  {
	    compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
	      "Could not cache header data for '%s'",
	      shaderName.GetData());
	  }
	}
      }
    }
    
    csRef<iDocument> synthShader;
    {
      csRef<iDataBuffer> cacheData;
      if (useShaderCache)
	cacheData = shaderCache->ReadCache (csString().Format ("/%s/%s",
	  shaderName.GetData(), cacheID_tech.GetData()));
      if (cacheData.IsValid())
      {
	csMemFile cacheFile (cacheData, true);
	const char* techCacheFailReason = 0;
	synthShader = LoadTechsFromCache (&cacheFile, techCacheFailReason);
	if (techCacheFailReason != 0)
	  cacheFailReason.Format ("Could not get cached techniques: %s",
	    techCacheFailReason);
      }
    }
    
    if (!synthShader.IsValid())
    {
      csRef<iDataBuffer> hashStream = hasher.GetHashStream ();
      // Hash hash stream (to get a smaller ID)
      csMD5::Digest hashDigest (csMD5::Encode (hashStream->GetData(),
        hashStream->GetSize()));
      /* In theory, anything would work as long as (a) it changes when
      some file the shader uses changes (b) the tag is reasonably
      unique (also over multiple program runs).
      E.g. a UUID, recomputed when the shader is 'touched',
      could do as well. */
      
      csString cacheTag (CS::Utility::EncodeBase64 (
        &hashDigest, sizeof (hashDigest)));
      
      csRef<iString> cacheState;
      csMemFile cacheFile;
      synthShader = LoadTechsFromDoc (techniques, aliases, 
	source, cacheID_base, cacheTag, cacheValid ? &cacheFile : 0,
	cacheState);
      if (cacheValid && !cacheState.IsValid ())
      {
	csRef<iDataBuffer> allCacheData = cacheFile.GetAllData();
	if (!shaderCache->CacheData (allCacheData->GetData(),
	  allCacheData->GetSize(), 
	  csString().Format ("/%s/%s",
            shaderName.GetData(), cacheID_tech.GetData())))
        {
          cacheState.AttachNew (new scfString ("error writing to cache"));
        }
      }
      if (cacheState.IsValid() && compiler->do_verbose)
      {
	compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "Could not cache '%s' because: %s",
	  shaderName.GetData(), cacheState->GetData());
      }
    }
    CS_ASSERT (synthShader.IsValid());
      
    if (compiler->doDumpWeaved)
    {
      csRef<iDocument> dumpDoc = compiler->xmlDocSys->CreateDocument ();
      
      csRef<iDocumentNode> rootNode = dumpDoc->CreateRoot();
      CS::DocSystem::CloneNode (synthShader->GetRoot(), rootNode);
      
      if (shaderName.IsEmpty())
      {
	static size_t counter = 0;
	shaderName.Format ("shader%zu", counter++);
      }
      dumpDoc->Write (compiler->vfs, 
	csString().Format ("/tmp/shader/generated_%s.xml",
	  shaderName.GetData()));
    }
    
    if (compiler->do_verbose && (!cacheFailReason.IsEmpty()))
    {
      compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
        "Could not get shader '%s' from cache because: %s",
        shaderName.GetData(), cacheFailReason.GetData());
    }

    return synthShader;
  }
    
  csRef<iDocumentNode> WeaverShader::GetNodeOrFromFile (iDocumentNode* node)
  {
    const char* fileAttr = node->GetAttributeValue ("file");
    if (!fileAttr || !*fileAttr) return node;
    
    csRef<iDocumentNode> newRoot (compiler->LoadDocumentFromFile (
      fileAttr, node));
    if (!newRoot.IsValid()) return 0;
    
    const char* wantNode = node->GetValue();
    csRef<iDocumentNode> newNode = newRoot->GetNode (wantNode);
    if (!newNode.IsValid())
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	"'%s' does not have a '%s' node", fileAttr, wantNode);
      return 0;
    }
    
    return newNode;
  }

  bool WeaverShader::Load (iLoaderContext* ldr_context, iDocumentNode* source,
                           int forcepriority)
  {
    iHierarchicalCache* shaderCache = shadermgr->GetShaderCache();
    
    csRef<iDocument> synthShader (DoSynthesis (source, shaderCache,
      forcepriority));
    
    csRef<iDocumentNode> shaderNode =
      synthShader->GetRoot()->GetNode ("shader");
    
    realShader = compiler->xmlshader->CompileShader (ldr_context,
      shaderNode);
      
    return realShader.IsValid();
  }
  
  bool WeaverShader::Precache (iDocumentNode* source,
                               iHierarchicalCache* cacheTo)
  {
    csRef<iDocument> synthShader (DoSynthesis (source, cacheTo, -1));
    
    csRef<iDocumentNode> shaderNode =
      synthShader->GetRoot()->GetNode ("shader");
    
    return compiler->xmlshader->PrecacheShader (shaderNode, cacheTo);
  }
  
  void WeaverShader::SelfDestruct ()
  {
    if (shadermgr)
      shadermgr->UnregisterShader (static_cast<iShader*> (this));
  }

  int WeaverShader::CompareTechniqueKeeper (
    TechniqueKeeper const& t1, TechniqueKeeper const& t2)
  {
    int v = t2.priority - t1.priority;
    if (v == 0) v = t2.tagPriority - t1.tagPriority;
    return v;
  }
  
  void WeaverShader::Parse (iDocumentNode* templ,
		            csArray<TechniqueKeeper>& techniquesTmp,
		            int forcepriority,
                            FileAliases& aliases)
  {
   csRef<iDocumentNodeIterator> it = templ->GetNodes();
  
    // Read in the techniques.
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      csStringID id = xmltokens.Request (child->GetValue ());
      switch (id)
      {
        case WeaverCompiler::XMLTOKEN_TECHNIQUE:
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
	  break;
        case WeaverCompiler::XMLTOKEN_ALIAS:
          {
            Snippet::ParseAliasNode (compiler, child, aliases);
          }
          break;
      }
    }
  
    techniquesTmp.Sort (&CompareTechniqueKeeper);
  }
}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)
