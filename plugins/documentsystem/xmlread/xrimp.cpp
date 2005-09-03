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
#include "xriface.h"
#include "xrpriv.h"
#include "csutil/scfstr.h"
#include "iutil/vfs.h"
#include "iutil/string.h"
#include "iutil/databuff.h"
#include "xr.h"

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csXmlReadDocumentSystem)
  SCF_IMPLEMENTS_INTERFACE (iDocumentSystem)
SCF_IMPLEMENT_IBASE_END

csXmlReadDocumentSystem::csXmlReadDocumentSystem (iBase* parent)
{
  SCF_CONSTRUCT_IBASE (0);
  csXmlReadDocumentSystem::parent = parent;
}

csXmlReadDocumentSystem::~csXmlReadDocumentSystem ()
{
  SCF_DESTRUCT_IBASE();
}

csRef<iDocument> csXmlReadDocumentSystem::CreateDocument ()
{
  csRef<iDocument> doc (csPtr<iDocument> (new csXmlReadDocument (this)));
  return doc;
}

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csXmlReadAttributeIterator)
  SCF_IMPLEMENTS_INTERFACE (iDocumentAttributeIterator)
SCF_IMPLEMENT_IBASE_END

csXmlReadAttributeIterator::csXmlReadAttributeIterator (TrDocumentNode* parent)
{
  SCF_CONSTRUCT_IBASE (0);
  csXmlReadAttributeIterator::parent = parent ? parent->ToElement () : 0;
  if (csXmlReadAttributeIterator::parent == 0)
  {
    current = csArrayItemNotFound;
    return;
  }
  count = csXmlReadAttributeIterator::parent->GetAttributeCount ();
  if (!count) 
  {
    current = csArrayItemNotFound;
    return;
  }
  current = 0;
}

csXmlReadAttributeIterator::~csXmlReadAttributeIterator()
{
  SCF_DESTRUCT_IBASE();
}

bool csXmlReadAttributeIterator::HasNext ()
{
  return current != csArrayItemNotFound;
}

csRef<iDocumentAttribute> csXmlReadAttributeIterator::Next ()
{
  csRef<iDocumentAttribute> attr;
  if (current != csArrayItemNotFound)
  {
    attr = csPtr<iDocumentAttribute> (new csXmlReadAttribute (
    	&parent->GetAttribute (current)));
    current++;
    if (current >= count)
      current = csArrayItemNotFound;
  }
  return attr;
}

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csXmlReadNodeIterator)
  SCF_IMPLEMENTS_INTERFACE (iDocumentNodeIterator)
SCF_IMPLEMENT_IBASE_END

csXmlReadNodeIterator::csXmlReadNodeIterator (
	csXmlReadDocument* doc, TrDocumentNodeChildren* parent,
	const char* value)
{
  SCF_CONSTRUCT_IBASE (0);
  csXmlReadNodeIterator::doc = doc;
  csXmlReadNodeIterator::parent = parent;
  csXmlReadNodeIterator::value = value ? csStrNew (value) : 0;
  use_contents_value = false;
  if (!parent)
    current = 0;
  else if (value)
    current = parent->FirstChild (value);
  else
  {
    if (parent->ToElement () && parent->ToElement ()->GetContentsValue ())
    {
      use_contents_value = true;
      current = parent;
    }
    else
    {
      current = parent->FirstChild ();
    }
  }
}

csXmlReadNodeIterator::~csXmlReadNodeIterator ()
{
  delete[] value;
  SCF_DESTRUCT_IBASE();
}

bool csXmlReadNodeIterator::HasNext ()
{
  return use_contents_value || current != 0;
}

csRef<iDocumentNode> csXmlReadNodeIterator::Next ()
{
  csRef<iDocumentNode> node;
  if (use_contents_value)
  {
    node = csPtr<iDocumentNode> (doc->Alloc (current, true));
    use_contents_value = false;
    current = parent->FirstChild ();
  }
  else if (current != 0)
  {
    node = csPtr<iDocumentNode> (doc->Alloc (current, false));
    if (value)
      current = current->NextSibling (value);
    else
      current = current->NextSibling ();
  }
  return node;
}

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csXmlReadAttribute)
  SCF_IMPLEMENTS_INTERFACE (iDocumentAttribute)
SCF_IMPLEMENT_IBASE_END

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_INCREF(csXmlReadNode)
void csXmlReadNode::DecRef ()
{
  scfRefCount--;
  if (scfRefCount <= 0)
  {
    if (scfParent) scfParent->DecRef ();
    doc->Free (this);
  }
}
SCF_IMPLEMENT_IBASE_GETREFCOUNT(csXmlReadNode)
SCF_IMPLEMENT_IBASE_REFOWNER(csXmlReadNode)
SCF_IMPLEMENT_IBASE_REMOVE_REF_OWNERS(csXmlReadNode)
SCF_IMPLEMENT_IBASE_QUERY(csXmlReadNode)
  SCF_IMPLEMENTS_INTERFACE (iDocumentNode)
SCF_IMPLEMENT_IBASE_END

csXmlReadNode::csXmlReadNode (csXmlReadDocument* doc)
{
  SCF_CONSTRUCT_IBASE (0);
  node = 0;
  node_children = 0;
  csXmlReadNode::doc = doc;	// Increase reference.
}

csXmlReadNode::~csXmlReadNode ()
{
  SCF_DESTRUCT_IBASE();
}

csRef<iDocumentNode> csXmlReadNode::GetParent ()
{
  csRef<iDocumentNode> child;
  if (use_contents_value)
  {
    // If we use contents value then the parent is actually this object.
    IncRef ();
    return csPtr<iDocumentNode> (this);
  }
  else
  {
    if (!node->Parent ()) return child;
    child = csPtr<iDocumentNode> (doc->Alloc (node->Parent (), false));
    return child;
  }
  
  return 0;
}

csDocumentNodeType csXmlReadNode::GetType ()
{
  if (use_contents_value) return CS_NODE_TEXT;

  switch (node->Type ())
  {
    case TrDocumentNode::DOCUMENT: return CS_NODE_DOCUMENT;
    case TrDocumentNode::ELEMENT: return CS_NODE_ELEMENT;
    case TrDocumentNode::COMMENT: return CS_NODE_COMMENT;
    case TrDocumentNode::CDATA:
    case TrDocumentNode::TEXT:
      return CS_NODE_TEXT;
    case TrDocumentNode::DECLARATION: return CS_NODE_DECLARATION;
    default: return CS_NODE_UNKNOWN;
  }
}

bool csXmlReadNode::Equals (iDocumentNode* other)
{
  csXmlReadNode* other_node = (csXmlReadNode*)other;
  return GetTiNode () == other_node->GetTiNode ()
  	&& use_contents_value == other_node->use_contents_value;
}

const char* csXmlReadNode::GetValue ()
{
  if (use_contents_value)
  {
    return node->ToElement ()->GetContentsValue ();
  }
  else
  {
    return node->Value ();
  }
}

csRef<iDocumentNodeIterator> csXmlReadNode::GetNodes ()
{
  csRef<iDocumentNodeIterator> it;
  it = csPtr<iDocumentNodeIterator> (new csXmlReadNodeIterator (
  	doc, use_contents_value ? 0 : node_children, 0));
  return it;
}

csRef<iDocumentNodeIterator> csXmlReadNode::GetNodes (const char* value)
{
  csRef<iDocumentNodeIterator> it;
  it = csPtr<iDocumentNodeIterator> (new csXmlReadNodeIterator (
  	doc, use_contents_value ? 0 : node_children, value));
  return it;
}

csRef<iDocumentNode> csXmlReadNode::GetNode (const char* value)
{
  if (!node_children || use_contents_value) return 0;
  csRef<iDocumentNode> child;
  TrDocumentNode* c = node_children->FirstChild (value);
  if (!c) return child;
  child = csPtr<iDocumentNode> (doc->Alloc (c, false));
  return child;
}

const char* csXmlReadNode::GetContentsValue ()
{
  if (!node_children || use_contents_value) return 0;
  TrXmlElement* el = node->ToElement ();
  if (el && el->GetContentsValue ())
  {
    return el->GetContentsValue ();
  }

  TrDocumentNode* child = node_children->FirstChild ();
  while (child)
  {
    if (child->Type () == TrDocumentNode::TEXT ||
    	child->Type () == TrDocumentNode::CDATA)
    {
      return child->Value ();
    }
    child = child->NextSibling ();
  } 
  return 0;
}

int csXmlReadNode::GetContentsValueAsInt ()
{
  const char* v = GetContentsValue ();
  if (!v) return 0;
  int val = 0;
  sscanf (v, "%d", &val);
  return val;
}

float csXmlReadNode::GetContentsValueAsFloat ()
{
  const char* v = GetContentsValue ();
  if (!v) return 0;
  float val = 0.0;
  sscanf (v, "%f", &val);
  return val;
}

csRef<iDocumentAttributeIterator> csXmlReadNode::GetAttributes ()
{
  csRef<iDocumentAttributeIterator> it;
  it = csPtr<iDocumentAttributeIterator> (
    new csXmlReadAttributeIterator (use_contents_value ? 0 : node));
  return it;
}

TrDocumentAttribute* csXmlReadNode::GetAttributeInternal (const char* name)
{
  if (use_contents_value) return 0;
  const TrXmlElement* elem = node->ToElement ();
  if (!elem) return 0;
  size_t count = elem->GetAttributeCount ();
  size_t i;
  for (i = 0 ; i < count ; i++)
  {
    TrDocumentAttribute& attrib = node->ToElement ()->GetAttribute (i);
    if (strcmp (name, attrib.Name ()) == 0)
      return &attrib;
  }

  return 0;
}

csRef<iDocumentAttribute> csXmlReadNode::GetAttribute (const char* name)
{
  if (use_contents_value) return 0;
  csRef<iDocumentAttribute> attr;
  TrDocumentAttribute* a = GetAttributeInternal (name);
  if (a)
  {
    attr = csPtr<iDocumentAttribute> (new csXmlReadAttribute (a));
  }
  return attr;
}

const char* csXmlReadNode::GetAttributeValue (const char* name)
{
  if (use_contents_value) return 0;
  TrXmlElement* el = node->ToElement ();
  if (el) return el->Attribute (name);
  else return 0;
}

int csXmlReadNode::GetAttributeValueAsInt (const char* name)
{
  TrDocumentAttribute* a = GetAttributeInternal (name);
  if (!a) return 0;
  return a->IntValue ();
}

float csXmlReadNode::GetAttributeValueAsFloat (const char* name)
{
  TrDocumentAttribute* a = GetAttributeInternal (name);
  if (!a) return 0;
  float f;
  sscanf (a->Value (), "%f", &f);
  return f;
}

bool csXmlReadNode::GetAttributeValueAsBool (const char* name,bool defaultvalue)
{
  TrDocumentAttribute* a = GetAttributeInternal (name);
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

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csXmlReadDocument)
  SCF_IMPLEMENTS_INTERFACE (iDocument)
SCF_IMPLEMENT_IBASE_END

csXmlReadDocument::csXmlReadDocument (csXmlReadDocumentSystem* sys)
{
  SCF_CONSTRUCT_IBASE (0);
  csXmlReadDocument::sys = sys;	// Increase ref.
  pool = 0;
  root = 0;
}

csXmlReadDocument::~csXmlReadDocument ()
{
  Clear ();
  while (pool)
  {
    csXmlReadNode* n = pool->next_pool;
    // The 'sys' member in pool should be 0 here.
    delete pool;
    pool = n;
  }
  SCF_DESTRUCT_IBASE();
}

void csXmlReadDocument::Clear ()
{
  if (!root) return;
  delete root;
  root = 0;
}

csRef<iDocumentNode> csXmlReadDocument::CreateRoot (char* buf)
{
  Clear ();
  root = new TrDocument (buf);
  return csPtr<iDocumentNode> (Alloc (root, false));
}

csRef<iDocumentNode> csXmlReadDocument::CreateRoot ()
{
  Clear ();
  root = new TrDocument ();
  return csPtr<iDocumentNode> (Alloc (root, false));
}

csRef<iDocumentNode> csXmlReadDocument::GetRoot ()
{
  return csPtr<iDocumentNode> (Alloc (root, false));
}

const char* csXmlReadDocument::Parse (iFile* file, bool collapse)
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

const char* csXmlReadDocument::Parse (iDataBuffer* buf, bool collapse)
{
  return Parse ((const char*)buf->GetData (), collapse);
}

const char* csXmlReadDocument::Parse (iString* str, bool collapse)
{
  return Parse ((const char*)*str, collapse);
}

const char* csXmlReadDocument::Parse (const char* buf, bool collapse)
{
  CreateRoot (csStrNew (buf));
  bool const old_collapse = root->IsWhiteSpaceCondensed();
  root->SetCondenseWhiteSpace(collapse);
  root->Parse (root, root->input_data);
  root->SetCondenseWhiteSpace(old_collapse);
  if (root->Error ())
    return root->ErrorDesc ();
  return 0;
}

const char* csXmlReadDocument::ParseInPlace (char* buf, bool collapse)
{
  CreateRoot (buf);
  bool const old_collapse = root->IsWhiteSpaceCondensed();
  root->SetCondenseWhiteSpace(collapse);
  root->Parse (root, root->input_data);
  root->SetCondenseWhiteSpace(old_collapse);
  if (root->Error ())
    return root->ErrorDesc ();
  return 0;
}

csXmlReadNode* csXmlReadDocument::Alloc ()
{
  if (pool)
  {
    csXmlReadNode* n = pool;
    pool = n->next_pool;
    n->scfRefCount = 1;
    n->doc = this;	// Incref.
    return n;
  }
  else
  {
    csXmlReadNode* n = new csXmlReadNode (this);
    return n;
  }
}

csXmlReadNode* csXmlReadDocument::Alloc (TrDocumentNode* node,
	bool use_contents_value)
{
  csXmlReadNode* n = Alloc ();
  n->SetTiNode (node, use_contents_value);
  return n;
}

void csXmlReadDocument::Free (csXmlReadNode* n)
{
  n->next_pool = pool;
  pool = n;
  n->doc = 0;	// Free ref.
}

//------------------------------------------------------------------------

