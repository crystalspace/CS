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

#include "cssysdef.h"

#include "docnodes.h"

CS_PLUGIN_NAMESPACE_BEGIN(ShaderWeaver)
{
  DocumentNodePI::DocumentNodePI (iDocumentNode* parent, 
                                  const char* contents) :
    parent (parent)
  {
    value.Format ("?%s?", contents);
  }

  //-------------------------------------------------------------------------

  DocumentNodeContainer::DocumentNodeContainer (iDocumentNode* parent) :
    parent (parent)
  {
  }

  void DocumentNodeContainer::AddAttributesOf (iDocumentNode* node)
  {
    csRef<iDocumentAttributeIterator> attrs = node->GetAttributes ();
    while (attrs->HasNext())
    {
      csRef<iDocumentAttribute> attr = attrs->Next ();
      attributes.Put (attr->GetName(), attr);
    }
  }

  void DocumentNodeContainer::AddChildrenOf (iDocumentNode* node)
  {
    csRef<iDocumentNodeIterator> children = node->GetNodes();
    while (children->HasNext())
    {
      csRef<iDocumentNode> child = children->Next();
      nodes.Push (child);
    }
  }

  void DocumentNodeContainer::AddChild (iDocumentNode* child)
  {
    nodes.Push (child);
  }

  csRef<iDocumentNodeIterator> DocumentNodeContainer::GetNodes ()
  {
    return csPtr<iDocumentNodeIterator> (new NodeIterator (nodes));
  }

  csRef<iDocumentNode> DocumentNodeContainer::GetNode (const char* value)
  {
    for (size_t n = 0; n < nodes.GetSize(); n++)
    {
      const char* val = nodes[n]->GetValue ();
      if ((val != 0) && (strcmp (val, value) == 0)) return nodes[n];
    }
    return 0;
  }

  csRef<iDocumentAttributeIterator> DocumentNodeContainer::GetAttributes ()
  {
    return csPtr<iDocumentAttributeIterator> (new AttributeIterator (
      attributes));
  }

  csRef<iDocumentAttribute> DocumentNodeContainer::GetAttribute (
    const char* attr)
  {
    return attributes.Get (attr, (iDocumentAttribute*)0);
  }
}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)
