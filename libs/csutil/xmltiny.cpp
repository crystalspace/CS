/*
    Copyright (C) 2002 by Jorrit Tyberghein

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "cssysdef.h"
#include "csutil/util.h"
#include "csutil/xmltiny.h"
#include "csutil/xmltinyp.h"
#include "csutil/tinyxml.h"
#include "iutil/vfs.h"
#include "iutil/string.h"
#include "iutil/databuff.h"

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csTinyXmlSystem)
  SCF_IMPLEMENTS_INTERFACE (iXmlSystem)
SCF_IMPLEMENT_IBASE_END

csTinyXmlSystem::csTinyXmlSystem ()
{
  SCF_CONSTRUCT_IBASE (NULL);
}

csTinyXmlSystem::~csTinyXmlSystem ()
{
}

csRef<iXmlDocument> csTinyXmlSystem::CreateDocument ()
{
  csRef<iXmlDocument> doc;
  doc.Take (new csTinyXmlDocument ());
  return doc;
}

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csTinyXmlAttributeIterator)
  SCF_IMPLEMENTS_INTERFACE (iXmlAttributeIterator)
SCF_IMPLEMENT_IBASE_END

csTinyXmlAttributeIterator::csTinyXmlAttributeIterator (TiXmlNode* parent)
{
  SCF_CONSTRUCT_IBASE (NULL);
  csTinyXmlAttributeIterator::parent = parent->ToElement ();
  if (csTinyXmlAttributeIterator::parent == NULL)
  {
    current = NULL;
    return;
  }
  current = csTinyXmlAttributeIterator::parent->FirstAttribute ();
}

bool csTinyXmlAttributeIterator::HasNext ()
{
  return current != NULL;
}

csRef<iXmlAttribute> csTinyXmlAttributeIterator::Next ()
{
  csRef<iXmlAttribute> attr;
  if (current != NULL)
  {
    attr.Take (new csTinyXmlAttribute (current));
    current = current->Next ();
  }
  return attr;
}

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csTinyXmlNodeIterator)
  SCF_IMPLEMENTS_INTERFACE (iXmlNodeIterator)
SCF_IMPLEMENT_IBASE_END

csTinyXmlNodeIterator::csTinyXmlNodeIterator (TiXmlNode* parent,
	const char* value)
{
  SCF_CONSTRUCT_IBASE (NULL);
  csTinyXmlNodeIterator::parent = parent;
  csTinyXmlNodeIterator::value = value ? csStrNew (value) : NULL;
  if (value)
    current = parent->FirstChild (value);
  else
    current = parent->FirstChild ();
}

bool csTinyXmlNodeIterator::HasNext ()
{
  return current != NULL;
}

csRef<iXmlNode> csTinyXmlNodeIterator::Next ()
{
  csRef<iXmlNode> node;
  if (current != NULL)
  {
    node.Take (new csTinyXmlNode (current));
    if (value)
      current = current->NextSibling (value);
    else
      current = current->NextSibling ();
  }
  return node;
}

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csTinyXmlAttribute)
  SCF_IMPLEMENTS_INTERFACE (iXmlAttribute)
SCF_IMPLEMENT_IBASE_END

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csTinyXmlNode)
  SCF_IMPLEMENTS_INTERFACE (iXmlNode)
SCF_IMPLEMENT_IBASE_END

csTinyXmlNode::csTinyXmlNode ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  node = NULL;
}

csTinyXmlNode::csTinyXmlNode (TiXmlNode* node)
{
  SCF_CONSTRUCT_IBASE (NULL);
  csTinyXmlNode::node = node;
}

csTinyXmlNode::~csTinyXmlNode ()
{
}

csRef<iXmlNode> csTinyXmlNode::GetParent ()
{
  csRef<iXmlNode> child;
  if (!node->Parent ()) return child;
  child.Take (new csTinyXmlNode (node->Parent ()));
  return child;
}

csXmlNodeType csTinyXmlNode::GetType ()
{
  switch (node->Type ())
  {
    case TiXmlNode::DOCUMENT: return CS_XMLNODE_DOCUMENT;
    case TiXmlNode::ELEMENT: return CS_XMLNODE_ELEMENT;
    case TiXmlNode::COMMENT: return CS_XMLNODE_COMMENT;
    case TiXmlNode::TEXT: return CS_XMLNODE_TEXT;
    case TiXmlNode::DECLARATION: return CS_XMLNODE_DECLARATION;
    default: return CS_XMLNODE_UNKNOWN;
  }
}

const char* csTinyXmlNode::GetValue ()
{
  return node->Value ();
}

void csTinyXmlNode::SetValue (const char* value)
{
  node->SetValue (value);
}

csRef<iXmlNodeIterator> csTinyXmlNode::GetNodes ()
{
  csRef<iXmlNodeIterator> it;
  it.Take (new csTinyXmlNodeIterator (node, NULL));
  return it;
}

csRef<iXmlNodeIterator> csTinyXmlNode::GetNodes (const char* type)
{
  csRef<iXmlNodeIterator> it;
  it.Take (new csTinyXmlNodeIterator (node, type));
  return it;
}

csRef<iXmlNode> csTinyXmlNode::GetNode (const char* type)
{
  csRef<iXmlNode> child;
  TiXmlNode* c = node->FirstChild (type);
  if (!c) return child;
  child.Take (new csTinyXmlNode (c));
  return child;
}

void csTinyXmlNode::RemoveNode (const csRef<iXmlNode>& child)
{
  CS_ASSERT (child.IsValid ());
  node->RemoveChild (((csTinyXmlNode*)(iXmlNode*)child)->GetTiNode ());
}

void csTinyXmlNode::RemoveNodes ()
{
  node->Clear ();
}

csRef<iXmlNode> csTinyXmlNode::CreateNode (const char* type)
{
  csRef<iXmlNode> child;
  // @@@ TODO
  return child;
}

csRef<iXmlNode> csTinyXmlNode::CreateNodeBefore (const char* type,
  	const csRef<iXmlNode>& node)
{
  csRef<iXmlNode> child;
  // @@@ TODO
  return child;
}

csRef<iXmlNode> csTinyXmlNode::CreateNodeAfter (const char* type,
  	const csRef<iXmlNode>& node)
{
  csRef<iXmlNode> child;
  // @@@ TODO
  return child;
}

void csTinyXmlNode::MoveNodeBefore (const csRef<iXmlNode>& node,
  	const csRef<iXmlNode>& before)
{
  // @@@ TODO
}

void csTinyXmlNode::MoveNodeAfter (const csRef<iXmlNode>& node,
  	const csRef<iXmlNode>& after)
{
  // @@@ TODO
}

const char* csTinyXmlNode::GetContentsValue ()
{
  TiXmlNode* child = node->FirstChild ();
  while (child)
  {
    if (child->Type () == TiXmlNode::TEXT)
    {
      return child->Value ();
    }
    child = child->NextSibling ();
  } 
  return NULL;
}

int csTinyXmlNode::GetContentsValueAsInt ()
{
  const char* v = GetContentsValue ();
  if (!v) return 0;
  int val;
  sscanf (v, "%d", &val);
  return val;
}

float csTinyXmlNode::GetContentsValueAsFloat ()
{
  const char* v = GetContentsValue ();
  if (!v) return 0;
  float val;
  sscanf (v, "%f", &val);
  return val;
}

csRef<iXmlAttributeIterator> csTinyXmlNode::GetAttributes ()
{
  csRef<iXmlAttributeIterator> it;
  it.Take (new csTinyXmlAttributeIterator (node));
  return it;
}

csRef<iXmlAttribute> csTinyXmlNode::GetAttribute (const char* name)
{
  csRef<iXmlAttributeIterator> it = GetAttributes ();
  while (it->HasNext ())
  {
    csRef<iXmlAttribute> attr = it->Next ();
    if (strcmp (name, attr->GetName ()) == 0)
      return attr;
  }
  csRef<iXmlAttribute> attr;
  return attr;
}

const char* csTinyXmlNode::GetAttributeValue (const char* name)
{
  TiXmlElement* el = node->ToElement ();
  if (el) return el->Attribute (name);
  else return NULL;
}

int csTinyXmlNode::GetAttributeValueAsInt (const char* name)
{
  csRef<iXmlAttribute> attr = GetAttribute (name);
  if (!attr) return 0;
  return attr->GetValueAsInt ();
}

float csTinyXmlNode::GetAttributeValueAsFloat (const char* name)
{
  csRef<iXmlAttribute> attr = GetAttribute (name);
  if (!attr) return 0;
  return attr->GetValueAsFloat ();
}

void csTinyXmlNode::RemoveAttribute (const csRef<iXmlAttribute>& attr)
{
  // @@@ TODO
}

void csTinyXmlNode::RemoveAttributes ()
{
  // @@@ TODO
}

void csTinyXmlNode::SetAttribute (const char* name, const char* value)
{
  // @@@ TODO
}

csRef<iXmlAttribute> csTinyXmlNode::CreateAttribute ()
{
  csRef<iXmlAttribute> attr;
  // @@@ TODO
  return attr;
}

csRef<iXmlAttribute> csTinyXmlNode::CreateAttributeBefore (
  	const csRef<iXmlAttribute>& before)
{
  csRef<iXmlAttribute> attr;
  // @@@ TODO
  return attr;
}

csRef<iXmlAttribute> csTinyXmlNode::CreateAttributeAfter (
  	const csRef<iXmlAttribute>& after)
{
  csRef<iXmlAttribute> attr;
  // @@@ TODO
  return attr;
}

void csTinyXmlNode::MoveAttributeBefore (const csRef<iXmlAttribute>& attr,
  	const csRef<iXmlAttribute>& before)
{
  // @@@ TODO
}

void csTinyXmlNode::MoveAttributeAfter (const csRef<iXmlAttribute>& attr,
  	const csRef<iXmlAttribute>& after)
{
  // @@@ TODO
}

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csTinyXmlDocument)
  SCF_IMPLEMENTS_INTERFACE (iXmlDocument)
SCF_IMPLEMENT_IBASE_END

csTinyXmlDocument::csTinyXmlDocument ()
{
  SCF_CONSTRUCT_IBASE (NULL);
}

csTinyXmlDocument::~csTinyXmlDocument ()
{
}

void csTinyXmlDocument::Clear ()
{
  root = NULL;
}

csRef<iXmlNode> csTinyXmlDocument::CreateRoot ()
{
  Clear ();
  TiXmlDocument* doc = new TiXmlDocument ();
  root.Take (new csTinyXmlNode (doc));
  return root;
}

csRef<iXmlNode> csTinyXmlDocument::GetRoot ()
{
  return root;
}

const char* csTinyXmlDocument::ParseXML (iFile* file)
{
  return "Not implemented yet!";
}

const char* csTinyXmlDocument::ParseXML (iDataBuffer* buf)
{
  return ParseXML ((const char*)buf->GetData ());
}

const char* csTinyXmlDocument::ParseXML (iString* str)
{
  return ParseXML ((const char*)*str);
}

const char* csTinyXmlDocument::ParseXML (const char* buf)
{
  CreateRoot ();
  TiXmlDocument* doc = (TiXmlDocument*)(((csTinyXmlNode*)(iXmlNode*)root)
  	->GetTiNode ());
  doc->Parse (buf);
  if (doc->Error ())
    return doc->ErrorDesc ();
  return NULL;
}

const char* csTinyXmlDocument::WriteXML (iFile* file)
{
  return "Not implemented yet!";
}

const char* csTinyXmlDocument::WriteXML (iString& str)
{
  return "Not implemented yet!";
}

//------------------------------------------------------------------------

