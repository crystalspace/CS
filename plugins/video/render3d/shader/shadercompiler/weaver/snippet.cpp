/*
  Copyright (C) 2003-2007 by Marten Svanfeldt
            (C) 2004-2011 by Frank Richter

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
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"

#include "csplugincommon/shader/shaderprogram.h"
#include "csplugincommon/shader/weavercombiner.h"
#include "cstool/identstrings.h"
#include "csutil/documenthelper.h"
#include "csutil/fifo.h"
#include "csutil/scopeddelete.h"
#include "csutil/stringquote.h"

#include "snippet.h"
#include "weaver.h"

CS_PLUGIN_NAMESPACE_BEGIN(ShaderWeaver)
{
  namespace WeaverCommon = CS::PluginCommon::ShaderWeaver;

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
    virtual T& Next()
    { 
      storage = array[pos++]; 
      return storage;
    }
    virtual size_t GetTotal() const
    { return array.GetSize(); }
  };

  //-------------------------------------------------------------------
  
  template<class HashType>
  class BasicIteratorImplHashValues :
    public BasicIterator<typename HashType::ValueType>
  {
    typename HashType::GlobalIterator iter;
    size_t total;
  public:
    BasicIteratorImplHashValues (HashType& hash) : iter (hash.GetIterator()),
      total (hash.GetSize()) {}
  
    virtual bool HasNext()
    { return iter.HasNext(); }
    virtual typename HashType::ValueType& Next()
    { return iter.Next(); }
    virtual size_t GetTotal() const
    { return total; }
  };

  //-------------------------------------------------------------------
  
  csString Snippet::Technique::GetInnerCondition() const
  {
    if (owner != 0) return owner->GetCondition();
    return (char*)0;
  }
      
  //-------------------------------------------------------------------

  Snippet::Snippet (const WeaverCompiler* compiler, iDocumentNode* node, 
                    const char* name, const FileAliases& aliases,
                    const Snippet* parent) : compiler (compiler), 
    xmltokens (compiler->xmltokens), name (name), node (node),
    isCompound (false), passForward (false), parent (parent)
  {
    bool okay = true;
    if (parent == 0)
    {
      isCompound = true;
      passForward = true;
      LoadCompoundTechnique (node, aliases);
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
	    "Unknown snippet type %s", CS::Quote::Single (snippetType));
	  okay = false;
	}
      }
      if (okay)
      {
	if (!isCompound)
	  LoadAtomTechniques (node, aliases);
	else
	{
	  LoadCompoundTechniques (node, aliases, parent == 0);
	}
      }
    }
  }
  
  Snippet::Snippet (const WeaverCompiler* compiler, const char* name) : compiler (compiler), 
    xmltokens (compiler->xmltokens), name (name), isCompound (false), parent (0)
  {
  }

  Snippet::~Snippet()
  {
  }
  
  csString Snippet::GetCondition() const
  {
    if (condition.IsEmpty())
    {
      if (parent == 0)
        return csString();
      else
        return parent->GetCondition();
    }
    else
    {
      csString cond;
      if (parent != 0)
      {
        cond = parent->GetCondition();;
        if (!cond.IsEmpty()) cond += " && ";
      }
      cond.AppendFmt ("(%s)", condition.GetData());
      return cond;
    }
  }
      
  BasicIterator<const Snippet::Technique*>* Snippet::GetTechniques() const
  {
    return new BasicIteratorImplCopyValue<const Technique*, TechniqueArray> (
      techniques);
  }
  
  BasicIterator<Snippet::Technique*>* Snippet::GetTechniques()
  {
    return new BasicIteratorImplCopyValue<Technique*, TechniqueArray> (
      techniques);
  }
  
  Snippet::Technique* Snippet::LoadLibraryTechnique (/*WeaverCompiler* compiler, */
    iDocumentNode* node, const Technique::CombinerPlugin& combiner,
    bool markAsCoercion) const
  {
    FileAliases aliases;
    Snippet::AtomTechnique* technique = 
      ParseAtomTechnique (node, true, aliases, combiner.name);
    technique->combiner = combiner;
    if (markAsCoercion)
    {
      CS::Utility::ScopedDelete<BasicIterator<Snippet::Technique::Output> > 
	outputIt (technique->GetOutputs());
      while (outputIt->HasNext())
      {
        Snippet::Technique::Output& outp = outputIt->Next();
	outp.coercionOutput = true;
      }
    }
    return technique;
  }
  
  Snippet::Technique* Snippet::CreatePassthrough (const char* varName, 
                                                  const char* type) const
  {
    csString hashStr;
    hashStr.Format ("__passthrough_%s_%s__", varName, type);
    AtomTechnique* newTech = new AtomTechnique (0, "(passthrough)", 
      csMD5::Encode (hashStr));
    
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
    
  bool Snippet::ParseAliasNode (const WeaverCompiler* compiler,
                                iDocumentNode* child, 
                                FileAliases& aliases)
  {
    const char* aliasName = child->GetAttributeValue ("name");
    if (aliasName == 0)
    {
      compiler->synldr->ReportBadToken (child);
      return false;
    }
    const char* aliasFile = child->GetAttributeValue ("file");
    if (aliasFile == 0)
    {
      compiler->synldr->ReportBadToken (child);
      return false;
    }
    
    bool isWeak = child->GetAttributeValueAsBool ("weak");
    if (!isWeak || !aliases.Contains (aliasName))
      aliases.PutUnique (aliasName, aliasFile);
    return true;
  }
  
  void Snippet::LoadAtomTechniques (iDocumentNode* node,
                                    const FileAliases& _aliases)
  {
    FileAliases aliases (_aliases);
    // Aliases
    {
      csRef<iDocumentNodeIterator> nodes = node->GetNodes ("alias");
      while (nodes->HasNext ())
      {
	csRef<iDocumentNode> child = nodes->Next ();
	if (child->GetType() != CS_NODE_ELEMENT) continue;
	
	ParseAliasNode (compiler, child, aliases);
      }
    }

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
            LoadAtomTechnique (child, aliases);
          }
          break;
        default:
          compiler->synldr->ReportBadToken (child);
      }
    }
  }
  
  void Snippet::LoadAtomTechnique (iDocumentNode* node, 
                                   const FileAliases& aliases)
  {
    AtomTechnique* newTech = ParseAtomTechnique (node, false, aliases);
    if (newTech != 0)
      techniques.InsertSorted (newTech, &CompareTechnique);
  }
  
  Snippet::AtomTechnique* Snippet::ParseAtomTechnique (
    iDocumentNode* node, bool canOmitCombiner,
    const FileAliases& _aliases, const char* defaultCombinerName) const
  {
    FileAliases aliases (_aliases);
    AtomTechnique newTech (this, GetName(),
      csMD5::Encode (CS::DocSystem::FlattenNode (node)));
    
    newTech.priority = node->GetAttributeValueAsInt ("priority");
    newTech.outerCondition = node->GetAttributeValue ("condition");
    
    // Aliases
    {
      csRef<iDocumentNodeIterator> nodes = node->GetNodes ("alias");
      while (nodes->HasNext ())
      {
	csRef<iDocumentNode> child = nodes->Next ();
	if (child->GetType() != CS_NODE_ELEMENT) continue;
	
	ParseAliasNode (compiler, child, aliases);
      }
    }

    // Combiner nodes
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
	    "Multiple %s nodes", CS::Quote::Single ("combiner"));
	}
	
	Technique::CombinerPlugin newCombiner;
        if (!ParseCombiner (child, newCombiner)) return 0;
	
	if (!hasCombiner)
	{
	  newTech.SetCombiner (newCombiner);
	  hasCombiner = true;
	}
      }
      if (!canOmitCombiner && !hasCombiner)
      {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	  "Technique without %s node", CS::Quote::Single ("combiner"));
        return 0;
      }
    }
    
    // Inputs
    {
      csRef<iDocumentNodeIterator> nodes = node->GetNodes ("input");
      while (nodes->HasNext ())
      {
	csRef<iDocumentNode> child = nodes->Next ();
	if (child->GetType() != CS_NODE_ELEMENT) continue;
	
        if (!ParseInput (child, newTech, aliases, defaultCombinerName))
          return 0;	
      }
    }

    // Outputs
    {
      csRef<iDocumentNodeIterator> nodes = node->GetNodes ("output");
      while (nodes->HasNext ())
      {
	csRef<iDocumentNode> child = nodes->Next ();
	if (child->GetType() != CS_NODE_ELEMENT) continue;
	
	Technique::Output newOutput;
        if (!ParseOutput (child, newOutput)) return 0;	
	newTech.AddOutput (newOutput);
      }
      
      csArray<Technique::Block> newBlocks;
      if (!ReadBlocks (compiler, node, newBlocks, aliases, defaultCombinerName))
        return 0;
      for (size_t b = 0; b < newBlocks.GetSize(); b++)
        newTech.AddBlock (newBlocks[b]);
    }
    return new AtomTechnique (newTech);
  }

  bool Snippet::ParseCombiner (iDocumentNode* child, 
                               Technique::CombinerPlugin& newCombiner) const
  {
    newCombiner.name = child->GetAttributeValue ("name");
    if (newCombiner.name.IsEmpty())
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
        "%s node without %s attribute",
	CS::Quote::Single ("combiner"), CS::Quote::Single ("name"));
      return false;
    }
    newCombiner.classId = child->GetAttributeValue ("plugin");
    if (newCombiner.classId.IsEmpty())
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
        "%s node without %s attribute",
	CS::Quote::Single ("combiner"), CS::Quote::Single ("plugin"));
      return false;
    }
    newCombiner.params = child;

    return true;
  }

  bool Snippet::ParseInput (iDocumentNode* child, 
                            AtomTechnique& newTech,
                            const FileAliases& aliases,
                            const char* defaultCombinerName) const
  {
    Technique::Input newInput;
    
    const char* condition = child->GetAttributeValue ("condition");
    newInput.condition = condition;
    if (child->GetAttributeValueAsBool ("private"))
      newInput.isPrivate = true;
    if (child->GetAttributeValueAsBool ("forcenomerge"))
      newInput.noMerge = true;

    csRef<iDocumentNode> inputNode = GetNodeOrFromFile (child, "input",
      compiler, aliases);
    if (!inputNode.IsValid()) return false;

    newInput.node = inputNode;
    newInput.name = inputNode->GetAttributeValue ("name");
    if (newInput.name.IsEmpty())
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, inputNode,
        "%s node without %s attribute",
	CS::Quote::Single ("input"), CS::Quote::Single ("name"));
      return false;
    }
    newInput.type = inputNode->GetAttributeValue ("type");
    if (newInput.type.IsEmpty())
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, inputNode,
        "%s node without %s attribute",
	CS::Quote::Single ("input"), CS::Quote::Single ("type"));
      return false;
    }
    const char* def = inputNode->GetAttributeValue ("default");
    if (def != 0)
    {
      if (strcmp (def, "complex") == 0)
      {
        newInput.defaultType = Technique::Input::Complex;
        if (!ReadBlocks (compiler, inputNode, newInput.complexBlocks, 
            aliases, defaultCombinerName))
	  return 0;
      }
      else if (strcmp (def, "value") == 0)
      {
        newInput.defaultType = Technique::Input::Value;
        newInput.defaultValue = inputNode->GetAttributeValue ("defval");
        if (newInput.defaultValue.IsEmpty())
        {
          compiler->Report (CS_REPORTER_SEVERITY_WARNING, inputNode,
            "%s node with a %s default but without %s attribute",
	    CS::Quote::Single ("input"),
	    CS::Quote::Single ("value"),
	    CS::Quote::Single ("defval"));
          return false;
        }
      }
      else if (strcmp (def, "undefined") == 0)
      {
        newInput.defaultType = Technique::Input::Undefined;
      }
      else if (strcmp (def, "shadervar") == 0)
      {
	const char* svName = inputNode->GetAttributeValue ("defsv");
        if (!svName || !*svName)
        {
          compiler->Report (CS_REPORTER_SEVERITY_WARNING, inputNode,
            "%s node with a %s default but without %s attribute",
	    CS::Quote::Single ("input"),
	    CS::Quote::Single ("shadervar"),
	    CS::Quote::Single ("defsv"));
          return false;
        }
        
	csRef<WeaverCommon::iCombinerLoader> combinerLoader = 
	  csLoadPluginCheck<WeaverCommon::iCombinerLoader> (compiler->objectreg,
	    newTech.combiner.classId, false);
	if (!combinerLoader.IsValid())
	{
	  // Don't complain, will happen later anyway
	  return false;
	}
	
	csRef<iDocumentNode> svBlocksNode = 
	  compiler->CreateAutoNode (CS_NODE_ELEMENT);
	combinerLoader->GenerateSVInputBlocks (svBlocksNode, "c", 
	  svName, newInput.type, newInput.name, newInput.name);
        if (!ReadBlocks (compiler, svBlocksNode, newInput.complexBlocks, 
            aliases, defaultCombinerName))
	  return false;
        newInput.defaultType = Technique::Input::Complex;
      }
      else
      {
        compiler->Report (CS_REPORTER_SEVERITY_WARNING, inputNode,
          "Invalid %s attribute for %s node: %s",
	  CS::Quote::Single ("default"), CS::Quote::Single ("input"),
	  def);
      }
    }

    csRef<iDocumentNodeIterator> attrNodes = child->GetNodes ("attribute");
    while (attrNodes->HasNext ())
    {
      csRef<iDocumentNode> attrNode = attrNodes->Next ();
      if (attrNode->GetType() != CS_NODE_ELEMENT) continue;

      Technique::Attribute newAttr;
      if (!ParseAttribute (attrNode, newAttr)) return false;
      newInput.attributes.Push (newAttr);
    }

    newTech.AddInput (newInput);
    return true;
  }

  bool Snippet::ParseOutput (iDocumentNode* child, 
                             Technique::Output& newOutput) const
  {
    newOutput.name = child->GetAttributeValue ("name");
    if (newOutput.name.IsEmpty())
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
        "%s node without %s attribute",
	CS::Quote::Single ("output"), CS::Quote::Single ("name"));
      return false;
    }
    newOutput.type = child->GetAttributeValue ("type");
    if (newOutput.type.IsEmpty())
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
        "%s node without %s attribute",
	CS::Quote::Single ("output"), CS::Quote::Single ("type"));
      return false;
    }

    newOutput.inheritAttrFrom = child->GetAttributeValue ("inheritattr");


    csRef<iDocumentNodeIterator> attrNodes = child->GetNodes ("attribute");
    while (attrNodes->HasNext ())
    {
      csRef<iDocumentNode> attrNode = attrNodes->Next ();
      if (attrNode->GetType() != CS_NODE_ELEMENT) continue;

      Technique::Attribute newAttr;
      if (!ParseAttribute (attrNode, newAttr)) return false;
      newOutput.attributes.Push (newAttr);
    }

    return true;
  }

  bool Snippet::ParseAttribute (iDocumentNode* child, 
                                Technique::Attribute& attr) const
  {
    attr.name = child->GetAttributeValue ("name");
    if (attr.name.IsEmpty())
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
        "%s node without %s attribute",
	CS::Quote::Single ("attribute"), CS::Quote::Single ("name"));
      return false;
    }
    attr.type = child->GetAttributeValue ("type");
    if (attr.type.IsEmpty())
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
        "%s node without %s attribute",
	CS::Quote::Single ("attribute"),
	CS::Quote::Single ("type"));
      return false;
    }

    attr.defaultValue = child->GetAttributeValue ("defval");

    return true;

  }

  bool Snippet::ReadBlocks (const WeaverCompiler* compiler, 
                            iDocumentNode* node, 
		            csArray<Technique::Block>& blocks,
		            const FileAliases& aliases,
                            const char* defaultCombinerName)
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
	  "%s node without %s attribute",
	  CS::Quote::Single ("block"),
	  CS::Quote::Single ("location"));
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
      {
        newBlock.combinerName = defaultCombinerName;
        newBlock.location = location;
      }
      newBlock.node = GetNodeOrFromFile (child, "block", compiler, aliases);
      if (!newBlock.node) return false;
      
      blocks.Push (newBlock);
    }
    return true;
  }
  
  csRef<iDocumentNode> Snippet::GetNodeOrFromFile (iDocumentNode* node,
    const char* rootName, const WeaverCompiler* compiler,
    const FileAliases& aliases, csString* outFilename)
  {
    const char* filename = 0;
    const char* filenameAlias = node->GetAttributeValue ("filealias");
    if (filenameAlias != 0)
    {
      filename = aliases.Get (filenameAlias, (const char*)0);
      if (!filename)
      {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
			  "filealias %s not specified",
			  CS::Quote::Single (filenameAlias));
      }
    }
    else
      filename = node->GetAttributeValue ("file");
    if (filename != 0)
    {
      csRef<iDocumentNode> rootNode = 
	  compiler->LoadDocumentFromFile (filename, node);
      if (!rootNode.IsValid()) return 0;
    
      csRef<iDocumentNode> fileNode = rootNode->GetNode (rootName);
      if (!fileNode.IsValid())
      {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING,
          "Expected %s node in file %s",
	  CS::Quote::Single (rootName), CS::Quote::Single (filename));
	return 0;
      }
      if (outFilename != 0) *outFilename = filename;
      return fileNode;
    }
    else
      return node;
  }
  
  int Snippet::CompareTechnique (Technique* const& t1, 
			         Technique* const& t2)
  {
    int v = t2->priority - t1->priority;
    return v;
  }
  
  void Snippet::LoadCompoundTechniques (iDocumentNode* node, 
                                        const FileAliases& _aliases,
                                        bool topLevel)
  {
    FileAliases aliases (_aliases);
  
    // Aliases
    {
      csRef<iDocumentNodeIterator> nodes = node->GetNodes ("alias");
      while (nodes->HasNext ())
      {
	csRef<iDocumentNode> child = nodes->Next ();
	if (child->GetType() != CS_NODE_ELEMENT) continue;
	
	ParseAliasNode (compiler, child, aliases);
      }
    }

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
            LoadCompoundTechnique (child, aliases);
          }
          break;
        case WeaverCompiler::XMLTOKEN_ALIAS:
          break;
        default:
          compiler->synldr->ReportBadToken (child);
      }
    }
  }
  
  void Snippet::LoadCompoundTechnique (iDocumentNode* node, 
                                       const FileAliases& _aliases)
  {
    FileAliases aliases (_aliases);
  
    // Aliases
    {
      csRef<iDocumentNodeIterator> nodes = node->GetNodes ("alias");
      while (nodes->HasNext ())
      {
	csRef<iDocumentNode> child = nodes->Next ();
	if (child->GetType() != CS_NODE_ELEMENT) continue;
	
	ParseAliasNode (compiler, child, aliases);
      }
    }

    CompoundTechnique* newTech = new CompoundTechnique (this,
      GetName());
  
    newTech->priority = node->GetAttributeValueAsInt ("priority");
    
    csRef<iDocumentNodeIterator> nodes = node->GetNodes ();
    while (nodes->HasNext ())
    {
      csRef<iDocumentNode> child = nodes->Next ();
      if (child->GetType() != CS_NODE_ELEMENT)
      {
        if (passForward) passForwardedNodes.Push (child);
        continue;
      }
      
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
            HandleSnippetNode (*newTech, child, aliases);
          }
          break;
	case WeaverCompiler::XMLTOKEN_CONNECTION:
	  {
	    HandleConnectionNode (*newTech, child);
	  }
	  break;
        case WeaverCompiler::XMLTOKEN_PARAMETER:
          {
            HandleParameterNode (*newTech, child, aliases);
          }
          break;
        case WeaverCompiler::XMLTOKEN_VARYING:
          {
            HandleVaryingNode (*newTech, child, aliases);
          }
          break;
        case WeaverCompiler::XMLTOKEN_ALIAS:
          break;
        default:
          if (passForward)
            passForwardedNodes.Push (child);
          else
            compiler->synldr->ReportBadToken (child);
      }
    }
    
    techniques.Push (newTech);
  }
  
  void Snippet::HandleSnippetNode (CompoundTechnique& tech,
                                   iDocumentNode* node,
                                   const FileAliases& aliases)
  {
    const char* id = node->GetAttributeValue ("id");
    
    if (id == 0)
    {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	  "Referenced snippets must have an %s attribute",
	  CS::Quote::Single ("id"));
	return;
    }
    if (tech.GetSnippet (id) != 0)
    {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	  "Duplicate snippet id %s", CS::Quote::Single (id));
	return;
    }
    
    const char* condition = node->GetAttributeValue ("condition");
    
    csString filename;
    csRef<iDocumentNode> snippetNode = GetNodeOrFromFile (node, "snippet",
      compiler, aliases, &filename);
    if (!snippetNode.IsValid()) return;
      
    csString snippetName;
    if (!name.IsEmpty ())
    {
      snippetName.AppendFmt ("%s<%d> -> ",  name.GetData(), tech.priority);
    }
    snippetName += id ? id : filename.GetData();
    Snippet* newSnippet = new Snippet (compiler, snippetNode, 
      snippetName, aliases, this);
    newSnippet->condition = condition;
    tech.AddSnippet (id, newSnippet);
  }
  
  namespace
  {
    struct TechSnippetPair
    {
      Snippet* snip;
      Snippet::CompoundTechnique* tech;
      
      TechSnippetPair (Snippet* snip, Snippet::CompoundTechnique* tech)
	: snip (snip), tech (tech) {}
    };
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
	    "%s node lacks %s attribute",
	    CS::Quote::Single ("connection"),
	    CS::Quote::Single ("from"));
	  return;
      }
      
      newConn.from = tech.GetSnippet (fromId);
      if (newConn.from == 0)
      {
	  compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	    "Invalid %s attribute %s",
	    CS::Quote::Single ("from"),
	    fromId);
	  return;
      }
    }
    {
      const char* toId = node->GetAttributeValue ("to");
      if (toId == 0)
      {
	  compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	    "%s node lacks %s attribute",
	    CS::Quote::Single ("connection"), CS::Quote::Single ("to"));
	  return;
      }
      
      newConn.to = tech.GetSnippet (toId);
      if (newConn.to == 0)
      {
	  compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	    "Invalid %s attribute %s",
	    CS::Quote::Single ("from"),
	    toId);
	  return;
      }
    }
    
    tech.AddConnection (newConn);
    
    csRef<iDocumentNodeIterator> nodes = node->GetNodes();
    while (nodes->HasNext())
    {
      csRef<iDocumentNode> child = nodes->Next();
      if (child->GetType() != CS_NODE_ELEMENT) continue;
      if (strcmp (child->GetValue(), "explicit") != 0)
      {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
	  "Expected %s node", CS::Quote::Single ("explicit"));
	return;
      }
      const char* fromId = child->GetAttributeValue ("from");
      if (fromId == 0)
      {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
	  "%s node lacks %s attribute",
	  CS::Quote::Single ("explicit"), CS::Quote::Single ("from"));
	return;
      }
      const char* toId = child->GetAttributeValue ("to");
      if (toId == 0)
      {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
	  "%s node lacks %s attribute",
	  CS::Quote::Single ("explicit"), CS::Quote::Single ("to"));
	return;
      }
      
      csFIFO<TechSnippetPair> snippetsToAddTo;
      snippetsToAddTo.Push (TechSnippetPair (newConn.to, &tech));
      while (snippetsToAddTo.GetSize() > 0)
      {
        TechSnippetPair to = snippetsToAddTo.PopTop();
        
	if (to.snip->IsCompound())
	{
	  /* For compound snippets replicate the explicit connection
	    targetting the contained connections. */
	  CS::Utility::ScopedDelete<BasicIterator<Snippet::Technique*> > techIter (
	    to.snip->GetTechniques());
	  while (techIter->HasNext())
	  {
	    Snippet::Technique* tech = techIter->Next();
	    if (tech->IsCompound ())
	    {
	      CompoundTechnique* compTech = static_cast<CompoundTechnique*> (
	       tech);
	      CS::Utility::ScopedDelete<BasicIterator<Snippet*> > snipIter (
		compTech->GetSnippets());
	      while (snipIter->HasNext())
	      {
	        Snippet* snip = snipIter->Next();
	        snippetsToAddTo.Push (TechSnippetPair (snip, compTech));
	      }
	    }
	  }
	}
	else
	{
	  ExplicitConnectionsHash& explConn =
	    to.tech->GetExplicitConnections (to.snip);
	  if (explConn.Contains (toId))
	  {
	    if (to.snip == newConn.to)
	      compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
		"An explicit input was already mapped to %s", CS::Quote::Single (toId));
	  }
	  else
	  {
	    ExplicitConnectionSource connSrc;
	    connSrc.from = newConn.from;
	    connSrc.outputName = fromId;
	    explConn.Put (toId, connSrc);
	    
	    if (to.snip != newConn.to)
	    {
	      Connection newConn2 (newConn);
	      newConn2.to = to.snip;
              tech.AddConnection (newConn2);
            }
	  }
	}
      }
    }
  }
    
  void Snippet::HandleCombinerNode (CompoundTechnique& tech, 
				    iDocumentNode* node)
  {
    if (!tech.combiner.classId.IsEmpty())
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	"Multiple %s nodes",
	CS::Quote::Single ("combiner"));
    }
  
    Technique::CombinerPlugin newCombiner;
    newCombiner.classId = node->GetAttributeValue ("plugin");
    if (newCombiner.classId.IsEmpty())
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	"%s node without %s attribute",
	CS::Quote::Single ("combiner"),
	CS::Quote::Single ("plugin"));
      return;
    }
    newCombiner.params = node;
    
    if (tech.combiner.classId.IsEmpty())
      tech.combiner = newCombiner;
  }

  CS_IDENT_STRING_LIST(SVTypes)
    CS_IDENT_STRING(csShaderVariable::UNKNOWN)
    //CS_IDENT_STRING(csShaderVariable::INT)
    //CS_IDENT_STRING(csShaderVariable::FLOAT)
    CS_IDENT_STRING(csShaderVariable::TEXTURE)
    CS_IDENT_STRING(csShaderVariable::RENDERBUFFER)
    //CS_IDENT_STRING(csShaderVariable::VECTOR2)
    //CS_IDENT_STRING(csShaderVariable::VECTOR3)
    //CS_IDENT_STRING(csShaderVariable::VECTOR4)
    CS_IDENT_STRING(csShaderVariable::MATRIX)
    CS_IDENT_STRING(csShaderVariable::TRANSFORM)
    CS_IDENT_STRING(csShaderVariable::ARRAY)
  CS_IDENT_STRING_LIST_END(SVTypes)
  
  void Snippet::HandleParameterNode (CompoundTechnique& tech, 
				     iDocumentNode* node,
                                     const FileAliases& aliases)
  {
    if (tech.combiner.classId.IsEmpty())
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	"Need a combiner to use <parameter>");
      return;
    }

    const char* id = node->GetAttributeValue ("id");
    if (!id || !*id)
    {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	  "Parameters must have an %s attribute",
	  CS::Quote::Single ("id"));
	return;
    }
    if (tech.GetSnippet (id) != 0)
    {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	  "Duplicate snippet id %s", CS::Quote::Single (id));
	return;
    }

    csShaderProgram::ProgramParamParser paramParser (compiler->synldr,
      compiler->svstrings);
    csShaderProgram::ProgramParam param;
    if (!paramParser.ParseProgramParam (node, param, 
      csShaderProgram::ParamVector | csShaderProgram::ParamShaderExp)) return;
    if (!param.valid) return;

    csRef<WeaverCommon::iCombinerLoader> combinerLoader = 
      csLoadPluginCheck<WeaverCommon::iCombinerLoader> (compiler->objectreg,
        tech.combiner.classId, false);
    if (!combinerLoader.IsValid())
    {
      // Don't complain, will happen later anyway
      return;
    }

    csRef<iDocumentNode> snippetNode = 
      compiler->CreateAutoNode (CS_NODE_ELEMENT);
    snippetNode->SetValue ("snippet");
    snippetNode->SetAttribute ("id", id);
    csRef<iDocumentNode> techNode = 
      snippetNode->CreateNodeBefore (CS_NODE_ELEMENT);
    techNode->SetValue ("technique");
    {
      csRef<iDocumentNode> combinerNode = 
        techNode->CreateNodeBefore (CS_NODE_ELEMENT);
      combinerNode->SetValue ("combiner");
      combinerNode->SetAttribute ("name", "c");
      combinerNode->SetAttribute ("plugin", tech.combiner.classId);
    }
    csString weaverType;

    if (param.name == csInvalidStringID)
    {
      int numComps = 0;
      csShaderVariable::VariableType svType = param.var->GetType ();
      const char* typeStr = "float";
      switch (svType)
      {
	case csShaderVariable::INT:
	  numComps = 1; 
	  typeStr = "int";
	  break;
	case csShaderVariable::FLOAT:   numComps = 1; break;
	case csShaderVariable::VECTOR2: numComps = 2; break;
	case csShaderVariable::VECTOR3: numComps = 3; break;
	case csShaderVariable::VECTOR4: numComps = 4; break;
	default:
	  // Should not happen really, but who knows...
	  compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	    "Constant parameter of unsupported type %s", 
	    SVTypes.StringForIdent (svType));
	  return;
      }
      weaverType = node->GetAttributeValue ("weavertype");
      if (weaverType.IsEmpty())
      {
	if (numComps > 1)
	  weaverType.Format ("%s%d", typeStr, numComps);
	else
	  weaverType = typeStr;
      }

      csVector4 v;
      param.var->GetValue (v);
      combinerLoader->GenerateConstantInputBlocks (techNode, "c", v, 
        numComps, "output");
    }
    else
    {
      weaverType = node->GetAttributeValue ("weavertype");
      if (weaverType.IsEmpty())
      {
        compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	  "Need a %s attribute for non-constant parameters",
	  CS::Quote::Single ("weavertype"));
        return;
      }
      combinerLoader->GenerateSVInputBlocks (techNode, "c", 
        compiler->svstrings->Request (param.name), weaverType, "output", id);
    }

    {
      csRef<iDocumentNode> outputNode = 
        techNode->CreateNodeBefore (CS_NODE_ELEMENT);
      outputNode->SetValue ("output");
      outputNode->SetAttribute ("type", weaverType);
      outputNode->SetAttribute ("name", "output");
    }
    
    HandleSnippetNode (tech, snippetNode, aliases);
  }
  
  void Snippet::HandleVaryingNode (CompoundTechnique& tech, 
				   iDocumentNode* node,
                                   const FileAliases& aliases)
  {
    if (tech.combiner.classId.IsEmpty())
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	"Need a combiner to use <varying>");
      return;
    }

    const char* id = node->GetAttributeValue ("id");
    if (!id || !*id)
    {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	  "Varyings must have an %s attribute",
	  CS::Quote::Single ("id"));
	return;
    }
    if (tech.GetSnippet (id) != 0)
    {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	  "Duplicate snippet id %s", CS::Quote::Single (id));
	return;
    }

    csRef<WeaverCommon::iCombinerLoader> combinerLoader = 
      csLoadPluginCheck<WeaverCommon::iCombinerLoader> (compiler->objectreg,
        tech.combiner.classId, false);
    if (!combinerLoader.IsValid())
    {
      // Don't complain, will happen later anyway
      return;
    }
    
    const char* source = node->GetAttributeValue ("source");
    if (!source || !*source)
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	"Varyings must have a %s attribute",
	CS::Quote::Single ("source"));
      return;
    }

    csRef<iDocumentNode> snippetNode = 
      compiler->CreateAutoNode (CS_NODE_ELEMENT);
    snippetNode->SetValue ("snippet");
    snippetNode->SetAttribute ("id", id);
    csRef<iDocumentNode> techNode = 
      snippetNode->CreateNodeBefore (CS_NODE_ELEMENT);
    techNode->SetValue ("technique");
    {
      csRef<iDocumentNode> combinerNode = 
        techNode->CreateNodeBefore (CS_NODE_ELEMENT);
      combinerNode->SetValue ("combiner");
      combinerNode->SetAttribute ("name", "c");
      combinerNode->SetAttribute ("plugin", tech.combiner.classId);
    }
    
    csString weaverType;
    weaverType = node->GetAttributeValue ("weavertype");
    if (weaverType.IsEmpty())
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	"Need a %s attribute for varyings",
	CS::Quote::Single ("weavertype"));
      return;
    }
    combinerLoader->GenerateBufferInputBlocks (techNode, "c", 
      source, weaverType, "output", id);

    {
      csRef<iDocumentNode> outputNode = 
        techNode->CreateNodeBefore (CS_NODE_ELEMENT);
      outputNode->SetValue ("output");
      outputNode->SetAttribute ("type", weaverType);
      outputNode->SetAttribute ("name", "output");
    }
    
    HandleSnippetNode (tech, snippetNode, aliases);
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
    snippetsOrdered.Push (snippet);
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
  
  BasicIterator<Snippet*>* Snippet::CompoundTechnique::GetSnippets()
  {
    return new BasicIteratorImplHashValues<IdSnippetHash> (snippets);
  }
  
  //-------------------------------------------------------------------
  
  size_t SnippetNumbers::GetSnippetNumber (const Snippet* snip)
  {
    if (snippetNums.Contains (snip->GetName()))
      return snippetNums.Get (snip->GetName(), (size_t)~0);
    size_t newNum = currentNum++;
    snippetNums.Put (snip->GetName(), newNum);
    return newNum;
  }
  
  //-------------------------------------------------------------------
  
  void SnippetTechPriorities::Merge (const SnippetTechPriorities& other)
  {
    if (prios.GetSize() < other.prios.GetSize())
      prios.SetSize (other.prios.GetSize(), INT_MIN);
    for (size_t s = 0; s < other.prios.GetSize(); s++)
    {
      int otherPrio = other.prios[s];
      if (otherPrio == INT_MIN) continue;
      if (prios[s] != INT_MIN)
      {
        CS_ASSERT(prios[s] == otherPrio);
      }
      prios[s] = otherPrio;
    }
  }
  
  //-------------------------------------------------------------------
  
  void TechniqueGraph::AddTechnique (const Snippet::Technique* tech)
  {
    techniques.Push (tech);
    inTechniques.Push (tech);
    outTechniques.Push (tech);
  }

  void TechniqueGraph::RemoveTechnique (const Snippet::Technique* tech)
  {
    techniques.Delete (tech);
    inTechniques.Delete (tech);
    outTechniques.Delete (tech);

    size_t c = 0;
    while (c < connections.GetSize())
    {
      Connection& conn = connections[c];
      if ((conn.to == tech) || (conn.from == tech))
        connections.DeleteIndex (c);
      else
        c++;
    }
  }
  
  void TechniqueGraph::AddConnection (const Connection& conn)
  {
    connections.PushSmart (conn);
    inTechniques.Delete (conn.to);
    outTechniques.Delete (conn.from);
    CS_ASSERT(conn.to != conn.from);
  }

  void TechniqueGraph::RemoveConnection (const Connection& conn)
  {
    connections.Delete (conn);
    // @@@ FIXME: is re-adding to inTechniques/outTechniques needed?
  }

  void TechniqueGraph::Merge (const TechniqueGraph& other)
  {
    for (size_t t = 0; t < other.techniques.GetSize(); t++)
      AddTechnique (other.techniques[t]);
    for (size_t c = 0; c < other.connections.GetSize(); c++)
      AddConnection (other.connections[c]);
    ExplicitConnectionsHashHash::ConstGlobalIterator otherExplicitIt (
      other.explicitConnections.GetIterator());
    while (otherExplicitIt.HasNext())
    {
      csConstPtrKey<Snippet::Technique> key;
      const ExplicitConnectionsHash& val = otherExplicitIt.Next (key);
      explicitConnections.Put (key, val);
    }
    
    snipPrios.Merge (other.snipPrios);
  }
  
  void TechniqueGraph::GetDependencies (const Snippet::Technique* tech, 
    csArray<const Snippet::Technique*>& deps, bool strongOnly) const
  {
    csSet<csConstPtrKey<Snippet::Technique> > addedDeps;
    for (size_t c = 0; c < connections.GetSize(); c++)
    {
      const Connection& conn = connections[c];
      if ((conn.to == tech) && (!addedDeps.Contains (conn.from))
        && (!strongOnly || !conn.inputConnection))
      {
        deps.Push (conn.from);
        addedDeps.AddNoTest (conn.from);
      }
    }
  }

  void TechniqueGraph::GetDependants (const Snippet::Technique* tech, 
    csArray<const Snippet::Technique*>& deps, bool strongOnly) const
  {
    csSet<csConstPtrKey<Snippet::Technique> > addedDeps;
    for (size_t c = 0; c < connections.GetSize(); c++)
    {
      const Connection& conn = connections[c];
      if ((conn.from == tech) && (!addedDeps.Contains (conn.to))
        && (!strongOnly || !conn.inputConnection))
      {
        deps.Push (conn.to);
        addedDeps.AddNoTest (conn.to);
      }
    }
  }
    
  bool TechniqueGraph::IsDependencyOf (const Snippet::Technique* tech,
    const Snippet::Technique* dependentOf) const
  {
    csArray<const Snippet::Technique*> deps;
    GetDependencies (dependentOf, deps);
    for (size_t i = 0; i < deps.GetSize(); i++)
    {
      if (deps[i] == tech) return true;
      if (IsDependencyOf (tech, deps[i])) return true;
    }
    return false;
  }
    
  void TechniqueGraph::SwitchTechs (const Snippet::Technique* oldTech,
                                    const Snippet::Technique* newTech,
                                    bool inputsOnly)
  {
    for (size_t c = 0; c < connections.GetSize(); c++)
    {
      Connection& conn = connections[c];
      if (inputsOnly && !conn.inputConnection) continue;
      if (conn.from == oldTech) conn.from = newTech;
      if (conn.to == oldTech) conn.to = newTech;
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
    CS::Utility::ScopedDelete<BasicIterator<const Snippet::Technique*> > techIter (
      snip->GetTechniques());
    size_t snipNum = (size_t)~0;
    if (techIter->GetTotal() > 1)
    {
      snipNum = snipNums.GetSnippetNumber (snip);
    }
    while (techIter->HasNext())
    {
      const Snippet::Technique* tech = techIter->Next();
      if (tech->IsCompound ())
      {
        csArray<GraphInfo> techGraphs;
        const Snippet::CompoundTechnique* compTech =
          static_cast<const Snippet::CompoundTechnique*> (tech);
	// Each sub-snippet ...
	for (size_t s = compTech->snippetsOrdered.GetSize(); s-- > 0; )
	{
	  Snippet* snippet = //snippetIter.Next ();
	    compTech->snippetsOrdered[s];
	  if (techGraphs.GetSize() == 0)
	  {
	    BuildSubGraphs (snippet, techGraphs);
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
	      SnippetTechPriorities& newGraphPrios =
	        newGraphs[g].graph.GetSnippetPrios();
	      for (size_t g2 = 0; g2 < techGraphs.GetSize(); g2++)
	      {
	        /* Conserve variations: don't merge a graph if one of the
	           contained snippets has a higher priority than the graph
	           to merge into */
	        bool skipTech = false;
	        const SnippetTechPriorities& graphPrios =
	          techGraphs[g2].graph.GetSnippetPrios();
		for (size_t s = 0; s < snipNums.GetAllSnippetsCount(); s++)
		{
		  if (newGraphPrios.IsSnippetPrioritySet (s)
		    && graphPrios.IsSnippetPrioritySet (s)
		    && (newGraphPrios.GetSnippetPriority (s) <
		      graphPrios.GetSnippetPriority(s)))
		  {
		    skipTech = true;
		    break;
		  }
		}
		if (skipTech) continue;
		for (size_t s = 0; s < snipNums.GetAllSnippetsCount(); s++)
		{
		  if (graphPrios.IsSnippetPrioritySet (s))
		  {
		    newGraphPrios.SetSnippetPriority (s, 
		      graphPrios.GetSnippetPriority (s));
		  }
		}
		
		GraphInfo graphMerged (techGraphs[g2]);
		graphMerged.Merge (newGraphs[g]);
		graphs2.Push (graphMerged);
	      }
	    }
	    techGraphs = graphs2;
	  }
	}
	/* Apply connections.
	 * Connect "out" techs to "in" techs.
	 */
	for (size_t g = 0; g < techGraphs.GetSize(); g++)
	{
	  GraphInfo& graphInfo = techGraphs[g];
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
	// Set priority for this snippet
	if (snipNum != (size_t)~0)
	{
	  int prio = tech->priority;
	  for (size_t g = 0; g < techGraphs.GetSize(); g++)
	    techGraphs[g].graph.GetSnippetPrios().SetSnippetPriority (snipNum, prio);
	}
        // Add all the graphs for the technique to the complete graph list
        for (size_t g = 0; g < techGraphs.GetSize(); g++)
          graphs.Push (techGraphs[g]);
      }
      else
      {
	GraphInfo graphInfo;
	graphInfo.graph.AddTechnique (tech);
	// Set priority for this snippet
	if (snipNum != (size_t)~0)
	{
	  int prio = tech->priority;
	  graphInfo.graph.GetSnippetPrios().SetSnippetPriority (snipNum, prio);
	}
	graphs.Push (graphInfo);
      }
    }
    MapGraphInputsOutputs (graphs, snip);
  }
  
  void TechniqueGraphBuilder::FixupExplicitConnections (const Snippet* snip, 
    csArray<GraphInfo>& graphs)
  {
    CS::Utility::ScopedDelete<BasicIterator<const Snippet::Technique*> > techIter (
      snip->GetTechniques());
    size_t snipNum = (size_t)~0;
    if (techIter->GetTotal() > 1)
    {
      snipNum = snipNums.GetSnippetNumber (snip);
    }
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
	  FixupExplicitConnections (snippet, graphs);
	}
	// Transfer explicit connections
	for (size_t g = 0; g < graphs.GetSize(); g++)
	{
	  GraphInfo& graphInfo = graphs[g];
	  snippetIter.Reset();
	  while (snippetIter.HasNext())
	  {
	    Snippet* toSnippet = snippetIter.Next ();
	    const Snippet::ExplicitConnectionsHash* explicitConns =
	      compTech->GetExplicitConnections (toSnippet);
	    if (!explicitConns) continue;
	    
	    TechniqueGraph::ExplicitConnectionsHash newExplicitConns;
	    SnippetToTechMap::Iterator toIt = 
	      graphInfo.snippetToTechIn.GetIterator (toSnippet);
	    while (toIt.HasNext())
	    {
	      const Snippet::Technique* toTech = toIt.Next();
	      Snippet::ExplicitConnectionsHash::ConstGlobalIterator
	        explicitConnIt = explicitConns->GetIterator ();
	      while (explicitConnIt.HasNext())
	      {
		csString dest;
		const Snippet::ExplicitConnectionSource& source = 
		  explicitConnIt.Next (dest);
		TechniqueGraph::ExplicitConnectionSource newSource;
		SnippetToTechMap::Iterator fromIt = 
		  graphInfo.snippetToTechOut.GetIterator (source.from);
		while (fromIt.HasNext())
		{
		  newSource.from = fromIt.Next();
		  newSource.outputName = source.outputName;
		  newExplicitConns.Put (dest, newSource);
		}
		graphInfo.graph.GetExplicitConnections (toTech) =
		  newExplicitConns;
	      }
	    }
	  }
	}
      }
    }
  }

  void TechniqueGraphBuilder::MapGraphInputsOutputs (GraphInfo& graphInfo, 
                                                     const Snippet* snip)
  {
    {
      csArray<const Snippet::Technique*> inTechs;
      graphInfo.graph.GetInputTechniques (inTechs);
      for (size_t t = 0; t < inTechs.GetSize(); t++)
      {
        graphInfo.snippetToTechIn.Put (snip, inTechs[t]);
      }
    }
    {
      csArray<const Snippet::Technique*> outTechs;
      graphInfo.graph.GetOutputTechniques (outTechs);
      for (size_t t = 0; t < outTechs.GetSize(); t++)
      {
        graphInfo.snippetToTechOut.Put (snip, outTechs[t]);
      }
    }
  }
      
  void TechniqueGraphBuilder::MapGraphInputsOutputs (
    csArray<GraphInfo>& graphs, const Snippet* snip)
  {
    for (size_t g = 0; g < graphs.GetSize(); g++)
    {
      MapGraphInputsOutputs (graphs[g], snip);
    }
  }

  void TechniqueGraphBuilder::BuildGraphs (const Snippet* snip, 
                                           csArray<TechniqueGraph>& graphs)
  {
    csArray<GraphInfo> graphInfos;
    BuildSubGraphs (snip, graphInfos);
    
    FixupExplicitConnections (snip, graphInfos);
    
    for (size_t g = 0; g < graphInfos.GetSize(); g++)
      graphs.Push (graphInfos[g].graph);
  }
}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)
