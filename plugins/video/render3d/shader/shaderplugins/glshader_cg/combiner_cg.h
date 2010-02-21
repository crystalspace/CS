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

#ifndef __COMBINER_CG_H__
#define __COMBINER_CG_H__

#include "csplugincommon/shader/weavercombiner.h"
#include "csutil/csstring.h"
#include "csutil/leakguard.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"
#include "csutil/strhash.h"

#include "iutil/comp.h"

#include "beautify_cg.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{
  namespace WeaverCommon = CS::PluginCommon::ShaderWeaver;

  struct FindCoerceChainHelper;

  class ShaderCombinerLoaderCg : 
    public scfImplementation2<ShaderCombinerLoaderCg, 
			      WeaverCommon::iCombinerLoader,
			      iComponent>
  {
  public:
    iObjectRegistry* object_reg;
  #define CS_TOKEN_ITEM_FILE \
    "plugins/video/render3d/shader/shaderplugins/glshader_cg/combiner_cg.tok"
  #include "cstool/tokenlist.h"
  #undef CS_TOKEN_ITEM_FILE
    csStringHash xmltokens;

    bool annotateCombined;
    
    CS_LEAKGUARD_DECLARE (ShaderCombinerLoaderCg);
  
    ShaderCombinerLoaderCg (iBase *parent);
    
    /**\name CS::PluginCommon::ShaderWeaver::iCombinerLoader implementation
    * @{ */
    csPtr<WeaverCommon::iCombiner> GetCombiner (iDocumentNode* params);

    void GenerateConstantInputBlocks (iDocumentNode* node,
      const char* locationPrefix, const csVector4& value,
      int usedComponents, const char* outputName);
    void GenerateSVInputBlocks (iDocumentNode* node,
      const char* locationPrefix, const char* svName, 
      const char* outputType, const char* outputName, 
      const char* uniqueTag);
    void GenerateBufferInputBlocks (iDocumentNode* node,
      const char* locationPrefix, const char* bufName, 
      const char* outputType, const char* outputName, 
      const char* uniqueTag);

    const char* GetCodeString() { return codeString; }
    /** @} */
  
    /**\name iComponent implementation
    * @{ */
    bool Initialize (iObjectRegistry* reg);
    /** @} */
  
    /// Report a message
    void Report (int severity, const char* msg, ...) CS_GNUC_PRINTF(3, 4);
    /// Report a parsing issue
    void Report (int severity, iDocumentNode* node, const char* msg, ...) 
      CS_GNUC_PRINTF(4, 5);
    
    csPtr<WeaverCommon::iCoerceChainIterator> QueryCoerceChain (
      const char* fromType, const char* toType);
    uint CoerceCost (const char* fromType, const char* toType);
  private:
    csString codeString;

    csStringHash typesSet;
    const char* StoredTypeName (const char* type)
    { return typesSet.Register (type); }

    struct CoerceItem
    {
      uint cost;
      const char* fromType;
      const char* toType;
      csRef<iDocumentNode> node;
    };
    typedef csArray<CoerceItem> CoerceItems;
    csHash<CoerceItems, const char*> coercions;
  
    bool LoadCoercionLibrary (const char* path);
    bool ParseCoercion (iDocumentNode* node);
    typedef csHash<csRef<iDocumentNode>, csString> CoercionTemplates;
    bool ParseCoercionTemplates (iDocumentNode* node, 
      CoercionTemplates& templates);
    bool SynthesizeDefaultCoercions (const CoercionTemplates& templates);
    
    static int CoerceItemCompare (CoerceItem const& i1, CoerceItem const& i2);
    
    void FindCoerceChain (const char* from, const char* to, 
      csArray<const CoerceItem*>& chain);
      
    class CoerceChainIterator : 
      public scfImplementation1<CoerceChainIterator,
                                WeaverCommon::iCoerceChainIterator>
    {
      size_t pos;
    public:
      csArray<const CoerceItem*> nodes;
      
      CoerceChainIterator() : scfImplementationType (this), pos (0) {}
      
      bool HasNext() { return pos < nodes.GetSize(); }
      csRef<iDocumentNode> Next () { return nodes[pos++]->node; }
      csRef<iDocumentNode> Next (const char*& fromType, const char*& toType)
      {
        fromType = nodes[pos]->fromType;
        toType = nodes[pos]->toType;
        return nodes[pos++]->node;
      }
      size_t GetNextPosition () { return pos; }
      size_t GetEndPosition () { return nodes.GetSize(); }
    };
  };
  
  class ShaderCombinerCg : 
    public scfImplementation1<ShaderCombinerCg,
			      CS::PluginCommon::ShaderWeaver::iCombiner>
  {
    csRef<ShaderCombinerLoaderCg> loader;
    bool writeVP, writeFP;
    
    struct Attribute
    {
      csString name;
      csString type;
    };
    typedef csArray<Attribute> AttributeArray;
    Attribute* FindAttr (AttributeArray& arr, const char* name, 
      const char* type);
    struct Snippet
    {
      csString annotation;
      csRefArray<iDocumentNode> vert2frag;
      csRefArray<iDocumentNode> vertexIn;
      csRefArray<iDocumentNode> fragmentIn; 
      
      csSet<csString> localIDs;
      csHash<csString, csString> v2fMaps;
      csString locals;
      csHash<csString, csString> inputMaps;
      csRefArray<iDocumentNode> vertexBody;
      csRefArray<iDocumentNode> fragmentBody;
      csHash<csString, csString> outputMaps;
      csString links;

      csHash<csString, csString> attrInputMaps;
      csHash<csString, csString> attrOutputMaps;
    };
    size_t uniqueCounter;
    csArray<Snippet> snippets;
    Snippet currentSnippet;
    csRefArray<iDocumentNode> vertexCompilerArgs;
    csRefArray<iDocumentNode> fragmentCompilerArgs; 
    csRefArray<iDocumentNode> variableMaps;
    csString outputAssign[rtaNumAttachments];
    csRefArray<iDocumentNode> definitions;
    csSet<csString> globalIDs;
    csString globals;

    csHash<AttributeArray, csString> attributes;
  public:
    ShaderCombinerCg (ShaderCombinerLoaderCg* loader, bool vp, bool fp);
    
    void BeginSnippet (const char* annotation = 0);
    void AddInput (const char* name, const char* type);
    void AddInputValue (const char* name, const char* type,
      const char* value);
    void AddOutput (const char* name, const char* type);
    void InputRename (const char* fromName, const char* toName);
    void OutputRename (const char* fromName, const char* toName);
    void PropagateAttributes (const char* fromInput, const char* toOutput);
    void AddOutputAttribute (const char* outputName,  const char* name, 
      const char* type);
    void AddInputAttribute (const char* inputName, const char* name, 
      const char* type, const char* defVal);
    void Link (const char* fromName, const char* toName);
    void WriteBlock (const char* location, iDocumentNode* blockNodes);
    bool EndSnippet ();
        
    void AddGlobal (const char* name, const char* type,
          const char* annotation = 0);
    void SetOutput (csRenderTargetAttachment target,
      const char* name, const char* annotation = 0);
    
    csPtr<WeaverCommon::iCoerceChainIterator> QueryCoerceChain (
      const char* fromType, const char* toType);

    uint CoerceCost (const char* fromType, const char* toType);
        
    void WriteToPass (iDocumentNode* pass);
    
    bool CompatibleParams (iDocumentNode* params);

    csRef<iString> QueryInputTag (const char* location, 
      iDocumentNode* blockNodes);
  private:
    class DocNodeCgAppender;
    class V2FAutoSematicsHelper;
  
    void AppendProgramInput (const csRefArray<iDocumentNode>& nodes, 
      DocNodeCgAppender& appender);
    void AppendProgramInput_V2FHead (const Snippet& snippet, 
      DocNodeCgAppender& appender);
    void AppendProgramInput_V2FDecl (const Snippet& snippet, 
      const V2FAutoSematicsHelper& semanticsHelper,
      DocNodeCgAppender& appender);
    void AppendProgramInput_V2FLocals (const Snippet& snippet, 
      DocNodeCgAppender& appender);
    void AppendProgramInput_V2FVP (const Snippet& snippet, 
      DocNodeCgAppender& appender);
    void AppendProgramInput_V2FFP (const Snippet& snippet, 
      DocNodeCgAppender& appender);
    void AppendProgramInput (iDocumentNode* node, DocNodeCgAppender& appender);
    csString CgType (const WeaverCommon::TypeInfo* typeInfo);
    csString CgType (const char* weaverType);
    csString GetAttrIdentifier (const char* var, const char* attr);

    csString annotateStr;
    const char* MakeComment (const char* s);
    void AppendSnippetMap (const csHash<csString, csString>& map, 
      DocNodeCgAppender& appender);
      
    void SplitOffArrayCount (csString& name, int& count);
    
    class DocNodeCgAppender
    {
      csRef<iDocumentNode> node;
      csString stringAppend;
      CgBeautifier beautifier;
      
      void FlushAppendString ();
    public:
      DocNodeCgAppender (iDocumentNode* node);
      ~DocNodeCgAppender ();
    
      void Append (const char* str);
      void Append (iDocumentNode* appendNode);
      void Append (const csRefArray<iDocumentNode>& nodes);
      void AppendFmt (const char* str, ...)
      {
        va_list args;
        va_start (args, str);
        csString s;
        s.FormatV (str, args);
        va_end (args);
        Append (s);
      }
    };
  };
}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)

#endif //__COMBINER_CG_H__
