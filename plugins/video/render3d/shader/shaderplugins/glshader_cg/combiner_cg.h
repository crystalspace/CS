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

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{
  struct FindCoerceChainHelper;

  class ShaderCombinerLoaderCg : 
    public scfImplementation2<ShaderCombinerLoaderCg, 
			      CS::PluginCommon::ShaderWeaver::iCombinerLoader,
			      iComponent>
  {
  public:
    iObjectRegistry* object_reg;
  #define CS_TOKEN_ITEM_FILE \
    "plugins/video/render3d/shader/shaderplugins/glshader_cg/combiner_cg.tok"
  #include "cstool/tokenlist.h"
  #undef CS_TOKEN_ITEM_FILE
    csStringHash xmltokens;
    
    CS_LEAKGUARD_DECLARE (ShaderCombinerLoaderCg);
  
    ShaderCombinerLoaderCg (iBase *parent);
    
    /**\name CS::PluginCommon::ShaderWeaver::iCombinerLoader implementation
    * @{ */
    csPtr<CS::PluginCommon::ShaderWeaver::iCombiner> GetCombiner (
      iDocumentNode* params);
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
    
    csPtr<iDocumentNodeIterator> QueryCoerceChain (const char* fromType,
      const char* toType);
    uint CoerceCost (const char* fromType, const char* toType);
  private:
    struct CoerceItem
    {
      uint cost;
      csString toType;
      csRef<iDocumentNode> node;
    };
    typedef csArray<CoerceItem> CoerceItems;
    csHash<CoerceItems, csString> coercions;
  
    bool LoadCoercionLibrary (const char* path);
    bool ParseCoercion (iDocumentNode* node);
    
    static int CoerceItemCompare (CoerceItem const& i1, CoerceItem const& i2);
    
    void FindCoerceChain (const char* from, const char* to, 
      csArray<const CoerceItem*>& chain);
      
    class CoerceChainIterator : 
      public scfImplementation1<CoerceChainIterator,
                                iDocumentNodeIterator>
    {
      size_t pos;
    public:
      csRefArray<iDocumentNode> nodes;
      
      CoerceChainIterator() : scfImplementationType (this), pos (0) {}
      
      bool HasNext() { return pos < nodes.GetSize(); }
      csRef<iDocumentNode> Next () { return nodes[pos++]; }
    };
  };
  
  class ShaderCombinerCg : 
    public scfImplementation1<ShaderCombinerCg,
			      CS::PluginCommon::ShaderWeaver::iCombiner>
  {
    csRef<ShaderCombinerLoaderCg> loader;
    bool writeVP, writeFP;
    
    struct Snippet
    {
      csRefArray<iDocumentNode> vert2frag;
      csRefArray<iDocumentNode> vertexIn;
      csRefArray<iDocumentNode> fragmentIn; 
      
      csSet<csString> localIDs;
      csString locals;
      csString inputMaps;
      csRefArray<iDocumentNode> vertexBody;
      csRefArray<iDocumentNode> fragmentBody;
      csString outputMaps;
      csString links;
    };
    csArray<Snippet> snippets;
    Snippet currentSnippet;
    csRefArray<iDocumentNode> variableMaps;
    csString outputAssign;
    csRefArray<iDocumentNode> includes;
    csSet<csString> globalIDs;
    csString globals;
  public:
    ShaderCombinerCg (ShaderCombinerLoaderCg* loader, bool vp, bool fp);
    
    void BeginSnippet ();
    void AddInput (const char* name, const char* type);
    void AddOutput (const char* name, const char* type);
    void InputRename (const char* fromName, const char* toName);
    void OutputRename (const char* fromName, const char* toName);
    csPtr<iDocumentNodeIterator> QueryCoerceChain (const char* fromType,
      const char* toType);
    void Link (const char* fromName, const char* toName);
    void WriteBlock (const char* location, iDocumentNode* blockNodes);
    bool EndSnippet ();
        
    void AddGlobal (const char* name, const char* type);
    void SetOutput (const char* name);
    
    uint CoerceCost (const char* fromType, const char* toType);
        
    void WriteToPass (iDocumentNode* pass);
    
    bool CompatibleParams (iDocumentNode* params);
  private:
    class DocNodeAppender;
  
    void AppendProgramInput (const csRefArray<iDocumentNode>& nodes, 
      DocNodeAppender& appender);
    csString CgType (const char* weaverType);
    
    class DocNodeAppender
    {
      csRef<iDocumentNode> node;
      csString stringAppend;
      
      void FlushAppendString ();
    public:
      DocNodeAppender (iDocumentNode* node);
      ~DocNodeAppender ();
    
      void Append (const char* str);
      void Append (iDocumentNode* appendNode);
      void Append (const csRefArray<iDocumentNode>& nodes);
    };
  };
}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)

#endif //__COMBINER_CG_H__
