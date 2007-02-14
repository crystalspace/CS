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
#include "iutil/vfs.h"
#include "ivaria/reporter.h"

#include "csutil/csstring.h"
#include "csutil/documenthelper.h"
#include "csutil/fifo.h"
#include "csutil/scopeddelete.h"
#include "csutil/xmltiny.h"

#include "combiner_default.h"
#include "weaver.h"
#include "snippet.h"
#include "synth.h"

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
    
    printf ("%zu\n", graphs.GetSize());
    if (graphs.GetSize() > 0)
    {
      size_t techNum = graphs[0].GetSize();
      for (size_t g = 1; g < graphs.GetSize(); g++)
      {
        techNum *= graphs[g].GetSize();
      }
      printf ("%zu\n", techNum);
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
    defaultCombiner.AttachNew (new CombinerDefault);

    TechniqueGraph graph (techGraph);
    /*
      Here comes some of the magic. What we have now is an array of 
      "snippets"; what we want is a graph structure a la Abstract Shade Trees
      (http://graphics.cs.brown.edu/games/AbstractShadeTrees/).
     
      So first we need to make a graph of our flat array.
      * Each element will become a node.
      * If an explicit connection is specified, use that. (@@@ TODO)
      * Otherwise, try to implicitly connect. For each element,  search (from 
        the element backwards) if some element provides an output that 
        corresponds to an input to the current element. If so, connect.
        (Correspondence is checked by name.)
     
      Once that graph is built, start off by the node that has the output 
      "outputColor" (if none has, pick an arbitrary color output; if none
      exists, pick an arbitrary output.)
      
      All dependent nodes will then be sent off to the combiner(s).
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
      - Starting from our output node, collect all nodes that go into it.
        These will be emitted.
      - For each node perform input/output renaming.
     */
    SynthesizeNodeTree synthTree;
    synthTree.AddAllInputNodes (graph, generatedOutput, combiner);
    synthTree.AddAllInputNodes (graph, outTechniquePos, combiner);
    
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
      CS::ScopedDelete<BasicIterator<const SynthesizeNodeTree::Node> > nodeIt (
        synthTree.GetNodes());
      while (nodeIt->HasNext())
      {
        const SynthesizeNodeTree::Node& node = nodeIt->Next();
        
        UsedOutputsHash usedOutputs;
	CS::ScopedDelete<BasicIterator<const Snippet::Technique::Input> > 
	  inputIt (node.tech->GetInputs());
	while (inputIt->HasNext())
	{
	  const Snippet::Technique::Input& inp = inputIt->Next();
	  
	  const Snippet::Technique* sourceTech;
          const Snippet::Technique::Output* output;
          
          printf ("find input: %p, %s", node.tech, inp.name.GetData());
          if (FindInput (graph, combiner, node.tech, inp, sourceTech, output,
            usedOutputs))
          {
            printf (" ...found\n");
            if (strcmp (output->type, inp.type) == 0)
            {
	      Link link;
	      link.fromName = 
	        synthTree.GetNodeForTech (sourceTech).outputRenames.Get (
		  output->name, (const char*)0);
	      CS_ASSERT(!link.fromName.IsEmpty());
	      //link.fromType = output->type;
	    
	      link.toName = node.inputRenames.Get (inp.name, (const char*)0);
	      CS_ASSERT(!link.toName.IsEmpty());
	      //link.toType = inp.type;
	    
  	      links.Put (sourceTech, link);
	    }
	    else
	    {
	      csRef<iDocumentNodeIterator> coerceChain = 
	        combiner->QueryCoerceChain (output->type, inp.type);
	      // Ugly?
	      Snippet::Technique::CombinerPlugin tempComb (*comb);
	      tempComb.name = "combiner";
              synthTree.AugmentCoerceChain (compiler, tempComb, combiner, coerceChain, 
	        graph, node.tech, inp.name, sourceTech, links);
	    }
	  }
	  else
	  {
            printf (" ...use default\n");
	    // Obtain default
	    switch (inp.defaultType)
	    {
	      case Snippet::Technique::Input::None:
	        break;
	      case Snippet::Technique::Input::Complex:
	        {
		  defaultCombiner->BeginSnippet();
		  combiner->BeginSnippet ();
		  for (size_t b = 0; b < inp.complexBlocks.GetSize(); b++)
		  {
		    const Snippet::Technique::Block& block = 
		      inp.complexBlocks[b];
		    csRef<WeaverCommon::iCombiner> theCombiner (GetCombiner (
		      combiner, *comb, node.tech->GetCombiner(),
		      block.combinerName));
		    if (theCombiner.IsValid())
		      theCombiner->WriteBlock (block.location, block.node);
		  }
		  
		  csString inpOutputName;
		  inpOutputName.Format ("in_%s", inp.name.GetData());
		  combiner->AddGlobal (inpOutputName, inp.type);
		  
		  combiner->AddOutput (inp.name, inp.type);
		  combiner->OutputRename (inp.name, inpOutputName);
		  combiner->Link (inpOutputName,
		    node.inputRenames.Get (inp.name, (const char*)0));
		  combiner->EndSnippet ();
		  
		  defaultCombiner->EndSnippet ();
	        }
	        break;
	    }
	  }
        }
      }
    }
    //synthTree.Resort (graph);
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
	  printf ("\n%s %s -> %s %s",
	   outp.type.GetData(), outp.name.GetData(),
	   input.type.GetData(), input.name.GetData());
	  if (outp.type == input.type)
	  {
	    printf (" match");
	    sourceTech = tech;
	    output = &outp;
	    usedOutputs.AddNoTest (output);
	    return true;
	  }
	  uint cost = combiner->CoerceCost (outp.type, input.type);
	  printf(" cost %u", cost);
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
      
      if (!techToNode.Contains (tech))
      {
        Node node;
        node.tech = tech;
        ComputeRenames (node, combiner);
        techToNode.Put (tech, nodes.Push (node));
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
      
      // @@@ Leaks!
      Snippet::Technique* tech = 
        Snippet::LoadLibraryTechnique (compiler, link, combinerPlugin);
      printf ("augment: %p\n", tech);
      graph.AddTechnique (tech);
      TechniqueGraph::Connection conn;
      conn.from = lastTech;
      conn.to = tech;
      graph.AddConnection (conn);
      lastTech = tech;
      
      Node node;
      node.tech = tech;
      ComputeRenames (node, combiner);
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
      
      if (!newTechToNode.Contains (tech))
      {
        Node node = GetNodeForTech (tech);
        newTechToNode.Put (tech, newNodes.Push (node));
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
    return new BasicIteratorImpl<
      const Synthesizer::SynthesizeNodeTree::Node, NodeArray> (nodes);
  }
  
  BasicIterator<const Synthesizer::SynthesizeNodeTree::Node>* 
  Synthesizer::SynthesizeNodeTree::GetNodesReverse()
  {
    return new BasicIteratorReverseImpl<
      const Synthesizer::SynthesizeNodeTree::Node, NodeArray> (nodes);
  }
  
}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)
