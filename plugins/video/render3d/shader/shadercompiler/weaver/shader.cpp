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
#include "iutil/databuff.h"
#include "iutil/document.h"
#include "iutil/vfs.h"

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

  WeaverShader::WeaverShader (WeaverCompiler* compiler) : 
    scfImplementationType (this), compiler (compiler), 
    xmltokens (compiler->xmltokens)
  {
    shadermgr =  csQueryRegistry<iShaderManager> (compiler->objectreg);
  }

  WeaverShader::~WeaverShader()
  {
  }

  bool WeaverShader::LoadFromDoc (iLoaderContext* ldr_context, iDocumentNode* source,
                                  int forcepriority, iFile* cacheFile,
                                  bool& cacheState)
  {
    cacheState = true;
    realShader.Invalidate();
    
    iDocumentSystem* cacheDocSys = compiler->binDocSys.IsValid()
      ? compiler->binDocSys : compiler->xmlDocSys;

    csArray<TechniqueKeeper> techniques;
    ScanForTechniques (source, techniques, forcepriority);
    
    if (cacheFile)
    {
      uint32 techNumLE = csLittleEndian::UInt32 (techniques.GetSize());
      cacheFile->Write ((char*)&techNumLE, sizeof (techNumLE));
    }
    
    for (size_t t = 0; t < techniques.GetSize(); t++)
    {
      csPDelArray<Snippet> passSnippets;
      csRef<iDocumentNodeIterator> it = techniques[t].node->GetNodes();
    
      // Read in the passes.
      while (it->HasNext ())
      {
	csRef<iDocumentNode> child = it->Next ();
	if (child->GetType () == CS_NODE_ELEMENT &&
	  xmltokens.Request (child->GetValue ()) == WeaverCompiler::XMLTOKEN_PASS)
	{
	  passSnippets.Push (new Snippet (compiler, child, 0, true));
	}
      }
	
      Synthesizer synth (compiler, passSnippets);
    
      csRef<iDocument> synthShader = synth.Synthesize (source);
      CS_ASSERT (synthShader.IsValid());
      
      if (compiler->doDumpWeaved)
      {
	csString shaderName (source->GetAttributeValue ("name"));
	if (shaderName.IsEmpty())
	{
	  static size_t counter = 0;
	  shaderName.Format ("shader%zu", counter++);
	}
	synthShader->Write (compiler->vfs, 
	  csString().Format ("/tmp/shader/generated_%s_%zu.xml",
	    shaderName.GetData(), t));
      }
      
      csRef<iDocumentNode> shaderNode =
        synthShader->GetRoot()->GetNode ("shader");
      
      if (cacheFile)
      {
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
	  uint32 sizeLE = csLittleEndian::UInt32 (cachedDocBuf->GetSize());
	  cacheFile->Write ((char*)&sizeLE, sizeof (sizeLE));
	  cacheFile->Write (cachedDocBuf->GetData(), cachedDocBuf->GetSize());
	}
      }
        
      if (!realShader.IsValid())
	realShader = compiler->xmlshader->CompileShader (ldr_context,
	  shaderNode);
      if (realShader.IsValid() && (!cacheFile || !cacheState)) break;
    }

    return realShader.IsValid();
  }
  
  bool WeaverShader::LoadFromCache (iLoaderContext* ldr_context, 
                                    iDocumentNode* source,
				    int forcepriority, iFile* cacheFile)
  {
    size_t read;
  
    uint32 techNum;
    read = cacheFile->Read ((char*)&techNum, sizeof (techNum));
    if (read != sizeof (techNum)) return false;
    techNum = csLittleEndian::UInt32 (techNum);
    
    csRef<iDataBuffer> cacheData = cacheFile->GetAllData();
    for (uint t = 0; t < techNum; t++)
    {
      uint32 cachedDocSize;
      read = cacheFile->Read ((char*)&cachedDocSize,
	sizeof (cachedDocSize));
      if (read != sizeof (cachedDocSize)) return false;
      cachedDocSize = csLittleEndian::UInt32 (cachedDocSize);
      csRef<iDataBuffer> cachedDocData;
      cachedDocData.AttachNew (new csParasiticDataBuffer (cacheData,
	cacheFile->GetPos(), cachedDocSize));
      if (cachedDocData->GetSize() != cachedDocSize) break;
      cacheFile->SetPos (cacheFile->GetPos() + cachedDocSize);
      
      csRef<iDocument> cacheDoc;
      cacheDoc = compiler->binDocSys->CreateDocument();
      if (cacheDoc->Parse (cachedDocData) != 0)
      {
        cacheDoc = compiler->xmlDocSys->CreateDocument();
        if (cacheDoc->Parse (cachedDocData) != 0) return false;
      }
      
      csRef<iDocumentNode> shaderNode =
        cacheDoc->GetRoot()->GetNode ("shader");
      
      if (!realShader.IsValid())
	realShader = compiler->xmlshader->CompileShader (ldr_context,
	  shaderNode);
      if (realShader.IsValid()) break;
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
