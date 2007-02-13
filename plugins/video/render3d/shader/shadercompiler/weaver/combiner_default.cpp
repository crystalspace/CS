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

#include "combiner_default.h"

CS_PLUGIN_NAMESPACE_BEGIN(ShaderWeaver)
{
  CombinerDefault::CombinerDefault () : scfImplementationType (this)
  {
  }
  
  void CombinerDefault::BeginSnippet ()
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
      CS::DocumentHelper::CloneNode (passNodes[n], newNode);
    }
  }
  
  bool CombinerDefault::CompatibleParams (iDocumentNode* params)
  {
    return true;
  }
}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)
