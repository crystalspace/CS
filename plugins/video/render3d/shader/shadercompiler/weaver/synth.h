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

#include "iutil/job.h"
#include "csplugincommon/shader/weavercombiner.h"
#include "csutil/blockallocator.h"
#include "csutil/hashr.h"
#include "csutil/set.h"
#include "csutil/strhash.h"

#include "snippet.h"
#include "weaver.h"

struct iDocumentNode;
struct iProgressMeter;

CS_PLUGIN_NAMESPACE_BEGIN(ShaderWeaver)
{
  class WeaverCompiler;

  class ShaderVarNodesHelper
  {
    CS::Threading::Mutex lock;
    iDocumentNode* shaderVarsNode;
    csSet<csString> seenVars;
  public:
    ShaderVarNodesHelper (iDocumentNode* shaderVarsNode);
    
    void AddNode (iDocumentNode* node);
  };
  
  class TagNodesHelper
  {
    csSet<csString> tags;
  public:
    void AddNode (iDocumentNode* node);
    void Merge (const TagNodesHelper& other);
    void AddToNode (iDocumentNode* node, iDocumentNode* before);
  };

  class CombinerLoaderSet
  {
  public:
    typedef CS::PluginCommon::ShaderWeaver::iCombinerLoader iCombinerLoader;
    typedef csSet<csRef<iCombinerLoader> > SetType;
    typedef SetType::GlobalIterator GlobalIterator;
  private:
    CS::Threading::Mutex lock;
    typedef CS::Threading::ScopedLock<CS::Threading::Mutex> LockType;
    SetType set;
  public:
    void Add (iCombinerLoader* loader)
    {
      LockType l (lock);
      set.Add (loader);
    }

    size_t UnlockedGetSize() const { return set.GetSize(); }
    GlobalIterator UnlockedGetIterator() const { return set.GetIterator(); }
  };
  
  class Synthesizer
  {
  public:
    typedef csRefArray<iDocumentNode> DocNodeArray;
  protected:
    SnippetNumbers snipNums;
    const char* shaderName;
    csArray<csArray<TechniqueGraph> > graphs;
    csArray<DocNodeArray> prePassNodes;
    csArray<Snippet*> outerSnippets;
    DocNodeArray postPassesNodes;
  public:
    Synthesizer (WeaverCompiler* compiler, 
      const char* shaderName,
      const csArray<DocNodeArray>& prePassNodes,
      const csPDelArray<Snippet>& outerSnippets,
      const DocNodeArray& postPassesNodes);
    
    void Synthesize (iDocumentNode* shaderNode,
      ShaderVarNodesHelper& shaderVarNodesHelper,
      csRefArray<iDocumentNode>& techNodes,
      iDocumentNode* sourceTechNode, CombinerLoaderSet& combiners,
      iProgressMeter* progress);
  private:
    class SynthesizeNodeTree;
    class SynthesizeTechnique :
      public scfImplementation1<SynthesizeTechnique, iJob>
    {
      friend class SynthesizeNodeTree;
    
      bool status;
      csString techniqueConditions;
    
      const WeaverCompiler* compiler;
      const Synthesizer* synth;
      csRef<CS::PluginCommon::ShaderWeaver::iCombiner> combiner;
      csRef<CS::PluginCommon::ShaderWeaver::iCombiner> defaultCombiner;
      
      ShaderVarNodesHelper& shaderVarNodes;
      iDocumentNode* errorNode;
      const Snippet* snippet;
      TagNodesHelper tags;
      const TechniqueGraph& graph;
      CombinerLoaderSet& combiners;
    
      csString annotateString;
      const char* GetAnnotation (const char* fmt, ...) CS_GNUC_PRINTF (2, 3)
      {
	if (!compiler->annotateCombined) return 0;
  
	va_list args;
	va_start (args, fmt);
	annotateString.FormatV (fmt, args);
	va_end (args);
	return annotateString.GetData();
      }
      
      bool FindOutput (const TechniqueGraph& graph,
	const char* desiredType,
	CS::PluginCommon::ShaderWeaver::iCombiner* combiner,
	const Snippet::Technique*& outTechnique,
	Snippet::Technique::Output& theOutput);
	
      typedef csSet<csConstPtrKey<Snippet::Technique::Output> > UsedOutputsHash;
      bool FindInput (const TechniqueGraph& graph,
	CS::PluginCommon::ShaderWeaver::iCombiner* combiner,
	csString& nodeAnnotation,
	const Snippet::Technique* receivingTech, 
	const Snippet::Technique::Input& input,
	const Snippet::Technique*& sourceTech,
	const Snippet::Technique::Output*& output,
	UsedOutputsHash& usedOutputs);
      bool FindExplicitInput (const TechniqueGraph& graph,
	CS::PluginCommon::ShaderWeaver::iCombiner* combiner,
	csString& nodeAnnotation,
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
      
      csString GetInputTag (CS::PluginCommon::ShaderWeaver::iCombiner* combiner,
	const Snippet::Technique::CombinerPlugin& comb,
	const Snippet::Technique::CombinerPlugin& combTech,
	const Snippet::Technique::Input& input);
	
      csPtr<iDocumentNode> EncloseInCondition (iDocumentNode* node,
        const char* condition) const;
	
      bool operator() (ShaderVarNodesHelper& shaderVarNodes, 
        iDocumentNode* errorNode, const Snippet* snippet,
        const TechniqueGraph& graph);
    public:
      SynthesizeTechnique (WeaverCompiler* compiler,
        const Synthesizer* synth,
        ShaderVarNodesHelper& shaderVarNodes, 
        iDocumentNode* errorNode, const Snippet* snippet,
        const TechniqueGraph& graph, CombinerLoaderSet& combiners)
       : scfImplementationType (this),
         status (false), compiler (compiler), synth (synth),
         shaderVarNodes (shaderVarNodes), errorNode (errorNode),
         snippet (snippet), graph (graph), combiners (combiners)
      {}
    
      bool GetStatus() const { return status; }
      void Run()
      { 
        status =  (*this) (shaderVarNodes, errorNode, snippet, graph);
      }
      void WriteToPass (iDocumentNode* passNode)
      {
        defaultCombiner->WriteToPass (passNode);
        combiner->WriteToPass (passNode);
      }
      const char* GetTechniqueConditions() const { return techniqueConditions; }
      const TagNodesHelper& GetTags() const { return tags; }
    };
    
    typedef csHash<csString, csString> StringStringHash;
    typedef csHashReversible<csString, csString> StringStringHashRev;
    class SynthesizeNodeTree
    {
    public:
      struct Node
      {
        csString annotation;
        const Snippet::Technique* tech;
        StringStringHash inputLinks;
        StringStringHash inputDefaults;
        StringStringHashRev outputRenames;
      };
      typedef csArray<Node*> NodeArray;
    private:
      size_t renameNr;
      csBlockAllocator<Node> nodeAlloc;
      NodeArray nodes;
      csHash<size_t, csConstPtrKey<Snippet::Technique> > techToNode;
      /// Techs created in an augmentation, to be cleaned up later
      csPDelArray<Snippet> scratchSnippets;
      csPDelArray<Snippet::Technique> augmentedTechniques;
      Synthesizer::SynthesizeTechnique& synthTech;

      void ComputeRenames (Node& node,
        CS::PluginCommon::ShaderWeaver::iCombiner* combiner);
    public:
      SynthesizeNodeTree (Synthesizer::SynthesizeTechnique& synthTech)
       : renameNr (0), synthTech (synthTech) {}
    
      void AddAllInputNodes (const TechniqueGraph& graph,
        const Snippet::Technique* tech,
        CS::PluginCommon::ShaderWeaver::iCombiner* combiner);
      void AugmentCoerceChain (const WeaverCompiler* compiler, 
        const Snippet::Technique::CombinerPlugin& combinerPlugin,
        CS::PluginCommon::ShaderWeaver::iCombiner* combiner,
        CS::PluginCommon::ShaderWeaver::iCoerceChainIterator* linkChain, 
        TechniqueGraph& graph, Node& inNode, 
        const char* inName, const Snippet::Technique* outTech,
        const char* outName);
      Node& GetNodeForTech (const Snippet::Technique* tech)
      { return *nodes[techToNode.Get (tech, csArrayItemNotFound)]; }

      void ReverseNodeArray();
      
      bool Rebuild (const TechniqueGraph& graph,
        const csArray<const Snippet::Technique*>& outputs);
        
      void Collapse (TechniqueGraph& graph);

      BasicIterator<Node>* GetNodes();
      BasicIterator<Node>* GetNodesReverse();
    private:
      template<bool reverse>
      class NodesIterator : public BasicIterator<Node>
      {
        const NodeArray& array;
        size_t nextItem;

        void SeekNext()
        {
          if (reverse)
          {
            while ((nextItem >= 1) && (array[nextItem-1]->tech == 0))
              nextItem--;
          }
          else
          {
            while ((nextItem < array.GetSize()) && (array[nextItem]->tech == 0))
              nextItem++;
          }
        }
      public:
        NodesIterator (const NodeArray& array) : array (array),
          nextItem (reverse ? array.GetSize() : 0)
        {
          SeekNext();
        }

        bool HasNext()
        { return reverse ? (nextItem > 0) : (nextItem < array.GetSize()); }
        Node& Next()
        { 
          Node* val = reverse ? array[--nextItem] : array[nextItem++];
          SeekNext();
          return *val; 
        }
	size_t GetTotal() const
	{
	  CS_ASSERT(false);
	  return array.GetSize(); 
	}
      };
    };

    /// Structure to track what default inputs to emit.
    struct EmittedInput
    {
      const SynthesizeNodeTree::Node* node;
      const Snippet::Technique::Input* input;
      csArray<csString> conditions;
      csString tag;
    };
  public:
    WeaverCompiler* compiler;
  
    csStringHash& xmltokens;
  };

}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)

#endif // __CS_SYNTH_H__
