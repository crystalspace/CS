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

csXmlReadDocumentSystem::csXmlReadDocumentSystem ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  pool = NULL;
}

csXmlReadDocumentSystem::~csXmlReadDocumentSystem ()
{
  while (pool)
  {
    csXmlReadNode* n = pool->next_pool;
    // The 'sys' member in pool should be NULL here.
    delete pool;
    pool = n;
  }
}

csRef<iDocument> csXmlReadDocumentSystem::CreateDocument ()
{
  csRef<iDocument> doc (csPtr<iDocument> (new csXmlReadDocument (this)));
  return doc;
}

csXmlReadNode* csXmlReadDocumentSystem::Alloc ()
{
  if (pool)
  {
    csXmlReadNode* n = pool;
    pool = n->next_pool;
    n->scfRefCount = 1;
    n->sys = this;	// Incref.
    return n;
  }
  else
  {
    csXmlReadNode* n = new csXmlReadNode (this);
    return n;
  }
}

csXmlReadNode* csXmlReadDocumentSystem::Alloc (TrDocumentNode* node)
{
  csXmlReadNode* n = Alloc ();
  n->SetTiNode (node);
  return n;
}

void csXmlReadDocumentSystem::Free (csXmlReadNode* n)
{
  n->next_pool = pool;
  pool = n;
  n->sys = 0;	// Free ref.
}

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csXmlReadAttributeIterator)
  SCF_IMPLEMENTS_INTERFACE (iDocumentAttributeIterator)
SCF_IMPLEMENT_IBASE_END

csXmlReadAttributeIterator::csXmlReadAttributeIterator (TrDocumentNode* parent)
{
  SCF_CONSTRUCT_IBASE (NULL);
  csXmlReadAttributeIterator::parent = parent->ToElement ();
  if (csXmlReadAttributeIterator::parent == NULL)
  {
    current = -1;
    return;
  }
  count = csXmlReadAttributeIterator::parent->GetAttributeCount ();
  if (!count) 
  {
    current = -1;
    return;
  }
  current = 0;
}

bool csXmlReadAttributeIterator::HasNext ()
{
  return current != -1;
}

csRef<iDocumentAttribute> csXmlReadAttributeIterator::Next ()
{
  csRef<iDocumentAttribute> attr;
  if (current != -1)
  {
    attr = csPtr<iDocumentAttribute> (new csXmlReadAttribute (
    	&parent->GetAttribute (current)));
    current++;
    if (current >= count)
      current = -1;
  }
  return attr;
}

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csXmlReadNodeIterator)
  SCF_IMPLEMENTS_INTERFACE (iDocumentNodeIterator)
SCF_IMPLEMENT_IBASE_END

csXmlReadNodeIterator::csXmlReadNodeIterator (
	csXmlReadDocumentSystem* sys, TrDocumentNodeChildren* parent,
	const char* value)
{
  SCF_CONSTRUCT_IBASE (NULL);
  csXmlReadNodeIterator::sys = sys;
  csXmlReadNodeIterator::parent = parent;
  csXmlReadNodeIterator::value = value ? csStrNew (value) : NULL;
  if (!parent)
    current = NULL;
  else if (value)
    current = parent->FirstChild (value);
  else
    current = parent->FirstChild ();
}

bool csXmlReadNodeIterator::HasNext ()
{
  return current != NULL;
}

csRef<iDocumentNode> csXmlReadNodeIterator::Next ()
{
  csRef<iDocumentNode> node;
  if (current != NULL)
  {
    node = csPtr<iDocumentNode> (sys->Alloc (current));
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
    sys->Free (this);
  }
}
SCF_IMPLEMENT_IBASE_GETREFCOUNT(csXmlReadNode)
SCF_IMPLEMENT_IBASE_QUERY(csXmlReadNode)
  SCF_IMPLEMENTS_INTERFACE (iDocumentNode)
SCF_IMPLEMENT_IBASE_END

csXmlReadNode::csXmlReadNode (csXmlReadDocumentSystem* sys)
{
  SCF_CONSTRUCT_IBASE (NULL);
  node = NULL;
  node_children = NULL;
  csXmlReadNode::sys = sys;	// Increase reference.
}

csXmlReadNode::~csXmlReadNode ()
{
}

csRef<iDocumentNode> csXmlReadNode::GetParent ()
{
  csRef<iDocumentNode> child;
  if (!node->Parent ()) return child;
  child = csPtr<iDocumentNode> (sys->Alloc (node->Parent ()));
  return child;
}

csDocumentNodeType csXmlReadNode::GetType ()
{
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
  return GetTiNode () == other_node->GetTiNode ();
}

const char* csXmlReadNode::GetValue ()
{
  return node->Value ();
}

csRef<iDocumentNodeIterator> csXmlReadNode::GetNodes ()
{
  csRef<iDocumentNodeIterator> it;
  it = csPtr<iDocumentNodeIterator> (new csXmlReadNodeIterator (
  	sys, node_children, NULL));
  return it;
}

csRef<iDocumentNodeIterator> csXmlReadNode::GetNodes (const char* value)
{
  csRef<iDocumentNodeIterator> it;
  it = csPtr<iDocumentNodeIterator> (new csXmlReadNodeIterator (
  	sys, node_children, value));
  return it;
}

csRef<iDocumentNode> csXmlReadNode::GetNode (const char* value)
{
  if (!node_children) return NULL;
  csRef<iDocumentNode> child;
  TrDocumentNode* c = node_children->FirstChild (value);
  if (!c) return child;
  child = csPtr<iDocumentNode> (sys->Alloc (c));
  return child;
}

const char* csXmlReadNode::GetContentsValue ()
{
  if (!node_children) return NULL;
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
  return NULL;
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
  	new csXmlReadAttributeIterator (node));
  return it;
}

TrDocumentAttribute* csXmlReadNode::GetAttributeInternal (const char* name)
{
  int count = node->ToElement ()->GetAttributeCount ();
  int i;
  for (i = 0 ; i < count ; i++)
  {
    TrDocumentAttribute& attrib = node->ToElement ()->GetAttribute (i);
    if (strcmp (name, attrib.Name ()) == 0)
      return &attrib;
  }

  return NULL;
}

csRef<iDocumentAttribute> csXmlReadNode::GetAttribute (const char* name)
{
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
  TrXmlElement* el = node->ToElement ();
  if (el) return el->Attribute (name);
  else return NULL;
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

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csXmlReadDocument)
  SCF_IMPLEMENTS_INTERFACE (iDocument)
SCF_IMPLEMENT_IBASE_END

csXmlReadDocument::csXmlReadDocument (csXmlReadDocumentSystem* sys)
{
  SCF_CONSTRUCT_IBASE (NULL);
  csXmlReadDocument::sys = sys;	// Increase ref.
}

csXmlReadDocument::~csXmlReadDocument ()
{
  Clear ();
}

void csXmlReadDocument::Clear ()
{
  if (!root) return;
  TrDocument* doc = (TrDocument*)(((csXmlReadNode*)(iDocumentNode*)root)->GetTiNode ());
  delete doc;
  root = 0;
}

csRef<iDocumentNode> csXmlReadDocument::CreateRoot (char* buf)
{
  Clear ();
  TrDocument* doc = new TrDocument (buf);
  root = csPtr<iDocumentNode> (sys->Alloc (doc));
  return root;
}

csRef<iDocumentNode> csXmlReadDocument::CreateRoot ()
{
  Clear ();
  TrDocument* doc = new TrDocument ();
  root = csPtr<iDocumentNode> (sys->Alloc (doc));
  return root;
}

csRef<iDocumentNode> csXmlReadDocument::GetRoot ()
{
  return root;
}

const char* csXmlReadDocument::Parse (iFile* file)
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
  const char *error = Parse (data);
  delete[] data;
  return error;
}

const char* csXmlReadDocument::Parse (iDataBuffer* buf)
{
  return Parse ((const char*)buf->GetData ());
}

const char* csXmlReadDocument::Parse (iString* str)
{
  return Parse ((const char*)*str);
}

const char* csXmlReadDocument::Parse (const char* buf)
{
  CreateRoot (csStrNew (buf));
  TrDocument* doc = (TrDocument*)(((csXmlReadNode*)(iDocumentNode*)root)
  	->GetTiNode ());
  doc->Parse (doc, doc->input_data);
  if (doc->Error ())
    return doc->ErrorDesc ();
  return NULL;
}

const char* csXmlReadDocument::ParseInPlace (char* buf)
{
  CreateRoot (buf);
  TrDocument* doc = (TrDocument*)(((csXmlReadNode*)(iDocumentNode*)root)
  	->GetTiNode ());
  doc->Parse (doc, doc->input_data);
  if (doc->Error ())
    return doc->ErrorDesc ();
  return NULL;
}

//------------------------------------------------------------------------

