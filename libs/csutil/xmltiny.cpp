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

SCF_IMPLEMENT_IBASE (csTinyDocumentSystem)
  SCF_IMPLEMENTS_INTERFACE (iDocumentSystem)
SCF_IMPLEMENT_IBASE_END

csTinyDocumentSystem::csTinyDocumentSystem ()
{
  SCF_CONSTRUCT_IBASE (0);
}

csTinyDocumentSystem::~csTinyDocumentSystem ()
{
  SCF_DESTRUCT_IBASE ();
}

csRef<iDocument> csTinyDocumentSystem::CreateDocument ()
{
  csRef<iDocument> doc (csPtr<iDocument> (new csTinyXmlDocument (this)));
  return doc;
}

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csTinyXmlAttributeIterator)
  SCF_IMPLEMENTS_INTERFACE (iDocumentAttributeIterator)
SCF_IMPLEMENT_IBASE_END

csTinyXmlAttributeIterator::csTinyXmlAttributeIterator (TiDocumentNode* parent)
{
  SCF_CONSTRUCT_IBASE (0);
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
  SCF_DESTRUCT_IBASE ();
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

SCF_IMPLEMENT_IBASE (csTinyXmlNodeIterator)
  SCF_IMPLEMENTS_INTERFACE (iDocumentNodeIterator)
SCF_IMPLEMENT_IBASE_END

csTinyXmlNodeIterator::csTinyXmlNodeIterator (
	csTinyXmlDocument* doc, TiDocumentNodeChildren* parent,
	const char* value)
{
  SCF_CONSTRUCT_IBASE (0);
  csTinyXmlNodeIterator::doc = doc;
  csTinyXmlNodeIterator::parent = parent;
  csTinyXmlNodeIterator::value = value ? csStrNew (value) : 0;
  if (!parent)
    current = 0;
  else if (value)
    current = parent->FirstChild (value);
  else
    current = parent->FirstChild ();
}

csTinyXmlNodeIterator::~csTinyXmlNodeIterator ()
{
  delete[] value;
  SCF_DESTRUCT_IBASE ();
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
  }
  return node;
}

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csTinyXmlAttribute)
  SCF_IMPLEMENTS_INTERFACE (iDocumentAttribute)
SCF_IMPLEMENT_IBASE_END

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_INCREF(csTinyXmlNode)
void csTinyXmlNode::DecRef ()
{
  csRefTrackerAccess::TrackDecRef (this, scfRefCount);
  scfRefCount--;
  if (scfRefCount <= 0)
  {
    if (scfParent) scfParent->DecRef ();
    doc->Free (this);
  }
}
SCF_IMPLEMENT_IBASE_GETREFCOUNT(csTinyXmlNode)
SCF_IMPLEMENT_IBASE_REFOWNER(csTinyXmlNode)
SCF_IMPLEMENT_IBASE_REMOVE_REF_OWNERS(csTinyXmlNode)
SCF_IMPLEMENT_IBASE_QUERY(csTinyXmlNode)
  SCF_IMPLEMENTS_INTERFACE (iDocumentNode)
SCF_IMPLEMENT_IBASE_END

csTinyXmlNode::csTinyXmlNode (csTinyXmlDocument* doc)
{
  SCF_CONSTRUCT_IBASE (0);
  node = 0;
  node_children = 0;
  csTinyXmlNode::doc = doc;	// Increase reference.
}

csTinyXmlNode::~csTinyXmlNode ()
{
  SCF_DESTRUCT_IBASE ();
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
    case TiDocumentNode::DOCUMENT: return CS_NODE_DOCUMENT;
    case TiDocumentNode::ELEMENT: return CS_NODE_ELEMENT;
    case TiDocumentNode::COMMENT: return CS_NODE_COMMENT;
    case TiDocumentNode::CDATA:
    case TiDocumentNode::TEXT:
      return CS_NODE_TEXT;
    case TiDocumentNode::DECLARATION: return CS_NODE_DECLARATION;
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
  	doc, node_children, 0));
  return it;
}

csRef<iDocumentNodeIterator> csTinyXmlNode::GetNodes (const char* value)
{
  csRef<iDocumentNodeIterator> it;
  it = csPtr<iDocumentNodeIterator> (new csTinyXmlNodeIterator (
  	doc, node_children, value));
  return it;
}

csRef<iDocumentNode> csTinyXmlNode::GetNode (const char* value)
{
  if (!node_children) return 0;
  csRef<iDocumentNode> child;
  TiDocumentNode* c = node_children->FirstChild (value);
  if (!c) return child;
  child = csPtr<iDocumentNode> (doc->Alloc (c));
  return child;
}

void csTinyXmlNode::RemoveNode (const csRef<iDocumentNode>& child)
{
  //CS_ASSERT (child.IsValid ());
  if (node_children)
    node_children->RemoveChild (
  	((csTinyXmlNode*)(iDocumentNode*)child)->GetTiNode ());
}

void csTinyXmlNode::RemoveNodes ()
{
  if (node_children) node_children->Clear ();
}

csRef<iDocumentNode> csTinyXmlNode::CreateNodeBefore (csDocumentNodeType type,
	iDocumentNode* before)
{
  if (!node_children) return 0;
  csRef<iDocumentNode> n;
  TiDocumentNode* child = 0;
  switch (type)
  {
    case CS_NODE_DOCUMENT:
      break;
    case CS_NODE_ELEMENT:
      {
        TiXmlElement el;
	if (before)
	  child = node_children->InsertBeforeChild (
	  	((csTinyXmlNode*)(iDocumentNode*)before)->GetTiNode (),
		el);
        else
	  child = node_children->InsertEndChild (el);
        //CS_ASSERT (child != 0);
      }
      break;
    case CS_NODE_COMMENT:
      {
        TiXmlComment el;
	if (before)
	  child = node_children->InsertBeforeChild (
	  	((csTinyXmlNode*)(iDocumentNode*)before)->GetTiNode (),
		el);
        else
	  child = node_children->InsertEndChild (el);
        //CS_ASSERT (child != 0);
      }
      break;
    case CS_NODE_TEXT:
      {
        TiXmlText el;
	if (before)
	  child = node_children->InsertBeforeChild (
	  	((csTinyXmlNode*)(iDocumentNode*)before)->GetTiNode (),
		el);
        else
	  child = node_children->InsertEndChild (el);
        //CS_ASSERT (child != 0);
      }
      break;
    case CS_NODE_DECLARATION:
      {
        TiXmlDeclaration el;
	if (before)
	  child = node_children->InsertBeforeChild (
	  	((csTinyXmlNode*)(iDocumentNode*)before)->GetTiNode (),
		el);
        else
	  child = node_children->InsertEndChild (el);
        //CS_ASSERT (child != 0);
      }
      break;
    case CS_NODE_UNKNOWN:
      {
        TiXmlUnknown el;
	if (before)
	  child = node_children->InsertBeforeChild (
	  	((csTinyXmlNode*)(iDocumentNode*)before)->GetTiNode (),
		el);
        else
	  child = node_children->InsertEndChild (el);
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
  if (!node_children) return 0;
  TiDocumentNode* child = node_children->FirstChild ();
  while (child)
  {
    if ((child->Type () == TiDocumentNode::TEXT)
      || (child->Type () == TiDocumentNode::CDATA))
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

TiDocumentAttribute* csTinyXmlNode::GetAttributeInternal (const char* name)
{
  size_t count = node->ToElement ()->GetAttributeCount ();
  size_t i;
  for (i = 0 ; i < count ; i++)
  {
    TiDocumentAttribute& attrib = node->ToElement ()->GetAttribute (i);
    if (strcmp (name, attrib.Name ()) == 0)
      return &attrib;
  }

  return 0;
}

csRef<iDocumentAttribute> csTinyXmlNode::GetAttribute (const char* name)
{
  csRef<iDocumentAttribute> attr;
  TiDocumentAttribute* a = GetAttributeInternal (name);
  if (a)
  {
    attr = csPtr<iDocumentAttribute> (new csTinyXmlAttribute (a));
  }
  return attr;
}

const char* csTinyXmlNode::GetAttributeValue (const char* name)
{
  TiXmlElement* el = node->ToElement ();
  if (el) return el->Attribute (name);
  else return 0;
}

int csTinyXmlNode::GetAttributeValueAsInt (const char* name)
{
  TiDocumentAttribute* a = GetAttributeInternal (name);
  if (!a) return 0;
  return a->IntValue ();
}

float csTinyXmlNode::GetAttributeValueAsFloat (const char* name)
{
  TiDocumentAttribute* a = GetAttributeInternal (name);
  if (!a) return 0;
  float f;
  sscanf (a->Value (), "%f", &f);
  return f;
}

bool csTinyXmlNode::GetAttributeValueAsBool(const char* name,
					    bool defaultvalue)
{
  TiDocumentAttribute* a = GetAttributeInternal (name);
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
  TiXmlElement* el = node->ToElement ();
  if (el) el->SetAttribute (el->GetDocument (), name, value);
}

void csTinyXmlNode::SetAttributeAsInt (const char* name, int value)
{
  TiXmlElement* el = node->ToElement ();
  if (el) el->SetAttribute (el->GetDocument (), name, value);
}

void csTinyXmlNode::SetAttributeAsFloat (const char* name, float value)
{
  TiXmlElement* el = node->ToElement ();
  if (el)
  {
    csString v;
    v.Format ("%g", value);
    el->SetAttribute (el->GetDocument (), name, v);
  }
}

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csTinyXmlDocument)
  SCF_IMPLEMENTS_INTERFACE (iDocument)
SCF_IMPLEMENT_IBASE_END

csTinyXmlDocument::csTinyXmlDocument (csTinyDocumentSystem* sys)
{
  SCF_CONSTRUCT_IBASE (0);
  csTinyXmlDocument::sys = sys;	// Increase ref.
  pool = 0;
  root = 0;
}

csTinyXmlDocument::~csTinyXmlDocument ()
{
  Clear ();
  while (pool)
  {
    csTinyXmlNode* n = pool->next_pool;
    // The 'sys' member in pool should be 0 here.
    delete pool;
    pool = n;
  }
  SCF_DESTRUCT_IBASE ();
}

void csTinyXmlDocument::Clear ()
{
  if (!root) return;
  delete root;
  root = 0;
}

csRef<iDocumentNode> csTinyXmlDocument::CreateRoot ()
{
  Clear ();
  root = new TiDocument ();
  return csPtr<iDocumentNode> (Alloc (root));
}

csRef<iDocumentNode> csTinyXmlDocument::GetRoot ()
{
  return csPtr<iDocumentNode> (Alloc (root));
}

const char* csTinyXmlDocument::Parse (iFile* file, bool collapse)
{
  size_t want_size = file->GetSize ();
  char *data = new char [want_size + 1];
  size_t real_size = file->Read (data, want_size);
  if (want_size != real_size)
  {
    delete[] data;
    return "Unexpected EOF encountered";
  }
  data[real_size] = '\0';
#ifdef CS_DEBUG
  if (strlen (data) != real_size)
  {
    delete[] data;
    return "File contains one or more null characters";
  }
#endif
  const char *error = Parse (data, collapse);
  delete[] data;
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
  if (pool)
  {
    csTinyXmlNode* n = pool;
    pool = n->next_pool;
    n->scfRefCount = 1;
    n->doc = this;	// Incref.
    return n;
  }
  else
  {
    csTinyXmlNode* n = new csTinyXmlNode (this);
    return n;
  }
}

csTinyXmlNode* csTinyXmlDocument::Alloc (TiDocumentNode* node)
{
  csTinyXmlNode* n = Alloc ();
  n->SetTiNode (node);
  return n;
}

void csTinyXmlDocument::Free (csTinyXmlNode* n)
{
  n->next_pool = pool;
  pool = n;
  n->doc = 0;	// Free ref.
}
