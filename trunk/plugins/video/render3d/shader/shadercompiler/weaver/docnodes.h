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

#ifndef __CS_DOCNODES_H__
#define __CS_DOCNODES_H__

#include "csutil/csstring.h"
#include "csutil/documentcommon.h"
#include "csutil/hash.h"
#include "csutil/refarr.h"
#include "csutil/weakref.h"

CS_PLUGIN_NAMESPACE_BEGIN(ShaderWeaver)
{
  class DocumentNodePI : public csDocumentNodeReadOnly
  {
    csWeakRef<iDocumentNode> parent;
    csString value;
  public:
    DocumentNodePI (iDocumentNode* parent, const char* contents);

    csDocumentNodeType GetType () { return CS_NODE_UNKNOWN; }
    bool Equals (iDocumentNode* other) 
    { return static_cast<iDocumentNode*> (this) == other; }
    const char* GetValue () { return value; }
    csRef<iDocumentNode> GetParent () 
    { return csRef<iDocumentNode> (parent); }
  };
  
  class DocumentNodeContainer : public csDocumentNodeReadOnly
  {
    csWeakRef<iDocumentNode> parent;
    csString value;
    typedef csHash<csRef<iDocumentAttribute>, csString> AttrHash;
    AttrHash attributes;
    typedef csRefArray<iDocumentNode> NodeArray;
    NodeArray nodes;
  public:
    DocumentNodeContainer (iDocumentNode* parent);

    void AddAttributesOf (iDocumentNode* node);
    void AddChildrenOf (iDocumentNode* node);
    void AddChild (iDocumentNode* child);

    csRef<iDocumentNodeIterator> GetNodes ();
    csRef<iDocumentNode> GetNode (const char* value);

    csRef<iDocumentAttributeIterator> GetAttributes ();
    csRef<iDocumentAttribute> GetAttribute (const char*);

    csDocumentNodeType GetType () { return CS_NODE_ELEMENT; }
    bool Equals (iDocumentNode* other) 
    { return static_cast<iDocumentNode*> (this) == other; }

    const char* GetValue () { return value; }
    void SetValue (const char* value) { this->value = value; }
    csRef<iDocumentNode> GetParent ()
    { return csRef<iDocumentNode> (parent); }
  private:
    class AttributeIterator : 
      public scfImplementation1<AttributeIterator, iDocumentAttributeIterator>
    {
      AttrHash::ConstGlobalIterator attrIter;
    public:
      AttributeIterator (const AttrHash& attributes) : 
          scfImplementationType (this), 
          attrIter (attributes.GetIterator()) {}
      virtual ~AttributeIterator () {}

      bool HasNext () { return attrIter.HasNext(); }
      csRef<iDocumentAttribute> Next () { return attrIter.Next(); }
    };
    class NodeIterator :
      public scfImplementation1<NodeIterator, iDocumentNodeIterator>
    {
      const NodeArray& nodes;
      size_t pos;
    public:
      NodeIterator (const NodeArray& nodes) : 
        scfImplementationType (this), nodes (nodes), pos (0) {}
      virtual ~NodeIterator () {}

      bool HasNext () { return pos < nodes.GetSize(); }
      csRef<iDocumentNode> Next () { return nodes[pos++]; }
      size_t GetNextPosition () { return pos; }
      size_t GetEndPosition () { return nodes.GetSize(); }
    };
  };
}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)

#endif // __CS_DOCNODES_H__
