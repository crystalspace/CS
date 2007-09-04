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
#include "iutil/document.h"

#include "csutil/objreg.h"

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

  bool WeaverShader::Load (iLoaderContext* ldr_context, iDocumentNode* source,
                           int forcepriority)
  {
    csArray<TechniqueKeeper> techniques;
    ScanForTechniques (source, techniques, forcepriority);
    
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
	  passSnippets.Push (new Snippet (compiler, child, true));
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
	  csString().Format ("/tmp/shader/generated_%s.xml", shaderName.GetData()));
      }
      
      realShader = compiler->xmlshader->CompileShader (ldr_context,
	synthShader->GetRoot()->GetNode ("shader"));
      if (realShader.IsValid())
        return true;
    }

    return false;
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
