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

  struct WeaverShader::CacheInfo
  {
    CS::PluginCommon::ShaderCacheHelper::ShaderDocHasher hasher;
    csString shaderName;
    csString cacheID_base;

    CacheInfo (WeaverCompiler* compiler, iDocumentNode* source)
     : hasher (compiler->objectreg, 0)
    {
      hasher = CS::PluginCommon::ShaderCacheHelper::ShaderDocHasher (
        compiler->objectreg, source);
      
      shaderName = source->GetAttributeValue ("name");
      CS::Utility::Checksum::MD5::Digest sourceDigest (
	CS::Utility::Checksum::MD5::Encode (CS::DocSystem::FlattenNode (source)));
      cacheID_base = sourceDigest.HexString();
    }
  };

  /* Magic value for cache file.
  * The most significant byte serves as a "version", increase when the
  * cache file format changes. */
  static const uint32 cacheFileMagic = 0x04727677;

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
        passgenNode, "<passgen> node needs %s attribute",
	CS::Quote::Single ("param"));
      return false;
    }
    const char* sequence = passgenNode->GetAttributeValue ("sequence");
    if (!sequence || !*sequence)
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING,
        passgenNode, "<passgen> node needs %s attribute",
	CS::Quote::Single ("sequence"));
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

  csRef<iDocument> WeaverShader::TryLoadShader (CacheInfo& ci,
                                                iDocumentNode* source,
                                                iHierarchicalCache* shaderCache)
  {
    csString cacheID_header;
    cacheID_header.Format ("%sWH", ci.cacheID_base.GetData());
    bool cacheValid = (shaderCache != 0) && !ci.shaderName.IsEmpty()
      && !cacheID_header.IsEmpty();
    bool useShaderCache = cacheValid;
    csString cacheFailReason;

    csRef<iDocument> synthShader;
    if (useShaderCache)
    {
      useShaderCache = false;
      csString cacheFileName;
      cacheFileName.Format ("/%s/%s",
	ci.shaderName.GetData(), cacheID_header.GetData());
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
	  
	  useShaderCache = ci.hasher.ValidateHashStream (hashStream);
	  if (!useShaderCache)
	  {
	    cacheFailReason = "Out of date (hash)";
	    break;
	  }

          if (!ValidateCombinerCodes (cacheFile, cacheFailReason))
	  {
	    break;
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
          synthShader = cacheDoc;
        }
	while (false);
      }
    }
    
    if (compiler->do_verbose && (!cacheFailReason.IsEmpty()))
    {
      compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
        "Could not get shader %s from cache because: %s",
        CS::Quote::Single (ci.shaderName.GetData()), cacheFailReason.GetData());
    }

    return synthShader;
  }

  bool WeaverShader::ValidateCombinerCodes (iFile* cacheFile,
    csString& cacheFailReason)
  {
    uint32 diskCombinerNum;
    size_t read = cacheFile->Read ((char*)&diskCombinerNum, sizeof (diskCombinerNum));
    if (read != sizeof (diskCombinerNum))
    {
      cacheFailReason = "Read error";
      return false;
    }
    for (uint i = 0; i < csLittleEndian::UInt32 (diskCombinerNum); i++)
    {
      csString combinerID =
	CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile);
      if (combinerID.IsEmpty()) return false;
      csString combinerCode =
	CS::PluginCommon::ShaderCacheHelper::ReadString (cacheFile);

      csRef<WeaverCommon::iCombinerLoader> loader = 
	csLoadPluginCheck<WeaverCommon::iCombinerLoader> (compiler->objectreg,
	  combinerID, false);
      if (!loader.IsValid())
      {
	cacheFailReason = "Failed to load combiner";
	return false;
      }
      if (combinerCode != loader->GetCodeString())
      {
	cacheFailReason = "Out of date (combiner code)";
	return false;
      }
    }
    return true;
  }

  csRef<iDocument> WeaverShader::SynthesizeShaderAndCache (CacheInfo& ci,
    iDocumentNode* source, iHierarchicalCache* cacheTo, int forcepriority)
  {
    FileAliases aliases;
    csArray<TechniqueKeeper> techniques;
    Parse (source, techniques, forcepriority, aliases);

    csString cacheID_header;
    cacheID_header.Format ("%sWH", ci.cacheID_base.GetData());
    csString cacheFileName;
    cacheFileName.Format ("/%s/%s",
      ci.shaderName.GetData(), cacheID_header.GetData());
    
    if (!cacheTo)
    {
      // No cache given - just synthesize; cache tag can be dummy value
      csString cacheTag;
      CombinerLoaderSet combiners;
      csRef<iDocument> synthShader (SynthesizeShader (techniques, aliases,
	source, ci.cacheID_base, cacheTag, cacheTag, combiners));
      
      CS_ASSERT (synthShader.IsValid());
      DumpWeaved (ci, synthShader);
      return synthShader;
    }

    csRef<iString> cacheState;

    csRef<iFile> cacheFile;
    // Prep for writing to cache
    cacheFile.AttachNew (new csMemFile ());
    // Write magic header
    uint32 diskMagic = csLittleEndian::UInt32 (cacheFileMagic);
    if (cacheFile->Write ((char*)&diskMagic, sizeof (diskMagic)) != sizeof (diskMagic))
    {
      cacheState.AttachNew (new scfString ("error writing to cache file"));
      cacheFile = 0;
    }
    // Write hash stream
    csRef<iDataBuffer> hashStream = ci.hasher.GetHashStream ();
    if (cacheFile.IsValid())
    {
      if (!CS::PluginCommon::ShaderCacheHelper::WriteDataBuffer (
        cacheFile, hashStream))
      {
        cacheState.AttachNew (new scfString ("error writing hash stream"));
        cacheFile = 0;
      }
    }

    
    csRef<iDocument> synthShader;
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
      
      csString cacheTag (CS::Utility::EncodeBase64 (
        &hashDigest, sizeof (hashDigest)));

      CombinerLoaderSet combiners;
      synthShader = SynthesizeShader (techniques, aliases, source, ci.cacheID_base,
        cacheTag, cacheTag, combiners);

      if (cacheFile.IsValid()
          && !WriteCombinerCodes (cacheFile, combiners))
      {
        cacheState.AttachNew (new scfString ("error writing to cache file"));
        cacheFile = 0;
      }

      if (cacheFile.IsValid())
      {
        iDocumentSystem* cacheDocSys = compiler->binDocSys.IsValid()
	  ? compiler->binDocSys : compiler->xmlDocSys;
  	
        csRef<iDocumentNode> shaderNode =
          synthShader->GetRoot()->GetNode ("shader");

        /* Store only a "torso" shader document.
           This is (obviously) much smaller. Since we provide a cache tag to
           xmlshader it will compare that against the cache tag it stored and
           fail to load the cached shader if these mismatch.
           Since that tag is derived from the shaderweaver source, any change
           to the weaver source will invalidate the cached xmlshader as well.
         */
        csRef<iDocument> cacheDoc = cacheDocSys->CreateDocument();
        csRef<iDocumentNode> cacheDocRoot = cacheDoc->CreateRoot();
        csRef<iDocumentNode> cacheShaderNode = cacheDocRoot->CreateNodeBefore (
	  CS_NODE_ELEMENT);
        cacheShaderNode->SetValue ("shader");
        /* This instructs xmlshader to fail loading if it can't load the shader
           from the cache */
        cacheShaderNode->SetAttributeAsInt ("_forceCacheLoad", 1);
        CS::DocSystem::CloneAttributes (shaderNode, cacheShaderNode);
        csRef<iDocumentNode> tokenChild =
          cacheShaderNode->CreateNodeBefore (CS_NODE_ELEMENT);
        // This is needed so xmlshader doesn't reject the torso right away
        tokenChild->SetValue ("Boo");
        
        csMemFile cachedDocFile;
        const char* writeErr = cacheDoc->Write (&cachedDocFile);
        if (writeErr != 0)
        {
	  cacheState.AttachNew (new scfString (
	    csString ("failed to write cache doc: ") + writeErr));
          cacheFile = 0;
        }
        else
        {
	  csRef<iDataBuffer> cachedDocBuf = cachedDocFile.GetAllData ();
	  if (!CS::PluginCommon::ShaderCacheHelper::WriteDataBuffer (
	    cacheFile, cachedDocBuf))
	  {
	    cacheState.AttachNew (new scfString ("failed to write cache doc buffer"));
            cacheFile = 0;
	  }
        }
      }

      csRef<iDataBuffer> allCacheData = cacheFile->GetAllData();
      if (!cacheTo->CacheData (allCacheData->GetData(),
          allCacheData->GetSize(), cacheFileName)
        && compiler->do_verbose)
      {
        cacheState.AttachNew (new scfString ("error writing to cache"));
      }
      if (cacheState.IsValid() && compiler->do_verbose)
      {
	compiler->Report (CS_REPORTER_SEVERITY_NOTIFY,
	  "Could not cache %s because: %s",
	  CS::Quote::Single (ci.shaderName.GetData()),
	  cacheState->GetData());
      }
    }
    CS_ASSERT (synthShader.IsValid());
      
    DumpWeaved (ci, synthShader);
    
    return synthShader;
  }

  csRef<iDocument> WeaverShader::SynthesizeShader (const csArray<TechniqueKeeper>& techniques,
						   const FileAliases& aliases,
						   iDocumentNode* docSource,
						   const char* cacheID, 
						   const char* _cacheTag,
                                                   csString& cacheTag,
                                                   CombinerLoaderSet& combiners)
  {
    csRef<iDocumentSystem> docsys;
    docsys.AttachNew (new csTinyDocumentSystem);
    csRef<iDocument> synthesizedDoc = docsys->CreateDocument ();
    
    csRef<iDocumentNode> rootNode = synthesizedDoc->CreateRoot();
    csRef<iDocumentNode> shaderNode = rootNode->CreateNodeBefore (CS_NODE_ELEMENT);
    shaderNode->SetValue ("shader");
    CS::DocSystem::CloneAttributes (docSource, shaderNode);
    shaderNode->SetAttribute ("compiler", "xmlshader");
    if (cacheID != 0) shaderNode->SetAttribute ("_cacheid", cacheID);
    cacheTag = _cacheTag;

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

    csRefArray<iDocumentNode> techniqueNodes;
    if (techniques.GetSize() > 0)
    {
      iDocumentNode* techSource = techniques[0].node;
      
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
      pmeter.SetGranularity (pmeter.GetTickScale());
      csPrintf ("shader %s: ", shaderName);
      synth.Synthesize (shaderNode, shaderVarNodesHelper, techniqueNodes,
        techSource, combiners, &pmeter);
    }
    if (techniques.GetSize() > 1)
    {
      csRef<iDocumentNode> newFallback = shaderNode->CreateNodeBefore (CS_NODE_ELEMENT);
      newFallback->SetValue ("fallbackshader");
      CS::DocSystem::CloneAttributes (docSource, newFallback);

      MakeFallbackShader (newFallback, docSource, techniques);
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

    return synthesizedDoc;
  }

  bool WeaverShader::WriteCombinerCodes (iFile* cacheFile,
                                         const CombinerLoaderSet& combiners)
  {
    uint32 diskCombinerNum = csLittleEndian::UInt32 ((uint32)combiners.UnlockedGetSize());
    if (cacheFile->Write ((char*)&diskCombinerNum, sizeof (diskCombinerNum))
        != sizeof (diskCombinerNum))
      return false;

    CombinerLoaderSet::GlobalIterator it (combiners.UnlockedGetIterator());
    while (it.HasNext())
    {
      WeaverCommon::iCombinerLoader* combinerLoader = it.Next();
      csRef<iFactory> scfFactory = scfQueryInterface<iFactory> (combinerLoader);
      if (!CS::PluginCommon::ShaderCacheHelper::WriteString (cacheFile, scfFactory->QueryClassID())
          || !CS::PluginCommon::ShaderCacheHelper::WriteString (cacheFile, combinerLoader->GetCodeString()))
        return false;
    }
    
    return true;
  }

  void WeaverShader::DumpWeaved (CacheInfo& ci, iDocument* synthShader)
  {
    if (compiler->doDumpWeaved)
    {
      csRef<iDocument> dumpDoc = compiler->xmlDocSys->CreateDocument ();
      
      csRef<iDocumentNode> rootNode = dumpDoc->CreateRoot();
      CS::DocSystem::CloneNode (synthShader->GetRoot(), rootNode);
      
      csString shaderName (ci.shaderName);
      if (shaderName.IsEmpty())
      {
	static size_t counter = 0;
	shaderName.Format ("shader%zu", counter++);
      }
      dumpDoc->Write (compiler->vfs, 
	csString().Format ("/tmp/shader/generated_%s.xml",
	  shaderName.GetData()));
    }
  }

  void WeaverShader::MakeFallbackShader (iDocumentNode* targetNode,
    iDocumentNode* docSource,
    const csArray<TechniqueKeeper>& techniques)
  {
    csString shaderNameDecorated (docSource->GetAttributeValue ("name"));
    size_t atat = shaderNameDecorated.FindFirst ("@@");
    if (atat != (size_t)-1)
      shaderNameDecorated.DeleteAt (atat, shaderNameDecorated.Length()-atat);
    shaderNameDecorated.AppendFmt ("@@%d", techniques[1].priority);
    targetNode->SetAttribute ("name", shaderNameDecorated);
    
    csRef<iDocumentNodeIterator> docNodes = docSource->GetNodes ();
    while (docNodes->HasNext())
    {
      csRef<iDocumentNode> orgNode = docNodes->Next();
      if (orgNode->Equals (techniques[0].node)) continue;
      
      csRef<iDocumentNode> newNode = targetNode->CreateNodeBefore (
        orgNode->GetType());
      CS::DocSystem::CloneNode (orgNode, newNode);
    }
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
	"%s does not have a %s node",
	CS::Quote::Single (fileAttr), CS::Quote::Single (wantNode));
      return 0;
    }
    
    return newNode;
  }

  bool WeaverShader::Load (iLoaderContext* ldr_context, iDocumentNode* source,
                           int forcepriority)
  {
    iHierarchicalCache* shaderCache = shadermgr->GetShaderCache();
    
    CacheInfo ci (compiler, source);

    realShader.Invalidate();
    csRef<iDocument> xmlShader (TryLoadShader (ci, source, shaderCache));
    if (xmlShader.IsValid())
    {
      csRef<iDocumentNode> shaderNode =
        xmlShader->GetRoot()->GetNode ("shader");
      
      realShader = compiler->xmlshader->CompileShader (ldr_context,
        shaderNode);
    }
    if (!realShader.IsValid())
    {
      xmlShader = SynthesizeShaderAndCache (ci, source, shaderCache,
        forcepriority);

      csRef<iDocumentNode> shaderNode =
        xmlShader->GetRoot()->GetNode ("shader");
      
      realShader = compiler->xmlshader->CompileShader (ldr_context,
        shaderNode);
    }

    realShaderXML = scfQueryInterfaceSafe<iXMLShaderInternal> (realShader);
      
    return realShader.IsValid();
  }
  
  bool WeaverShader::Precache (iDocumentNode* source,
                               iHierarchicalCache* cacheTo,
                               bool quick)
  {
    CacheInfo ci (compiler, source);

    csRef<iDocument> xmlShader (SynthesizeShaderAndCache (ci, source, cacheTo,
        -1));
    
    csRef<iDocumentNode> shaderNode =
      xmlShader->GetRoot()->GetNode ("shader");
    
    return compiler->xmlshader->PrecacheShader (shaderNode, cacheTo, quick);
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
