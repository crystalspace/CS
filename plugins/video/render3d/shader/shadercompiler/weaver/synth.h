/*
  Copyright (C) 2007 by Frank Richter

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

#ifndef __CS_SYNTH_H__
#define __CS_SYNTH_H__

#include "csplugincommon/shader/weavercombiner.h"
#include "csutil/strhash.h"

#include "snippet.h"

struct iDocumentNode;

CS_PLUGIN_NAMESPACE_BEGIN(ShaderWeaver)
{
  class WeaverCompiler;

  class Synthesizer
  {
    csArray<csArray<TechniqueGraph> > graphs;
    csArray<Snippet*> outerSnippets;
  public:
    Synthesizer (WeaverCompiler* compiler, 
      const csPDelArray<Snippet>& outerSnippets);
    
    csPtr<iDocument> Synthesize (iDocumentNode* sourceNode);
  private:
    bool SynthesizeTechnique (iDocumentNode* passNode,
      const Snippet* snippet, const TechniqueGraph& graph);
    
    struct Link
    {
      csString fromName;
      csString toName;
    };
    typedef csHash<Link, csConstPtrKey<Snippet::Technique> > LinkHash;
  
    class SynthesizeNodeTree
    {
    public:
      struct Node
      {
        const Snippet::Technique* tech;
        csHash<csString, csString> inputRenames;
        csHash<csString, csString> outputRenames;
      };
      typedef csArray<Node> NodeArray;
    private:
      size_t renameNr;
      NodeArray nodes;
      csHash<size_t, csConstPtrKey<Snippet::Technique> > techToNode;
      
      void ComputeRenames (Node& node,
        CS::PluginCommon::ShaderWeaver::iCombiner* combiner);
    public:
      SynthesizeNodeTree () : renameNr (0) {}
    
      void AddAllInputNodes (const TechniqueGraph& graph,
        const Snippet::Technique* tech,
        CS::PluginCommon::ShaderWeaver::iCombiner* combiner);
      void AugmentCoerceChain (WeaverCompiler* compiler, 
        const Snippet::Technique::CombinerPlugin& combinerPlugin,
        CS::PluginCommon::ShaderWeaver::iCombiner* combiner,
        iDocumentNodeIterator* linkChain, TechniqueGraph& graph,
        const Snippet::Technique* inTech, const char* inName,
        const Snippet::Technique* outTech, LinkHash& links);
      const Node& GetNodeForTech (const Snippet::Technique* tech)
      { return nodes[techToNode.Get (tech, csArrayItemNotFound)]; }
      
      void Rebuild (const TechniqueGraph& graph,
        const csArray<const Snippet::Technique*>& outputs);
        
      BasicIterator<const Node>* GetNodes();
      BasicIterator<const Node>* GetNodesReverse();
    };
    
    bool FindOutput (const TechniqueGraph& graph,
      const char* desiredType,
      CS::PluginCommon::ShaderWeaver::iCombiner* combiner,
      const Snippet::Technique*& outTechnique,
      Snippet::Technique::Output& theOutput);
  
    typedef csSet<csConstPtrKey<Snippet::Technique::Output> > UsedOutputsHash;
    bool FindInput (const TechniqueGraph& graph,
      CS::PluginCommon::ShaderWeaver::iCombiner* combiner,
      const Snippet::Technique* receivingTech, 
      const Snippet::Technique::Input& input,
      const Snippet::Technique*& sourceTech,
      const Snippet::Technique::Output*& output,
      UsedOutputsHash& usedOutputs);
    CS::PluginCommon::ShaderWeaver::iCombiner* GetCombiner (
      CS::PluginCommon::ShaderWeaver::iCombiner* used, 
      const Snippet::Technique::CombinerPlugin& comb,
      const Snippet::Technique::CombinerPlugin& requested,
      const char* requestedName);
  public:
    WeaverCompiler* compiler;
  
    csStringHash& xmltokens;
    csRef<CS::PluginCommon::ShaderWeaver::iCombiner> defaultCombiner;
  };

}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)

#endif // __CS_SYNTH_H__
