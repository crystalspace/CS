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
#include "iutil/document.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"

#include "csutil/documenthelper.h"
#include "csutil/scopeddelete.h"
#include "csutil/xmltiny.h"

#include "snippet.h"
#include "weaver.h"

CS_PLUGIN_NAMESPACE_BEGIN(ShaderWeaver)
{
  
  template<typename T, class ArrayType>
  class BasicIteratorImplCopyValue : public BasicIterator<T>
  {
    const ArrayType& array;
    size_t pos;
    T storage;
  public:
    BasicIteratorImplCopyValue (const ArrayType& array) : array (array), pos (0) {}
  
    virtual bool HasNext()
    { return pos < array.GetSize(); }
    virtual const T& Next()
    { 
      storage = array[pos++]; 
      return storage;
    }
  };

  /*bool Snippet::Technique::HasOutput (const char* name) const
  {
    for (size_t o = 0; o < outputs.GetSize(); o++)
    {
      if (strcmp (outputs[o].name, name) == 0)
        return true;
    }
    return false;
  }*/

  //-------------------------------------------------------------------

  Snippet::Snippet (WeaverCompiler* compiler, iDocumentNode* node, 
                    bool topLevel) : compiler (compiler), 
    xmltokens (compiler->xmltokens), isCompound (false)
  {
    bool okay = true;
    if (topLevel)
    {
      isCompound = true;
      LoadCompoundTechnique (node);
    }
    else
    {
      const char* snippetType = node->GetAttributeValue ("type");
      if (snippetType != 0)
      {
	if (strcmp (snippetType, "atom") == 0)
	{
	  isCompound = false;
	}
	else if (strcmp (snippetType, "compound") == 0)
	{
	  isCompound = true;
	}
	else
	{
	  compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	    "Unknown snippet type '%s'", snippetType);
	  okay = false;
	}
      }
      if (okay)
      {
	if (!isCompound)
	  LoadAtomTechniques (node);
	else
	{
	  LoadCompoundTechniques (node, topLevel);
	}
      }
    }
  }
  
  Snippet::~Snippet()
  {
  }
  
  BasicIterator<const Snippet::Technique*>* Snippet::GetTechniques() const
  {
    return new BasicIteratorImplCopyValue<const Technique*, TechniqueArray> (
      techniques);
  }
  
  Snippet::Technique* Snippet::LoadLibraryTechnique (WeaverCompiler* compiler, 
    iDocumentNode* node, const Technique::CombinerPlugin& combiner)
  {
    Snippet::AtomTechnique* technique = ParseAtomTechnique (compiler, node);
    technique->combiner = combiner;
    for (size_t b = 0; b < technique->blocks.GetSize(); b++)
    {
      technique->blocks[b].combinerName = combiner.name;
    }
    return technique;
  }
  
  Snippet::Technique* Snippet::CreatePassthrough (const char* varName, 
                                                  const char* type)
  {
    csString hashStr;
    hashStr.Format ("__passthrough_%s_%s__", varName, type);
    AtomTechnique* newTech = new AtomTechnique (csMD5::Encode (hashStr));
    
    {
      Technique::Input newInput;
      newInput.name = varName;
      newInput.type = type;
      newTech->AddInput (newInput);
    }
    {
      Technique::Output newOutput;
      newOutput.name = varName;
      newOutput.type = type;
      newTech->AddOutput (newOutput);
    }
    
    return newTech;
  }
  
  void Snippet::LoadAtomTechniques (iDocumentNode* node)
  {
    csRef<iDocumentNodeIterator> nodes = node->GetNodes ();
    while (nodes->HasNext ())
    {
      csRef<iDocumentNode> child = nodes->Next ();
      if (child->GetType() != CS_NODE_ELEMENT) continue;
      
      csStringID id = xmltokens.Request (child->GetValue());
      switch (id)
      {
        case WeaverCompiler::XMLTOKEN_TECHNIQUE:
          {
            LoadAtomTechnique (child);
          }
          break;
        default:
          compiler->synldr->ReportBadToken (child);
      }
    }
  }
  
  void Snippet::LoadAtomTechnique (iDocumentNode* node)
  {
    AtomTechnique* newTech = ParseAtomTechnique (compiler, node);
    if (newTech != 0)
      techniques.InsertSorted (newTech, &CompareTechnique);
  }
  
  Snippet::AtomTechnique* Snippet::ParseAtomTechnique (
    WeaverCompiler* compiler, iDocumentNode* node)
  {
    AtomTechnique newTech (
      csMD5::Encode (CS::DocumentHelper::FlattenNode (node)));
    
    newTech.priority = node->GetAttributeValueAsInt ("priority");
    
    {
      bool hasCombiner = false;
      csRef<iDocumentNodeIterator> nodes = node->GetNodes ("combiner");
      while (nodes->HasNext ())
      {
	csRef<iDocumentNode> child = nodes->Next ();
	if (child->GetType() != CS_NODE_ELEMENT) continue;
	
	if (hasCombiner)
	{
	  compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
	    "Multiple 'combiner' nodes");
	}
	
	Technique::CombinerPlugin newCombiner;
	newCombiner.name = child->GetAttributeValue ("name");
	if (newCombiner.name.IsEmpty())
	{
	  compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
	    "'combiner' node without 'name' attribute");
	  return 0;
	}
	newCombiner.classId = child->GetAttributeValue ("plugin");
	if (newCombiner.classId.IsEmpty())
	{
	  compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
	    "'combiner' node without 'plugin' attribute");
	  return 0;
	}
	newCombiner.params = child;
	
	if (!hasCombiner)
	{
	  newTech.SetCombiner (newCombiner);
	  hasCombiner = true;
	}
      }
    }
    
    {
      csRef<iDocumentNodeIterator> nodes = node->GetNodes ("input");
      while (nodes->HasNext ())
      {
	csRef<iDocumentNode> child = nodes->Next ();
	if (child->GetType() != CS_NODE_ELEMENT) continue;
	
	Technique::Input newInput;
	newInput.name = child->GetAttributeValue ("name");
	if (newInput.name.IsEmpty())
	{
	  compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
	    "'input' node without 'name' attribute");
	  return 0;
	}
	newInput.type = child->GetAttributeValue ("type");
	if (newInput.type.IsEmpty())
	{
	  compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
	    "'input' node without 'type' attribute");
	  return 0;
	}
	const char* def = child->GetAttributeValue ("default");
	if (def != 0)
	{
	  if (strcmp (def, "complex") == 0)
	  {
	    newInput.defaultType = Technique::Input::Complex;
	    if (!ReadBlocks (compiler, child, newInput.complexBlocks))
	      return 0;
	  }
	  else
	  {
	    compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
	      "Invalid 'default' attribute for 'input' node: %s", def);
	  }
	}
        const char* condition = child->GetAttributeValue ("condition");
        newInput.condition = condition;
        if (child->GetAttributeValueAsBool ("private"))
          newInput.flags |= Technique::Input::flagPrivate;
        if (child->GetAttributeValueAsBool ("forcenomerge"))
          newInput.flags |= Technique::Input::flagNoMerge;
	
	newTech.AddInput (newInput);
      }
    }
    
    {
      csRef<iDocumentNodeIterator> nodes = node->GetNodes ("output");
      while (nodes->HasNext ())
      {
	csRef<iDocumentNode> child = nodes->Next ();
	if (child->GetType() != CS_NODE_ELEMENT) continue;
	
	Technique::Output newOutput;
	newOutput.name = child->GetAttributeValue ("name");
	if (newOutput.name.IsEmpty())
	{
	  compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
	    "'output' node without 'name' attribute");
	  return 0;
	}
	newOutput.type = child->GetAttributeValue ("type");
	if (newOutput.type.IsEmpty())
	{
	  compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
	    "'output' node without 'type' attribute");
	  return 0;
	}
	
	newTech.AddOutput (newOutput);
      }
      
      csArray<Technique::Block> newBlocks;
      if (!ReadBlocks (compiler, node, newBlocks))
        return 0;
      for (size_t b = 0; b < newBlocks.GetSize(); b++)
        newTech.AddBlock (newBlocks[b]);
    }
    return new AtomTechnique (newTech);
  }
  
  bool Snippet::ReadBlocks (WeaverCompiler* compiler, iDocumentNode* node, 
		            csArray<Technique::Block>& blocks)
  {
    csRef<iDocumentNodeIterator> nodes = node->GetNodes ("block");
    while (nodes->HasNext ())
    {
      csRef<iDocumentNode> child = nodes->Next ();
      if (child->GetType() != CS_NODE_ELEMENT) continue;
      
      Technique::Block newBlock;
      csString location = child->GetAttributeValue ("location");
      if (location.IsEmpty())
      {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
	  "'block' node without 'location' attribute");
	return false;
      }
      size_t colon = location.FindFirst (':');
      if (colon != (size_t)-1)
      {
        // @@@ FIXME: Validate
        newBlock.combinerName = location.Slice (0, colon);
        newBlock.location = location.Slice (colon + 1);
      }
      else
        newBlock.location = location;
      newBlock.node = child;
      
      blocks.Push (newBlock);
    }
    return true;
  }
  
  int Snippet::CompareTechnique (Technique* const& t1, 
			         Technique* const& t2)
  {
    int v = t2->priority - t1->priority;
    return v;
  }
  
  void Snippet::LoadCompoundTechniques (iDocumentNode* node, bool topLevel)
  {
    csRef<iDocumentNodeIterator> nodes = node->GetNodes ();
    while (nodes->HasNext ())
    {
      csRef<iDocumentNode> child = nodes->Next ();
      if (child->GetType() != CS_NODE_ELEMENT) continue;
      
      csStringID id = xmltokens.Request (child->GetValue());
      switch (id)
      {
        case WeaverCompiler::XMLTOKEN_TECHNIQUE:
          {
            LoadCompoundTechnique (child);
          }
          break;
        default:
          compiler->synldr->ReportBadToken (child);
      }
    }
  }
  
  void Snippet::LoadCompoundTechnique (iDocumentNode* node)
  {
    CompoundTechnique* newTech = new CompoundTechnique;
  
    csRef<iDocumentNodeIterator> nodes = node->GetNodes ();
    while (nodes->HasNext ())
    {
      csRef<iDocumentNode> child = nodes->Next ();
      if (child->GetType() != CS_NODE_ELEMENT) continue;
      
      csStringID id = xmltokens.Request (child->GetValue());
      switch (id)
      {
        case WeaverCompiler::XMLTOKEN_COMBINER:
          {
            HandleCombinerNode (*newTech, child);
          }
          break;
        case WeaverCompiler::XMLTOKEN_SNIPPET:
          {
            HandleSnippetNode (*newTech, child);
          }
          break;
	case WeaverCompiler::XMLTOKEN_CONNECTION:
	  {
	    HandleConnectionNode (*newTech, child);
	  }
	  break;
        default:
          compiler->synldr->ReportBadToken (child);
      }
    }
    
    techniques.Push (newTech);
  }
  
  void Snippet::HandleSnippetNode (CompoundTechnique& tech,
                                   iDocumentNode* node)
  {
    const char* id = node->GetAttributeValue ("id");
    
    if (id == 0)
    {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	  "Referenced snippets must have 'id' attribute");
	return;
    }
    if (tech.GetSnippet (id) != 0)
    {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	  "Duplicate snippet id '%s'", id);
	return;
    }
      
    csRef<iDocumentNode> snippetNode;
    const char* filename = node->GetAttributeValue ("file");
    if (filename != 0)
    {
      csRef<iFile> file = compiler->vfs->Open (filename, VFS_FILE_READ);
      if (!file)
      {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	  "Unable to open snippet program file '%s'", filename);
	return;
      }
      csRef<iDocumentSystem> docsys (
	csQueryRegistry<iDocumentSystem> (compiler->objectreg));
      if (docsys == 0)
	docsys.AttachNew (new csTinyDocumentSystem ());
    
      csRef<iDocument> doc = docsys->CreateDocument ();
      const char* err = doc->Parse (file);
      if (err != 0)
      {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	  "Unable to parse snippet file '%s': %s", filename, err);
	return;
      }
    
      snippetNode = doc->GetRoot ()->GetNode ("snippet");
      if (!snippetNode.IsValid())
      {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING,
	  "Expected 'snippet' node in file '%s'", filename);
	return;
      }
    }
    else
      snippetNode = node;
      
    Snippet* newSnippet = new Snippet (compiler, snippetNode);
    tech.AddSnippet (id, newSnippet);
  }
    
  void Snippet::HandleConnectionNode (CompoundTechnique& tech, 
                                      iDocumentNode* node)
  {
    Connection newConn;
  
    {
      const char* fromId = node->GetAttributeValue ("from");
      if (fromId == 0)
      {
	  compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	    "'connection' node lacks 'from' attribute");
	  return;
      }
      
      newConn.from = tech.GetSnippet (fromId);
      if (newConn.from == 0)
      {
	  compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	    "Invalid 'from' attribute %s", fromId);
	  return;
      }
    }
    {
      const char* toId = node->GetAttributeValue ("to");
      if (toId == 0)
      {
	  compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	    "'connection' node lacks 'to' attribute");
	  return;
      }
      
      newConn.to = tech.GetSnippet (toId);
      if (newConn.to == 0)
      {
	  compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	    "Invalid 'from' attribute %s", toId);
	  return;
      }
    }
    tech.AddConnection (newConn);
  }
    
  void Snippet::HandleCombinerNode (CompoundTechnique& tech, 
				    iDocumentNode* node)
  {
    if (!tech.combiner.classId.IsEmpty())
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	"Multiple 'combiner' nodes");
    }
  
    Technique::CombinerPlugin newCombiner;
    newCombiner.classId = node->GetAttributeValue ("plugin");
    if (newCombiner.classId.IsEmpty())
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	"'combiner' node without 'plugin' attribute");
      return;
    }
    newCombiner.params = node;
    
    if (tech.combiner.classId.IsEmpty())
      tech.combiner = newCombiner;
  }
  
  //-------------------------------------------------------------------
  Snippet::CompoundTechnique::~CompoundTechnique()
  {
    IdSnippetHash::GlobalIterator it (snippets.GetIterator());
    while (it.HasNext())
    {
      Snippet* snip = it.Next();
      delete snip;
    }
  }

  void Snippet::CompoundTechnique::AddSnippet (const char* id, 
                                               Snippet* snippet)
  {
    snippets.Put (id, snippet);
    inSnippets.Push (snippet);
    outSnippets.Push (snippet);
  }
  
  void Snippet::CompoundTechnique::AddConnection (const Connection& conn)
  {
    connections.Push (conn);
    inSnippets.Delete (conn.to);
    outSnippets.Delete (conn.from);
  }
  
  BasicIterator<const Snippet::Technique::Input>* 
  Snippet::CompoundTechnique::GetInputs() const
  {
    CS_ASSERT(false);
    return 0;
  }
  
  BasicIterator<const Snippet::Technique::Output>* 
  Snippet::CompoundTechnique::GetOutputs() const
  {
    CS_ASSERT(false);
    return 0;
  }
  
  //-------------------------------------------------------------------
  
  void TechniqueGraph::AddTechnique (const Snippet::Technique* tech)
  {
    techniques.Push (tech);
    outTechniques.Push (tech);
  }
  
  void TechniqueGraph::AddConnection (const Connection& conn)
  {
    connections.Push (conn);
    outTechniques.Delete (conn.from);
  }

  void TechniqueGraph::RemoveConnection (const Connection& conn)
  {
    connections.Delete (conn);
  }

  void TechniqueGraph::Merge (const TechniqueGraph& other)
  {
    for (size_t t = 0; t < other.techniques.GetSize(); t++)
      AddTechnique (other.techniques[t]);
    for (size_t c = 0; c < other.connections.GetSize(); c++)
      AddConnection (other.connections[c]);
  }
  
  void TechniqueGraph::GetDependencies (const Snippet::Technique* tech, 
    csArray<const Snippet::Technique*>& deps) const
  {
    csSet<csConstPtrKey<Snippet::Technique> > addedDeps;
    for (size_t c = 0; c < connections.GetSize(); c++)
    {
      const Connection& conn = connections[c];
      if ((conn.to == tech) && (!addedDeps.Contains (conn.from)))
      {
        deps.Push (conn.from);
        addedDeps.AddNoTest (conn.from);
      }
    }
  }
  
  //-------------------------------------------------------------------
  
  void TechniqueGraphBuilder::GraphInfo::Merge (const GraphInfo& other)
  {
    graph.Merge (other.graph);
    {
      SnippetToTechMap::ConstGlobalIterator inIt = 
        other.snippetToTechIn.GetIterator();
      while (inIt.HasNext())
      {
        csConstPtrKey<Snippet> snippet;
        const Snippet::Technique* tech = inIt.Next (snippet);
        snippetToTechIn.Put (snippet, tech);
      }
    }
    {
      SnippetToTechMap::ConstGlobalIterator outIt = 
        other.snippetToTechOut.GetIterator();
      while (outIt.HasNext())
      {
        csConstPtrKey<Snippet> snippet;
        const Snippet::Technique* tech = outIt.Next (snippet);
        snippetToTechOut.Put (snippet, tech);
      }
    }
  }
  
  void TechniqueGraphBuilder::BuildSubGraphs (const Snippet* snip, 
                                              csArray<GraphInfo>& graphs)
  {
    CS::ScopedDelete<BasicIterator<const Snippet::Technique*> > techIter (
      snip->GetTechniques());
    while (techIter->HasNext())
    {
      const Snippet::Technique* tech = techIter->Next();
      if (tech->IsCompound ())
      {
        const Snippet::CompoundTechnique* compTech =
          static_cast<const Snippet::CompoundTechnique*> (tech);
	// Each sub-snippet ...
	Snippet::CompoundTechnique::IdSnippetHash::ConstGlobalIterator
	  snippetIter = compTech->snippets.GetIterator();
	while (snippetIter.HasNext())
	{
	  Snippet* snippet = snippetIter.Next ();
	  if (graphs.GetSize() == 0)
	  {
	    BuildSubGraphs (snippet, graphs);
	  }
	  else
	  {
	    csArray<GraphInfo> newGraphs;
	    // Returns a number of techniques
  	    BuildSubGraphs (snippet, newGraphs);
	    csArray<GraphInfo> graphs2;
	    /* Merge each technique by copying the current graph(s), adding
	     * the technique into each. */
	    for (size_t g = 0; g < newGraphs.GetSize(); g++)
	    {
	      for (size_t g2 = 0; g2 < graphs.GetSize(); g2++)
	      {
		GraphInfo graphMerged (graphs[g2]);
		graphMerged.Merge (newGraphs[g]);
		graphs2.Push (graphMerged);
	      }
	    }
	    graphs = graphs2;
	  }
	}
	/* Apply connections.
	 * Connect "out" techs to "in" techs.
	 */
	for (size_t g = 0; g < graphs.GetSize(); g++)
	{
	  GraphInfo& graphInfo = graphs[g];
	  for (size_t c = 0; c < compTech->connections.GetSize(); c++)
	  {
	    TechniqueGraph::Connection newConn;
	    const Snippet::Connection& conn = compTech->connections[c];
	    SnippetToTechMap::Iterator fromIt = 
	      graphInfo.snippetToTechOut.GetIterator (conn.from);
	    while (fromIt.HasNext())
	    {
	      newConn.from = fromIt.Next();
	      SnippetToTechMap::Iterator toIt = 
		graphInfo.snippetToTechIn.GetIterator (conn.to);
	      while (toIt.HasNext())
	      {
	        newConn.to = toIt.Next();
	        graphInfo.graph.AddConnection (newConn);
	      }
	    }
	  }
	}
      }
      else
      {
	GraphInfo graphInfo;
	graphInfo.snippetToTechIn.PutUnique (snip, tech);
	graphInfo.snippetToTechOut.PutUnique (snip, tech);
	graphInfo.graph.AddTechnique (tech);
	graphs.Push (graphInfo);
      }
    }
  }
      
  void TechniqueGraphBuilder::BuildGraphs (const Snippet* snip, 
                                           csArray<TechniqueGraph>& graphs)
  {
    csArray<GraphInfo> graphInfos;
    BuildSubGraphs (snip, graphInfos);
    for (size_t g = 0; g < graphInfos.GetSize(); g++)
      graphs.Push (graphInfos[g].graph);
  }
}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)
