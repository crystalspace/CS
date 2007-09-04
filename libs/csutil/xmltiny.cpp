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

#include "cssysdef.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "csutil/util.h"
#include "csutil/xmltiny.h"
#include "csutil/scfstr.h"
#include "xmltinyp.h"
#include "iutil/vfs.h"
#include "iutil/string.h"
#include "iutil/databuff.h"
#include "tinyxml.h"

//------------------------------------------------------------------------


csTinyDocumentSystem::csTinyDocumentSystem (iBase* parent)
  : scfImplementationType (this, parent)
{
}

csTinyDocumentSystem::~csTinyDocumentSystem ()
{
}

csRef<iDocument> csTinyDocumentSystem::CreateDocument ()
{
  csRef<iDocument> doc (csPtr<iDocument> (new csTinyXmlDocument (this)));
  return doc;
}

//------------------------------------------------------------------------


csTinyXmlAttributeIterator::csTinyXmlAttributeIterator (CS::TiDocumentNode* parent)
  : scfImplementationType (this)
{
  csTinyXmlAttributeIterator::parent = parent->ToElement ();
  if (csTinyXmlAttributeIterator::parent == 0)
  {
    current = (size_t)-1;
    return;
  }
  count = csTinyXmlAttributeIterator::parent->GetAttributeCount ();
  if (!count) 
  {
    current = (size_t)-1;
    return;
  }
  current = 0;
}

csTinyXmlAttributeIterator::~csTinyXmlAttributeIterator()
{
}

bool csTinyXmlAttributeIterator::HasNext ()
{
  return current != (size_t)-1;
}

csRef<iDocumentAttribute> csTinyXmlAttributeIterator::Next ()
{
  csRef<iDocumentAttribute> attr;
  if (current != (size_t)-1)
  {
    attr = csPtr<iDocumentAttribute> (new csTinyXmlAttribute (
    	&parent->GetAttribute (current)));
    current++;
    if (current >= count)
      current = (size_t)-1;
  }
  return attr;
}

//------------------------------------------------------------------------


csTinyXmlNodeIterator::csTinyXmlNodeIterator (
	csTinyXmlDocument* doc, csTinyXmlNode* parent,
	const char* value)
  : scfImplementationType (this), doc (doc), parent (parent),
  currentPos (0), endPos ((size_t)~0)
{
  csTinyXmlNodeIterator::value = value ? CS::StrDup (value) : 0;

  CS::TiDocumentNodeChildren* node_children = 0;
  if (parent && 
    ((parent->GetTiNode()->Type() == CS::TiDocumentNode::ELEMENT)
      || (parent->GetTiNode()->Type() == CS::TiDocumentNode::DOCUMENT)))
    node_children = parent->GetTiNodeChildren ();
  if (!node_children)
    current = 0;
  else if (value)
    current = node_children->FirstChild (value);
  else
    current = node_children->FirstChild ();
}

csTinyXmlNodeIterator::~csTinyXmlNodeIterator ()
{
  cs_free (value);
}

bool csTinyXmlNodeIterator::HasNext ()
{
  return current != 0;
}

csRef<iDocumentNode> csTinyXmlNodeIterator::Next ()
{
  csRef<iDocumentNode> node;
  if (current != 0)
  {
    node = csPtr<iDocumentNode> (doc->Alloc (current));
    if (value)
      current = current->NextSibling (value);
    else
      current = current->NextSibling ();
    currentPos++;
  }
  return node;
}

size_t csTinyXmlNodeIterator::GetEndPosition ()
{
  if (endPos == (size_t)~0)
  {
    endPos = currentPos;
    CS::TiDocumentNode* node = current;
    while (node != 0)
    {
      endPos++;
      node = node->NextSibling ();
    }
  }
  return endPos;
}

//------------------------------------------------------------------------

csTinyXmlNode::csTinyXmlNode (csTinyXmlDocument* doc) 
  : scfPooledImplementationType (this), node (0), lastChild (0), doc (doc)
{
}

csTinyXmlNode::~csTinyXmlNode ()
{
}

csRef<iDocumentNode> csTinyXmlNode::GetParent ()
{
  csRef<iDocumentNode> child;
  if (!node->Parent ()) return child;
  child = csPtr<iDocumentNode> (doc->Alloc (node->Parent ()));
  return child;
}

csDocumentNodeType csTinyXmlNode::GetType ()
{
  switch (node->Type ())
  {
    case CS::TiDocumentNode::DOCUMENT: return CS_NODE_DOCUMENT;
    case CS::TiDocumentNode::ELEMENT: return CS_NODE_ELEMENT;
    case CS::TiDocumentNode::COMMENT: return CS_NODE_COMMENT;
    case CS::TiDocumentNode::CDATA:
    case CS::TiDocumentNode::TEXT:
      return CS_NODE_TEXT;
    case CS::TiDocumentNode::DECLARATION: return CS_NODE_DECLARATION;
    default: return CS_NODE_UNKNOWN;
  }
}

bool csTinyXmlNode::Equals (iDocumentNode* other)
{
  csTinyXmlNode* other_node = (csTinyXmlNode*)other;
  return GetTiNode () == other_node->GetTiNode ();
}

const char* csTinyXmlNode::GetValue ()
{
  return node->Value ();
}

void csTinyXmlNode::SetValue (const char* value)
{
  node->SetValue (value);
}

void csTinyXmlNode::SetValueAsInt (int value)
{
  csString buf;
  buf.Format ("%d", value);
  node->SetValue (buf);
}

void csTinyXmlNode::SetValueAsFloat (float value)
{
  csString buf;
  buf.Format ("%g", value);
  node->SetValue (buf);
}

csRef<iDocumentNodeIterator> csTinyXmlNode::GetNodes ()
{
  csRef<iDocumentNodeIterator> it;
  it = csPtr<iDocumentNodeIterator> (new csTinyXmlNodeIterator (
  	doc, this, 0));
  return it;
}

csRef<iDocumentNodeIterator> csTinyXmlNode::GetNodes (const char* value)
{
  csRef<iDocumentNodeIterator> it;
  it = csPtr<iDocumentNodeIterator> (new csTinyXmlNodeIterator (
  	doc, this, value));
  return it;
}

csRef<iDocumentNode> csTinyXmlNode::GetNode (const char* value)
{
  if ((node->Type() != CS::TiDocumentNode::ELEMENT)
    && (node->Type() != CS::TiDocumentNode::DOCUMENT)) return 0;
  CS::TiDocumentNodeChildren* node_children = GetTiNodeChildren ();
  csRef<iDocumentNode> child;
  CS::TiDocumentNode* c = node_children->FirstChild (value);
  if (c) child = csPtr<iDocumentNode> (doc->Alloc (c));
  return child;
}

void csTinyXmlNode::RemoveNode (const csRef<iDocumentNode>& child)
{
  if ((node->Type() != CS::TiDocumentNode::ELEMENT)
    && (node->Type() != CS::TiDocumentNode::DOCUMENT)) return;
  CS::TiDocumentNodeChildren* node_children = GetTiNodeChildren ();

  //CS_ASSERT (child.IsValid ());
  if (node_children)
  {
    csTinyXmlNode* tinyChild = 
      static_cast<csTinyXmlNode*>((iDocumentNode*)child);
    CS::TiDocumentNode* tiNode = tinyChild->GetTiNode ();
    node_children->RemoveChild (tiNode);
    if (tiNode == lastChild) lastChild = 0;
  }
}

void csTinyXmlNode::RemoveNodes (csRef<iDocumentNodeIterator> children)
{
  if ((node->Type() != CS::TiDocumentNode::ELEMENT)
    && (node->Type() != CS::TiDocumentNode::DOCUMENT)) return;
  CS::TiDocumentNodeChildren* node_children = GetTiNodeChildren ();

  while (children->HasNext ())
  {
    csRef<iDocumentNode> n = children->Next ();
    csTinyXmlNode* tiNode = static_cast<csTinyXmlNode*>((iDocumentNode*)n);
    node_children->RemoveChild (tiNode->GetTiNode ());
  }
  lastChild = 0;
}

void csTinyXmlNode::RemoveNodes ()
{
  if ((node->Type() != CS::TiDocumentNode::ELEMENT)
    && (node->Type() != CS::TiDocumentNode::DOCUMENT)) return;
  CS::TiDocumentNodeChildren* node_children = GetTiNodeChildren ();

  node_children->Clear ();
  lastChild = 0;
}

csRef<iDocumentNode> csTinyXmlNode::CreateNodeBefore (csDocumentNodeType type,
	iDocumentNode* before)
{
  if ((node->Type() != CS::TiDocumentNode::ELEMENT)
    && (node->Type() != CS::TiDocumentNode::DOCUMENT)) return 0;
  CS::TiDocumentNodeChildren* node_children = GetTiNodeChildren ();

  csRef<iDocumentNode> n;
  CS::TiDocumentNode* child = 0;
  switch (type)
  {
    case CS_NODE_DOCUMENT:
      break;
    case CS_NODE_ELEMENT:
      {
        CS::TiXmlElement el;
	if (before)
	  child = node_children->InsertBeforeChild (
	  	static_cast<csTinyXmlNode*> (before)->GetTiNode (),
		el);
        else
        {
          if (lastChild == 0) lastChild = node_children->LastChild();
	  lastChild = child = node_children->InsertAfterChild (lastChild, el);
        }
        //CS_ASSERT (child != 0);
      }
      break;
    case CS_NODE_COMMENT:
      {
        CS::TiXmlComment el;
	if (before)
	  child = node_children->InsertBeforeChild (
	  	static_cast<csTinyXmlNode*> (before)->GetTiNode (),
		el);
        else
        {
          if (lastChild == 0) lastChild = node_children->LastChild();
	  lastChild = child = node_children->InsertAfterChild (lastChild, el);
        }
        //CS_ASSERT (child != 0);
      }
      break;
    case CS_NODE_TEXT:
      {
        CS::TiXmlText el;
	if (before)
	  child = node_children->InsertBeforeChild (
	  	static_cast<csTinyXmlNode*> (before)->GetTiNode (),
		el);
        else
        {
          if (lastChild == 0) lastChild = node_children->LastChild();
	  lastChild = child = node_children->InsertAfterChild (lastChild, el);
        }
        //CS_ASSERT (child != 0);
      }
      break;
    case CS_NODE_DECLARATION:
      {
        CS::TiXmlDeclaration el;
	if (before)
	  child = node_children->InsertBeforeChild (
	  	static_cast<csTinyXmlNode*> (before)->GetTiNode (),
		el);
        else
        {
          if (lastChild == 0) lastChild = node_children->LastChild();
	  lastChild = child = node_children->InsertAfterChild (lastChild, el);
        }
        //CS_ASSERT (child != 0);
      }
      break;
    case CS_NODE_UNKNOWN:
      {
        CS::TiXmlUnknown el;
	if (before)
	  child = node_children->InsertBeforeChild (
	  	static_cast<csTinyXmlNode*> (before)->GetTiNode (),
		el);
        else
        {
          if (lastChild == 0) lastChild = node_children->LastChild();
	  lastChild = child = node_children->InsertAfterChild (lastChild, el);
        }
        //CS_ASSERT (child != 0);
      }
      break;
    default:
      break;
  }
  if (child)
    n = csPtr<iDocumentNode> (doc->Alloc (child));
  return n;
}

const char* csTinyXmlNode::GetContentsValue ()
{
  if ((node->Type() != CS::TiDocumentNode::ELEMENT)
    && (node->Type() != CS::TiDocumentNode::DOCUMENT)) return 0;
  CS::TiDocumentNodeChildren* node_children = GetTiNodeChildren ();
  
  CS::TiDocumentNode* child = node_children->FirstChild ();
  while (child)
  {
    if ((child->Type () == CS::TiDocumentNode::TEXT)
      || (child->Type () == CS::TiDocumentNode::CDATA))
    {
      return child->Value ();
    }
    child = child->NextSibling ();
  } 
  return 0;
}

int csTinyXmlNode::GetContentsValueAsInt ()
{
  const char* v = GetContentsValue ();
  if (!v) return 0;
  int val = 0;
  sscanf (v, "%d", &val);
  return val;
}

float csTinyXmlNode::GetContentsValueAsFloat ()
{
  const char* v = GetContentsValue ();
  if (!v) return 0;
  float val = 0.0;
  sscanf (v, "%f", &val);
  return val;
}

csRef<iDocumentAttributeIterator> csTinyXmlNode::GetAttributes ()
{
  csRef<iDocumentAttributeIterator> it;
  it = csPtr<iDocumentAttributeIterator> (
  	new csTinyXmlAttributeIterator (node));
  return it;
}

CS::TiDocumentAttribute* csTinyXmlNode::GetAttributeInternal (const char* name)
{
  CS::TiXmlElement* element = node->ToElement ();
  if (!element) return 0;
  size_t count = element->GetAttributeCount ();
  size_t i;
  for (i = 0 ; i < count ; i++)
  {
    CS::TiDocumentAttribute& attrib = node->ToElement ()->GetAttribute (i);
    if (strcmp (name, attrib.Name ()) == 0)
      return &attrib;
  }

  return 0;
}

csRef<iDocumentAttribute> csTinyXmlNode::GetAttribute (const char* name)
{
  csRef<iDocumentAttribute> attr;
  CS::TiDocumentAttribute* a = GetAttributeInternal (name);
  if (a)
  {
    attr = csPtr<iDocumentAttribute> (new csTinyXmlAttribute (a));
  }
  return attr;
}

const char* csTinyXmlNode::GetAttributeValue (const char* name)
{
  CS::TiXmlElement* el = node->ToElement ();
  if (el) return el->Attribute (name);
  else return 0;
}

int csTinyXmlNode::GetAttributeValueAsInt (const char* name)
{
  CS::TiDocumentAttribute* a = GetAttributeInternal (name);
  if (!a) return 0;
  return a->IntValue ();
}

float csTinyXmlNode::GetAttributeValueAsFloat (const char* name)
{
  CS::TiDocumentAttribute* a = GetAttributeInternal (name);
  if (!a) return 0;
  float f;
  sscanf (a->Value (), "%f", &f);
  return f;
}

bool csTinyXmlNode::GetAttributeValueAsBool(const char* name,
					    bool defaultvalue)
{
  CS::TiDocumentAttribute* a = GetAttributeInternal (name);
  if (!a || !a->Value () ) return defaultvalue;
  if (strcasecmp(a->Value(),"true")==0 ||
      strcasecmp(a->Value(),"yes")==0 ||
      atoi(a->Value())!=0)
  {
    return true;
  }
  else
    return false;
}

void csTinyXmlNode::RemoveAttribute (const csRef<iDocumentAttribute>&)
{
  // @@@ TODO
}

void csTinyXmlNode::RemoveAttributes ()
{
  // @@@ TODO
}

void csTinyXmlNode::SetAttribute (const char* name, const char* value)
{
  CS::TiXmlElement* el = node->ToElement ();
  if (el) el->SetAttribute (el->GetDocument (), name, value);
}

void csTinyXmlNode::SetAttributeAsInt (const char* name, int value)
{
  CS::TiXmlElement* el = node->ToElement ();
  if (el) el->SetAttribute (el->GetDocument (), name, value);
}

void csTinyXmlNode::SetAttributeAsFloat (const char* name, float value)
{
  CS::TiXmlElement* el = node->ToElement ();
  if (el)
  {
    csString v;
    v.Format ("%g", value);
    el->SetAttribute (el->GetDocument (), name, v);
  }
}

//------------------------------------------------------------------------

csTinyXmlDocument::csTinyXmlDocument (csTinyDocumentSystem* sys)
  : scfImplementationType (this), root (0), sys (sys)
{
}

csTinyXmlDocument::~csTinyXmlDocument ()
{
  Clear ();
}

void csTinyXmlDocument::Clear ()
{
  if (!root) return;
  root.Invalidate ();
}

csRef<iDocumentNode> csTinyXmlDocument::CreateRoot ()
{
  Clear ();
  root.AttachNew (new CS::TiDocument ());
  return csPtr<iDocumentNode> (Alloc (root));
}

csRef<iDocumentNode> csTinyXmlDocument::GetRoot ()
{
  return csPtr<iDocumentNode> (Alloc (root));
}

const char* csTinyXmlDocument::Parse (iFile* file, bool collapse)
{
  size_t want_size = file->GetSize ();
  char *data = (char*)cs_malloc (want_size + 1);
  size_t real_size = file->Read (data, want_size);
  if (want_size != real_size)
  {
    cs_free (data);
    return "Unexpected EOF encountered";
  }
  data[real_size] = '\0';
#ifdef CS_DEBUG
  if (strlen (data) != real_size)
  {
    cs_free (data);
    return "File contains one or more null characters";
  }
#endif
  const char *error = Parse (data, collapse);
  cs_free (data);
  return error;
}

const char* csTinyXmlDocument::Parse (iDataBuffer* buf, bool collapse)
{
  return Parse ((const char*)buf->GetData (), collapse);
}

const char* csTinyXmlDocument::Parse (iString* str, bool collapse)
{
  return Parse ((const char*)*str, collapse);
}

const char* csTinyXmlDocument::Parse (const char* buf, bool collapse)
{
  CreateRoot ();
  bool const old_collapse = root->IsWhiteSpaceCondensed();
  root->SetCondenseWhiteSpace(collapse);
  root->Parse (root, buf);
  root->SetCondenseWhiteSpace(old_collapse);
  if (root->Error ())
    return root->ErrorDesc ();
  return 0;
}

const char* csTinyXmlDocument::Write (iFile* file)
{
  scfString str;
  const char* error = Write (&str);
  if (error) return error;
  if (!file->Write (str.GetData (), str.Length ()))
    return "Error writing file!";
  return 0;
}

const char* csTinyXmlDocument::Write (iString* str)
{
  str->SetGrowsBy (0);
  root->Print (str, 0);
  return 0;
}

const char* csTinyXmlDocument::Write (iVFS* vfs, const char* filename)
{
  scfString str;
  const char* error = Write (&str);
  if (error) return error;
  if (!vfs->WriteFile (filename, str.GetData (), str.Length ()))
    return "Error writing file!";
  return 0;
}

int csTinyXmlDocument::Changeable ()
{
  return CS_CHANGEABLE_YES;
}

csTinyXmlNode* csTinyXmlDocument::Alloc ()
{
  return new (pool) csTinyXmlNode (this);
}

csTinyXmlNode* csTinyXmlDocument::Alloc (CS::TiDocumentNode* node)
{
  csTinyXmlNode* n = Alloc ();
  n->SetTiNode (node);
  return n;
}
