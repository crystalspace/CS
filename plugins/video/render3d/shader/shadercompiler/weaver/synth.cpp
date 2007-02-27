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
#include "iutil/plugin.h"
#include "iutil/string.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"

#include "csplugincommon/shader/weavertypes.h"
#include "csutil/csstring.h"
#include "csutil/documenthelper.h"
#include "csutil/fifo.h"
#include "csutil/scopeddelete.h"
#include "csutil/xmltiny.h"

#include "combiner_default.h"
#include "docnodes.h"
#include "weaver.h"
#include "snippet.h"
#include "synth.h"

// Debugging
#define WEAVER_PRINTF   if (false) csPrintf

CS_PLUGIN_NAMESPACE_BEGIN(ShaderWeaver)
{
  namespace WeaverCommon = CS::PluginCommon::ShaderWeaver;

  Synthesizer::Synthesizer (WeaverCompiler* compiler, 
                            const csPDelArray<Snippet>& outerSnippets) : 
    compiler (compiler), xmltokens (compiler->xmltokens)
  {
    graphs.SetSize (outerSnippets.GetSize());
    for (size_t s = 0; s < outerSnippets.GetSize(); s++)
    {
      TechniqueGraphBuilder builder;
      this->outerSnippets.Push (outerSnippets[s]);
      builder.BuildGraphs (outerSnippets[s], graphs[s]);
    }
  }

  csPtr<iDocument> Synthesizer::Synthesize (iDocumentNode* sourceNode)
  {
    csRef<iDocumentSystem> docsys;
    docsys.AttachNew (new csTinyDocumentSystem);
    csRef<iDocument> synthesizedDoc = docsys->CreateDocument ();
    
    csRef<iDocumentNode> rootNode = synthesizedDoc->CreateRoot();
    csRef<iDocumentNode> shaderNode = rootNode->CreateNodeBefore (CS_NODE_ELEMENT);
    shaderNode->SetValue ("shader");
    CS::DocumentHelper::CloneAttributes (sourceNode, shaderNode);
    shaderNode->SetAttribute ("compiler", "xmlshader");
    
    WEAVER_PRINTF ("%zu graphs\n", graphs.GetSize());
    if (graphs.GetSize() > 0)
    {
      size_t techNum = graphs[0].GetSize();
      for (size_t g = 1; g < graphs.GetSize(); g++)
      {
        techNum *= graphs[g].GetSize();
      }
      WEAVER_PRINTF ("%zu techniques\n", techNum);
      size_t currentTech = 0;
      while (currentTech < techNum)
      {
        csRef<iDocumentNode> techniqueNode =
          shaderNode->CreateNodeBefore (CS_NODE_ELEMENT);
        techniqueNode->SetValue ("technique");
        techniqueNode->SetAttributeAsInt ("priority", int (techNum - currentTech));
        
        size_t current = currentTech;
        bool aPassSucceeded = false;
	for (size_t g = 0; g < graphs.GetSize(); g++)
	{
          const TechniqueGraph& graph = graphs.Get (g)[(current % graphs[g].GetSize())];
          current = current / graphs[g].GetSize();
	  csRef<iDocumentNode> passNode =
	    techniqueNode->CreateNodeBefore (CS_NODE_ELEMENT);
	  passNode->SetValue ("pass");
	  
	  if (!SynthesizeTechnique (passNode, outerSnippets[g], graph))
            techniqueNode->RemoveNode (passNode);
          else
            aPassSucceeded = true;
	}
        if (!aPassSucceeded)
          shaderNode->RemoveNode (techniqueNode);
	
	currentTech++;
      }
    }
    
    {
      csRef<iDocumentNode> fallback = sourceNode->GetNode ("fallbackshader");
      if (fallback.IsValid())
      {
        csRef<iDocumentNode> newFallback = shaderNode->CreateNodeBefore (CS_NODE_ELEMENT);
        CS::DocumentHelper::CloneNode (fallback, newFallback);
      }
    }
    
    return csPtr<iDocument> (synthesizedDoc);
  }

  bool Synthesizer::SynthesizeTechnique (iDocumentNode* passNode,
                                         const Snippet* snippet, 
                                         const TechniqueGraph& techGraph)
  {
    defaultCombiner.AttachNew (new CombinerDefault (compiler));

    TechniqueGraph graph (techGraph);
    /*
      Here comes some of the magic. What we have is a graph structure a la 
      Abstract Shade Trees
      (http://graphics.cs.brown.edu/games/AbstractShadeTrees/).

      Given that graph, actual links between input and output parameters have
      to be determined from the abstract connections.
     
      Searched are all dependencies, the dependencies of dependencies and so on
      for the best match. The best match is either perfect or a coercion
      with the lowest cost. 
      
      If no match is found, the "default" block are emitted.

      Since a coercion may require input itself the coercions cause the graph
      to be augmented, introducing new dependencies and connections.
     */
     
    CS_ASSERT(snippet->IsCompound());
    
    csRef<WeaverCommon::iCombiner> combiner;
    const Snippet::Technique::CombinerPlugin* comb;
    {
      // Here, snippet should contain 1 tech.
      CS::ScopedDelete<BasicIterator<const Snippet::Technique*> > 
        snippetTechIter (snippet->GetTechniques());
      CS_ASSERT(snippetTechIter->HasNext());
      const Snippet::Technique* snipTech = snippetTechIter->Next();
      CS_ASSERT(!snippetTechIter->HasNext());
      comb = &snipTech->GetCombiner();
      
      csRef<WeaverCommon::iCombinerLoader> loader = 
	csLoadPluginCheck<WeaverCommon::iCombinerLoader> (compiler->objectreg,
	  comb->classId);
      if (!loader.IsValid())
      {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING, passNode,
	  "Could not load combiner plugin '%s'", comb->classId.GetData());
	return false;
      }
      combiner = loader->GetCombiner (comb->params); 
      if (!combiner.IsValid())
      {
	compiler->Report (CS_REPORTER_SEVERITY_WARNING, passNode,
	  "Could not get combiner from '%s'", comb->classId.GetData());
	return false;
      }
    }
    
    // Two outputs are needed: color (usually from the fragment part)...
    const Snippet::Technique* outTechnique;
    Snippet::Technique::Output theOutput;
    if (!FindOutput (graph, "rgba", combiner, outTechnique, theOutput))
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, passNode,
	"No suitable color output found");
      return false;
    }
    
    // ...and position (usually from the vertex part).
    const Snippet::Technique* outTechniquePos;
    Snippet::Technique::Output thePosOutput;
    if (!FindOutput (graph, "position4_screen", combiner, outTechniquePos, thePosOutput))
    {
      compiler->Report (CS_REPORTER_SEVERITY_WARNING, passNode,
	"No suitable position output found");
      return false;
    }
    
    
    // Generate special technique for output
    CS::ScopedDelete<Snippet::Technique> generatedOutput (
      Snippet::CreatePassthrough ("output", "rgba"));
    graph.AddTechnique (generatedOutput);
    {
      TechniqueGraph::Connection conn;
      conn.from = outTechnique;
      conn.to = generatedOutput;
      graph.AddConnection (conn);
    }
    /*
      - Starting from our output nodes, collect all nodes that go into them.
        These will be emitted.
      - For each node perform input/output renaming.
     */
    SynthesizeNodeTree synthTree;
    synthTree.AddAllInputNodes (graph, generatedOutput, combiner);
    synthTree.AddAllInputNodes (graph, outTechniquePos, combiner);
    // The input linking works better if "top" nodes come first
    synthTree.ReverseNodeArray();
    
    LinkHash links;
    /* Linking.
     * Given are "A depends on B" style connections between nodes. Now actual
     * parameter links have to be established.
     * For each node, for each input parameter the output parameters of the
     * dependencies are searched for a suitable(1) match. When linking the
     * parameters, a coercion may be required, and the tree is augmented with
     * "coercion nodes".
     *
     * (1) "Suitable" means a coercion exists.
     */
    {
      typedef csHash<size_t, csString> TaggedInputHash;
      csArray<EmittedInput> emitInputs;
      TaggedInputHash taggedInputs;

      CS::ScopedDelete<BasicIterator<const SynthesizeNodeTree::Node> > nodeIt (
        synthTree.GetNodes());
      while (nodeIt->HasNext())
      {
        const SynthesizeNodeTree::Node& node = nodeIt->Next();
        /* For new connections introduced by input merging.
         * Connections are added delayed to avoid interference with
         * input resolution. */
        csArray<TechniqueGraph::Connection> newConnections;
        
        UsedOutputsHash usedOutputs;
	CS::ScopedDelete<BasicIterator<const Snippet::Technique::Input> > 
	  inputIt (node.tech->GetInputs());
	while (inputIt->HasNext())
	{
	  const Snippet::Technique::Input& inp = inputIt->Next();
	  
	  const Snippet::Technique* sourceTech;
          const Snippet::Technique::Output* output;

          // Look if there's already an input that has the same tag as this.
          EmittedInput* prevInput = 0;
          csString tag;
          if (!(inp.flags & Snippet::Technique::Input::flagNoMerge))
          {
            tag = GetInputTag (combiner, *comb, node.tech->GetCombiner(), 
              inp);
            if (!tag.IsEmpty())
            {
              TaggedInputHash::Iterator iter (
                taggedInputs.GetIterator (tag));
              while (iter.HasNext())
              {
                size_t taggedInpIndex = iter.Next();
                if (emitInputs[taggedInpIndex].input->type == inp.type)
                {
                  prevInput = &emitInputs[taggedInpIndex];
                  break;
                }
              }
            }
          }

          WEAVER_PRINTF ("find input: %p, %s", node.tech, inp.name.GetData());
          if (!(inp.flags & Snippet::Technique::Input::flagPrivate)
            && FindInput (graph, combiner, node.tech, inp, sourceTech, output,
            usedOutputs))
          {
            WEAVER_PRINTF (" ...found\n");
            if (strcmp (output->type, inp.type) == 0)
            {
	      Link link;
	      link.fromName = 
	        synthTree.GetNodeForTech (sourceTech).outputRenames.Get (
		  output->name, (const char*)0);
	      CS_ASSERT(!link.fromName.IsEmpty());
	    
	      link.toName = node.inputRenames.Get (inp.name, (const char*)0);
	      CS_ASSERT(!link.toName.IsEmpty());
	    
  	      links.Put (sourceTech, link);
	    }
	    else
	    {
	      csRef<iDocumentNodeIterator> coerceChain = 
	        combiner->QueryCoerceChain (output->type, inp.type);
	      // Ugly?
	      Snippet::Technique::CombinerPlugin tempComb (*comb);
	      tempComb.name = "combiner";
              // Subtle: may add nodes to the node tree
              synthTree.AugmentCoerceChain (compiler, tempComb, combiner, coerceChain, 
	        graph, node.tech, inp.name, sourceTech, links);
	    }
	  }
	  else
	  {
            WEAVER_PRINTF (" ...use default\n");
	    // Obtain default
	    switch (inp.defaultType)
	    {
	      case Snippet::Technique::Input::None:
	        break;
	      case Snippet::Technique::Input::Complex:
	        {
                  if (prevInput != 0)
                  {
	            Link link;
	            link.fromName = 
	              synthTree.GetNodeForTech (prevInput->node->tech).inputRenames.Get (
		        prevInput->input->name, (const char*)0);
	            CS_ASSERT(!link.fromName.IsEmpty());
      	    
	            link.toName = node.inputRenames.Get (inp.name, (const char*)0);
	            CS_ASSERT(!link.toName.IsEmpty());
      	    
  	            links.Put (prevInput->node->tech, link);

                    // There's now a dependency on the node providing prevInput.
                    TechniqueGraph::Connection conn;
                    conn.from = prevInput->node->tech;
                    conn.to = node.tech;
                    newConnections.Push (conn);
                  }
                  else
                  {
                    EmittedInput emit;
                    emit.node = &node;
                    emit.input = &inp;
                    if (!inp.condition.IsEmpty())
                      emit.conditions.Push (inp.condition);
                    size_t index = emitInputs.Push (emit);
                    if (!tag.IsEmpty()
                      && !(inp.flags & Snippet::Technique::Input::flagNoMerge))
                      taggedInputs.Put (tag, index);
                  }
                }
	        break;
	    }
	  }
          if (prevInput != 0)
          {
            if (inp.condition.IsEmpty())
              /* No condition. Make sure input is always pulled. */
              prevInput->conditions.DeleteAll();
            else
            {
              /* This input has a condition. If all other previous inputs with
                 the same tag have a condition add ours as well, so any of the
                 conditions being true pulls in the input. */
              if (prevInput->conditions.GetSize() != 0)
                prevInput->conditions.Push (inp.condition);
            }
          }
        }

        for (size_t c = 0; c < newConnections.GetSize(); c++)
          graph.AddConnection (newConnections[c]);
      }

      for (size_t i = 0; i < emitInputs.GetSize(); i++)
      {
        const EmittedInput& emitted = emitInputs[i];
        const Snippet::Technique::Input& inp = *emitted.input;

	defaultCombiner->BeginSnippet();
	combiner->BeginSnippet ();
	for (size_t b = 0; b < inp.complexBlocks.GetSize(); b++)
	{
	  const Snippet::Technique::Block& block = 
	    inp.complexBlocks[b];
	  csRef<WeaverCommon::iCombiner> theCombiner (GetCombiner (
	    combiner, *comb, emitted.node->tech->GetCombiner(),
	    block.combinerName));
          if (theCombiner.IsValid())
          {
            csRef<iDocumentNode> node;
            if (emitted.conditions.GetSize() > 0)
            {
              /* Synthesize <?if?>/<?endif?> nodes around block 
                 contents */
              DocumentNodeContainer* container = 
                new DocumentNodeContainer (block.node->GetParent ());
              container->AddAttributesOf (block.node);
              container->SetValue (block.node->GetValue ());
              csRef<iDocumentNode> piNode;
              csString conditionStr;
              conditionStr.AppendFmt ("(%s)", emitted.conditions[0].GetData());
              for (size_t c = 1; c < emitted.conditions.GetSize(); c++)
                conditionStr.AppendFmt (" || (%s)", 
                  emitted.conditions[c].GetData());
              csString conditionPI;
              conditionPI.Format ("if %s", conditionStr.GetData());
              piNode.AttachNew (new DocumentNodePI (container, 
                conditionPI));
              container->AddChild (piNode);
              container->AddChildrenOf (block.node);
              piNode.AttachNew (new DocumentNodePI (container, 
                "endif"));
              container->AddChild (piNode);
              node.AttachNew (container);
            }
            else
              node = block.node;
            theCombiner->WriteBlock (block.location, node);
          }
        }
		  
        csString inpOutputName;
	inpOutputName.Format ("in_%s", inp.name.GetData());
	combiner->AddGlobal (inpOutputName, inp.type);
		  
	combiner->AddOutput (inp.name, inp.type);
	combiner->OutputRename (inp.name, inpOutputName);
	combiner->Link (inpOutputName,
	  emitted.node->inputRenames.Get (inp.name, (const char*)0));

	combiner->EndSnippet ();
	defaultCombiner->EndSnippet ();
      }
    }
    {
      csArray<const Snippet::Technique*> outputs;
      outputs.Push (generatedOutput);
      outputs.Push (outTechniquePos);
      synthTree.Rebuild (graph, outputs);
    }
    
    // Writing
    {
      CS::ScopedDelete<BasicIterator<const SynthesizeNodeTree::Node> > nodeIt (
        synthTree.GetNodesReverse());
      while (nodeIt->HasNext())
      {
        const SynthesizeNodeTree::Node& node = nodeIt->Next();
        
        defaultCombiner->BeginSnippet();
        combiner->BeginSnippet ();
        {
          LinkHash::Iterator linkIt (links.GetIterator (node.tech));
          while (linkIt.HasNext())
          {
            const Link& link = linkIt.Next ();
            combiner->Link (link.fromName, link.toName);
          }
        }
        
	{
	  CS::ScopedDelete<BasicIterator<const Snippet::Technique::Input> > 
	    inputIt (node.tech->GetInputs());
	  while (inputIt->HasNext())
	  {
	    const Snippet::Technique::Input& inp = inputIt->Next();
	    combiner->AddInput (inp.name, inp.type);
	    combiner->InputRename (node.inputRenames.Get (inp.name, (const char*)0),
	      inp.name);
	  }
	}
	{
	  CS::ScopedDelete<BasicIterator<const Snippet::Technique::Output> > 
	    outputIt (node.tech->GetOutputs());
	  while (outputIt->HasNext())
	  {
	    const Snippet::Technique::Output& outp = outputIt->Next();
	    combiner->AddOutput (outp.name, outp.type);
	    combiner->OutputRename (outp.name, 
	      node.outputRenames.Get (outp.name, (const char*)0));
	  }
	}
	
	{
	  CS::ScopedDelete<BasicIterator<const Snippet::Technique::Block> >
	    blockIt (node.tech->GetBlocks());
	  while (blockIt->HasNext())
	  {
	    const Snippet::Technique::Block& block = blockIt->Next();
	    csRef<WeaverCommon::iCombiner> theCombiner (
	      GetCombiner (combiner, *comb, node.tech->GetCombiner(),
	      block.combinerName));
	    if (theCombiner.IsValid())
	      theCombiner->WriteBlock (block.location, block.node);
	  }
	}
	
        combiner->EndSnippet ();
        defaultCombiner->EndSnippet ();
      }
    }
    
    {
      csString outputRenamed = 
	synthTree.GetNodeForTech (generatedOutput).outputRenames.Get (
	  "output", (const char*)0);
      CS_ASSERT(!outputRenamed.IsEmpty());
      
      combiner->SetOutput (outputRenamed);
    }
    
    defaultCombiner->WriteToPass (passNode);
    combiner->WriteToPass (passNode);
    return true;
  }
  
  bool Synthesizer::FindOutput (const TechniqueGraph& graph,
                                const char* desiredType,
                                WeaverCommon::iCombiner* combiner,
                                const Snippet::Technique*& outTechnique,
                                Snippet::Technique::Output& theOutput)
  {
    outTechnique = 0;
    /* Search for an output of a type.
       - First search all output nodes for a suitable output.
       - Then check their dependencies if maybe one of the has a suitable
         output.
     */
  
    csArray<const Snippet::Technique*> outTechs;
    graph.GetOutputTechniques (outTechs);
    uint coerceCost = WeaverCommon::NoCoercion;
    for (size_t t = 0; t < outTechs.GetSize(); t++)
    {
      CS::ScopedDelete<BasicIterator<const Snippet::Technique::Output> > 
	outputs (outTechs[t]->GetOutputs());
      while (outputs->HasNext())
      {
	const Snippet::Technique::Output& output = outputs->Next();
	// pick output var that has lowest coercion cost to desiredType
	uint cost = (strcmp (output.type, desiredType) == 0) ? 0 :
	  combiner->CoerceCost (output.type, desiredType);
	if (cost < coerceCost)
	{
	  coerceCost = cost;
	  outTechnique = outTechs[t];
	  theOutput = output;
          if (cost == 0) return true;
	}
      }
    }

    if (outTechnique == 0)
    {
      csFIFO<const Snippet::Technique*> depTechs;
      for (size_t t = 0; t < outTechs.GetSize(); t++)
      {
        csArray<const Snippet::Technique*> deps;
        graph.GetDependencies (outTechs[t], deps);
        for (size_t d = 0; d < deps.GetSize(); d++)
          depTechs.Push (deps[d]);
      }
      while (depTechs.GetSize() > 0)
      {
        const Snippet::Technique* tech = depTechs.PopTop ();
        CS::ScopedDelete<BasicIterator<const Snippet::Technique::Output> > 
	  outputs (tech->GetOutputs());
        while (outputs->HasNext())
        {
	  const Snippet::Technique::Output& output = outputs->Next();
	  // pick output var that has lowest coercion cost to desiredType
	  uint cost = (strcmp (output.type, desiredType) == 0) ? 0 :
	    combiner->CoerceCost (output.type, desiredType);
	  if (cost < coerceCost)
	  {
	    coerceCost = cost;
	    outTechnique = tech;
	    theOutput = output;
            if (cost == 0) return true;
	  }
        }
        csArray<const Snippet::Technique*> deps;
        graph.GetDependencies (tech, deps);
        for (size_t d = 0; d < deps.GetSize(); d++)
          depTechs.Push (deps[d]);
      }
    }
    
    return outTechnique != 0;
  }
      
  bool Synthesizer::FindInput (const TechniqueGraph& graph,
    WeaverCommon::iCombiner* combiner,
    const Snippet::Technique* receivingTech, 
    const Snippet::Technique::Input& input,
    const Snippet::Technique*& sourceTech,
    const Snippet::Technique::Output*& output,
    UsedOutputsHash& usedOutputs)
  {
    const WeaverCommon::TypeInfo* inputTypeInfo = 
      WeaverCommon::QueryTypeInfo (input.type);
    csSet<csConstPtrKey<const Snippet::Technique> > checkedTechs;
    
    csFIFO<const Snippet::Technique*> techsToTry;
    {
      csArray<const Snippet::Technique*> deps;
      graph.GetDependencies (receivingTech, deps);
      for (size_t d = 0; d < deps.GetSize(); d++)
        techsToTry.Push (deps[d]);
    }
    
    uint coerceCost = WeaverCommon::NoCoercion;
    while (techsToTry.GetSize() > 0)
    {
      const Snippet::Technique* tech = techsToTry.PopTop();
      if (!checkedTechs.Contains (tech))
      {
	CS::ScopedDelete<BasicIterator<const Snippet::Technique::Output> > 
	  outputIt (tech->GetOutputs());
	while (outputIt->HasNext())
	{
	  const Snippet::Technique::Output& outp = outputIt->Next();
	  if (usedOutputs.Contains (&outp)) continue;
	  WEAVER_PRINTF ("\n%s %s -> %s %s",
	   outp.type.GetData(), outp.name.GetData(),
	   input.type.GetData(), input.name.GetData());
	  if (outp.type == input.type)
	  {
	    WEAVER_PRINTF (" match");
	    sourceTech = tech;
	    output = &outp;
	    usedOutputs.AddNoTest (output);
	    return true;
	  }
	  uint cost = combiner->CoerceCost (outp.type, input.type);
	  WEAVER_PRINTF(" cost %u", cost);
	  if (cost < coerceCost)
	  {
	    sourceTech = tech;
	    output = &outp;
	    if (cost == 0)
	    {
	      usedOutputs.AddNoTest (output);
	      return true;
	    }
	    coerceCost = cost;
	  }
          // See if maybe we can just drop a property.
          const WeaverCommon::TypeInfo* outputTypeInfo = 
            WeaverCommon::QueryTypeInfo (outp.type);
          if (inputTypeInfo && outputTypeInfo
            && (outputTypeInfo->baseType == inputTypeInfo->baseType)
            && (outputTypeInfo->samplerIsCube == inputTypeInfo->samplerIsCube)
            && (outputTypeInfo->dimensions == inputTypeInfo->dimensions)
            && ((outputTypeInfo->semantics == inputTypeInfo->semantics)
              || (inputTypeInfo->semantics == WeaverCommon::TypeInfo::NoSemantics))
            && ((outputTypeInfo->space == inputTypeInfo->space)
              || (inputTypeInfo->space == WeaverCommon::TypeInfo::NoSpace))
            && ((outputTypeInfo->unit == inputTypeInfo->unit)
              || (!inputTypeInfo->unit)))
          {
	    sourceTech = tech;
	    output = &outp;
            usedOutputs.AddNoTest (output);
            return true;
          }
	}
	
	csArray<const Snippet::Technique*> deps;
	graph.GetDependencies (tech, deps);
	for (size_t d = 0; d < deps.GetSize(); d++)
	  techsToTry.Push (deps[d]);
      
        checkedTechs.AddNoTest (tech);
      }
    }
    
    return coerceCost != WeaverCommon::NoCoercion;
  }
      
  WeaverCommon::iCombiner* Synthesizer::GetCombiner (
      WeaverCommon::iCombiner* used, 
      const Snippet::Technique::CombinerPlugin& comb,
      const Snippet::Technique::CombinerPlugin& requested,
      const char* requestedName)
  {
    if ((requestedName == 0) || (strlen (requestedName) == 0))
      return defaultCombiner;
      
    if (comb.classId != requested.classId) return 0;
    if (requested.name != requestedName) return 0;
    if (!used->CompatibleParams (requested.params)) return 0;
    return used;
  }

  csString Synthesizer::GetInputTag (WeaverCommon::iCombiner* combiner,
    const Snippet::Technique::CombinerPlugin& comb, 
    const Snippet::Technique::CombinerPlugin& combTech,
    const Snippet::Technique::Input& input)
  {
    csString tag;
    if (input.defaultType == Snippet::Technique::Input::Complex)
    {
      // First check if the default combiner can give a tag
      for (size_t b = 0; b < input.complexBlocks.GetSize(); b++)
      {
        const Snippet::Technique::Block& block = input.complexBlocks[b];
        if (!block.combinerName.IsEmpty()) continue;
        csRef<iString> newTag = defaultCombiner->QueryInputTag (block.location,
          block.node);
        if (!newTag.IsValid() || newTag->IsEmpty()) continue;
        // Ambiguity: return no tag
        if (!tag.IsEmpty() && (tag != newTag->GetData())) 
          return csString();
        tag = newTag->GetData();
      }
      // Now check if the user combiner can give a tag
      if (tag.IsEmpty())
      {
        for (size_t b = 0; b < input.complexBlocks.GetSize(); b++)
        {
          const Snippet::Technique::Block& block = input.complexBlocks[b];
          if (block.combinerName.IsEmpty()) continue;
          csRef<WeaverCommon::iCombiner> theCmbiner (GetCombiner (
            combiner, comb, combTech, block.combinerName));
          csRef<iString> newTag = combiner->QueryInputTag (block.location,
            block.node);
          if (!newTag.IsValid() || newTag->IsEmpty()) continue;
          // Ambiguity: return no tag
          if (!tag.IsEmpty() && (tag != newTag->GetData()))
            return csString();
          tag = newTag->GetData();
        }
      }
    }
    return tag;
  }

  //-------------------------------------------------------------------------
      
  void Synthesizer::SynthesizeNodeTree::ComputeRenames (Node& node,
    WeaverCommon::iCombiner* combiner)
  {
    {
      CS::ScopedDelete<BasicIterator<const Snippet::Technique::Input> > 
        inputIt (node.tech->GetInputs());
      while (inputIt->HasNext())
      {
        const Snippet::Technique::Input& inp = inputIt->Next();
        
        csString newName;
        newName.Format ("%s_%zu", inp.name.GetData(), renameNr++);
        node.inputRenames.Put (inp.name, newName);
        combiner->AddGlobal (newName, inp.type);
      }
    }
    {
      CS::ScopedDelete<BasicIterator<const Snippet::Technique::Output> > 
        outputIt (node.tech->GetOutputs());
      while (outputIt->HasNext())
      {
        const Snippet::Technique::Output& outp = outputIt->Next();
        
        csString newName;
        newName.Format ("%s_%zu", outp.name.GetData(), renameNr++);
        node.outputRenames.Put (outp.name, newName);
        combiner->AddGlobal (newName, outp.type);
      }
    }
  }

  void Synthesizer::SynthesizeNodeTree::AddAllInputNodes (
    const TechniqueGraph& graph, const Snippet::Technique* tech,
    WeaverCommon::iCombiner* combiner)
  {
    csFIFO<const Snippet::Technique*> techsToAdd;
    
    techsToAdd.Push (tech);
    
    while (techsToAdd.GetSize() > 0)
    {
      const Snippet::Technique* tech = techsToAdd.PopTop();
      
      size_t oldIndex = techToNode.Get (tech, csArrayItemNotFound);
      if (oldIndex == csArrayItemNotFound)
      {
        Node* node = nodeAlloc.Alloc();
        node->tech = tech;
        ComputeRenames (*node, combiner);
        techToNode.Put (tech, nodes.Push (node));
      }
      else
      {
        /* A dependency was added earlier. However, dependencies of a node
           must come after the depending node in the array; so push the
           node back. */
        Node* node = nodeAlloc.Alloc();
        *node = *nodes[oldIndex];
        nodes[oldIndex]->tech = 0;
        nodes[oldIndex]->inputRenames.DeleteAll();
        nodes[oldIndex]->outputRenames.DeleteAll();
        techToNode.PutUnique (tech, nodes.Push (node));
      }
      
      csArray<const Snippet::Technique*> deps;
      graph.GetDependencies (tech, deps);
      for (size_t d = 0; d < deps.GetSize(); d++)
      {
        techsToAdd.Push (deps[d]);
      }
    }
  }
  
  void Synthesizer::SynthesizeNodeTree::AugmentCoerceChain (
    WeaverCompiler* compiler, 
    const Snippet::Technique::CombinerPlugin& combinerPlugin,
    WeaverCommon::iCombiner* combiner, iDocumentNodeIterator* linkChain,  
    TechniqueGraph& graph, 
    const Snippet::Technique* inTech, const char* inName, 
    const Snippet::Technique* outTech, LinkHash& links)
  {
    {
      TechniqueGraph::Connection conn;
      conn.from = outTech;
      conn.to = inTech;
      graph.RemoveConnection (conn);
    }
  
    const Snippet::Technique* lastTech = outTech;
    while (linkChain->HasNext())
    {
      csRef<iDocumentNode> link = linkChain->Next ();
      
      Snippet::Technique* tech = 
        Snippet::LoadLibraryTechnique (compiler, link, combinerPlugin);
      augmentedTechniques.Push (tech);
      WEAVER_PRINTF ("augment: %p\n", tech);
      graph.AddTechnique (tech);
      TechniqueGraph::Connection conn;
      conn.from = lastTech;
      conn.to = tech;
      graph.AddConnection (conn);
      lastTech = tech;
      
      Node* node = nodeAlloc.Alloc();
      node->tech = tech;
      ComputeRenames (*node, combiner);
      techToNode.Put (tech, nodes.Push (node));
    }
    TechniqueGraph::Connection conn;
    conn.from = lastTech;
    conn.to = inTech;
    graph.AddConnection (conn);
    
    {
      CS::ScopedDelete<BasicIterator<const Snippet::Technique::Output> > 
        outputIt (lastTech->GetOutputs());
      if (outputIt->HasNext())
      {
        const Snippet::Technique::Output& outp = outputIt->Next();
        
        Link link;
        link.fromName = GetNodeForTech (lastTech).outputRenames.Get (outp.name,
          (const char*)0);
        link.toName = GetNodeForTech (inTech).inputRenames.Get (inName,
          (const char*)0);
        links.Put (lastTech, link);
      }
      else
      {
        /* error? */
      }
    }
  }
  
  void Synthesizer::SynthesizeNodeTree::ReverseNodeArray()
  {
    const size_t maxIndex = nodes.GetSize()-1;
    for (size_t n = 0, n2 = maxIndex; n < nodes.GetSize() / 2; n++, n2--)
    {
      Node* tmp = nodes[n];
      nodes[n] = nodes[n2];
      nodes[n2] = tmp;
    }
    csHash<size_t, csConstPtrKey<Snippet::Technique> >::GlobalIterator it (
      techToNode.GetIterator());
    while (it.HasNext())
    {
      size_t& v = it.Next();
      v = maxIndex - v;
    }
  }

  void Synthesizer::SynthesizeNodeTree::Rebuild (const TechniqueGraph& graph,
    const csArray<const Snippet::Technique*>& outputs)
  {
    NodeArray newNodes;
    csHash<size_t, csConstPtrKey<Snippet::Technique> > newTechToNode;
    
    csFIFO<const Snippet::Technique*> techsToAdd;
    for (size_t o = 0; o < outputs.GetSize(); o++)
    {
      techsToAdd.Push (outputs[o]);
    }
    
    while (techsToAdd.GetSize() > 0)
    {
      const Snippet::Technique* tech = techsToAdd.PopTop();
      
      size_t oldIndex = newTechToNode.Get (tech, csArrayItemNotFound);
      if (oldIndex == csArrayItemNotFound)
      {
        Node* node = nodeAlloc.Alloc();
        *node = GetNodeForTech (tech);
        newTechToNode.Put (tech, newNodes.Push (node));
      }
      else
      {
        /* A dependency was added earlier. However, dependencies of a node
           must come after the depending node in the array; so push the
           node back. */
        Node* node = nodeAlloc.Alloc();
        *node = *newNodes[oldIndex];
        newNodes[oldIndex]->tech = 0;
        newNodes[oldIndex]->inputRenames.DeleteAll();
        newNodes[oldIndex]->outputRenames.DeleteAll();
        newTechToNode.PutUnique (tech, newNodes.Push (node));
      }
      
      csArray<const Snippet::Technique*> deps;
      graph.GetDependencies (tech, deps);
      for (size_t d = 0; d < deps.GetSize(); d++)
      {
        techsToAdd.Push (deps[d]);
      }
    }
    
    nodes = newNodes;
    techToNode = newTechToNode;
  }
  
  BasicIterator<const Synthesizer::SynthesizeNodeTree::Node>* 
  Synthesizer::SynthesizeNodeTree::GetNodes()
  {
    return new NodesIterator<false> (nodes);
  }
  
  BasicIterator<const Synthesizer::SynthesizeNodeTree::Node>* 
  Synthesizer::SynthesizeNodeTree::GetNodesReverse()
  {
    return new NodesIterator<true> (nodes);
  }

}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)
