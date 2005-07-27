/*
  Copyright (C) 2004 by Frank Richter
	    (C) 2004 by Jorrit Tyberghein

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

#ifndef __DOCWRAP_H__
#define __DOCWRAP_H__

#include "csutil/csstring.h"
#include "csutil/strhash.h"
#include "csutil/leakguard.h"
#include "csutil/parray.h"
#include "csutil/pooledscfclass.h"
#include "csutil/ref.h"
#include "csutil/refarr.h"
#include "csutil/scf.h"
#include "csutil/weakref.h"
#include "iutil/document.h"
#include "iutil/objreg.h"

class csWrappedDocumentNodeIterator;
struct WrapperStackEntry;


typedef size_t csConditionID;
const csConditionID csCondAlwaysFalse = (csConditionID)~0;
const csConditionID csCondAlwaysTrue = (csConditionID)~1;

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
};

/**
 * Wrapper around a document node, supporting conditionals.
 */
class csWrappedDocumentNode : public iDocumentNode
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
  };
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

  struct NodeProcessingState;
  void ParseCondition (WrapperStackEntry& newWrapper, const char* cond,
    size_t condLen, iDocumentNode* node);
  void CreateElseWrapper (NodeProcessingState* state, 
    WrapperStackEntry& elseWrapper);
  void ProcessInclude (const csString& filename, NodeProcessingState* state, 
    iDocumentNode* node);
  void ProcessTemplate (iDocumentNode* templNode, 
    NodeProcessingState* state);
  bool InvokeTemplate (const char* name, iDocumentNode* node, 
    NodeProcessingState* state);
  void ValidateTemplateEnd (iDocumentNode* node, 
    NodeProcessingState* state);

  void ProcessSingleWrappedNode (NodeProcessingState* state, iDocumentNode* wrappedNode);
  void ProcessWrappedNode (NodeProcessingState* state, iDocumentNode* wrappedNode);
  void ProcessWrappedNode ();
  void Report (int severity, iDocumentNode* node, const char* msg, ...);
  
  static void AppendNodeText (WrapperWalker& walker, csString& text);

  typedef csRefArray<iDocumentNode> Template;
  struct GlobalProcessingState : public csRefCount
  {
    csHash<Template, csString> templates;
  };
  csRef<GlobalProcessingState> globalState;

  csWrappedDocumentNode (iDocumentNode* wrappedNode,
    csWrappedDocumentNode* parent,
    csWrappedDocumentNodeFactory* shared, 
    GlobalProcessingState* globalState);
  csWrappedDocumentNode (csWrappedDocumentNodeFactory* shared,
    iDocumentNode* wrappedNode,
    iConditionResolver* resolver);
public:
  CS_LEAKGUARD_DECLARE(csWrappedDocumentNode);
  SCF_DECLARE_IBASE;

  virtual ~csWrappedDocumentNode ();

  virtual csDocumentNodeType GetType ()
  { return wrappedNode->GetType(); }
  virtual bool Equals (iDocumentNode* other);
  virtual const char* GetValue ();
  virtual void SetValue (const char* value) {}
  virtual void SetValueAsInt (int value) {}
  virtual void SetValueAsFloat (float value) {}

  virtual csRef<iDocumentNode> GetParent ()
  { return (iDocumentNode*)parent; }
  virtual csRef<iDocumentNodeIterator> GetNodes ();
  virtual csRef<iDocumentNodeIterator> GetNodes (const char* value);
  virtual csRef<iDocumentNode> GetNode (const char* value);

  virtual void RemoveNode (const csRef<iDocumentNode>& child) {}
  virtual void RemoveNodes () {}

  virtual csRef<iDocumentNode> CreateNodeBefore (csDocumentNodeType type,
  	iDocumentNode* before)
  { return 0; }

  virtual const char* GetContentsValue ();
  virtual int GetContentsValueAsInt ();
  virtual float GetContentsValueAsFloat ();

  virtual csRef<iDocumentAttributeIterator> GetAttributes ();
  virtual csRef<iDocumentAttribute> GetAttribute (const char* name);
  virtual const char* GetAttributeValue (const char* name);
  virtual int GetAttributeValueAsInt (const char* name);
  virtual float GetAttributeValueAsFloat (const char* name);
  virtual bool GetAttributeValueAsBool (const char* name, 
    bool defaultvalue = false);

  virtual void RemoveAttribute (const csRef<iDocumentAttribute>& attr) {}
  virtual void RemoveAttributes () {}

  virtual void SetAttribute (const char* name, const char* value) {}
  virtual void SetAttributeAsInt (const char* name, int value) {}
  virtual void SetAttributeAsFloat (const char* name, float value) {}
};

class csEmptyDocumentNodeIterator : public iDocumentNodeIterator
{
public:
  SCF_DECLARE_IBASE;
  CS_LEAKGUARD_DECLARE(csEmptyDocumentNodeIterator);

  csEmptyDocumentNodeIterator ();
  virtual ~csEmptyDocumentNodeIterator ();

  virtual bool HasNext () { return false; }
  virtual csRef<iDocumentNode> Next () { return 0; }
};

class csEmptyDocumentAttributeIterator : public iDocumentAttributeIterator
{
public:
  SCF_DECLARE_IBASE;
  CS_LEAKGUARD_DECLARE(csEmptyDocumentAttributeIterator);

  csEmptyDocumentAttributeIterator ();
  virtual ~csEmptyDocumentAttributeIterator ();

  virtual bool HasNext () { return false; }
  virtual csRef<iDocumentAttribute> Next () { return 0; }
};

class csTextNodeWrapper : public iDocumentNode
{
  char* nodeText;
  csRef<iDocumentNode> realMe;
public:
  SCF_DECLARE_IBASE_POOLED(csTextNodeWrapper);

  csTextNodeWrapper (Pool* pool);
  virtual ~csTextNodeWrapper ();
  void SetData (iDocumentNode* realMe, const char* text);

  virtual csDocumentNodeType GetType ()
  { return CS_NODE_TEXT; }
  virtual bool Equals (iDocumentNode* other)
  { return realMe->Equals (other); }

  virtual const char* GetValue ()
  { return nodeText; }
  virtual void SetValue (const char* value) {}
  virtual void SetValueAsInt (int value) {}
  virtual void SetValueAsFloat (float value) {}

  virtual csRef<iDocumentNode> GetParent ()
  { return realMe->GetParent (); }

  virtual csRef<iDocumentNodeIterator> GetNodes ()
  { return csPtr<iDocumentNodeIterator> (new csEmptyDocumentNodeIterator); }
  virtual csRef<iDocumentNodeIterator> GetNodes (const char* value)
  { return csPtr<iDocumentNodeIterator> (new csEmptyDocumentNodeIterator); }
  virtual csRef<iDocumentNode> GetNode (const char* value)
  { return 0; }

  virtual void RemoveNode (const csRef<iDocumentNode>& child) {}
  virtual void RemoveNodes () {}

  virtual csRef<iDocumentNode> CreateNodeBefore (csDocumentNodeType type,
  	iDocumentNode* before = 0)
  { return 0; }

  virtual const char* GetContentsValue ()
  { return 0; }
  virtual int GetContentsValueAsInt ()
  { return 0; }
  virtual float GetContentsValueAsFloat ()
  { return 0.0f; }

  virtual csRef<iDocumentAttributeIterator> GetAttributes ()
  { 
    return csPtr<iDocumentAttributeIterator> 
      (new csEmptyDocumentAttributeIterator); 
  }

  virtual csRef<iDocumentAttribute> GetAttribute (const char* name)
  { return 0; }
  virtual const char* GetAttributeValue (const char* name)
  { return 0; }
  virtual int GetAttributeValueAsInt (const char* name)
  { return 0; }
  virtual float GetAttributeValueAsFloat (const char* name)
  { return 0.0f; }
  virtual bool GetAttributeValueAsBool (const char* name,
  	bool defaultvalue=false)
  { return defaultvalue; }

  virtual void RemoveAttribute (const csRef<iDocumentAttribute>& attr) {}
  virtual void RemoveAttributes () {}

  virtual void SetAttribute (const char* name, const char* value) {}
  virtual void SetAttributeAsInt (const char* name, int value) {}
  virtual void SetAttributeAsFloat (const char* name, float value) {}
};

class csWrappedDocumentNodeIterator : public iDocumentNodeIterator
{
  char* filter;

  csWrappedDocumentNode* parentNode;
  csWrappedDocumentNode::WrapperWalker walker;
  csRef<iDocumentNode> next;
  void SeekNext();
public:
  CS_LEAKGUARD_DECLARE(csWrappedDocumentNodeIterator);
  SCF_DECLARE_IBASE_POOLED(csWrappedDocumentNodeIterator);

  csWrappedDocumentNodeIterator (Pool* pool);
  virtual ~csWrappedDocumentNodeIterator ();

  void SetData (csWrappedDocumentNode* node, const char* filter);

  virtual bool HasNext ();
  virtual csRef<iDocumentNode> Next ();
};

class csWrappedDocumentNodeFactory
{
  friend class csWrappedDocumentNode;
  friend class csWrappedDocumentNodeIterator;

  iObjectRegistry* objreg;
  csTextNodeWrapper::Pool textNodePool;
  csWrappedDocumentNodeIterator::Pool iterPool;

  csStringHash pitokens;
#define CS_TOKEN_ITEM_FILE \
  "plugins/video/render3d/shader/shadercompiler/xmlshader/docwrap.tok"
#define CS_TOKEN_LIST_TOKEN_PREFIX PITOKEN_
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE
#undef CS_TOKEN_LIST_TOKEN_PREFIX

  csString* currentOut;
  void DumpCondition (size_t id, const char* condStr, size_t condLen);
public:
  csWrappedDocumentNodeFactory (iObjectRegistry* objreg);

  csWrappedDocumentNode* CreateWrapper (iDocumentNode* wrappedNode,
    iConditionResolver* resolver, csString* dumpOut);
};

#endif // __DOCWRAP_H__
