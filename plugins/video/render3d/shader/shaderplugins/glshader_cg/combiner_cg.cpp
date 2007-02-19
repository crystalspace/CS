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
#include "iutil/cfgmgr.h"
#include "iutil/document.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"

#include "csplugincommon/shader/weavertypes.h"
#include "csutil/documenthelper.h"
#include "csutil/fifo.h"
#include "csutil/scfstr.h"
#include "csutil/set.h"
#include "csutil/xmltiny.h"

#include "combiner_cg.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{
  using namespace CS::PluginCommon;

  CS_LEAKGUARD_IMPLEMENT (ShaderCombinerLoaderCg);
  
  SCF_IMPLEMENT_FACTORY (ShaderCombinerLoaderCg)
  
  ShaderCombinerLoaderCg::ShaderCombinerLoaderCg (iBase* parent) : 
    scfImplementationType (this, parent)
  {
    InitTokenTable (xmltokens);
  }
  
  csPtr<ShaderWeaver::iCombiner> 
    ShaderCombinerLoaderCg::GetCombiner (iDocumentNode* params)
  {
    bool writeVP = true/*false*/, writeFP = true/*false*/;
    
#if 0
    if (params != 0)
    {
      csRef<iDocumentNode> vertexNode = params->GetNode ("vertex");
      if (vertexNode.IsValid())
      {
        const char* content = vertexNode->GetContentsValue ();
        if (content)
          writeVP = (strcasecmp (content, "yes") == 0)
            || (strcasecmp (content, "true") == 0)
            || (atoi (content) != 0);
      }
      
      csRef<iDocumentNode> fragmentNode = params->GetNode ("fragment");
      if (fragmentNode.IsValid())
      {
        const char* content = fragmentNode->GetContentsValue ();
        if (content)
          writeFP = (strcasecmp (content, "yes") == 0)
            || (strcasecmp (content, "true") == 0)
            || (atoi (content) != 0);
      }
    }
#endif
    csRef<ShaderCombinerCg> newCombiner;
    newCombiner.AttachNew (new ShaderCombinerCg (this, writeVP, writeFP));
    
    return csPtr<ShaderWeaver::iCombiner> (newCombiner);
  }

  bool ShaderCombinerLoaderCg::Initialize (iObjectRegistry* reg)
  {
    object_reg = reg;
    
    csRef<iConfigManager> config(csQueryRegistry<iConfigManager> (object_reg));
    
    const char* libraryPath =
      config->GetStr ("Video.OpenGL.Shader.Cg.Combiner.CoerceLibrary");
    if (!libraryPath || !*libraryPath)
    {
      Report (CS_REPORTER_SEVERITY_ERROR, "No coercion library set up");
      return false;
    }
    
    if (!LoadCoercionLibrary (libraryPath))
      return false;
    
    return true;
  }
  
  namespace
  {
    static const char messageID[] =
      "crystalspace.graphics3d.shader.combiner.glcg";
  }
  
  void ShaderCombinerLoaderCg::Report (int severity, const char* msg, ...)
  {
    va_list args;
    va_start (args, msg);
    csReportV (object_reg, severity, messageID, msg, args);
    va_end (args);
  }
  
  void ShaderCombinerLoaderCg::Report (int severity, iDocumentNode* node, 
                                       const char* msg, ...)
  {
    va_list args;
    va_start (args, msg);
    
    csRef<iSyntaxService> synsrv = csQueryRegistry<iSyntaxService> (
      object_reg);
    if (synsrv.IsValid())
    {
      csString msgStr;
      msgStr.FormatV (msg, args);
      synsrv->Report (messageID, severity, node, "%s", msgStr.GetData());
    }
    else
    {
      csReportV (object_reg, severity, messageID, msg, args);
    }
    va_end (args);
  }
    
  csPtr<iDocumentNodeIterator> ShaderCombinerLoaderCg::QueryCoerceChain (
    const char* fromType, const char* toType)
  {
    csArray<const CoerceItem*> chain;
    
    FindCoerceChain (fromType, toType, chain);
    
    csRef<CoerceChainIterator> iterator;
    iterator.AttachNew (new CoerceChainIterator);
    for (size_t i = 0; i < chain.GetSize(); i++)
      iterator->nodes.Push (chain[i]->node);
      
    return csPtr<iDocumentNodeIterator> (iterator);
  }
  
  uint ShaderCombinerLoaderCg::CoerceCost (const char* fromType, 
                                           const char* toType)
  {
    csArray<const CoerceItem*> chain;
    
    FindCoerceChain (fromType, toType, chain);
    if (chain.GetSize() == 0)
      return ShaderWeaver::NoCoercion;
      
    uint cost = 0;
    for (size_t i = 0; i < chain.GetSize(); i++)
      cost += chain[i]->cost;
    return cost;
  }
  
  bool ShaderCombinerLoaderCg::LoadCoercionLibrary (const char* path)
  {
    csRef<iVFS> vfs (csQueryRegistry<iVFS> (object_reg));
    if (!vfs.IsValid()) return false;
    
    csRef<iFile> libfile = vfs->Open (path, VFS_FILE_READ);
    if (!libfile.IsValid())
    {
      Report (CS_REPORTER_SEVERITY_ERROR, "Can't open %s", path);
      return false;
    }
    
    csRef<iDocumentSystem> docsys (
      csQueryRegistry<iDocumentSystem> (object_reg));
    if (!docsys.IsValid())
      docsys.AttachNew (new csTinyDocumentSystem);
    csRef<iDocument> doc = docsys->CreateDocument();
    {
      const char* err = doc->Parse (libfile);
      if (err)
      {
        Report (CS_REPORTER_SEVERITY_ERROR, "Error parsing %s: %s",
          path, err);
	return false;
      }
    }
    csRef<iDocumentNode> startNode = 
      doc->GetRoot ()->GetNode ("combinerlibrary");
    if (!startNode.IsValid())
    {
      Report (CS_REPORTER_SEVERITY_ERROR,
	"Expected 'combinerlibrary' node in file '%s'", path);
      return false;
    }
    
    csRef<iDocumentNodeIterator> nodes = startNode->GetNodes();
    while (nodes->HasNext())
    {
      csRef<iDocumentNode> child = nodes->Next();
      if (child->GetType() != CS_NODE_ELEMENT) continue;
      
      csStringID id = xmltokens.Request (child->GetValue());
      switch (id)
      {
        case XMLTOKEN_COERCION:
          if (!ParseCoercion (child))
            return false;
          break;
	default:
	  {
	    csRef<iSyntaxService> synsrv = csQueryRegistry<iSyntaxService> (
	      object_reg);
	    if (synsrv.IsValid())
	      synsrv->ReportBadToken (child);
	    return false;
	  }
      }
    }
    return true;
  }
  
  bool ShaderCombinerLoaderCg::ParseCoercion (iDocumentNode* node)
  {
    const char* from = node->GetAttributeValue ("from");
    if (!from || !*from)
    {
      Report (CS_REPORTER_SEVERITY_ERROR, node, 
        "Non-empty 'from' attribute expeected");
      return false;
    }
    const char* to = node->GetAttributeValue ("to");
    if (!to || !*to)
    {
      Report (CS_REPORTER_SEVERITY_ERROR, node, 
        "Non-empty 'to' attribute expeected");
      return false;
    }
    int cost;
    csRef<iDocumentAttribute> costAttr = node->GetAttribute ("cost");
    if (!costAttr.IsValid())
    {
      Report (CS_REPORTER_SEVERITY_WARNING, node,
        "No 'cost' attribute, assuming cost 0");
      cost = 0;
    }
    else
    {
      cost = costAttr->GetValueAsInt ();
    }
    
    CoerceItem item;
    item.toType = to;
    item.cost = cost;
    item.node = node;
    CoerceItems* items = coercions.GetElementPointer (from);
    if (items == 0)
    {
      coercions.Put (from, CoerceItems());
      items = coercions.GetElementPointer (from);
    }
    items->InsertSorted (item, &CoerceItemCompare);
    
    return true;
  }

  int ShaderCombinerLoaderCg::CoerceItemCompare (CoerceItem const& i1, 
						 CoerceItem const& i2)
  {
    if (i1.cost < i2.cost)
      return -1;
    else if (i1.cost < i2.cost)
      return 1;
      
    // If two types have same cost, put the more specialized one first.
    const ShaderWeaver::TypeInfo* t1 = ShaderWeaver::QueryTypeInfo (i1.toType);
    const ShaderWeaver::TypeInfo* t2 = ShaderWeaver::QueryTypeInfo (i2.toType);
    if ((t1 == 0) || (t2 == 0)) return 0;
    
    if ((t1->semantics != ShaderWeaver::TypeInfo::NoSemantics)
      && (t2->semantics == ShaderWeaver::TypeInfo::NoSemantics))
      return -1;
    if ((t1->semantics == ShaderWeaver::TypeInfo::NoSemantics)
      && (t2->semantics != ShaderWeaver::TypeInfo::NoSemantics))
      return 1;
    
    if ((t1->space != ShaderWeaver::TypeInfo::NoSpace)
      && (t2->space == ShaderWeaver::TypeInfo::NoSpace))
      return -1;
    if ((t1->space == ShaderWeaver::TypeInfo::NoSpace)
      && (t2->space != ShaderWeaver::TypeInfo::NoSpace))
      return 1;
    
    /*
    if (t1->unit && !t2->unit)
      return -1;
    if (!t1->unit && t2->unit)
      return 1;
    */
    
    return 0;
  }
  
  /* Bit of a kludge... helpers uses by FindCoerceChain, however, can't be made
   * local b/c used as template parameters...
   */
  namespace
  {
    template<typename T>
    struct Hierarchy
    {
      const T* item;
      size_t parent;
      
      Hierarchy (const T* item, size_t parent) : item (item),
        parent (parent) {}
    };
    template<typename T>
    struct TestSource
    {
      csString type;
      const T* item;
      size_t depth;
      size_t hierarchy;
      
      TestSource (const T& item, size_t depth, size_t hierarchy) :
        type (item.toType), item (&item), depth (depth), hierarchy (hierarchy) {}
    };
  }
  
  void ShaderCombinerLoaderCg::FindCoerceChain (const char* from, 
                                                const char* to,
                                                csArray<const CoerceItem*>& chain)
  {
    csArray<Hierarchy<CoerceItem> > hierarchy;
    csFIFO<TestSource<CoerceItem> > sourcesToTest;
    
    const CoerceItems* items = coercions.GetElementPointer (from);
    if (items != 0)
    {
      // Search for direct match
      for (size_t i = 0; i < items->GetSize(); i++)
      {
	if (items->Get (i).toType == to)
	{
	  chain.Push (&items->Get (i));
	  return;
	}
	else
	{
	  // Otherwise, search if no match is found.
          const CoerceItem& item = items->Get (i);
	  sourcesToTest.Push (TestSource<CoerceItem> (item, 
	    0, 
	    hierarchy.Push (Hierarchy<CoerceItem> (&item, 
	      csArrayItemNotFound))));
	}
      }
    }
    
    csSet<csConstPtrKey<CoerceItem> > checkedItems;
    while (sourcesToTest.GetSize() > 0)
    {
      TestSource<CoerceItem> testFrom = sourcesToTest.PopTop ();
      if (checkedItems.Contains (testFrom.item)) continue;
      const CoerceItems* items = coercions.GetElementPointer (testFrom.type);
      if (items != 0)
      {
        // Search for direct match
	for (size_t i = 0; i < items->GetSize(); i++)
	{
	  if (items->Get (i).toType == to)
	  {
	    // Generate chain
	    size_t d = testFrom.depth+1;
	    chain.SetSize (d);
            chain[d--] = testFrom.item;
	    size_t h = testFrom.hierarchy;
	    while (d-- > 0)
	    {
	      chain[d] = hierarchy[h].item;
	      h = hierarchy[h].parent;
	    }
	    return;
	  }
	  else
	  {
	    // Otherwise, search if no match is found.
	    const CoerceItem& item = items->Get (i);
	    sourcesToTest.Push (TestSource<CoerceItem> (item,
	      testFrom.depth+1, 
	      hierarchy.Push (Hierarchy<CoerceItem> (&item, 
	        testFrom.hierarchy))));
	  }
	}
      }
      checkedItems.AddNoTest (testFrom.item);
    }
  }
  
  //---------------------------------------------------------------------
  
  ShaderCombinerCg::ShaderCombinerCg (ShaderCombinerLoaderCg* loader, 
                                      bool vp, bool fp) : 
    scfImplementationType (this), loader (loader), writeVP (vp), writeFP (fp)
  {
  }
  
  void ShaderCombinerCg::BeginSnippet ()
  {
  }
  
  void ShaderCombinerCg::AddInput (const char* name, const char* type)
  {
    if (!currentSnippet.localIDs.Contains (name))
    {
      currentSnippet.localIDs.AddNoTest (name);
      currentSnippet.locals.AppendFmt ("%s %s;\n", 
	CgType (type).GetData(), name);
    }
  }
  
  void ShaderCombinerCg::AddOutput (const char* name, const char* type)
  {
    if (!currentSnippet.localIDs.Contains (name))
    {
      currentSnippet.localIDs.AddNoTest (name);
      currentSnippet.locals.AppendFmt ("%s %s;\n", 
	CgType (type).GetData(), name);
    }
  }
  
  void ShaderCombinerCg::InputRename (const char* fromName, 
                                      const char* toName)
  {
    currentSnippet.inputMaps.AppendFmt ("%s = %s;\n",
      toName, fromName);
  }
  
  void ShaderCombinerCg::OutputRename (const char* fromName, 
				      const char* toName)
  {
    currentSnippet.outputMaps.AppendFmt ("%s = %s;\n",
      toName, fromName);
  }
  
  csPtr<iDocumentNodeIterator> ShaderCombinerCg::QueryCoerceChain (
    const char* fromType, const char* toType)
  {
    return loader->QueryCoerceChain (fromType, toType);
  }
  
  void ShaderCombinerCg::Link (const char* fromName, const char* toName)
  {
    currentSnippet.links.AppendFmt ("%s = %s;\n",
      toName, fromName);
  }
      
  void ShaderCombinerCg::WriteBlock (const char* location, 
                                     iDocumentNode* blockNode)
  {
    csRefArray<iDocumentNode>* destNodes = 0;
    if (strcmp (location, "variablemap") == 0)
    {
      destNodes = &variableMaps;
    }
    else if (strcmp (location, "vertexToFragment") == 0)
    {
      destNodes = &currentSnippet.vert2frag;
    }
    else if (strcmp (location, "vertexMain") == 0)
    {
      destNodes = &currentSnippet.vertexBody;
    }
    else if (strcmp (location, "fragmentMain") == 0)
    {
      destNodes = &currentSnippet.fragmentBody;
    }
    else if (strcmp (location, "vertexIn") == 0)
    {
      destNodes = &currentSnippet.vertexIn;
    }
    else if (strcmp (location, "fragmentIn") == 0)
    {
      destNodes = &currentSnippet.fragmentIn;
    }
    else if (strcmp (location, "includes") == 0)
    {
      destNodes = &includes;
    }
    
    if (destNodes != 0)
    {
      csRef<iDocumentNodeIterator> nodes = blockNode->GetNodes();
      while (nodes->HasNext())
      {
        csRef<iDocumentNode> next = nodes->Next();
        destNodes->Push (next);
      }
    }
  }
  
  bool ShaderCombinerCg::EndSnippet ()
  {
    snippets.Push (currentSnippet);
    currentSnippet = Snippet ();
    return true;
  }
    
  void ShaderCombinerCg::AddGlobal (const char* name, const char* type)
  {
    if (!globalIDs.Contains (name))
    {
      globalIDs.AddNoTest (name);
      globals.AppendFmt ("%s %s;\n", CgType (type).GetData(), name);
    }
  }
    
  void ShaderCombinerCg::SetOutput (const char* name)
  {
    outputAssign.Format ("outputColor = %s;\n", name);
  }
  
  uint ShaderCombinerCg::CoerceCost (const char* fromType, const char* toType)
  {
    return loader->CoerceCost (fromType, toType);
  }
        
  void ShaderCombinerCg::WriteToPass (iDocumentNode* pass)
  {
    if (writeVP)
    {
      csRef<iDocumentNode> vpNode = pass->CreateNodeBefore (CS_NODE_ELEMENT);
      vpNode->SetValue ("vp");
      vpNode->SetAttribute ("plugin", "glcg");
      
      csRef<iDocumentNode> cgvpNode = vpNode->CreateNodeBefore (CS_NODE_ELEMENT);
      cgvpNode->SetValue ("cgvp");
      
      csRef<iDocumentNode> entryNode = cgvpNode->CreateNodeBefore (CS_NODE_ELEMENT);
      entryNode->SetValue ("entry");
      csRef<iDocumentNode> entryValue = entryNode->CreateNodeBefore (CS_NODE_TEXT);
      entryValue->SetValue ("vertexMain");
      
      for (size_t n = 0; n < variableMaps.GetSize(); n++)
      {
        csRef<iDocumentNode> newNode = 
          cgvpNode->CreateNodeBefore (variableMaps[n]->GetType());
        CS::DocumentHelper::CloneNode (variableMaps[n], newNode);
      }
      
      csRef<iDocumentNode> programNode = cgvpNode->CreateNodeBefore (CS_NODE_ELEMENT);
      programNode->SetValue ("program");
      
      DocNodeAppender appender (programNode);

      if (includes.GetSize() > 0)
      {
        appender.Append ("\n");
        appender.Append (includes);
        appender.Append ("\n");
      }
      
      appender.Append ("struct vertex2fragment\n");
      appender.Append ("{\n");
      appender.Append ("  void dummy() {}\n");
      for (size_t s = 0; s < snippets.GetSize(); s++)
      {
        AppendProgramInput (snippets[s].vert2frag, appender);
      }
      
      appender.Append ("};\n\n");
      
      appender.Append ("struct VertexInput\n");
      appender.Append ("{\n");
      appender.Append ("  void dummy() {}\n");
      for (size_t s = 0; s < snippets.GetSize(); s++)
      {
        AppendProgramInput (snippets[s].vertexIn, appender);
      }
      appender.Append ("};\n\n");
      
      appender.Append ("vertex2fragment vertexMain (VertexInput vertexIn)\n");
      appender.Append ("{\n");
      appender.Append ("  vertex2fragment vertexToFragment;\n");
      appender.Append (globals);
      
      for (size_t s = 0; s < snippets.GetSize(); s++)
      {
        if (!snippets[s].locals.IsEmpty()
          || !snippets[s].inputMaps.IsEmpty()
          || (snippets[s].vertexBody.GetSize() > 0)
          || !snippets[s].outputMaps.IsEmpty())
	{
	  appender.Append ("{\n");
	  appender.Append (snippets[s].locals);
	  appender.Append (snippets[s].inputMaps);
	  appender.Append (snippets[s].vertexBody);
	  appender.Append (snippets[s].outputMaps);
	  appender.Append ("}\n");
	}
	appender.Append (snippets[s].links);
      }
      
      appender.Append ("  return vertexToFragment;\n");
      appender.Append ("}\n");
    }
    
    if (writeFP)
    {
      csRef<iDocumentNode> fpNode = pass->CreateNodeBefore (CS_NODE_ELEMENT);
      fpNode->SetValue ("fp");
      fpNode->SetAttribute ("plugin", "glcg");
      
      csRef<iDocumentNode> cgfpNode = fpNode->CreateNodeBefore (CS_NODE_ELEMENT);
      cgfpNode->SetValue ("cgfp");
      
      csRef<iDocumentNode> entryNode = cgfpNode->CreateNodeBefore (CS_NODE_ELEMENT);
      entryNode->SetValue ("entry");
      csRef<iDocumentNode> entryValue = entryNode->CreateNodeBefore (CS_NODE_TEXT);
      entryValue->SetValue ("fragmentMain");
      
      for (size_t n = 0; n < variableMaps.GetSize(); n++)
      {
        csRef<iDocumentNode> newNode = 
          cgfpNode->CreateNodeBefore (variableMaps[n]->GetType());
        CS::DocumentHelper::CloneNode (variableMaps[n], newNode);
      }
      
      csRef<iDocumentNode> programNode = cgfpNode->CreateNodeBefore (CS_NODE_ELEMENT);
      programNode->SetValue ("program");
      
      DocNodeAppender appender (programNode);
      
      if (includes.GetSize() > 0)
      {
        appender.Append ("\n");
        appender.Append (includes);
        appender.Append ("\n");
      }
      
      appender.Append ("struct vertex2fragment\n");
      appender.Append ("{\n");
      appender.Append ("  void dummy() {}\n");
      for (size_t s = 0; s < snippets.GetSize(); s++)
      {
        AppendProgramInput (snippets[s].vert2frag, appender);
      }
      
      appender.Append ("};\n\n");
      
      appender.Append ("struct FragmentInput\n");
      appender.Append ("{\n");
      appender.Append ("  void dummy() {}\n");
      for (size_t s = 0; s < snippets.GetSize(); s++)
      {
        AppendProgramInput (snippets[s].fragmentIn, appender);
      }
      appender.Append ("};\n\n");
      
      appender.Append ("float4 fragmentMain (vertex2fragment vertexToFragment, FragmentInput fragmentIn) : COLOR\n");
      appender.Append ("{\n");
      appender.Append ("  float4 outputColor;\n");
      appender.Append (globals);
      
      for (size_t s = 0; s < snippets.GetSize(); s++)
      {
        if (!snippets[s].locals.IsEmpty()
          || !snippets[s].inputMaps.IsEmpty()
          || (snippets[s].fragmentBody.GetSize() > 0)
          || !snippets[s].outputMaps.IsEmpty())
	{
	  appender.Append ("{\n");
	  appender.Append (snippets[s].locals);
	  appender.Append (snippets[s].inputMaps);
	  appender.Append (snippets[s].fragmentBody);
	  appender.Append (snippets[s].outputMaps);
	  appender.Append ("}\n");
	}
	appender.Append (snippets[s].links);
      }
      
      appender.Append (outputAssign);
      appender.Append ("  return outputColor;\n");
      appender.Append ("}\n");
    }
  }
    
  bool ShaderCombinerCg::CompatibleParams (iDocumentNode* params)
  {
    return true;
  }
    
  csRef<iString> ShaderCombinerCg::QueryInputTag (const char* location, 
                                                  iDocumentNode* blockNode)
  {
    /* @@@ FIXME: Also check vertexToFragment and fragmentIn? */
    if (strcmp (location, "vertexIn") == 0)
    {
      csRef<iString> result;
      bool hasBinding = false;
      csRef<iDocumentNodeIterator> nodes = blockNode->GetNodes();
      while (nodes->HasNext())
      {
        csRef<iDocumentNode> node = nodes->Next();
        if (node->GetType() != CS_NODE_ELEMENT) continue;

        csStringID id = loader->xmltokens.Request (node->GetValue());
        if ((id == ShaderCombinerLoaderCg::XMLTOKEN_UNIFORM)
          || (id == ShaderCombinerLoaderCg::XMLTOKEN_VARYING))
        {
          const char* binding = node->GetAttributeValue ("binding");
          if (!binding || !*binding) continue;
          // For now only support 1 binding...
          if (hasBinding) return 0;
          hasBinding = true;
          result.AttachNew (new scfString (binding));
        }
      }
      return result;
    }
    return 0;
  }

  void ShaderCombinerCg::AppendProgramInput (
    const csRefArray<iDocumentNode>& nodes,
    DocNodeAppender& appender)
  {
    // FIXME: error handling here
    for (size_t n = 0; n < nodes.GetSize(); n++)
    {
      iDocumentNode* node = nodes[n];
      if (node->GetType() == CS_NODE_ELEMENT)
      {
	csStringID id = loader->xmltokens.Request (node->GetValue());
	switch (id)
	{
	  case ShaderCombinerLoaderCg::XMLTOKEN_UNIFORM:
	  case ShaderCombinerLoaderCg::XMLTOKEN_VARYING:
	    {
	      const char* name = node->GetAttributeValue ("name");
	      const char* type = node->GetAttributeValue ("type");
	      const char* binding = node->GetAttributeValue ("binding");
	      if (name && *name && type && *type)
	      {
	        csString bindingStr;
	        if (binding) bindingStr.Format (" : %s", binding);
	        csString str;
		str.Format ("%s %s %s%s;\n", 
		  (id == ShaderCombinerLoaderCg::XMLTOKEN_UNIFORM) ? "uniform" :
		    "varying",
		  CgType (type).GetData(), name, bindingStr.GetDataSafe());
		appender.Append (str);
	      }
	    }
	    break;
	}
      }
      else
      {
        appender.Append (node);
      }
    }
  }
  
  csString ShaderCombinerCg::CgType (const char* weaverType)
  {
    const ShaderWeaver::TypeInfo* typeInfo = 
      ShaderWeaver::QueryTypeInfo (weaverType);
    
    if (typeInfo)
    {
      switch (typeInfo->baseType)
      {
	case ShaderWeaver::TypeInfo::Vector:
	  if (typeInfo->dimensions == 1)
	    return "float";
	  else
	    return csString().Format ("float%d", typeInfo->dimensions);
	case ShaderWeaver::TypeInfo::Sampler:
	  if (typeInfo->samplerIsCube)
	    return "samplerCUBE";
	  else
	    return csString().Format ("sampler%dD", typeInfo->dimensions);
      }
    }
    return weaverType; // @@@ Hmmm... what fallback, if any?
  }
  
  //-------------------------------------------------------------------------
  
  void ShaderCombinerCg::DocNodeAppender::FlushAppendString ()
  {
    if (!stringAppend.IsEmpty ())
    {
      csRef<iDocumentNode> newNode = node->CreateNodeBefore (CS_NODE_TEXT);
      newNode->SetValue (stringAppend);
      stringAppend.Empty();
    }
  }
  
  ShaderCombinerCg::DocNodeAppender::DocNodeAppender (iDocumentNode* node) :
    node (node)
  {
  }
  
  ShaderCombinerCg::DocNodeAppender::~DocNodeAppender ()
  {
    FlushAppendString();
  }

  void ShaderCombinerCg::DocNodeAppender::Append (const char* str)
  {
    stringAppend += str;
  }
  
  void ShaderCombinerCg::DocNodeAppender::Append (iDocumentNode* appendNode)
  {
    csDocumentNodeType nodeType = appendNode->GetType();
    if (nodeType == CS_NODE_TEXT)
    {
      Append (appendNode->GetValue());
    }
    else if (nodeType == CS_NODE_COMMENT)
    {
      // Skip
    }
    else
    {
      FlushAppendString();
      csRef<iDocumentNode> newNode = 
        node->CreateNodeBefore (nodeType);
      CS::DocumentHelper::CloneNode (appendNode, newNode);
    }
  }

  void ShaderCombinerCg::DocNodeAppender::Append (
    const csRefArray<iDocumentNode>& nodes)
  {
    for (size_t n = 0; n < nodes.GetSize(); n++)
    {
      Append (nodes[n]);
    }
  }
}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)
