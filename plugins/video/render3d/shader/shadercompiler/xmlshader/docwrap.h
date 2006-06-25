/*
  Copyright (C) 2004-2006 by Frank Richter
	    (C) 2004-2006 by Jorrit Tyberghein

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

#ifndef __CS_DOCWRAP_H__
#define __CS_DOCWRAP_H__

#include "csutil/csstring.h"
#include "csutil/documentcommon.h"
#include "csutil/leakguard.h"
#include "csutil/parray.h"
#include "csutil/pooledscfclass.h"
#include "csutil/ref.h"
#include "csutil/refarr.h"
#include "csutil/scf.h"
#include "csutil/scf_implementation.h"
#include "csutil/strhash.h"
#include "csutil/weakref.h"

#include "iutil/document.h"
#include "iutil/objreg.h"

#include "condeval.h"
#include "docwrap_replacer.h"

// Hack: Work around problems caused by #defining 'new'.
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# undef new
#endif
#include <new>

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

class csXMLShaderCompiler;

class csWrappedDocumentNodeIterator;
struct WrapperStackEntry;

struct csConditionNode;
class csWrappedDocumentNodeFactory;

/**
 * Callback to parse and evaluate conditions, used by
 * csWrappedDocumentNode.
 */
struct iConditionResolver
{
  virtual ~iConditionResolver() {}

  /**
   * Parse a condition and return it's ID. 
   */
  virtual const char* ParseCondition (const char* str, size_t len, 
    csConditionID& result) = 0;
  /**
   * Evaluate a condition.
   */
  virtual bool Evaluate (csConditionID condition) = 0;

  /**
   * Add a condition to the condition tree. When later a specific variant of
   * a document will be chosen, this tree is used to quickly determine it.
   */
  virtual void AddNode (csConditionNode* parent,
    csConditionID condition, csConditionNode*& trueNode, 
    csConditionNode*& falseNode) = 0;
  /// Finish adding of nodes. Frees up some tempoarily used resources.
  virtual void FinishAdding () = 0;

  virtual const char* ParseCondition (const char* str, size_t len, 
    CondOperation& operation) = 0;
};

/**
 * Wrapper around a document node, supporting conditionals.
 */
class csWrappedDocumentNode : 
  public scfImplementationExt0<csWrappedDocumentNode, csDocumentNodeReadOnly>
{
  friend class csWrappedDocumentNodeIterator;
  friend struct WrapperStackEntry;
  friend class csTextNodeWrapper;
  friend class csWrappedDocumentNodeFactory;

  csRef<iDocumentNode> wrappedNode;
  csWeakRef<csWrappedDocumentNode> parent;
  iConditionResolver* resolver;
  iObjectRegistry* objreg;
  csString contents;
  csWrappedDocumentNodeFactory* shared;

public:
  /**
   * Contains all the consecutive children that are dependant on the same
   * condition.
   */
  struct WrappedChild
  {
    csRef<iDocumentNode> childNode;

    csConditionID condition;
    bool conditionValue;
    csPDelArray<WrappedChild> childrenWrappers;

    WrappedChild()
    {
      condition = csCondAlwaysTrue;
      conditionValue = true;
    }

    typedef csBlockAllocator<WrappedChild> WrappedChildAlloc;
    CS_DECLARE_STATIC_CLASSVAR_REF (childAlloc,
      ChildAlloc, WrappedChildAlloc);

    inline void* operator new (size_t n)
    {
      (void)n; // Pacify compiler warnings
      CS_ASSERT(n == sizeof (WrappedChild));
      return ChildAlloc().AllocUninit();
    }
    inline void operator delete (void* p)
    {
      ChildAlloc().FreeUninit (p);
    }
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
    inline void* operator new (size_t n, void*, int)
    { return WrappedChild::operator new (n); }
    inline void operator delete (void* p, void*, int)
    { WrappedChild::operator delete (p); }
#endif
  };
protected:
  csPDelArray<WrappedChild> wrappedChildren;

  /**
   * Helper class to go over the wrapped children in a linear fashion,
   * without having to care that a hierarchy of WrappedChild structures 
   * are actually used.
   */
  class WrapperWalker
  {
    struct WrapperPosition
    {
      size_t currentIndex;
      csPDelArray<WrappedChild>* currentWrappers;
    };
    csArray<WrapperPosition> posStack;
    WrapperPosition* currentPos;
    csRef<iDocumentNode> next;
    iConditionResolver* resolver;

    void SeekNext ();
  public:
    WrapperWalker (csPDelArray<WrappedChild>& wrappedChildren,
      iConditionResolver* resolver);
    WrapperWalker ();
    void SetData (csPDelArray<WrappedChild>& wrappedChildren,
      iConditionResolver* resolver);

    bool HasNext ();
    iDocumentNode* Peek ();
    iDocumentNode* Next ();
  };
  friend class WrapperWalker;

  struct Template
  {
    typedef csRefArray<iDocumentNode, TempHeapAlloc> Nodes;
    typedef csArray<TempString<>, csArrayElementHandler<TempString<> >, 
      TempHeapAlloc> Params;
    Nodes nodes;
    Params paramMap;
  };
  struct GlobalProcessingState : public csRefCount, protected TempHeap
  {
    static GlobalProcessingState* Create()
    {
      GlobalProcessingState* gps;
      gps = (GlobalProcessingState*)TempHeap::Alloc (sizeof (GlobalProcessingState));
      new (gps) GlobalProcessingState;
      return gps;
    }
    virtual void Delete ()
    {
      this->~GlobalProcessingState();
      TempHeap::Free (this);
    }

    csHash<Template, TempString<>, TempHeapAlloc> templates;
    csArray<int, csArrayElementHandler<int>, TempHeapAlloc> ascendStack;
  };
  csRef<GlobalProcessingState> globalState;

  struct NodeProcessingState;
  void ParseCondition (WrapperStackEntry& newWrapper, const char* cond,
    size_t condLen, iDocumentNode* node);
  void CreateElseWrapper (NodeProcessingState* state, 
    WrapperStackEntry& elseWrapper);
  template<typename ConditionEval>
  void ProcessInclude (ConditionEval& eval, const csString& filename, 
    NodeProcessingState* state, iDocumentNode* node);
  template<typename ConditionEval>
  void ProcessTemplate (ConditionEval& eval, iDocumentNode* templNode, 
    NodeProcessingState* state);
  bool InvokeTemplate (Template* templ, const Template::Params& params,
    Template::Nodes& templatedNodes);
  template<typename ConditionEval>
  bool InvokeTemplate (ConditionEval& eval, const char* name, 
    iDocumentNode* node, NodeProcessingState* state, 
    const Template::Params& params);
  void ValidateTemplateEnd (iDocumentNode* node, 
    NodeProcessingState* state);
  void ValidateGenerateEnd (iDocumentNode* node, 
    NodeProcessingState* state);
  void ParseTemplateArguments (const char* str, 
    Template::Params& strings);

  template<typename ConditionEval>
  void ProcessSingleWrappedNode (ConditionEval& eval, 
    NodeProcessingState* state, iDocumentNode* wrappedNode);
  template<typename ConditionEval>
  void ProcessWrappedNode (ConditionEval& eval, NodeProcessingState* state,
  	iDocumentNode* wrappedNode);
  template<typename ConditionEval>
  void ProcessWrappedNode (ConditionEval& eval);
  void Report (int severity, iDocumentNode* node, const char* msg, ...);
  
  static void AppendNodeText (WrapperWalker& walker, csString& text);

  template<typename ConditionEval>
  csWrappedDocumentNode (ConditionEval& eval,
    iDocumentNode* wrappedNode,
    iConditionResolver* resolver,
    csWrappedDocumentNodeFactory* shared, 
    GlobalProcessingState* globalState);
public:
  CS_LEAKGUARD_DECLARE(csWrappedDocumentNode);

  virtual ~csWrappedDocumentNode ();

  virtual csDocumentNodeType GetType ()
  { return wrappedNode->GetType(); }
  virtual bool Equals (iDocumentNode* other);
  virtual const char* GetValue ();

  virtual csRef<iDocumentNode> GetParent ()
  { return (iDocumentNode*)parent; }
  virtual csRef<iDocumentNodeIterator> GetNodes ();
  virtual csRef<iDocumentNodeIterator> GetNodes (const char* value);
  virtual csRef<iDocumentNode> GetNode (const char* value);

  virtual const char* GetContentsValue ();

  virtual csRef<iDocumentAttributeIterator> GetAttributes ();
  virtual csRef<iDocumentAttribute> GetAttribute (const char* name);
  virtual const char* GetAttributeValue (const char* name);
  virtual int GetAttributeValueAsInt (const char* name);
  virtual float GetAttributeValueAsFloat (const char* name);
  virtual bool GetAttributeValueAsBool (const char* name, 
    bool defaultvalue = false);
};

class csTextNodeWrapper : 
  public scfImplementationPooled<scfImplementationExt0<csTextNodeWrapper, 
                                                       csDocumentNodeReadOnly> >
{
  char* nodeText;
  csRef<iDocumentNode> realMe;
public:
  csTextNodeWrapper (iDocumentNode* realMe, const char* text);
  virtual ~csTextNodeWrapper ();

  virtual csDocumentNodeType GetType ()
  { return CS_NODE_TEXT; }
  virtual bool Equals (iDocumentNode* other)
  { return realMe->Equals (other); }

  virtual const char* GetValue ()
  { return nodeText; }

  virtual csRef<iDocumentNode> GetParent ()
  { return realMe->GetParent (); }

  virtual const char* GetContentsValue ()
  { return 0; }
};

class csWrappedDocumentNodeIterator : 
  public scfImplementationPooled<
    scfImplementation1<csWrappedDocumentNodeIterator, 
                       iDocumentNodeIterator> >
{
  csString filter;

  csWrappedDocumentNode* parentNode;
  csWrappedDocumentNode::WrapperWalker walker;
  csRef<iDocumentNode> next;
  void SeekNext();
public:
  CS_LEAKGUARD_DECLARE(csWrappedDocumentNodeIterator);

  csWrappedDocumentNodeIterator (csWrappedDocumentNode* node, const char* filter);
  virtual ~csWrappedDocumentNodeIterator ();

  virtual bool HasNext ();
  virtual csRef<iDocumentNode> Next ();
};

class csWrappedDocumentNodeFactory
{
  friend class csWrappedDocumentNode;
  friend class csWrappedDocumentNodeIterator;

  csXMLShaderCompiler* plugin;
  csTextNodeWrapper::Pool textNodePool;
  csWrappedDocumentNodeIterator::Pool iterPool;
  csReplacerDocumentNodeFactory replacerFactory;

  csStringHash pitokens;
#define CS_TOKEN_ITEM_FILE \
  "plugins/video/render3d/shader/shadercompiler/xmlshader/docwrap.tok"
#define CS_TOKEN_LIST_TOKEN_PREFIX PITOKEN_
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE
#undef CS_TOKEN_LIST_TOKEN_PREFIX
  enum
  {
    PITOKEN_TEMPLATE_NEW = 0xfeeb1e,
    PITOKEN_ENDTEMPLATE_NEW,
    PITOKEN_INCLUDE_NEW,
    PITOKEN_GENERATE,
    PITOKEN_ENDGENERATE
  };

  csString* currentOut;
  void DumpCondition (size_t id, const char* condStr, size_t condLen);
public:
  csWrappedDocumentNodeFactory (csXMLShaderCompiler* plugin);

  csWrappedDocumentNode* CreateWrapper (iDocumentNode* wrappedNode,
    iConditionResolver* resolver, csConditionEvaluator& evaluator, 
    csString* dumpOut);
  csWrappedDocumentNode* CreateWrapperStatic (iDocumentNode* wrappedNode,
    iConditionResolver* resolver, csString* dumpOut);
};

}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# define new CS_EXTENSIVE_MEMDEBUG_NEW
#endif

#endif // __CS_DOCWRAP_H__
