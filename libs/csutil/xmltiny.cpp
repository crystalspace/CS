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
#include "iutil/vfs.h"
#include "iutil/string.h"
#include "iutil/databuff.h"
#include "csutil/tinyxml.h"

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csTinyDocumentSystem)
  SCF_IMPLEMENTS_INTERFACE (iDocumentSystem)
SCF_IMPLEMENT_IBASE_END

csTinyDocumentSystem::csTinyDocumentSystem ()
{
  SCF_CONSTRUCT_IBASE (NULL);
}

csTinyDocumentSystem::~csTinyDocumentSystem ()
{
}

csRef<iDocument> csTinyDocumentSystem::CreateDocument ()
{
  csRef<iDocument> doc;
  doc.Take (new csTinyXmlDocument ());
  return doc;
}

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csTinyXmlAttributeIterator)
  SCF_IMPLEMENTS_INTERFACE (iDocumentAttributeIterator)
SCF_IMPLEMENT_IBASE_END

csTinyXmlAttributeIterator::csTinyXmlAttributeIterator (TiDocumentNode* parent)
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

csRef<iDocumentAttribute> csTinyXmlAttributeIterator::Next ()
{
  csRef<iDocumentAttribute> attr;
  if (current != NULL)
  {
    attr.Take (new csTinyXmlAttribute (current));
    current = current->Next ();
  }
  return attr;
}

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csTinyXmlNodeIterator)
  SCF_IMPLEMENTS_INTERFACE (iDocumentNodeIterator)
SCF_IMPLEMENT_IBASE_END

csTinyXmlNodeIterator::csTinyXmlNodeIterator (TiDocumentNode* parent,
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

csRef<iDocumentNode> csTinyXmlNodeIterator::Next ()
{
  csRef<iDocumentNode> node;
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
  SCF_IMPLEMENTS_INTERFACE (iDocumentAttribute)
SCF_IMPLEMENT_IBASE_END

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csTinyXmlNode)
  SCF_IMPLEMENTS_INTERFACE (iDocumentNode)
SCF_IMPLEMENT_IBASE_END

csTinyXmlNode::csTinyXmlNode ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  node = NULL;
}

csTinyXmlNode::csTinyXmlNode (TiDocumentNode* node)
{
  SCF_CONSTRUCT_IBASE (NULL);
  csTinyXmlNode::node = node;
}

csTinyXmlNode::~csTinyXmlNode ()
{
}

csRef<iDocumentNode> csTinyXmlNode::GetParent ()
{
  csRef<iDocumentNode> child;
  if (!node->Parent ()) return child;
  child.Take (new csTinyXmlNode (node->Parent ()));
  return child;
}

csDocumentNodeType csTinyXmlNode::GetType ()
{
  switch (node->Type ())
  {
    case TiDocumentNode::DOCUMENT: return CS_NODE_DOCUMENT;
    case TiDocumentNode::ELEMENT: return CS_NODE_ELEMENT;
    case TiDocumentNode::COMMENT: return CS_NODE_COMMENT;
    case TiDocumentNode::TEXT: return CS_NODE_TEXT;
    case TiDocumentNode::DECLARATION: return CS_NODE_DECLARATION;
    default: return CS_NODE_UNKNOWN;
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

csRef<iDocumentNodeIterator> csTinyXmlNode::GetNodes ()
{
  csRef<iDocumentNodeIterator> it;
  it.Take (new csTinyXmlNodeIterator (node, NULL));
  return it;
}

csRef<iDocumentNodeIterator> csTinyXmlNode::GetNodes (const char* type)
{
  csRef<iDocumentNodeIterator> it;
  it.Take (new csTinyXmlNodeIterator (node, type));
  return it;
}

csRef<iDocumentNode> csTinyXmlNode::GetNode (const char* type)
{
  csRef<iDocumentNode> child;
  TiDocumentNode* c = node->FirstChild (type);
  if (!c) return child;
  child.Take (new csTinyXmlNode (c));
  return child;
}

void csTinyXmlNode::RemoveNode (const csRef<iDocumentNode>& child)
{
  //CS_ASSERT (child.IsValid ());
  node->RemoveChild (((csTinyXmlNode*)(iDocumentNode*)child)->GetTiNode ());
}

void csTinyXmlNode::RemoveNodes ()
{
  node->Clear ();
}

csRef<iDocumentNode> csTinyXmlNode::CreateNodeBefore (csDocumentNodeType type,
	iDocumentNode* before)
{
  csRef<iDocumentNode> n;
  TiDocumentNode* child = NULL;
  switch (type)
  {
    case CS_NODE_DOCUMENT:
      break;
    case CS_NODE_ELEMENT:
      {
        TiXmlElement el (NULL);
	if (before)
	  child = node->InsertBeforeChild (
	  	((csTinyXmlNode*)(iDocumentNode*)before)->GetTiNode (),
		el);
        else
	  child = node->InsertEndChild (el);
        //CS_ASSERT (child != NULL);
      }
      break;
    case CS_NODE_COMMENT:
      {
        TiXmlComment el;
	if (before)
	  child = node->InsertBeforeChild (
	  	((csTinyXmlNode*)(iDocumentNode*)before)->GetTiNode (),
		el);
        else
	  child = node->InsertEndChild (el);
        //CS_ASSERT (child != NULL);
      }
      break;
    case CS_NODE_TEXT:
      {
        TiXmlText el (NULL);
	if (before)
	  child = node->InsertBeforeChild (
	  	((csTinyXmlNode*)(iDocumentNode*)before)->GetTiNode (),
		el);
        else
	  child = node->InsertEndChild (el);
        //CS_ASSERT (child != NULL);
      }
      break;
    case CS_NODE_DECLARATION:
      {
        TiXmlDeclaration el;
	if (before)
	  child = node->InsertBeforeChild (
	  	((csTinyXmlNode*)(iDocumentNode*)before)->GetTiNode (),
		el);
        else
	  child = node->InsertEndChild (el);
        //CS_ASSERT (child != NULL);
      }
      break;
    case CS_NODE_UNKNOWN:
      {
        TiXmlUnknown el;
	if (before)
	  child = node->InsertBeforeChild (
	  	((csTinyXmlNode*)(iDocumentNode*)before)->GetTiNode (),
		el);
        else
	  child = node->InsertEndChild (el);
        //CS_ASSERT (child != NULL);
      }
      break;
    default:
      break;
  }
  if (child)
    n.Take (new csTinyXmlNode (child));
  return n;
}

const char* csTinyXmlNode::GetContentsValue ()
{
  TiDocumentNode* child = node->FirstChild ();
  while (child)
  {
    if (child->Type () == TiDocumentNode::TEXT)
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

csRef<iDocumentAttributeIterator> csTinyXmlNode::GetAttributes ()
{
  csRef<iDocumentAttributeIterator> it;
  it.Take (new csTinyXmlAttributeIterator (node));
  return it;
}

csRef<iDocumentAttribute> csTinyXmlNode::GetAttribute (const char* name)
{
  csRef<iDocumentAttributeIterator> it = GetAttributes ();
  while (it->HasNext ())
  {
    csRef<iDocumentAttribute> attr = it->Next ();
    if (strcmp (name, attr->GetName ()) == 0)
      return attr;
  }
  csRef<iDocumentAttribute> attr;
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
  csRef<iDocumentAttribute> attr = GetAttribute (name);
  if (!attr) return 0;
  return attr->GetValueAsInt ();
}

float csTinyXmlNode::GetAttributeValueAsFloat (const char* name)
{
  csRef<iDocumentAttribute> attr = GetAttribute (name);
  if (!attr) return 0;
  return attr->GetValueAsFloat ();
}

void csTinyXmlNode::RemoveAttribute (const csRef<iDocumentAttribute>& attr)
{
  // @@@ TODO
}

void csTinyXmlNode::RemoveAttributes ()
{
  // @@@ TODO
}

void csTinyXmlNode::SetAttribute (const char* name, const char* value)
{
  TiXmlElement* el = node->ToElement ();
  if (el) el->SetAttribute (name, value);
}

void csTinyXmlNode::SetAttributeAsInt (const char* name, int value)
{
  TiXmlElement* el = node->ToElement ();
  if (el) el->SetAttribute (name, value);
}

void csTinyXmlNode::SetAttributeAsFloat (const char* name, float value)
{
  TiXmlElement* el = node->ToElement ();
  if (el)
  {
    char v[64];
    sprintf (v, "%g", value);
    el->SetAttribute (name, v);
  }
}

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csTinyXmlDocument)
  SCF_IMPLEMENTS_INTERFACE (iDocument)
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

csRef<iDocumentNode> csTinyXmlDocument::CreateRoot ()
{
  Clear ();
  TiDocument* doc = new TiDocument ();
  root.Take (new csTinyXmlNode (doc));
  return root;
}

csRef<iDocumentNode> csTinyXmlDocument::GetRoot ()
{
  return root;
}

const char* csTinyXmlDocument::Parse (iFile* file)
{
  return "Not implemented yet!";
}

const char* csTinyXmlDocument::Parse (iDataBuffer* buf)
{
  return Parse ((const char*)buf->GetData ());
}

const char* csTinyXmlDocument::Parse (iString* str)
{
  return Parse ((const char*)*str);
}

const char* csTinyXmlDocument::Parse (const char* buf)
{
  CreateRoot ();
  TiDocument* doc = (TiDocument*)(((csTinyXmlNode*)(iDocumentNode*)root)
  	->GetTiNode ());
  doc->Parse (buf);
  if (doc->Error ())
    return doc->ErrorDesc ();
  return NULL;
}

const char* csTinyXmlDocument::Write (iFile* file)
{
  scfString str;
  const char* error = Write (&str);
  if (error) return error;
  if (!file->Write (str.GetData (), str.Length ()))
    return "Error writing file!";
  return NULL;
}

const char* csTinyXmlDocument::Write (iString* str)
{
  TiDocument* doc = (TiDocument*)(((csTinyXmlNode*)(iDocumentNode*)root)
  	->GetTiNode ());
  doc->Print (str, 0);
  return NULL;
}

const char* csTinyXmlDocument::Write (iVFS* vfs, const char* filename)
{
  scfString str;
  const char* error = Write (&str);
  if (error) return error;
  if (!vfs->WriteFile (filename, str.GetData (), str.Length ()))
    return "Error writing file!";
  return NULL;
}

//------------------------------------------------------------------------

