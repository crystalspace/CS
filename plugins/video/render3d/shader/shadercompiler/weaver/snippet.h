/*
  Copyright (C) 2003-2007 by Marten Svanfeldt
		2004-2011 by Frank Richter

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

#include "csutil/csstring.h"
#include "csutil/customallocated.h"
#include "csutil/md5.h"
#include "csutil/noncopyable.h"
#include "csutil/parray.h"
#include "csutil/strhash.h"

#include "basiciter.h"

CS_PLUGIN_NAMESPACE_BEGIN(ShaderWeaver)
{
  class WeaverCompiler;
  class TechniqueGraphBuilder;
  
  typedef csHash<csString, csString> FileAliases;

  class Snippet : public CS::Memory::CustomAllocated
  {
  public:
    class Technique : public CS:: Utility::FastRefCount<Technique>
    {
    public:
      const Snippet* owner;
      const char* snippetName;
      int priority;
      csString outerCondition;
      struct CombinerPlugin
      {
        csString name;
        csString classId;
        csRef<iDocumentNode> params;
      };
      struct Block
      {
        csString combinerName;
        csString location;
        csRef<iDocumentNode> node;
      };
      struct Attribute
      {
        csString name;
        csString type;
        csString defaultValue;
      };
      struct Input
      {
        csRef<iDocumentNode> node;
        csString name;
        csString type;
        csString condition;
        csString defaultValue;
        enum
        {
          None,
	  Undefined,
          Value,
          Complex
        } defaultType : 3;
        bool isPrivate : 1;
        bool noMerge : 1;
        csArray<Block> complexBlocks;
        csArray<Attribute> attributes;
        
        Input() : defaultType (None), isPrivate (false), noMerge (false) {}
      };
      struct Output
      {
        csString name;
        csString type;
        csString inheritAttrFrom;
        bool coercionOutput;
        csArray<Attribute> attributes;
        
        Output() : coercionOutput (false) {}
      };
      
      Technique (const Snippet* owner, const char* snippetName) : owner (owner),
        snippetName (snippetName), priority (0) {}
      virtual ~Technique() {}
      
      // Inner condition: put around blocks from the technique
      csString GetInnerCondition() const;
      // Outer condition: put around xmlshader techniques
      const char* GetOuterCondition() const { return outerCondition; }
      
      virtual bool IsCompound() const = 0;
      
      virtual const CombinerPlugin& GetCombiner() const = 0;
      virtual BasicIterator<const Input>* GetInputs() const = 0;
      virtual BasicIterator<const Output>* GetOutputs() const = 0;
      
      virtual BasicIterator<const Block>* GetBlocks() const = 0;
    };
    
    class AtomTechnique : public Snippet::Technique
    {
      friend class Snippet;
      CS::Utility::Checksum::MD5::Digest id;
      CombinerPlugin combiner;
      csArray<Block> blocks;
      csArray<Input> inputs;
      csArray<Output> outputs;
    public:
      AtomTechnique (const Snippet* owner, const char* snippetName,
        const CS::Utility::Checksum::MD5::Digest& id) : Technique (owner, snippetName), id (id) {}
    
      virtual bool IsCompound() const { return false; }
      const CS::Utility::Checksum::MD5::Digest& GetID() const { return id; }
      
      void SetCombiner (const CombinerPlugin& comb) { combiner = comb; }
      void AddBlock (const Block& block) { blocks.Push (block); }
      void AddInput (const Input& input) { inputs.Push (input); }
      void AddOutput (const Output& output) { outputs.Push (output); }
    
      virtual const CombinerPlugin& GetCombiner() const { return combiner; }
      virtual BasicIterator<const Block>* GetBlocks() const
      { return new BasicIteratorImpl<const Block, csArray<Block> > (blocks); }
      virtual BasicIterator<const Input>* GetInputs() const
      { return new BasicIteratorImpl<const Input, csArray<Input> > (inputs); }
      virtual BasicIterator<const Output>* GetOutputs() const
      { return new BasicIteratorImpl<const Output, csArray<Output> > (outputs); }
      
      BasicIterator<Output>* GetOutputs()
      { return new BasicIteratorImplNonConst<Output, csArray<Output> > (outputs); }
    };
    
    struct ExplicitConnectionSource
    {
      const Snippet* from;
      csString outputName;
    };
    typedef csHash<ExplicitConnectionSource, csString> ExplicitConnectionsHash;
    struct Connection
    {
      Snippet* from;
      Snippet* to;
    };
    
    class CompoundTechnique : public Snippet::Technique,
                              public CS::NonCopyable
    {
      friend class TechniqueGraphBuilder;
      friend class Snippet;
      
      typedef csHash<Snippet*, csString> IdSnippetHash;
      IdSnippetHash snippets;
      // Snippets in the order they were added
      csArray<Snippet*> snippetsOrdered;
      csArray<Connection> connections;
      /// "Input" snippets - those having no connections in
      csArray<Snippet*> inSnippets;
      /// "Output" snippets - those having no connections out
      csArray<Snippet*> outSnippets;
      Technique::CombinerPlugin combiner;
      csHash<ExplicitConnectionsHash, csPtrKey<Snippet> > explicitConnections;
    public:
      CompoundTechnique (const Snippet* owner, const char* snippetName) : 
        Technique (owner, snippetName) {}
      ~CompoundTechnique();

      virtual bool IsCompound() const { return true; }
      
      void AddSnippet (const char* id, Snippet* snippet);
      Snippet* GetSnippet (const char* id)
      { return snippets.Get (id, 0); }
      void AddConnection (const Connection& conn);
      
      ExplicitConnectionsHash& GetExplicitConnections (Snippet* to)
      { return explicitConnections.GetOrCreate (to); }
      const ExplicitConnectionsHash* GetExplicitConnections (Snippet* to) const
      { return explicitConnections.GetElementPointer (to); }
      
      virtual const CombinerPlugin& GetCombiner() const { return combiner; }
      virtual BasicIterator<const Block>* GetBlocks() const { return 0; }
      virtual BasicIterator<const Input>* GetInputs() const;
      virtual BasicIterator<const Output>* GetOutputs() const;
      
      BasicIterator<Snippet*>* GetSnippets();
      BasicIterator<Snippet* const>* GetSnippets() const;
    };
    
    Snippet (const WeaverCompiler* compiler, iDocumentNode* node,
      const char* name, const FileAliases& aliases,
      const Snippet* parent);
    Snippet (const WeaverCompiler* compiler, const char* name);
    virtual ~Snippet();
    
    const char* GetName() const { return name; }
    bool IsCompound() const { return isCompound; }
    iDocumentNode* GetSourceNode() const { return node; }
    
    csString GetCondition() const;
    
    BasicIterator<const Technique*>* GetTechniques() const;
    BasicIterator<Technique*>* GetTechniques();
    
    Technique* LoadLibraryTechnique (
      iDocumentNode* node, const Technique::CombinerPlugin& combiner,
      bool markAsCoercion = false) const;
    Technique* CreatePassthrough (const char* varName, const char* type) const;
    
    const csRefArray<iDocumentNode>& GetPassForwardedNodes() const
    { return passForwardedNodes; }
    
    static bool ParseAliasNode (const WeaverCompiler* compiler,
      iDocumentNode* node, FileAliases& aliases);
  private:
    const WeaverCompiler* compiler;
    const csStringHash& xmltokens;
    csString name;
    csRef<iDocumentNode> node;
    csString condition;
    typedef csPDelArray<Technique> TechniqueArray;
    TechniqueArray techniques;
    bool isCompound;
    bool passForward;
    csRefArray<iDocumentNode> passForwardedNodes;
    const Snippet* parent;
    
    void LoadAtomTechniques (iDocumentNode* node, const FileAliases& aliases);
    void LoadAtomTechnique (iDocumentNode* node, const FileAliases& aliases);
    AtomTechnique* ParseAtomTechnique (/*WeaverCompiler* compiler,*/
      iDocumentNode* node, bool canOmitCombiner,
      const FileAliases& aliases, const char* defaultCombinerName = 0) const;
    bool ParseCombiner (iDocumentNode* child, 
      Technique::CombinerPlugin& newCombiner) const;
    bool ParseInput (iDocumentNode* child, AtomTechnique& newTech,
      const FileAliases& aliases, const char* defaultCombinerName) const;
    bool ParseOutput (iDocumentNode* child, 
      Technique::Output& newOutput) const;
    bool ParseAttribute (iDocumentNode* child, 
      Technique::Attribute& attr) const;
    static bool ReadBlocks (const WeaverCompiler* compiler, iDocumentNode* node,
      csArray<Technique::Block>& blocks, const FileAliases& aliases, 
      const char* defaultCombinerName = 0);
    
    static csRef<iDocumentNode> GetNodeOrFromFile (iDocumentNode* node,
      const char* rootName, const WeaverCompiler* compiler,
      const FileAliases& aliases, csString* outFilename = 0);
    
    static int CompareTechnique (Technique* const&, Technique* const&);
    
    void LoadCompoundTechniques (iDocumentNode* node, 
      const FileAliases& aliases, bool topLevel);
    void LoadCompoundTechnique (iDocumentNode* node, 
      const FileAliases& aliases);
    
    void HandleSnippetNode (CompoundTechnique& tech, iDocumentNode* node,
      const FileAliases& aliases);
    void HandleConnectionNode (CompoundTechnique& tech, iDocumentNode* node);
    void HandleCombinerNode (CompoundTechnique& tech, iDocumentNode* node);
    void HandleParameterNode (CompoundTechnique& tech, iDocumentNode* node,
      const FileAliases& aliases);
    void HandleVaryingNode (CompoundTechnique& tech, iDocumentNode* node,
      const FileAliases& aliases);
    void HandleCompoundInput (CompoundTechnique& tech, iDocumentNode* node,
      const FileAliases& aliases);
  };
  
  /// Helper to assign a running ID to each snippet
  class SnippetNumbers
  {
    csHash<size_t, csString> snippetNums;
    size_t currentNum;
  public:
    SnippetNumbers() : currentNum (0) {}
  
    size_t GetAllSnippetsCount() const { return snippetNums.GetSize(); }
    size_t GetSnippetNumber (const Snippet* snip);
  };
  
  /// Helper to manage priorities for snippets
  class SnippetTechPriorities
  {
    csArray<int> prios;
  public:
    int GetSnippetPriority (size_t snippetNum) const
    { return prios[snippetNum]; }
    void SetSnippetPriority (size_t snippetNum, int prio)
    {
      if (prios.GetSize() <= snippetNum)
        prios.SetSize (snippetNum+1, INT_MIN);
      prios[snippetNum] = prio;
    }
    bool IsSnippetPrioritySet (size_t snippetNum) const
    { 
      return (prios.GetSize() > snippetNum)
        && (prios[snippetNum] != INT_MIN); 
    }
    
    void Merge (const SnippetTechPriorities& other);
  };

  class TechniqueGraph
  {
  public:
    struct ExplicitConnectionSource
    {
      const Snippet::Technique* from;
      csString outputName;
    };
    typedef csHash<ExplicitConnectionSource, csString> ExplicitConnectionsHash;
    
    struct Connection
    {
      /* Don't use connection for input/output matching.
         (But it still affects ordering.) */
      bool inputConnection;
      const Snippet::Technique* from;
      const Snippet::Technique* to;

      Connection () : inputConnection (false), from (0), to (0) {}
      
      inline bool operator==(const Connection& other)
      { return (from == other.from) && (to == other.to); }
    };
    
    void AddTechnique (const Snippet::Technique* tech);
    void RemoveTechnique (const Snippet::Technique* tech);
    void AddConnection (const Connection& conn);
    void RemoveConnection (const Connection& conn);
    
    void Merge (const TechniqueGraph& other);
    
    void GetInputTechniques (csArray<const Snippet::Technique*>& inTechs) const
    { inTechs = inTechniques; }
    void GetOutputTechniques (csArray<const Snippet::Technique*>& outTechs) const
    { outTechs = outTechniques; }
    void GetDependencies (const Snippet::Technique* tech, csArray<const Snippet::Technique*>& deps,
      bool strongOnly = true) const;
    void GetDependants (const Snippet::Technique* tech, csArray<const Snippet::Technique*>& deps,
      bool strongOnly = true) const;
      
    bool IsDependencyOf (const Snippet::Technique* tech,
      const Snippet::Technique* dependentOf) const;
      
    void SwitchTechs (const Snippet::Technique* oldTech,
      const Snippet::Technique* newTech, bool inputsOnly);
      
    const ExplicitConnectionsHash* GetExplicitConnections (const Snippet::Technique* to) const
    { return explicitConnections.GetElementPointer (to); }
    ExplicitConnectionsHash& GetExplicitConnections (const Snippet::Technique* to)
    { return explicitConnections.GetOrCreate (to); }
    
    SnippetTechPriorities& GetSnippetPrios () { return snipPrios; } 
    const SnippetTechPriorities& GetSnippetPrios () const { return snipPrios; } 
  private:
    typedef csArray<const Snippet::Technique*> TechniquePtrArray;
    TechniquePtrArray techniques;
    csArray<Connection> connections;
    TechniquePtrArray inTechniques;
    TechniquePtrArray outTechniques;
    typedef csHash<ExplicitConnectionsHash, csConstPtrKey<Snippet::Technique> >
      ExplicitConnectionsHashHash;
    ExplicitConnectionsHashHash explicitConnections;
    SnippetTechPriorities snipPrios;
  };

  class TechniqueGraphBuilder
  {
    SnippetNumbers& snipNums;
    typedef csHash<const Snippet::Technique*, csConstPtrKey<Snippet> > SnippetToTechMap;
    struct GraphInfo
    {
      TechniqueGraph graph;
      SnippetToTechMap snippetToTechIn;
      SnippetToTechMap snippetToTechOut;
      
      void Merge (const GraphInfo& other);
    };
    void BuildSubGraphs (const Snippet* snip, csArray<GraphInfo>& graphs);
    void FixupExplicitConnections (const Snippet* snip, csArray<GraphInfo>& graphs);
    void MapGraphInputsOutputs (GraphInfo& graphInfo, const Snippet* snip);
    void MapGraphInputsOutputs (csArray<GraphInfo>& graphs, 
      const Snippet* snip);
  public:
    TechniqueGraphBuilder (SnippetNumbers& snipNums) : snipNums (snipNums) {}
    
    void BuildGraphs (const Snippet* snip, csArray<TechniqueGraph>& graphs);
  };
}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)

#endif // __CS_SNIPPET_H__
