/*
  Copyright (C) 2008-2009 by Frank Richter

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

#ifndef __FOREIGNNODES_H__
#define __FOREIGNNODES_H__

#include "csutil/documenthelper.h"

#include "xmlshader.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

  class ForeignNodeStorage
  {
    csRef<iDocumentSystem> docSys;
  
    csRef<iFile> file;
    size_t headPos;
    csRef<iDocument> doc;
    csRef<iDocumentNode> baseNode;
    int32 currentID;
    csHash<int32, csString> knownNodes;
  public:
    ForeignNodeStorage (csXMLShaderCompiler* plugin)
    {
      docSys = plugin->binDocSys.IsValid() ? plugin->binDocSys : plugin->xmlDocSys;
    }
  
    bool StartUse (iFile* file)
    {
      CS_ASSERT(!this->file.IsValid());
      this->file = file;
      headPos = file->GetPos();
      
      uint32 dummy = (uint32)~0;
      if (file->Write ((char*)&dummy, sizeof (dummy)) != sizeof (dummy))
      {
	this->file.Invalidate();
	return false;
      }
      
      currentID = 0;
      doc = docSys->CreateDocument();
      baseNode = doc->CreateRoot()->CreateNodeBefore (CS_NODE_ELEMENT);
      baseNode->SetValue ("XD");
      
      return true;
    }
    bool EndUse ()
    {
      CS_ASSERT(this->file.IsValid());
      
      size_t curFilePos = file->GetPos();
      
      baseNode->SetAttributeAsInt ("n", currentID);
      csMemFile docFile;
      if (doc->Write (&docFile) != 0) return false;
      csRef<iDataBuffer> docBuf = docFile.GetAllData();
      if (!CS::PluginCommon::ShaderCacheHelper::WriteDataBuffer (file, docBuf))
	return false;
      
      uint32 ofsLE = (uint32)(curFilePos - headPos);
      curFilePos = file->GetPos();
      bool ret = false;
      file->SetPos (headPos);
      ofsLE = csLittleEndian::UInt32 (ofsLE);
      ret = (file->Write ((char*)&ofsLE, sizeof (ofsLE)) == sizeof (ofsLE));
      
      file->SetPos (curFilePos);
      file.Invalidate();
      return ret;
    }
    
    int32 StoreNodeShallow (iDocumentNode* node)
    {
      csString nodeHash (CS::DocSystem::FlattenNodeShallow (node));
      int32 nodeID = knownNodes.Get (nodeHash, -1);
      if (nodeID == -1)
      {
        csRef<iDocumentNode> storeNode = baseNode->CreateNodeBefore (CS_NODE_ELEMENT);
        storeNode->SetValueAsInt (currentID);
        csRef<iDocumentNode> realStoreNode = storeNode->CreateNodeBefore (node->GetType());
        realStoreNode->SetValue (node->GetValue());
        CS::DocSystem::CloneAttributes (node, realStoreNode);
        nodeID = currentID++;
        knownNodes.Put (nodeHash, nodeID);
      }
      return nodeID;
    }
    int32 StoreNodeDeep (iDocumentNode* node)
    {
      csString nodeHash (CS::DocSystem::FlattenNode (node));
      int32 nodeID = knownNodes.Get (nodeHash, -1);
      if (nodeID == -1)
      {
        csRef<iDocumentNode> storeNode = baseNode->CreateNodeBefore (CS_NODE_ELEMENT);
        storeNode->SetValueAsInt (currentID);
        csRef<iDocumentNode> realStoreNode = storeNode->CreateNodeBefore (node->GetType());
        CS::DocSystem::CloneNode (node, realStoreNode);
        nodeID = currentID++;
        knownNodes.Put (nodeHash, nodeID);
      }
      return nodeID;
    }
  };
  
  class ForeignNodeReader
  {
    csRef<iDocumentSystem> binDocSys;
    csRef<iDocumentSystem> xmlDocSys;
  
    csRef<iFile> file;
    size_t endPos;
    csRefArray<iDocumentNode> nodes;
  public:
    ForeignNodeReader (csXMLShaderCompiler* plugin)
    {
      binDocSys = plugin->binDocSys;
      xmlDocSys = plugin->xmlDocSys;
    }
  
    bool StartUse (iFile* file)
    {
      CS_ASSERT(!this->file.IsValid());
      
      size_t curFilePos = file->GetPos();
      uint32 ofsLE;
      if (file->Read ((char*)&ofsLE, sizeof (ofsLE)) != sizeof (ofsLE))
	return false;
      ofsLE = csLittleEndian::UInt32 (ofsLE);
      
      file->SetPos (curFilePos + ofsLE);
      csRef<iDataBuffer> docBuf =
	CS::PluginCommon::ShaderCacheHelper::ReadDataBuffer (file);
      if (!docBuf.IsValid ())
	return false;
      endPos = file->GetPos();
      file->SetPos (curFilePos + sizeof (ofsLE));
      
      csRef<iDocumentNode> baseNode;
      if (binDocSys.IsValid())
      {
	csRef<iDocument> doc = binDocSys->CreateDocument();
	if (doc->Parse (docBuf) == 0)
	  baseNode = doc->GetRoot()->GetNode ("XD");
      }
      if (!baseNode.IsValid())
      {
	csRef<iDocument> doc = xmlDocSys->CreateDocument();
	if (doc->Parse (docBuf) == 0)
	  baseNode = doc->GetRoot()->GetNode ("XD");
      }
      if (!baseNode.IsValid())
	return false;
	
      int numNodes = baseNode->GetAttributeValueAsInt ("n");
      this->nodes.SetSize (numNodes);
      csRef<iDocumentNodeIterator> nodes = baseNode->GetNodes();
      int num = 0;
      while (nodes->HasNext())
      {
	csRef<iDocumentNode> node = nodes->Next();
	csRef<iDocumentNodeIterator> subNodes = node->GetNodes();
	if (!subNodes->HasNext()) return false;
	this->nodes.Put (num++, subNodes->Next());
      }
	
      this->file = file;
      return true;
    }
    bool EndUse ()
    {
      CS_ASSERT(this->file.IsValid());
      
      file->SetPos (endPos);
      file.Invalidate();
      return true;
    }
    
    csRef<iDocumentNode> GetNode (int32 ID)
    {
      if (size_t (ID) >= nodes.GetSize()) return 0;
      return nodes[ID];
    }
  };

}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#endif // __FOREIGNNODES_H__
