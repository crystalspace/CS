/*
  Copyright (C) 2005 by Frank Richter
	    (C) 2005 by Jorrit Tyberghein

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

#include "csutil/set.h"
#include "csutil/sysfunc.h"
#include "csutil/util.h"
#include "csutil/xmltiny.h"
#include "imap/services.h"
#include "ivaria/reporter.h"
#include "iutil/vfs.h"
#include "iutil/document.h"

#include "docwrap_replacer.h"

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_POOLED(csReplacerDocumentNode)
  SCF_IMPLEMENTS_INTERFACE(iDocumentNode)
SCF_IMPLEMENT_IBASE_END

csReplacerDocumentNode::csReplacerDocumentNode (Pool* pool)
{
  SCF_CONSTRUCT_IBASE_POOLED (pool);
}

csReplacerDocumentNode::~csReplacerDocumentNode ()
{
}

void csReplacerDocumentNode::Set (iDocumentNode* wrappedNode, 
				  csReplacerDocumentNode* parent, 
				  csReplacerDocumentNodeFactory* shared, 
				  Substitutions* subst)
{
  this->wrappedNode = wrappedNode;
  this->parent = parent;
  this->shared = shared;
  this->subst = subst;

  shared->Substitute (wrappedNode->GetValue(), value, *this->subst);
}

bool csReplacerDocumentNode::Equals (iDocumentNode* other)
{
  return (csReplacerDocumentNode*)other->Equals (wrappedNode);
}

csRef<iDocumentNodeIterator> csReplacerDocumentNode::GetNodes ()
{
  csReplacerDocumentNodeIterator* iter = shared->iterPool.Alloc();
  iter->SetData (this);
  return csPtr<iDocumentNodeIterator> (iter);
}

csRef<iDocumentNode> csReplacerDocumentNode::GetNode (
  const char* value)
{
  csRef<iDocumentNode> retNode = wrappedNode->GetNode (value);
  if (!retNode.IsValid()) return 0;
  return shared->CreateWrapper (retNode, this, *subst);
}

csRef<iDocumentAttributeIterator> 
csReplacerDocumentNode::GetAttributes ()
{
  csReplacerDocumentAttributeIterator* iter = 
    shared->attrIterPool.Alloc();
  iter->SetData (this);
  return csPtr<iDocumentAttributeIterator> (iter);
}

csRef<iDocumentAttribute> csReplacerDocumentNode::GetAttribute (
  const char* name)
{
  csRef<iDocumentAttribute> wrappedAttr = attrCache.Get (name, 0);
  if (wrappedAttr.IsValid()) return wrappedAttr;

  wrappedAttr = wrappedNode->GetAttribute (name);
  if (!wrappedAttr.IsValid()) return 0;
  csReplacerDocumentAttribute* attr = shared->attrPool.Alloc();
  attr->Set (this, wrappedAttr);
  wrappedAttr.AttachNew (attr);
  attrCache.Put (name, wrappedAttr);
  return wrappedAttr;
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_POOLED(csReplacerDocumentNodeIterator)
  SCF_IMPLEMENTS_INTERFACE(iDocumentNodeIterator)
SCF_IMPLEMENT_IBASE_END

csReplacerDocumentNodeIterator::csReplacerDocumentNodeIterator (Pool* pool)
{
  SCF_CONSTRUCT_IBASE_POOLED (pool);
}

csReplacerDocumentNodeIterator::~csReplacerDocumentNodeIterator ()
{
}

void csReplacerDocumentNodeIterator::SetData (csReplacerDocumentNode* node)
{
  this->node = node;
  wrappedIter = node->wrappedNode->GetNodes();
}

bool csReplacerDocumentNodeIterator::HasNext ()
{
  if (!wrappedIter.IsValid()) return false;
  return wrappedIter->HasNext();
}

csRef<iDocumentNode> csReplacerDocumentNodeIterator::Next ()
{
  csRef<iDocumentNode> wrappedNode = wrappedIter->Next();
  if (!wrappedNode.IsValid()) return 0;
  return node->shared->CreateWrapper (wrappedNode, node, *node->subst);
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_POOLED(csReplacerDocumentAttributeIterator)
  SCF_IMPLEMENTS_INTERFACE(iDocumentAttributeIterator)
SCF_IMPLEMENT_IBASE_END

csReplacerDocumentAttributeIterator::csReplacerDocumentAttributeIterator (
  Pool* pool)
{
  SCF_CONSTRUCT_IBASE_POOLED (pool);
}

csReplacerDocumentAttributeIterator::~csReplacerDocumentAttributeIterator ()
{
}

void csReplacerDocumentAttributeIterator::SetData (csReplacerDocumentNode* node)
{
  this->node = node;
  wrappedIter = node->wrappedNode->GetAttributes();
}

bool csReplacerDocumentAttributeIterator::HasNext ()
{
  if (!wrappedIter.IsValid()) return false;
  return wrappedIter->HasNext();
}

csRef<iDocumentAttribute> csReplacerDocumentAttributeIterator::Next ()
{
  csRef<iDocumentAttribute> wrappedAttr = wrappedIter->Next();
  if (!wrappedAttr.IsValid()) return 0;
  csReplacerDocumentAttribute* attr = node->shared->attrPool.Alloc();
  attr->Set (node, wrappedAttr);
  return csPtr<iDocumentAttribute> (attr);
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_POOLED(csReplacerDocumentAttribute)
  SCF_IMPLEMENTS_INTERFACE(iDocumentAttribute)
SCF_IMPLEMENT_IBASE_END

csReplacerDocumentAttribute::csReplacerDocumentAttribute (Pool* pool)
{
  SCF_CONSTRUCT_IBASE_POOLED (pool);
}

csReplacerDocumentAttribute::~csReplacerDocumentAttribute ()
{
}

void csReplacerDocumentAttribute::Set (csReplacerDocumentNode* node, 
				       iDocumentAttribute* wrappedAttr)
{
  node->shared->Substitute (wrappedAttr->GetName(), name, *node->subst);
  node->shared->Substitute (wrappedAttr->GetValue(), val, *node->subst);
}

//---------------------------------------------------------------------------

csReplacerDocumentNodeFactory::csReplacerDocumentNodeFactory ()
{
}

csRef<iDocumentNode> csReplacerDocumentNodeFactory::CreateWrapper (
  iDocumentNode* wrappedNode, csReplacerDocumentNode* parent, 
  const Substitutions& subst)
{
  csReplacerDocumentNode* newNode = nodePool.Alloc ();
  csRef<Substitutions> newSubst;
  newSubst.AttachNew (new Substitutions (subst));
  newNode->Set (wrappedNode, parent, this, newSubst);
  return csPtr<iDocumentNode> (newNode);
}

void csReplacerDocumentNodeFactory::Substitute (const char* in, 
						csString& out, 
						const Substitutions& subst)
{
  out.Clear();
  if (in == 0) return;

  const char* p = in;
  while (*p != 0)
  {
    if (*p == '$')
    {
      p++;
      const char* varStart = p;
      while ((*p != '$') && (*p != 0)) p++;
      const char* varEnd = p;
      csString varName (varStart, varEnd-varStart);
      if (varName.IsEmpty())
	out << '$';
      else
	out << subst.Get (varName, ""); // @@@ FIXME: Error reporting
      if (*p == 0) break;
    }
    else
      out << *p;
    p++;
  }
}
