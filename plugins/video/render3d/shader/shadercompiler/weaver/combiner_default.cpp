/*
  Copyright (C) 2003-2007 by Marten Svanfeldt
		2004-2007 by Frank Richter

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

#include "csutil/documenthelper.h"
#include "csutil/scfstr.h"

#include "combiner_default.h"
#include "synth.h"
#include "weaver.h"

CS_PLUGIN_NAMESPACE_BEGIN(ShaderWeaver)
{
  CombinerDefault::CombinerDefault (const WeaverCompiler* compiler,
                                    ShaderVarNodesHelper& shaderVarNodes,
				    TagNodesHelper& tagNodes) : 
    scfImplementationType (this), compiler (compiler),
    xmltokens (compiler->xmltokens), shaderVarNodes (shaderVarNodes),
    tagNodes (tagNodes)
  {
  }
  
  void CombinerDefault::BeginSnippet (const char* annotation)
  {
  }
  
  void CombinerDefault::InputRename (const char* fromName, 
				     const char* toName)
  {
  }
  
  void CombinerDefault::OutputRename (const char* fromName, 
                                      const char* toName)
  {
  }
  
  void CombinerDefault::WriteBlock (const char* location, 
                                    iDocumentNode* blockNode)
  {
    if (strcmp (location, "pass") == 0)
    {
      csRef<iDocumentNodeIterator> nodes = blockNode->GetNodes();
      while (nodes->HasNext())
      {
        passNodes.Push (nodes->Next());
      }
    }
    else if (strcmp (location, "shadervars") == 0)
    {
      csRef<iDocumentNodeIterator> nodes = blockNode->GetNodes();
      while (nodes->HasNext())
      {
        csRef<iDocumentNode> child = nodes->Next();
        shaderVarNodes.AddNode (child);
      }
    }
    else if (strcmp (location, "tags") == 0)
    {
      csRef<iDocumentNodeIterator> nodes = blockNode->GetNodes();
      while (nodes->HasNext())
      {
        csRef<iDocumentNode> child = nodes->Next();
        tagNodes.AddNode (child);
      }
    }
  }
  
  bool CombinerDefault::EndSnippet ()
  {
    return true;
  }

  uint CombinerDefault::CoerceCost (const char* fromType, const char* toType)
  {
    return CS::PluginCommon::ShaderWeaver::NoCoercion;
  }

  void CombinerDefault::WriteToPass (iDocumentNode* pass)
  {
    for (size_t n = 0; n < passNodes.GetSize(); n++)
    {
      csRef<iDocumentNode> newNode = 
        pass->CreateNodeBefore (passNodes[n]->GetType());
      CS::DocSystem::CloneNode (passNodes[n], newNode);
    }
  }
  
  bool CombinerDefault::CompatibleParams (iDocumentNode* params)
  {
    return true;
  }

  csRef<iString> CombinerDefault::QueryInputTag (const char* location, 
                                                 iDocumentNode* blockNode)
  {
    if (strcmp (location, "pass") == 0)
    {
      csRef<iString> result;
      bool hasTag = false;
      csRef<iDocumentNodeIterator> nodes = blockNode->GetNodes();
      while (nodes->HasNext())
      {
        csRef<iDocumentNode> node = nodes->Next();
        if (node->GetType() != CS_NODE_ELEMENT) continue;
        csStringID id = xmltokens.Request (node->GetValue());
        switch (id)
        {
          case WeaverCompiler::XMLTOKEN_BUFFER:
            {
              // For now only support 1 tag...
              if (hasTag) return 0;
              hasTag = true;
              result.AttachNew (new scfString (
                node->GetAttributeValue ("source")));
            }
            break;
          case WeaverCompiler::XMLTOKEN_TEXTURE:
            {
              // For now only support 1 tag...
              if (hasTag) return 0;
              hasTag = true;
	      csString tagStr (node->GetAttributeValue ("name"));
	      csString fallbackTex (node->GetAttributeValue ("fallback"));
	      if (!fallbackTex.IsEmpty())
	      {
		tagStr.Append ("|");
		tagStr.Append (fallbackTex);
	      }
              result.AttachNew (new scfString (tagStr));
            }
            break;
          case WeaverCompiler::XMLTOKEN_INSTANCEPARAM:
            {
              // For now only support 1 tag...
              if (hasTag) return 0;
              hasTag = true;
              result.AttachNew (new scfString (
                node->GetAttributeValue ("source")));
            }
            break;
          default:
            break;
        }
      }
      return result;
    }
    return 0;
  }
}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)
