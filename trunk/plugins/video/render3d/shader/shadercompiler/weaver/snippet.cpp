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
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"

#include "csplugincommon/shader/shaderprogram.h"
#include "csplugincommon/shader/weavercombiner.h"
#include "cstool/identstrings.h"
#include "csutil/documenthelper.h"
#include "csutil/scopeddelete.h"

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
  };

  //-------------------------------------------------------------------

  Snippet::Snippet (WeaverCompiler* compiler, iDocumentNode* node, 
                    const char* name, bool topLevel) : compiler (compiler), 
    xmltokens (compiler->xmltokens), name (name), isCompound (false)
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
  
  Snippet::Snippet (WeaverCompiler* compiler, const char* name) : compiler (compiler), 
    xmltokens (compiler->xmltokens), name (name), isCompound (false)
  {
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
    iDocumentNode* node, const Technique::CombinerPlugin& combiner) const
  {
    Snippet::AtomTechnique* technique = 
      ParseAtomTechnique (compiler, node, true, combiner.name);
    technique->combiner = combiner;
    return technique;
  }
  
  Snippet::Technique* Snippet::CreatePassthrough (const char* varName, 
                                                  const char* type) const
  {
    csString hashStr;
    hashStr.Format ("__passthrough_%s_%s__", varName, type);
    AtomTechnique* newTech = new AtomTechnique ("(passthrough)", 
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
    AtomTechnique* newTech = ParseAtomTechnique (compiler, node, false);
    if (newTech != 0)
      techniques.InsertSorted (newTech, &CompareTechnique);
  }
  
  Snippet::AtomTechnique* Snippet::ParseAtomTechnique (
    WeaverCompiler* compiler, iDocumentNode* node, bool canOmitCombiner,
    const char* defaultCombinerName) const
  {
    AtomTechnique newTech (GetName(),
      csMD5::Encode (CS::DocSystem::FlattenNode (node)));
    
    newTech.priority = node->GetAttributeValueAsInt ("priority");
    
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
	    "Multiple 'combiner' nodes");
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
	  "Technique without 'combiner' node");
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
	
	Technique::Input newInput;
        if (!ParseInput (child, newInput, defaultCombinerName)) return 0;	
	newTech.AddInput (newInput);
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
      if (!ReadBlocks (compiler, node, newBlocks, defaultCombinerName))
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
        "'combiner' node without 'name' attribute");
      return false;
    }
    newCombiner.classId = child->GetAttributeValue ("plugin");
    if (newCombiner.classId.IsEmpty())
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
        "'combiner' node without 'plugin' attribute");
      return false;
    }
    newCombiner.params = child;

    return true;
  }

  bool Snippet::ParseInput (iDocumentNode* child, 
                            Technique::Input& newInput, 
                            const char* defaultCombinerName) const
  {
    const char* condition = child->GetAttributeValue ("condition");
    newInput.condition = condition;
    if (child->GetAttributeValueAsBool ("private"))
      newInput.isPrivate = true;
    if (child->GetAttributeValueAsBool ("forcenomerge"))
      newInput.noMerge = true;

    csRef<iDocumentNode> inputNode = GetNodeOrFromFile (child, "input",
      compiler);
    if (!inputNode.IsValid()) return false;

    newInput.node = inputNode;
    newInput.name = inputNode->GetAttributeValue ("name");
    if (newInput.name.IsEmpty())
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, inputNode,
        "'input' node without 'name' attribute");
      return false;
    }
    newInput.type = inputNode->GetAttributeValue ("type");
    if (newInput.type.IsEmpty())
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, inputNode,
        "'input' node without 'type' attribute");
      return false;
    }
    const char* def = inputNode->GetAttributeValue ("default");
    if (def != 0)
    {
      if (strcmp (def, "complex") == 0)
      {
        newInput.defaultType = Technique::Input::Complex;
        if (!ReadBlocks (compiler, inputNode, newInput.complexBlocks, 
            defaultCombinerName))
	  return 0;
      }
      else if (strcmp (def, "value") == 0)
      {
        newInput.defaultType = Technique::Input::Value;
        newInput.defaultValue = inputNode->GetAttributeValue ("defval");
        if (newInput.defaultValue.IsEmpty())
        {
          compiler->Report (CS_REPORTER_SEVERITY_WARNING, inputNode,
            "'input' node with a 'value' default but without 'defval' attribute");
          return false;
        }
      }
      else
      {
        compiler->Report (CS_REPORTER_SEVERITY_WARNING, inputNode,
          "Invalid 'default' attribute for 'input' node: %s", def);
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

    return true;
  }

  bool Snippet::ParseOutput (iDocumentNode* child, 
                             Technique::Output& newOutput) const
  {
    newOutput.name = child->GetAttributeValue ("name");
    if (newOutput.name.IsEmpty())
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
        "'output' node without 'name' attribute");
      return false;
    }
    newOutput.type = child->GetAttributeValue ("type");
    if (newOutput.type.IsEmpty())
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
        "'output' node without 'type' attribute");
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
        "'attribute' node without 'name' attribute");
      return false;
    }
    attr.type = child->GetAttributeValue ("type");
    if (attr.type.IsEmpty())
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, child,
        "'attribute' node without 'type' attribute");
      return false;
    }

    attr.defaultValue = child->GetAttributeValue ("defval");

    return true;

  }

  bool Snippet::ReadBlocks (WeaverCompiler* compiler, iDocumentNode* node, 
		            csArray<Technique::Block>& blocks,
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
      {
        newBlock.combinerName = defaultCombinerName;
        newBlock.location = location;
      }
      newBlock.node = GetNodeOrFromFile (child, "block", compiler);
      if (!newBlock.node) return false;
      
      blocks.Push (newBlock);
    }
    return true;
  }
  
  csRef<iDocumentNode> Snippet::GetNodeOrFromFile (iDocumentNode* node,
      const char* rootName, WeaverCompiler* compiler,
      csString* outFilename)
  {
    const char* filename = node->GetAttributeValue ("file");
    if (filename != 0)
    {
      csRef<iDocumentNode> rootNode = 
	  compiler->LoadDocumentFromFile (filename, node);
      if (!rootNode.IsValid()) return 0;
    
      csRef<iDocumentNode> fileNode = rootNode->GetNode (rootName);
      if (!fileNode.IsValid())
      {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING,
          "Expected '%s' node in file '%s'", rootName, filename);
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
    CompoundTechnique* newTech = new CompoundTechnique (GetName());
  
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
        case WeaverCompiler::XMLTOKEN_PARAMETER:
          {
            HandleParameterNode (*newTech, child);
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
	  "Referenced snippets must have an 'id' attribute");
	return;
    }
    if (tech.GetSnippet (id) != 0)
    {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	  "Duplicate snippet id '%s'", id);
	return;
    }
    
    csString filename;
    csRef<iDocumentNode> snippetNode = GetNodeOrFromFile (node, "snippet",
      compiler, &filename);
    if (!snippetNode.IsValid()) return;
      
    csString snippetName;
    if (!name.IsEmpty ())
    {
      snippetName.AppendFmt ("%s<%d> -> ",  name.GetData(), tech.priority);
    }
    snippetName += id ? id : filename.GetData();
    Snippet* newSnippet = new Snippet (compiler, snippetNode, 
      snippetName);
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
				     iDocumentNode* node)
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
	  "Parameters must have an 'id' attribute");
	return;
    }
    if (tech.GetSnippet (id) != 0)
    {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING, node,
	  "Duplicate snippet id '%s'", id);
	return;
    }

    csShaderProgram::ProgramParamParser paramParser (compiler->synldr,
      compiler->strings);
    csShaderProgram::ProgramParam param;
    if (!paramParser.ParseProgramParam (node, param, 
      csShaderProgram::ParamVector | csShaderProgram::ParamShaderExp)) return;
    if (!param.valid) return;

    csRef<WeaverCommon::iCombinerLoader> combinerLoader = 
      csLoadPluginCheck<WeaverCommon::iCombinerLoader> (compiler->objectreg,
        tech.combiner.classId);
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
      switch (svType)
      {
        case csShaderVariable::INT:
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
      if (numComps > 1)
        weaverType.Format ("float%d", numComps);
      else
        weaverType = "float";

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
	  "Need a 'weavertype' attribute for non-constant parameters");
        return;
      }
      combinerLoader->GenerateSVInputBlocks (techNode, "c", 
        compiler->strings->Request (param.name), weaverType, "output", id);
    }

    {
      csRef<iDocumentNode> outputNode = 
        techNode->CreateNodeBefore (CS_NODE_ELEMENT);
      outputNode->SetValue ("output");
      outputNode->SetAttribute ("type", weaverType);
      outputNode->SetAttribute ("name", "output");
    }
    
    HandleSnippetNode (tech, snippetNode);
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
    connections.Push (conn);
    inTechniques.Delete (conn.to);
    outTechniques.Delete (conn.from);
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
  }
  
  void TechniqueGraph::GetDependencies (const Snippet::Technique* tech, 
    csArray<const Snippet::Technique*>& deps, bool strongOnly) const
  {
    csSet<csConstPtrKey<Snippet::Technique> > addedDeps;
    for (size_t c = 0; c < connections.GetSize(); c++)
    {
      const Connection& conn = connections[c];
      if ((conn.to == tech) && (!addedDeps.Contains (conn.from))
        && (!strongOnly || !conn.weak))
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
        && (!strongOnly || !conn.weak))
      {
        deps.Push (conn.to);
        addedDeps.AddNoTest (conn.to);
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
    CS::Utility::ScopedDelete<BasicIterator<const Snippet::Technique*> > techIter (
      snip->GetTechniques());
    while (techIter->HasNext())
    {
      const Snippet::Technique* tech = techIter->Next();
      if (tech->IsCompound ())
      {
        csArray<GraphInfo> techGraphs;
        const Snippet::CompoundTechnique* compTech =
          static_cast<const Snippet::CompoundTechnique*> (tech);
	// Each sub-snippet ...
	Snippet::CompoundTechnique::IdSnippetHash::ConstGlobalIterator
	  snippetIter = compTech->snippets.GetIterator();
	while (snippetIter.HasNext())
	{
	  Snippet* snippet = snippetIter.Next ();
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
	      for (size_t g2 = 0; g2 < techGraphs.GetSize(); g2++)
	      {
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
        // Add all the graphs for the technique to the complete graph list
        for (size_t g = 0; g < techGraphs.GetSize(); g++)
          graphs.Push (techGraphs[g]);
      }
      else
      {
	GraphInfo graphInfo;
	graphInfo.graph.AddTechnique (tech);
	graphs.Push (graphInfo);
      }
    }
    MapGraphInputsOutputs (graphs, snip);
  }

  void TechniqueGraphBuilder::MapGraphInputsOutputs (GraphInfo& graphInfo, 
                                                     const Snippet* snip)
  {
    {
      csArray<const Snippet::Technique*> inTechs;
      graphInfo.graph.GetInputTechniques (inTechs);
      for (size_t t = 0; t < inTechs.GetSize(); t++)
      {
        graphInfo.snippetToTechIn.PutUnique (snip, inTechs[t]);
      }
    }
    {
      csArray<const Snippet::Technique*> outTechs;
      graphInfo.graph.GetOutputTechniques (outTechs);
      for (size_t t = 0; t < outTechs.GetSize(); t++)
      {
        graphInfo.snippetToTechOut.PutUnique (snip, outTechs[t]);
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
    for (size_t g = 0; g < graphInfos.GetSize(); g++)
      graphs.Push (graphInfos[g].graph);
  }
}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)
