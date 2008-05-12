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
#include "iutil/cache.h"
#include "iutil/databuff.h"
#include "iutil/document.h"
#include "iutil/vfs.h"

#include "csplugincommon/shader/shadercachehelper.h"
#include "csutil/csendian.h"
#include "csutil/documenthelper.h"
#include "csutil/memfile.h"
#include "csutil/objreg.h"
#include "csutil/parasiticdatabuffer.h"

#include "shader.h"
#include "weaver.h"
#include "synth.h"

CS_PLUGIN_NAMESPACE_BEGIN(ShaderWeaver)
{
  CS_LEAKGUARD_IMPLEMENT (WeaverShader);

  /* Magic value for cache file.
  * The most significant byte serves as a "version", increase when the
  * cache file format changes. */
  static const uint32 cacheFileMagic = 0x00727677;

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

  bool WeaverShader::LoadTechFromDoc (iLoaderContext* ldr_context, 
                                      iDocumentNode* docSource,
                                      iDocumentNode* techSource, size_t techNum,
                                      iFile* cacheFile, bool& cacheState)
  {
    cacheState = true;
    realShader.Invalidate();
    
    iDocumentSystem* cacheDocSys = compiler->binDocSys.IsValid()
      ? compiler->binDocSys : compiler->xmlDocSys;

    csRef<iDocumentNodeIterator> it = techSource->GetNodes();
  
    // Read in the passes.
    Synthesizer::DocNodeArray nonPassNodes;
    csArray<Synthesizer::DocNodeArray> prePassNodes;
    csPDelArray<Snippet> passSnippets;
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () == CS_NODE_ELEMENT &&
	xmltokens.Request (child->GetValue ()) == WeaverCompiler::XMLTOKEN_PASS)
      {
	passSnippets.Push (new Snippet (compiler, child, 0, true));
	prePassNodes.Push (nonPassNodes);
	nonPassNodes.Empty();
      }
      else
        nonPassNodes.Push (child);
    }
      
    Synthesizer synth (compiler, prePassNodes, passSnippets, nonPassNodes);
  
    csRef<iDocument> synthShader = synth.Synthesize (docSource);
    CS_ASSERT (synthShader.IsValid());
    
    if (compiler->doDumpWeaved)
    {
      csString shaderName (docSource->GetAttributeValue ("name"));
      if (shaderName.IsEmpty())
      {
	static size_t counter = 0;
	shaderName.Format ("shader%zu", counter++);
      }
      synthShader->Write (compiler->vfs, 
	csString().Format ("/tmp/shader/generated_%s_%zu.xml",
	  shaderName.GetData(), techNum));
    }
    
    csRef<iDocumentNode> shaderNode =
      synthShader->GetRoot()->GetNode ("shader");
    
    if (cacheFile)
    {
      // Write magic header
      uint32 diskMagic = csLittleEndian::UInt32 (cacheFileMagic);
      cacheFile->Write ((char*)&diskMagic, sizeof (diskMagic));
      
      csRef<iDocument> cacheDoc = cacheDocSys->CreateDocument();
      csRef<iDocumentNode> cacheDocRoot = cacheDoc->CreateRoot();
      csRef<iDocumentNode> cacheShaderNode = cacheDocRoot->CreateNodeBefore (
	CS_NODE_ELEMENT);
      CS::DocSystem::CloneNode (shaderNode, cacheShaderNode);
      
      csMemFile cachedDocFile;
      if (cacheDoc->Write (&cachedDocFile) != 0)
	cacheState = false;
      else
      {
	csRef<iDataBuffer> cachedDocBuf = cachedDocFile.GetAllData ();
	CS::PluginCommon::ShaderCacheHelper::WriteDataBuffer (
	  cacheFile, cachedDocBuf);
      }
    }
      
    realShader = compiler->xmlshader->CompileShader (ldr_context,
      shaderNode);

    return realShader.IsValid();
  }
  
  bool WeaverShader::LoadTechFromCache (iLoaderContext* ldr_context, 
                                        iFile* cacheFile)
  {
    size_t read;
  
    csRef<iDataBuffer> cacheData = cacheFile->GetAllData();
    // Read magic header
    uint32 diskMagic;
    read = cacheFile->Read ((char*)&diskMagic, sizeof (diskMagic));
    if (read != sizeof (diskMagic)) return false;
    if (csLittleEndian::UInt32 (diskMagic) != cacheFileMagic) return false;
    
    csRef<iDataBuffer> cachedDocData = 
      CS::PluginCommon::ShaderCacheHelper::ReadDataBuffer (cacheFile);
    if (!cachedDocData.IsValid()) return false;
    
    csRef<iDocument> cacheDoc;
    cacheDoc = compiler->binDocSys->CreateDocument();
    if (cacheDoc->Parse (cachedDocData) != 0)
    {
      cacheDoc = compiler->xmlDocSys->CreateDocument();
      if (cacheDoc->Parse (cachedDocData) != 0) return false;
    }
    
    csRef<iDocumentNode> shaderNode =
      cacheDoc->GetRoot()->GetNode ("shader");
    
    realShader = compiler->xmlshader->CompileShader (ldr_context,
      shaderNode);

    return realShader.IsValid();
  }
  
  bool WeaverShader::Load (iLoaderContext* ldr_context, iDocumentNode* source,
                           int forcepriority)
  {
    csArray<TechniqueKeeper> techniques;
    ScanForTechniques (source, techniques, forcepriority);
    
    CS::PluginCommon::ShaderCacheHelper::ShaderDocHasher hasher (
      compiler->objectreg, source);
    
    iCacheManager* shaderCache = shadermgr->GetShaderCache();
    csString shaderName (source->GetAttributeValue ("name"));
    csString cacheID_header;
    csString cacheID_tech;
    {
      csMD5::Digest sourceDigest (csMD5::Encode (CS::DocSystem::FlattenNode (source)));
      csString digestStr (sourceDigest.HexString());
      cacheID_header.Format ("%sWH", digestStr.GetData());
      cacheID_tech.Format ("%sWT", digestStr.GetData());
    }
    bool cacheValid = (shaderCache != 0) && !shaderName.IsEmpty()
      && !cacheID_header.IsEmpty() && !cacheID_tech.IsEmpty();
    bool useShaderCache = cacheValid;
      
    if (useShaderCache)
    {
      useShaderCache = false;
      csRef<iFile> cacheFile;
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
	if (CS::PluginCommon::ShaderCacheHelper::WriteDataBuffer (
	  cacheFile, hashStream))
	{
	  csRef<iDataBuffer> allCacheData = cacheFile->GetAllData();
	  shaderCache->CacheData (allCacheData->GetData(),
	    allCacheData->GetSize(), shaderName, cacheID_header, ~0);
	}
      }
    }
    
    for (size_t t = 0; t < techniques.GetSize(); t++)
    {
      csRef<iDataBuffer> cacheData;
      if (useShaderCache)
        cacheData = shaderCache->ReadCache (shaderName, cacheID_tech,
          techniques[t].priority);
      
      bool res = false;
      if (cacheData.IsValid())
      {
        csMemFile cacheFile (cacheData, true);
        res = LoadTechFromCache (ldr_context, &cacheFile);
      }
      
      if (!res)
      {
        bool cacheState;
        csMemFile cacheFile;
        res = LoadTechFromDoc (ldr_context, 
          source, techniques[t].node, t,
          cacheValid ? &cacheFile : 0, cacheState);
        if (cacheValid && cacheState)
        {
          csRef<iDataBuffer> allCacheData = cacheFile.GetAllData();
	  shaderCache->CacheData (allCacheData->GetData(),
	    allCacheData->GetSize(), shaderName, cacheID_tech,
	    techniques[t].priority);
	}
      }
    
      if (res) break;
    }
    
    return realShader.IsValid();
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
  
  void WeaverShader::ScanForTechniques (iDocumentNode* templ,
		                        csArray<TechniqueKeeper>& techniquesTmp,
		                        int forcepriority)
  {
   csRef<iDocumentNodeIterator> it = templ->GetNodes();
  
    // Read in the techniques.
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () == CS_NODE_ELEMENT &&
	xmltokens.Request (child->GetValue ()) == WeaverCompiler::XMLTOKEN_TECHNIQUE)
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
}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)
