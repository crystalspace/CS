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
  delete node;
}

const char* csTinyXmlNode::GetType ()
{
  return node->Value ();
}

csRef<iXmlNode> csTinyXmlNode::GetParent ()
{
  csRef<iXmlNode> child;
  child.Take (new csTinyXmlNode (node->Parent ()));
  return child;
}

void csTinyXmlNode::SetType (const char* type)
{
  node->SetValue (type);
}

csRef<iXmlNodeIterator> csTinyXmlNode::GetChildren ()
{
  csRef<iXmlNodeIterator> it;
  // @@@ TODO
  return it;
}

csRef<iXmlNodeIterator> csTinyXmlNode::GetChildren (const char* type)
{
  csRef<iXmlNodeIterator> it;
  // @@@ TODO
  return it;
}

void csTinyXmlNode::RemoveChild (const csRef<iXmlNode>& child)
{
  CS_ASSERT (child.IsValid ());
  node->RemoveChild (((csTinyXmlNode*)(iXmlNode*)child)->GetTiNode ());
}

void csTinyXmlNode::RemoveChildren ()
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

csRef<iXmlAttributeIterator> csTinyXmlNode::GetAttributes ()
{
  csRef<iXmlAttributeIterator> it;
  // @@@ TODO
  return it;
}

csRef<iXmlAttributeIterator> csTinyXmlNode::GetAttributes (const char* name)
{
  csRef<iXmlAttributeIterator> it;
  // @@@ TODO
  return it;
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
  return "Not implemented yet!";
}

const char* csTinyXmlDocument::ParseXML (iString* str)
{
  return "Not implemented yet!";
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

