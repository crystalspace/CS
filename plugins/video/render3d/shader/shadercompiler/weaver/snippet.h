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
#include "csutil/noncopyable.h"
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
      const char* snippetName;
      int priority;
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
        csArray<Attribute> attributes;
      };
      
      Technique (const char* snippetName) : snippetName (snippetName), 
        priority (0) {}
      virtual ~Technique() {}
      
      virtual bool IsCompound() const = 0;
      
      virtual const CombinerPlugin& GetCombiner() const = 0;
      virtual BasicIterator<const Input>* GetInputs() const = 0;
      virtual BasicIterator<const Output>* GetOutputs() const = 0;
      
      virtual BasicIterator<const Block>* GetBlocks() const = 0;
    };
    
    class AtomTechnique : public Snippet::Technique
    {
      friend class Snippet;
      csMD5::Digest id;
      CombinerPlugin combiner;
      csArray<Block> blocks;
      csArray<Input> inputs;
      csArray<Output> outputs;
    public:
      AtomTechnique (const char* snippetName, const csMD5::Digest& id) : 
        Technique (snippetName), id (id) {}
    
      virtual bool IsCompound() const { return false; }
      const csMD5::Digest& GetID() const { return id; }
      
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
    };
    
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
      csArray<Connection> connections;
      /// "Input" snippets - those having no connections in
      csArray<Snippet*> inSnippets;
      /// "Output" snippets - those having no connections out
      csArray<Snippet*> outSnippets;
      Technique::CombinerPlugin combiner;
    public:
      CompoundTechnique (const char* snippetName) : Technique (snippetName) {}
      ~CompoundTechnique();

      virtual bool IsCompound() const { return true; }
      
      void AddSnippet (const char* id, Snippet* snippet);
      Snippet* GetSnippet (const char* id)
      { return snippets.Get (id, 0); }
      void AddConnection (const Connection& conn);
      
      virtual const CombinerPlugin& GetCombiner() const { return combiner; }
      virtual BasicIterator<const Block>* GetBlocks() const { return 0; }
      virtual BasicIterator<const Input>* GetInputs() const;
      virtual BasicIterator<const Output>* GetOutputs() const;
    };
    
    Snippet (WeaverCompiler* compiler, iDocumentNode* node, const char* name,
      bool topLevel = false);
    Snippet (WeaverCompiler* compiler, const char* name);
    virtual ~Snippet();
    
    const char* GetName() const { return name; }
    bool IsCompound() const { return isCompound; }
    
    BasicIterator<const Technique*>* GetTechniques() const;
    
    Technique* LoadLibraryTechnique (WeaverCompiler* compiler,
      iDocumentNode* node, const Technique::CombinerPlugin& combiner) const;
    Technique* CreatePassthrough (const char* varName, const char* type) const;
  private:
    WeaverCompiler* compiler;
    csStringHash& xmltokens;
    csString name;
    typedef csPDelArray<Technique> TechniqueArray;
    TechniqueArray techniques;
    bool isCompound;
    
    void LoadAtomTechniques (iDocumentNode* node);
    void LoadAtomTechnique (iDocumentNode* node);
    AtomTechnique* ParseAtomTechnique (WeaverCompiler* compiler,
      iDocumentNode* node, bool canOmitCombiner, 
      const char* defaultCombinerName = 0) const;
    bool ParseCombiner (iDocumentNode* child, 
      Technique::CombinerPlugin& newCombiner) const;
    bool ParseInput (iDocumentNode* child, Technique::Input& newInput, 
      const char* defaultCombinerName) const;
    bool ParseOutput (iDocumentNode* child, 
      Technique::Output& newOutput) const;
    bool ParseAttribute (iDocumentNode* child, 
      Technique::Attribute& attr) const;
    static bool ReadBlocks (WeaverCompiler* compiler, iDocumentNode* node,
      csArray<Technique::Block>& blocks, const char* defaultCombinerName = 0);
    
    static csRef<iDocumentNode> GetNodeOrFromFile (iDocumentNode* node,
      const char* rootName, WeaverCompiler* compiler,
      csString* outFilename = 0);
    
    static int CompareTechnique (Technique* const&, Technique* const&);
    
    void LoadCompoundTechniques (iDocumentNode* node, bool topLevel);
    void LoadCompoundTechnique (iDocumentNode* node);
    
    void HandleSnippetNode (CompoundTechnique& tech, iDocumentNode* node);
    void HandleConnectionNode (CompoundTechnique& tech, iDocumentNode* node);
    void HandleCombinerNode (CompoundTechnique& tech, iDocumentNode* node);
    void HandleParameterNode (CompoundTechnique& tech, iDocumentNode* node);
  };

  class TechniqueGraph
  {
  public:
    struct Connection
    {
      /* If a connection is weak, don't use it for input/output matching.
         (But it still affects ordering.) */
      bool weak;
      const Snippet::Technique* from;
      const Snippet::Technique* to;

      Connection () : weak (false), from (0), to (0) {}
      
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
  private:
    typedef csArray<const Snippet::Technique*> TechniquePtrArray;
    TechniquePtrArray techniques;
    csArray<Connection> connections;
    TechniquePtrArray inTechniques;
    TechniquePtrArray outTechniques;
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
    void MapGraphInputsOutputs (GraphInfo& graphInfo, const Snippet* snip);
    void MapGraphInputsOutputs (csArray<GraphInfo>& graphs, 
      const Snippet* snip);
  public:
    void BuildGraphs (const Snippet* snip, csArray<TechniqueGraph>& graphs);
  };
}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)

#endif // __CS_SNIPPET_H__
