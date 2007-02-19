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

#ifndef __CS_SNIPPET_H__
#define __CS_SNIPPET_H__

#include "csutil/csmd5.h"
#include "csutil/csstring.h"
#include "csutil/customallocated.h"
#include "csutil/parray.h"
#include "csutil/strhash.h"

#include "basiciter.h"

CS_PLUGIN_NAMESPACE_BEGIN(ShaderWeaver)
{
  class WeaverCompiler;
  class TechniqueGraphBuilder;

  class Snippet : public CS::Memory::CustomAllocated
  {
  public:
    class Technique
    {
    public:
      int priority;
      struct CombinerPlugin
      {
        csString name;
        csString classId;
        csRef<iDocumentNode> params;
      };
      //csArray<CombinerPlugin> combiners;
      struct Block
      {
        csString combinerName;
        csString location;
        csRef<iDocumentNode> node;
      };
      //csArray<Block> blocks;
      struct Input
      {
        csString name;
        csString type;
        enum
        {
          None,
          Complex
        } defaultType;
        csArray<Block> complexBlocks;
        
        Input() : defaultType (None) {}
      };
      //csArray<Input> inputs;
      struct Output
      {
        csString name;
        csString type;
      };
      //csArray<Output> outputs;
      
      Technique() : priority (0) {}
      virtual ~Technique() {}
      
      virtual bool IsCompound() const = 0;
      
      //virtual BasicIterator<const CombinerPlugin>* GetCombiners() const = 0;
      virtual const CombinerPlugin& GetCombiner() const = 0;
      virtual BasicIterator<const Input>* GetInputs() const = 0;
      virtual BasicIterator<const Output>* GetOutputs() const = 0;
      
      virtual BasicIterator<const Block>* GetBlocks() const = 0;
      //virtual BasicIterator<const Technique>* GetTechniques() const = 0;
      
      //void Link() = 0;
      
      //bool HasOutput (const char* name) const;
    };
    
    class AtomTechnique : public Snippet::Technique
    {
      friend class Snippet;
      csMD5::Digest id;
      //csArray<CombinerPlugin> combiners;
      CombinerPlugin combiner;
      csArray<Block> blocks;
      csArray<Input> inputs;
      csArray<Output> outputs;
    public:
      AtomTechnique (const csMD5::Digest& id) : id (id) {}
    
      virtual bool IsCompound() const { return false; }
      
      //void AddCombiner (const CombinerPlugin& comb) { combiners.Push (comb); }
      void SetCombiner (const CombinerPlugin& comb) { combiner = comb; }
      void AddBlock (const Block& block) { blocks.Push (block); }
      void AddInput (const Input& input) { inputs.Push (input); }
      void AddOutput (const Output& output) { outputs.Push (output); }
    
      //virtual BasicIterator<const CombinerPlugin>* GetCombiners() const;
      virtual const CombinerPlugin& GetCombiner() const { return combiner; }
      virtual BasicIterator<const Block>* GetBlocks() const
      { return new BasicIteratorImpl<const Block, csArray<Block> > (blocks); }
      virtual BasicIterator<const Input>* GetInputs() const
      { return new BasicIteratorImpl<const Input, csArray<Input> > (inputs); }
      virtual BasicIterator<const Output>* GetOutputs() const
      { return new BasicIteratorImpl<const Output, csArray<Output> > (outputs); }
      //virtual BasicIterator<const Technique>* GetTechniques() const { return 0; }
    };
    
    struct Connection
    {
      Snippet* from;
      Snippet* to;
    };
    
    class CompoundTechnique : public Snippet::Technique
    {
      friend class TechniqueGraphBuilder;
      friend class Snippet;
      
      typedef csHash<Snippet*, csString> IdSnippetHash;
      IdSnippetHash snippets;
      csArray<Connection> connections;
      /// "Input" snippets - those having no connections in
      csArray<Snippet*> inSnippets;
      /// "Output" snippets - those having no connections out
      csArray<Snippet*> outSnippets;
      Technique::CombinerPlugin combiner;
    public:
      virtual bool IsCompound() const { return true; }
      
      void AddSnippet (const char* id, Snippet* snippet);
      Snippet* GetSnippet (const char* id)
      { return snippets.Get (id, 0); }
      void AddConnection (const Connection& conn);
      
      //virtual BasicIterator<const CombinerPlugin>* GetCombiners() const { return 0; }
      virtual const CombinerPlugin& GetCombiner() const { return combiner; }
      virtual BasicIterator<const Block>* GetBlocks() const { return 0; }
      virtual BasicIterator<const Input>* GetInputs() const;
      virtual BasicIterator<const Output>* GetOutputs() const;
      //virtual BasicIterator<const Technique>* GetTechniques() const;
    };
    
    Snippet (WeaverCompiler* compiler, iDocumentNode* node, bool topLevel = false);
    virtual ~Snippet();
    
    bool IsCompound() const { return isCompound; }
    
    BasicIterator<const Technique*>* GetTechniques() const;
    
    static Technique* LoadLibraryTechnique (WeaverCompiler* compiler,
      iDocumentNode* node, const Technique::CombinerPlugin& combiner);
    static Technique* CreatePassthrough (const char* varName, const char* type);
  private:
    WeaverCompiler* compiler;
    csStringHash& xmltokens;
    typedef csPDelArray<Technique> TechniqueArray;
    TechniqueArray techniques;
    bool isCompound;
    
    void LoadAtomTechniques (iDocumentNode* node);
    void LoadAtomTechnique (iDocumentNode* node);
    static AtomTechnique* ParseAtomTechnique (WeaverCompiler* compiler,
      iDocumentNode* node);
    static bool ReadBlocks (WeaverCompiler* compiler, iDocumentNode* node,
      csArray<Technique::Block>& blocks);
    
    static int CompareTechnique (Technique* const&, Technique* const&);
    
    //void ClearSnippets ();
    
    void LoadCompoundTechniques (iDocumentNode* node, bool topLevel);
    void LoadCompoundTechnique (iDocumentNode* node);
    
    void HandleSnippetNode (CompoundTechnique& tech, iDocumentNode* node);
    void HandleConnectionNode (CompoundTechnique& tech, iDocumentNode* node);
    void HandleCombinerNode (CompoundTechnique& tech, iDocumentNode* node);
  };

  class TechniqueGraph
  {
  public:
    struct Connection
    {
      const Snippet::Technique* from;
      const Snippet::Technique* to;
      
      inline bool operator==(const Connection& other)
      { return (from == other.from) && (to == other.to); }
    };
    
    void AddTechnique (const Snippet::Technique* tech);
    void AddConnection (const Connection& conn);
    void RemoveConnection (const Connection& conn);
    
    void Merge (const TechniqueGraph& other);
    
    void GetOutputTechniques (csArray<const Snippet::Technique*>& outTechs) const
    { outTechs = outTechniques; }
    void GetDependencies (const Snippet::Technique* tech, csArray<const Snippet::Technique*>& deps) const;
  private:
    csArray<const Snippet::Technique*> techniques;
    csArray<Connection> connections;
    csArray<const Snippet::Technique*> outTechniques;
  };

  class TechniqueGraphBuilder
  {
    typedef csHash<const Snippet::Technique*, csConstPtrKey<Snippet> > SnippetToTechMap;
    struct GraphInfo
    {
      TechniqueGraph graph;
      SnippetToTechMap snippetToTechIn;
      SnippetToTechMap snippetToTechOut;
      
      void Merge (const GraphInfo& other);
    };
    void BuildSubGraphs (const Snippet* snip, csArray<GraphInfo>& graphs);
  public:
    void BuildGraphs (const Snippet* snip, csArray<TechniqueGraph>& graphs);
  };
}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)

#endif // __CS_SNIPPET_H__
