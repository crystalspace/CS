/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter <resqu@gmx.ch>

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

#include <ctype.h>
#include "cssysdef.h"

#include "csutil/csstring.h"
#include "csutil/databuf.h"
#include "csutil/memfile.h"
#include "csutil/snprintf.h"
#include "csutil/sysfunc.h"
#include "csutil/util.h"

#include "iutil/databuff.h"
#include "iutil/document.h"
#include "iutil/string.h"
#include "iutil/vfs.h"

#include "bindoc.h"

// =================================================
//  csBinaryDocAttributeIterator
// =================================================

SCF_IMPLEMENT_IBASE(csBinaryDocAttributeIterator)
  SCF_IMPLEMENTS_INTERFACE(iDocumentAttributeIterator)
SCF_IMPLEMENT_IBASE_END

csBinaryDocAttributeIterator::csBinaryDocAttributeIterator ()
{
  SCF_CONSTRUCT_IBASE (0);
}

void csBinaryDocAttributeIterator::SetTo (csBdNode* node,
					  csBinaryDocNode* parent)
{
  if (!(node->flags & BD_NODE_HAS_ATTR))
  {
    iteratedNode = 0;
  }
  else
  {
    iteratedNode = node;
  }
  parentNode = parent;
  pos = 0;
}

csBinaryDocAttributeIterator::~csBinaryDocAttributeIterator()
{
  SCF_DESTRUCT_IBASE();
}

bool csBinaryDocAttributeIterator::HasNext ()
{
  return ((iteratedNode) && (pos < iteratedNode->atNum()));
}

csRef<iDocumentAttribute> csBinaryDocAttributeIterator::Next ()
{
  csBdAttr* attrPtr = iteratedNode->atGetItem (pos++);
  csBinaryDocAttribute* attr = parentNode->doc->GetPoolAttr();
  attr->SetTo (attrPtr, parentNode);
  return csPtr<iDocumentAttribute> (attr);
}

// =================================================
//  csBdAttr
// =================================================

csBdAttr::csBdAttr (const char* name)
{
  flags = BD_ATTR_MODIFIED;
  nameID = 0;
  value = 0;
  vstr = 0;
  nstr = csStrNew (name);
}

csBdAttr::csBdAttr ()
{
  flags = BD_ATTR_MODIFIED;
  nameID = 0;
  value = 0;
  vstr = 0;
  nstr = 0;
}

csBdAttr::~csBdAttr ()
{
  if (flags & BD_ATTR_MODIFIED)
  {
    delete[] nstr;
    delete[] vstr;
  }
}

void csBdAttr::SetName (const char* name)
{
  CS_ASSERT(flags & BD_ATTR_MODIFIED);
  delete[] nstr;
  nstr = csStrNew (name);
}

const char* csBdAttr::GetNameStr (csBinaryDocument* doc) const
{
  if (GetRealFlags() & BD_ATTR_MODIFIED)
  {
    return nstr;
  }
  else
  {
    if (GetRealFlags() & BD_ATTR_NAME_IMMEDIATE)
    {
      return (char*)&nameID;
    }
    else
    {
      return doc->GetInIDString (GetRealNameID());
    }
  }
}

const char* csBdAttr::GetValueStr (csBinaryDocument* doc) const
{
  if (GetRealFlags() & BD_ATTR_MODIFIED)
  {
    return vstr;
  }
  else
  {
    if ((GetRealFlags() & BD_VALUE_TYPE_MASK) == BD_VALUE_TYPE_STR_IMMEDIATE)
    {
      return (char*)&value;
    }
    else
    {
      return doc->GetInIDString (
	csLittleEndianLong (value));
    }
  }
}

// =================================================
//  csBinaryDocAttribute
// =================================================

void csBinaryDocAttribute::IncRef ()
{
  scfRefCount++;
}

void csBinaryDocAttribute::DecRef ()
{
  if (scfRefCount == 1)
  {
    node->doc->RecyclePoolAttr (this);
    return;
  }
  scfRefCount--;
}

SCF_IMPLEMENT_IBASE_GETREFCOUNT(csBinaryDocAttribute)
SCF_IMPLEMENT_IBASE_REFOWNER(csBinaryDocAttribute)
SCF_IMPLEMENT_IBASE_REMOVE_REF_OWNERS(csBinaryDocAttribute)
SCF_IMPLEMENT_IBASE_QUERY(csBinaryDocAttribute)
  SCF_IMPLEMENTS_INTERFACE(iDocumentAttribute)
SCF_IMPLEMENT_IBASE_END

csBinaryDocAttribute::csBinaryDocAttribute ()
{
  SCF_CONSTRUCT_IBASE (0);

  vstr = 0;
  vsptr = 0;
}

csBinaryDocAttribute::~csBinaryDocAttribute ()
{
  CleanData ();
  delete vstr; 
  SCF_DESTRUCT_IBASE();
}

void csBinaryDocAttribute::CleanData ()
{
}

void csBinaryDocAttribute::SetTo (csBdAttr* ptr,
				  csBinaryDocNode* owner)
{
  CleanData();
  node = owner;
  attrPtr = ptr;
}

const char* csBinaryDocAttribute::GetName ()
{
  return attrPtr->GetNameStr (node->doc);
}

const char* csBinaryDocAttribute::GetValue ()
{
  switch (attrPtr->GetRealFlags() & BD_VALUE_TYPE_MASK)
  {
    case BD_VALUE_TYPE_STR:
    case BD_VALUE_TYPE_STR_IMMEDIATE:
      {
	return attrPtr->GetValueStr (node->doc);
      }
    case BD_VALUE_TYPE_INT:
      {
	if (vsptr != attrPtr)
	{
  	  char buf[50];
	  cs_snprintf (buf, sizeof (buf) - 1, "%" PRId32, 
	    (int32)csLittleEndianLong (attrPtr->value));
	  delete[] vstr; 
	  vstr = csStrNew (buf);
	  vsptr = attrPtr;
	}
	return vstr;
      }
    case BD_VALUE_TYPE_FLOAT:
      {
	if (vsptr != attrPtr)
	{
  	  char buf[50];
	  cs_snprintf (buf, sizeof (buf) - 1,
	    "%g", csLongToFloat (csLittleEndianLong (attrPtr->value)));
	  delete[] vstr; 
	  vstr = csStrNew (buf);
	  vsptr = attrPtr;
	}
	return vstr;
      }
    default:
      return 0;
  }
}

int csBinaryDocAttribute::GetValueAsInt ()
{
  switch (attrPtr->GetRealFlags() & BD_VALUE_TYPE_MASK)
  {
    case BD_VALUE_TYPE_STR:
    case BD_VALUE_TYPE_STR_IMMEDIATE:
      {
	if (!attrPtr->GetValueStr (node->doc)) return 0;
	int val = 0;
	sscanf (attrPtr->GetValueStr (node->doc), "%d", &val);
	return val;
      }
    case BD_VALUE_TYPE_INT:
      {
	return (int)((int32)csLittleEndianLong (attrPtr->value));
      }
    case BD_VALUE_TYPE_FLOAT:
      {
	return (int)csLongToFloat (csLittleEndianLong (attrPtr->value));
      }
    default:
      return 0;
  }
}

float csBinaryDocAttribute::GetValueAsFloat ()
{
  switch (attrPtr->GetRealFlags() & BD_VALUE_TYPE_MASK)
  {
    case BD_VALUE_TYPE_STR:
    case BD_VALUE_TYPE_STR_IMMEDIATE:
      {
	if (!attrPtr->GetValueStr (node->doc)) return 0.0f;
	float val = 0.0f;
	sscanf (attrPtr->GetValueStr (node->doc), "%g", &val);
	return val;
      }
    case BD_VALUE_TYPE_INT:
      {
	return (float)((int32)csLittleEndianLong (attrPtr->value));
      }
    case BD_VALUE_TYPE_FLOAT:
      {
	return csLongToFloat (csLittleEndianLong (attrPtr->value));
      }
    default:
      return 0.0f;
  }
}

bool csBinaryDocAttribute::GetValueAsBool ()
{
  switch (attrPtr->GetRealFlags() & BD_VALUE_TYPE_MASK)
  {
    case BD_VALUE_TYPE_STR:
    case BD_VALUE_TYPE_STR_IMMEDIATE:
      {
	if (!attrPtr->GetValueStr (node->doc)) return false;
	const char *val = attrPtr->GetValueStr (node->doc);
	if (strcasecmp (val, "true") == 0 ||
	    strcasecmp (val, "yes") == 0 ||
	    atoi(val)!=0)
	{
	  return true;
	}
	else
	  return false;
      }
    case BD_VALUE_TYPE_INT:
      {
	return (attrPtr->value != 0);
      }
    case BD_VALUE_TYPE_FLOAT:
      {
	return (csLongToFloat (csLittleEndianLong (attrPtr->value))== 0);
      }
    default:
      return false;
  }
}

void csBinaryDocAttribute::SetName (const char* name)
{
  if (attrPtr->flags & BD_NODE_MODIFIED)
  {
    delete[] attrPtr->nstr;
    attrPtr->nstr = csStrNew (name);
    node->ResortAttrs();
  }
}

static inline bool checkInt (const char* str, int &v)
{
  const char *c = str;
  if (*c == 0) return false;
  while (*c)
  {
    if (!(isdigit (*c)) && (*c != '-')) return false;
    c++;
  }
  sscanf (str, "%d", &v);
  return true;
}

static inline bool checkFloat (const char* str, float &v)
{
  const char *c = str;
  if (*c == 0) return false;
  while (*c)
  {
    if (!(isdigit (*c)) && (*c != '-') && (*c != '.') &&
      (*c != 'e') && (*c != 'E'))
      return false;
    c++;
  }
  char dummy;
  int ret = sscanf (str, "%g%c", &v, &dummy);
  return (ret == 1);
}

void csBinaryDocAttribute::SetValue (const char* val)
{
  if (attrPtr->flags & BD_NODE_MODIFIED)
  {
    delete[] attrPtr->vstr; attrPtr->vstr = 0;
    delete[] vstr; vstr = 0;
    int v;
    float f;
    if (val == 0) val = ""; 
      // "<Jorrit> A null value is equivalent to empty."
    if (checkInt (val, v))
    {
      attrPtr->flags = (attrPtr->flags & ~BD_VALUE_TYPE_MASK) | 
	BD_VALUE_TYPE_INT;
      attrPtr->value = csLittleEndianLong ((long)v);
    }
    else if (checkFloat (val, f))
    {
      attrPtr->flags = (attrPtr->flags & ~BD_VALUE_TYPE_MASK) | 
	BD_VALUE_TYPE_FLOAT;
      attrPtr->value = csLittleEndianLong (csFloatToLong (f));
    }
    else 
    {
      attrPtr->flags = (attrPtr->flags & ~BD_VALUE_TYPE_MASK) | 
	BD_VALUE_TYPE_STR;
      attrPtr->vstr = csStrNew (val);
    }
  }
}

void csBinaryDocAttribute::SetValueAsInt (int v)
{
  if (attrPtr->flags & BD_NODE_MODIFIED)
  {
    delete[] attrPtr->vstr; attrPtr->vstr = 0;
    delete[] vstr; vstr = 0;
    attrPtr->flags = (attrPtr->flags & ~BD_VALUE_TYPE_MASK) | 
      BD_VALUE_TYPE_INT;
    attrPtr->value = csLittleEndianLong ((long)v);
  }
}

void csBinaryDocAttribute::SetValueAsFloat (float f)
{
  if (attrPtr->flags & BD_NODE_MODIFIED)
  {
    delete[] attrPtr->vstr;	attrPtr->vstr = 0;
    delete[] vstr; vstr = 0;
    attrPtr->flags = (attrPtr->flags & ~BD_VALUE_TYPE_MASK) | 
      BD_VALUE_TYPE_FLOAT;
    attrPtr->value = csLittleEndianLong (csFloatToLong (f));
  }
}

void csBinaryDocAttribute::Store (csMemFile* nodesFile)
{
  bdNodeAttribute diskAttr;
  size_t attrSize = sizeof (diskAttr);

  diskAttr.flags = attrPtr->flags & BD_VALUE_TYPE_MASK;
  if ((diskAttr.flags & BD_VALUE_TYPE_MASK) == BD_VALUE_TYPE_STR)
  {
    if (strlen (attrPtr->GetValueStr (node->doc)) < MAX_IMM_ATTR_VALUE_STR)
    {
      diskAttr.flags = 
	(diskAttr.flags & ~BD_VALUE_TYPE_MASK) | BD_VALUE_TYPE_STR_IMMEDIATE;
      diskAttr.value = 0;
      strcpy ((char*)&diskAttr.value, attrPtr->GetValueStr (node->doc));
    }
    else
    {
      diskAttr.value = csLittleEndianLong (
	node->doc->GetOutStringID (attrPtr->GetValueStr (node->doc)));
    }
  }
  else
    diskAttr.value = attrPtr->value;

  bool putFlagsInName = false;
  const size_t nameLen = strlen (attrPtr->GetNameStr (node->doc));
  if (nameLen < MAX_IMM_ATTR_NAME_STR)
  {
    diskAttr.flags |= BD_ATTR_NAME_IMMEDIATE;
    diskAttr.nameID = 0;
    strcpy ((char*)&diskAttr.nameID, attrPtr->GetNameStr (node->doc));
    putFlagsInName = (nameLen < MAX_IMM_ATTR_NAME_STR_W_FLAGS);
  }
  else
  {
    diskAttr.nameID = csLittleEndianLong (
      node->doc->GetOutStringID (attrPtr->GetNameStr (node->doc)));
    // @@@ What to do if nameID > BD_ATTR_MAX_NAME_ID?
    putFlagsInName = 
      csLittleEndianLong (diskAttr.nameID) <= BD_ATTR_MAX_NAME_ID_WITH_FLAGS;
  }
  if (putFlagsInName)
  {
    diskAttr.nameID |= BD_ATTR_FLAGS_IN_NAME;
    diskAttr.nameID |= (diskAttr.flags << BD_ATTR_FLAGS_IN_NAME_SHIFT);
    attrSize -= sizeof (uint32);
  }
  nodesFile->Write ((char*)&diskAttr, attrSize);
}

// =================================================
//  csBinaryDocNodeIterator
// =================================================

SCF_IMPLEMENT_IBASE(csBinaryDocNodeIterator)
  SCF_IMPLEMENTS_INTERFACE(iDocumentNodeIterator)
SCF_IMPLEMENT_IBASE_END

csBinaryDocNodeIterator::csBinaryDocNodeIterator ()
{
  SCF_CONSTRUCT_IBASE (0);
  value = 0;
}

void csBinaryDocNodeIterator::SetTo (csBdNode* node,
 				     csBinaryDocNode* parent,
				     const char* onlyval)
{
  parentNode = parent; 
  pos = 0;
  delete[] value;
  if (onlyval) 
  {
    value = csStrNew (onlyval);
  }
  else
  {
    value = 0;
  }
  if (!(node->flags & BD_NODE_HAS_CHILDREN))
  {
    iteratedNode = 0;
  }
  else
  {
    iteratedNode = node;
  }
  FastForward();
}

csBinaryDocNodeIterator::~csBinaryDocNodeIterator ()
{
  delete[] value;
  SCF_DESTRUCT_IBASE();
}

void csBinaryDocNodeIterator::FastForward()
{
  if (value && iteratedNode)
  {
    while (pos < iteratedNode->ctNum())
    {
      const char* vstr = iteratedNode->ctGetItem (pos)->
        GetValueStr (parentNode->doc); 
      if (strcmp (vstr, value) != 0)
      {
        pos++;
      }
      else
      {
	break;
      }
    }
  }
}

bool csBinaryDocNodeIterator::HasNext ()
{
  return ((iteratedNode) && 
    (pos < iteratedNode->ctNum()));
}

csRef<iDocumentNode> csBinaryDocNodeIterator::Next ()
{
  csBdNode* nodeData = iteratedNode->ctGetItem (pos++);
  csBinaryDocNode* retNode = parentNode->doc->GetPoolNode();
  retNode->SetTo (nodeData, parentNode);
  FastForward();
  return csPtr<iDocumentNode> (retNode);
}

// =================================================
//  csBdNode
// =================================================

csBdNode::csBdNode (uint32 newType)
{
  flags = newType | BD_NODE_MODIFIED;
  value = 0;
  vstr = 0;

  attrs = new csArray<csBdAttr*>;
  nodes = new csArray<csBdNode*>;
}

csBdNode::csBdNode ()
{
  flags = BD_NODE_MODIFIED;
  value = 0;
  vstr = 0;

  attrs = new csArray<csBdAttr*>;
  nodes = new csArray<csBdNode*>;
}

csBdNode::~csBdNode ()
{
  if (flags & BD_NODE_MODIFIED)
  {
    delete[] vstr;
    size_t i;
    for (i = 0; i < attrs->Length(); i++)
      doc->FreeBdAttr (attrs->Get (i));
    delete attrs;
    for (i = 0; i < nodes->Length(); i++)
      doc->FreeBdNode (nodes->Get (i));
    delete nodes;
  }
}

void csBdNode::SetType (uint32 newType)
{
  CS_ASSERT(flags & BD_NODE_MODIFIED);
  flags = (flags & ~BD_NODE_TYPE_MASK) | newType;
}

void csBdNode::SetDoc (csBinaryDocument* doc)
{
  csBdNode::doc = doc;
}

bdNodeChildTab* csBdNode::GetChildTab () const
{
  if ((flags & (BD_NODE_HAS_CHILDREN | BD_NODE_MODIFIED))
    == BD_NODE_HAS_CHILDREN)
  {
    return (bdNodeChildTab*)GetFromOffset (GetChildTabOffset());
  }
  else
    return 0;
}

bdNodeAttrTab* csBdNode::GetAttrTab () const
{
  if ((flags & (BD_NODE_HAS_ATTR | BD_NODE_MODIFIED))
    == BD_NODE_HAS_ATTR)
  {
    return (bdNodeAttrTab*)GetFromOffset (GetAttrTabOffset());
  }
  else
    return 0;
}

const char* csBdNode::GetValueStr (csBinaryDocument* doc) const
{
  if (flags & BD_NODE_MODIFIED)
  {
    return vstr;
  }
  else
  {
    if ((flags & BD_VALUE_TYPE_MASK) == BD_VALUE_TYPE_STR_IMMEDIATE)
    {
      return (char*)&value;
    }
    else
    {
      return doc->GetInIDString (value);
    }
  }
}

csBdAttr* csBdNode::atGetItem(int pos)
{
  if (flags & BD_NODE_MODIFIED)
  {
    return (*attrs)[pos];
  }
  else
  {
    void* at =  (void*)GetAttrTab();
    return (csBdAttr*)((uint8*)at +
      csLittleEndianLong (*((uint32*)at + pos + 1)));
  }
}

void csBdNode::atSetItem (csBdAttr* item, int pos)
{
  if (flags & BD_NODE_MODIFIED)
  {
    attrs->Put (pos, item);
    if (attrs->Length() != 0)
      flags |= BD_NODE_HAS_ATTR;
  }
}

int csBdNode::atItemPos (csBdAttr* item)
{
  uint i;
  for (i = 0; i < atNum(); i++)
  {
    if (atGetItem (i) == item) 
    {
      return i;
    }
  }
  return -1;
}

void csBdNode::atInsertBefore (csBdAttr* item, int pos)
{
  if (flags & BD_NODE_MODIFIED)
  {
    attrs->Insert (pos, item);
    if (attrs->Length() != 0)
      flags |= BD_NODE_HAS_ATTR;
  }
}

void csBdNode::atRemove (int pos)
{
  if (flags & BD_NODE_MODIFIED)
  {
    attrs->DeleteIndex (pos);
    if (attrs->Length() == 0)
      flags &= ~BD_NODE_HAS_ATTR;
  }
}

uint csBdNode::atNum () 
{ 
  if (flags & BD_NODE_MODIFIED)
  {
    return (uint)attrs->Length();
  }
  else
  {
    return GetAttrTab()->num; 
  }
}

csBdNode* csBdNode::ctGetItem(int pos)
{
  if (flags & BD_NODE_MODIFIED)
  {
    return (*nodes)[pos];
  }
  else
  {
    void* ct = (void*)GetChildTab();
    return (csBdNode*)((uint8*)ct +
      csLittleEndianLong (*((uint32*)ct + pos + 1)));
  }
}

void csBdNode::ctSetItem (csBdNode* item, int pos)
{
  if (flags & BD_NODE_MODIFIED)
  {
    nodes->Put (pos, item);
    if (nodes->Length() != 0)
      flags |= BD_NODE_HAS_CHILDREN;
  }
}

int csBdNode::ctItemPos (csBdNode* item)
{
  uint i;
  for (i = 0; i < ctNum(); i++)
  {
    if (ctGetItem (i) == item) 
    {
      return i;
    }
  }
  return -1;
}

void csBdNode::ctInsertBefore (csBdNode* item, int pos)
{
  if (flags & BD_NODE_MODIFIED)
  {
    nodes->Insert (pos, item);
    if (nodes->Length() != 0)
      flags |= BD_NODE_HAS_CHILDREN;
  }
}
  
void csBdNode::ctRemove (int pos)
{
  if (flags & BD_NODE_MODIFIED)
  {
    nodes->DeleteIndex (pos);
    if (nodes->Length() == 0)
      flags &= ~BD_NODE_HAS_CHILDREN;
  }
}

uint csBdNode::ctNum ()
{
  if (flags & BD_NODE_MODIFIED)
  {
    return (uint)nodes->Length();
  }
  else
  {
    return GetChildTab()->num; 
  }
}

// =================================================
//  csBinaryDocNode
// =================================================

void csBinaryDocNode::IncRef ()
{
  scfRefCount++;
}

void csBinaryDocNode::DecRef ()
{
  if (scfRefCount == 1)
  {
    doc->RecyclePoolNode (this);
    return;
  }
  scfRefCount--;
}

SCF_IMPLEMENT_IBASE_GETREFCOUNT(csBinaryDocNode)
SCF_IMPLEMENT_IBASE_REFOWNER(csBinaryDocNode)
SCF_IMPLEMENT_IBASE_REMOVE_REF_OWNERS(csBinaryDocNode)
SCF_IMPLEMENT_IBASE_QUERY(csBinaryDocNode)
  SCF_IMPLEMENTS_INTERFACE(iDocumentNode)
SCF_IMPLEMENT_IBASE_END

csBinaryDocNode::csBinaryDocNode ()
{
  SCF_CONSTRUCT_IBASE (0);
  vstr = 0;
  vsptr = 0;
}

csBinaryDocNode::~csBinaryDocNode ()
{
  CleanData();
  delete vstr;
  SCF_DESTRUCT_IBASE();
}

void csBinaryDocNode::SetTo (csBdNode* ptr,
			     csBinaryDocNode* parent)
{
  CleanData();

  // we have to keep a ref on the parent.
  // RecyclePoolNode() takes care of DecRef()ing.
  PoolNextOrParent = parent;
  if (parent) parent->scfRefCount++;
  nodeData = ptr;
}

csDocumentNodeType csBinaryDocNode::GetType ()
{
  switch (nodeData->flags & BD_NODE_TYPE_MASK)
  {
    case BD_NODE_TYPE_COMMENT:
      return CS_NODE_COMMENT;
    case BD_NODE_TYPE_DECLARATION:
      return CS_NODE_DECLARATION;
    case BD_NODE_TYPE_DOCUMENT:
      return CS_NODE_DOCUMENT;
    case BD_NODE_TYPE_ELEMENT:
      return CS_NODE_ELEMENT;
    case BD_NODE_TYPE_TEXT:
      return CS_NODE_TEXT;
    case BD_NODE_TYPE_UNKNOWN:
      return CS_NODE_UNKNOWN;

    default:
      return CS_NODE_UNKNOWN;
  }
}

bool csBinaryDocNode::Equals (iDocumentNode* other)
{
  return (nodeData == ((csBinaryDocNode*)other)->nodeData);
}

const char* csBinaryDocNode::nodeValueStr (csBdNode* nodeData)
{
  switch (nodeData->flags & BD_VALUE_TYPE_MASK)
  {
    case BD_VALUE_TYPE_STR:
    case BD_VALUE_TYPE_STR_IMMEDIATE:
      {
	return nodeData->GetValueStr(doc);
      }
    case BD_VALUE_TYPE_INT:
      {
	if (vsptr != nodeData)
	{
  	  char buf[50];
	  cs_snprintf (buf, sizeof (buf) - 1, "%" PRId32, 
	    (int32)csLittleEndianLong (nodeData->value));
	  delete[] vstr; 
	  vstr = csStrNew (buf);
	  vsptr = nodeData;
	}
	return vstr;
      }
    case BD_VALUE_TYPE_FLOAT:
      {
	if (vsptr != nodeData)
	{
  	  char buf[50];
	  cs_snprintf (buf, sizeof (buf) - 1, "%g", 
	    csLongToFloat (csLittleEndianLong (nodeData->value)));
	  delete[] vstr; 
	  vstr = csStrNew (buf);
	  vsptr = nodeData;
	}
	return vstr;
      }
    default:
      return 0;
  }
}

int csBinaryDocNode::nodeValueInt (csBdNode* nodeData)
{
  switch (nodeData->flags & BD_VALUE_TYPE_MASK)
  {
    case BD_VALUE_TYPE_STR:
    case BD_VALUE_TYPE_STR_IMMEDIATE:
      {
	if (!nodeData->GetValueStr (doc)) return 0;
	int val = 0;
	sscanf (nodeData->GetValueStr (doc), "%d", &val);
	return val;
      }
    case BD_VALUE_TYPE_INT:
      {
	return (int32)csLittleEndianLong (nodeData->value);
      }
    case BD_VALUE_TYPE_FLOAT:
      {
	return (int)csLongToFloat (csLittleEndianLong (nodeData->value));
      }
    default:
      return 0;
  }
}

float csBinaryDocNode::nodeValueFloat (csBdNode* nodeData)
{
  switch (nodeData->flags & BD_VALUE_TYPE_MASK)
  {
    case BD_VALUE_TYPE_STR:
    case BD_VALUE_TYPE_STR_IMMEDIATE:
      {
	if (!nodeData->GetValueStr (doc)) return 0.0f;
	float val = 0.0f;
	sscanf (nodeData->GetValueStr (doc), "%g", &val);
	return val;
      }
    case BD_VALUE_TYPE_INT:
      {
	return (float)((int32)csLittleEndianLong (nodeData->value));
      }
    case BD_VALUE_TYPE_FLOAT:
      {
	return csLongToFloat (csLittleEndianLong (nodeData->value));
      }
    default:
      return 0.0f;
  }
}

const char* csBinaryDocNode::GetValue ()
{
  return nodeValueStr (nodeData);
}

int csBinaryDocNode::GetValueAsInt ()
{
  return nodeValueInt (nodeData);
}

float csBinaryDocNode::GetValueAsFloat ()
{
  return nodeValueFloat (nodeData);
}

int csBinaryDocNode::IndexOfAttr (const char* attr)
{
  if (!(nodeData->flags & BD_NODE_HAS_ATTR)) return -1;
  int lo = 0, hi = nodeData->atNum() - 1;
  int mid = 0;
  while (lo <= hi)
  {
    mid = (lo + hi) / 2;
    int c = strcmp (attr, nodeData->atGetItem (mid)->GetNameStr (doc));
    if (c == 0)
    {
      return mid;
    }
    else if (c > 0)
    {
      lo = mid + 1;
    }
    else
    {
      hi = mid - 1;
    }
  }
  return -1;
}

void csBinaryDocNode::CleanData ()
{
}

void csBinaryDocNode::ResortAttrs()
{
  ResortAttrs (0, nodeData->atNum());
}

void csBinaryDocNode::ResortAttrs (int Left, int Right)
{
  int i = Left, j = Right;
  int x = (Left + Right) / 2;
  do
  {
    const char* nxn = nodeData->atGetItem(x)->GetNameStr(doc);
    while ((i != x) && (strcmp (nodeData->atGetItem(i)->GetNameStr(doc),
      nxn) < 0))
      i++;
    while ((j != x) && (strcmp (nodeData->atGetItem(j)->GetNameStr(doc),
      nxn) > 0))
      j--;
    if (i < j)
    {
      csBdAttr* t;
      t = nodeData->atGetItem (i);
      nodeData->atSetItem (nodeData->atGetItem (j), i);
      nodeData->atSetItem (t, j);
      if (x == i)
        x = j;
      else if (x == j)
        x = i;
    }
    if (i <= j)
    {
      i++;
      if (j > Left)
        j--;
    }
  } while (i <= j);

  if (j - Left < Right - i)
  {
    if (Left < j)
      ResortAttrs (Left, j);
    if (i < Right)
    {
      ResortAttrs (i, Right);
    }
  }
  else
  {
    if (i < Right)
      ResortAttrs (i, Right);
    if (Left < j)
    {
      ResortAttrs (Left, j);
    }
  }
}

void csBinaryDocNode::SetValue (const char* value)
{
  if (nodeData->flags & BD_NODE_MODIFIED)
  {
    delete[] vstr; vstr = 0;
    delete[] nodeData->vstr; nodeData->vstr = 0;
    int v;
    float f;
    if (value == 0) value = ""; 
      // "<Jorrit> A null value is equivalent to empty."
    if (checkInt (value, v))
    {
      nodeData->flags = (nodeData->flags & ~BD_VALUE_TYPE_MASK) | 
	BD_VALUE_TYPE_INT;
      nodeData->value = csLittleEndianLong (v);
    }
    else if (checkFloat (value, f))
    {
      nodeData->flags = (nodeData->flags & ~BD_VALUE_TYPE_MASK) | 
	BD_VALUE_TYPE_FLOAT;
      nodeData->value = csLittleEndianLong (csFloatToLong (f));
    }
    else 
    {
      nodeData->flags = (nodeData->flags & ~BD_VALUE_TYPE_MASK) | 
	BD_VALUE_TYPE_STR;
      nodeData->vstr = csStrNew (value);
    }
  }
}

void csBinaryDocNode::SetValueAsInt (int value)
{
  if (nodeData->flags & BD_NODE_MODIFIED)
  {
    delete[] vstr; vstr = 0;
    nodeData->flags = (nodeData->flags & ~BD_VALUE_TYPE_MASK) |
      BD_VALUE_TYPE_INT;
    nodeData->value = csLittleEndianLong ((int32)value);
  }
}

void csBinaryDocNode::SetValueAsFloat (float value)
{
  if (nodeData->flags & BD_NODE_MODIFIED)
  {
    delete[] vstr; vstr = 0;
    nodeData->flags = (nodeData->flags & ~BD_VALUE_TYPE_MASK) | 
      BD_VALUE_TYPE_FLOAT;
    nodeData->value = csLittleEndianLong (csFloatToLong (value));
  }
}

csRef<iDocumentNode> csBinaryDocNode::GetParent ()
{
  return (iDocumentNode*) (PoolNextOrParent);
}

csRef<iDocumentNodeIterator> csBinaryDocNode::GetNodes ()
{
  csBinaryDocNodeIterator* it = new csBinaryDocNodeIterator ();
  it->SetTo (nodeData, this);
  return csPtr<iDocumentNodeIterator> (it);
}

csRef<iDocumentNodeIterator> csBinaryDocNode::GetNodes (const char* value)
{
  csBinaryDocNodeIterator* it = new csBinaryDocNodeIterator ();
  it->SetTo (nodeData, this, value);
  return csPtr<iDocumentNodeIterator> (it);
}

csRef<iDocumentNode> csBinaryDocNode::GetNode (const char* value)
{
  if (nodeData->flags & BD_NODE_HAS_CHILDREN)
  {
    uint i;
    for (i = 0; i < nodeData->ctNum(); i++)
    {
      csBdNode* nodeData = csBinaryDocNode::nodeData->ctGetItem (i);
      if (strcmp (nodeData->GetValueStr (doc), value) == 0)
      {
	csBinaryDocNode* node = doc->GetPoolNode();
	node->SetTo (nodeData, this);
	return csPtr<iDocumentNode> (node);
      }
    }
  }
  return 0;
}

void csBinaryDocNode::RemoveNode (const csRef<iDocumentNode>& child)
{
  if (nodeData->flags & BD_NODE_MODIFIED)
  {
    csBdNode* nd = ((csRef<csBinaryDocNode>&)child)->nodeData;
    int pos = nodeData->ctItemPos (nd);
    if (pos != -1) nodeData->ctRemove (pos);
  }
}

void csBinaryDocNode::RemoveNodes ()
{
  if (nodeData->flags & BD_NODE_MODIFIED)
  {
    while (nodeData->ctNum() > 0)
    {
      nodeData->ctRemove (0);
    }
  }
}

csRef<iDocumentNode> csBinaryDocNode::CreateNodeBefore (csDocumentNodeType type,
      iDocumentNode* before)
{
  if (nodeData->flags & BD_NODE_MODIFIED)
  {
    uint32 newType;
    switch (type)
    {
      case CS_NODE_COMMENT:
	newType = BD_NODE_TYPE_COMMENT;
	break;
      case CS_NODE_DECLARATION:
	newType = BD_NODE_TYPE_DECLARATION;
	break;
      case CS_NODE_ELEMENT:
	newType = BD_NODE_TYPE_ELEMENT;
	break;
      case CS_NODE_TEXT:
	newType = BD_NODE_TYPE_TEXT;
	break;
      case CS_NODE_UNKNOWN:
	newType = BD_NODE_TYPE_UNKNOWN;
	break;

      default:
	return 0;
    }

    csBdNode* newData = doc->AllocBdNode ();
    newData->SetType (newType);
    newData->SetDoc (doc);

    int oldChildCount = nodeData->ctNum();

    int newPos = -1;
    if (oldChildCount && before)
    {
      newPos = nodeData->ctItemPos (
	((csBinaryDocNode*)before)->nodeData);
    }
    if (newPos < 0) newPos = oldChildCount;
    nodeData->ctInsertBefore (newData, newPos);

    csBinaryDocNode* newNode = doc->GetPoolNode ();
    newNode->SetTo (newData, this);

    return csPtr<iDocumentNode> (newNode);
  }
  else
  {
    return 0;
  }
}

const char* csBinaryDocNode::GetContentsValue ()
{
  if (nodeData->flags & BD_NODE_HAS_CHILDREN)
  {
    uint i;
    for (i = 0; i < nodeData->ctNum(); i++)
    {
      csBdNode* nodeData = csBinaryDocNode::nodeData->ctGetItem (i);
      if ((nodeData->flags & BD_NODE_TYPE_MASK) == 
        BD_NODE_TYPE_TEXT)
      {
	return nodeValueStr (nodeData);
      }
    }
  }
  return 0;
}

int csBinaryDocNode::GetContentsValueAsInt ()
{
  if (nodeData->flags & BD_NODE_HAS_CHILDREN)
  {
    uint i;
    for (i = 0; i < nodeData->ctNum(); i++)
    {
      csBdNode* nodeData = csBinaryDocNode::nodeData->ctGetItem (i);
      if ((nodeData->flags & BD_NODE_TYPE_MASK) == 
        BD_NODE_TYPE_TEXT)
      {
	return nodeValueInt (nodeData);
      }
    }
  }
  return 0;
}

float csBinaryDocNode::GetContentsValueAsFloat ()
{
  if (nodeData->flags & BD_NODE_HAS_CHILDREN)
  {
    uint i;
    for (i = 0; i < nodeData->ctNum(); i++)
    {
      csBdNode* nodeData = csBinaryDocNode::nodeData->ctGetItem (i);
      if ((nodeData->flags & BD_NODE_TYPE_MASK) == 
        BD_NODE_TYPE_TEXT)
      {
	return nodeValueFloat (nodeData);
      }
    }
  }
  return 0.0f;
}

csRef<iDocumentAttributeIterator> csBinaryDocNode::GetAttributes ()
{
  csBinaryDocAttributeIterator* it = new csBinaryDocAttributeIterator ();
  it->SetTo (nodeData, this);
  return csPtr<iDocumentAttributeIterator> (it);
}

csRef<iDocumentAttribute> csBinaryDocNode::GetAttribute (const char* name)
{
  int index = IndexOfAttr (name);
  if (index < 0)
  {
    if (nodeData->flags & BD_NODE_MODIFIED)
    {
      csBdAttr* newAttr = doc->AllocBdAttr ();
      newAttr->SetName (name);
      int newpos = 0;
      if (nodeData->flags & BD_NODE_HAS_ATTR)
      {
	int lo = 0, hi = nodeData->atNum() - 1;
	int c, mid = 0;
	while (lo <= hi)
	{
	  mid = (lo + hi) / 2;
	  c = strcmp (name, nodeData->atGetItem (mid)->GetNameStr (doc));
	  if (c > 0)
	  {
	    lo = mid + 1;
	  }
	  else
	  {
	    hi = mid - 1;
	  }
	}
	newpos = lo;
      }
      nodeData->atInsertBefore (newAttr, newpos);

      csBinaryDocAttribute* attr = doc->GetPoolAttr ();
      attr->SetTo (newAttr, this);
      return csPtr<iDocumentAttribute> (attr);
    }
    else
    {
      return 0;
    }
  }
  else
  {
    csBdAttr* ptr = nodeData->atGetItem (index);
    csBinaryDocAttribute* attr = doc->GetPoolAttr ();
    attr->SetTo (ptr, this);
    return csPtr<iDocumentAttribute> (attr);
  }

  return 0;
}

const char* csBinaryDocNode::GetAttributeValue (const char* name)
{
  csRef<iDocumentAttribute> attr = GetAttribute (name);
  if (attr)
  {
    return attr->GetValue ();
  }
  else
  {
    return 0;
  }
}

int csBinaryDocNode::GetAttributeValueAsInt (const char* name)
{
  csRef<iDocumentAttribute> attr = GetAttribute (name);
  if (attr)
  {
    return attr->GetValueAsInt ();
  }
  else
  {
    return 0;
  }
}

float csBinaryDocNode::GetAttributeValueAsFloat (const char* name)
{
  csRef<iDocumentAttribute> attr = GetAttribute (name);
  if (attr)
  {
    return attr->GetValueAsFloat ();
  }
  else
  {
    return 0.0f;
  }
}

bool csBinaryDocNode::GetAttributeValueAsBool (const char* name,
					       bool defaultvalue)
{
  csRef<iDocumentAttribute> attr = GetAttribute (name);
  if (attr)
  {
    return attr->GetValueAsBool ();
  }
  else
  {
    return defaultvalue;
  }
}

void csBinaryDocNode::RemoveAttribute (const csRef<iDocumentAttribute>& attr)
{
  if (nodeData->flags & BD_NODE_MODIFIED)
  {
    csBdAttr* ad = ((csRef<csBinaryDocAttribute>&)attr)->attrPtr;
    int pos = nodeData->atItemPos (ad);
    if (pos != -1) nodeData->atRemove (pos);
  }
}

void csBinaryDocNode::RemoveAttributes ()
{
  if (nodeData->flags & BD_NODE_MODIFIED)
  {
    while (nodeData->atNum() > 0)
    {
      nodeData->atRemove (0);
    }
  }
}

void csBinaryDocNode::SetAttribute (const char* name, const char* value)
{
  if (nodeData->flags & BD_NODE_MODIFIED)
  {
    csRef<iDocumentAttribute> attr = GetAttribute (name);
    attr->SetValue (value);
  }
}

void csBinaryDocNode::SetAttributeAsInt (const char* name, int value)
{
  if (nodeData->flags & BD_NODE_MODIFIED)
  {
    csRef<iDocumentAttribute> attr = GetAttribute (name);
    attr->SetValueAsInt (value);
  }
}

void csBinaryDocNode::SetAttributeAsFloat (const char* name, float value)
{
  if (nodeData->flags & BD_NODE_MODIFIED)
  {
    csRef<iDocumentAttribute> attr = GetAttribute (name);
    attr->SetValueAsFloat (value);
  }
}

void csBinaryDocNode::Store (csMemFile* nodesFile)
{
  bdNode diskNode;
  size_t nodeSize = sizeof(bdNode);

  memcpy (&diskNode, nodeData, nodeSize);
  diskNode.flags &= ~(BD_NODE_HAS_ATTR | 
    BD_NODE_HAS_CHILDREN);
  diskNode.flags &= BD_DISK_FLAGS_MASK;

  if ((nodeData->flags & BD_VALUE_TYPE_MASK) == BD_VALUE_TYPE_STR) 
  {
    unsigned int maximmvalue = MAX_IMM_NODE_VALUE_STR;
    const uint32 newFlags = 
	(diskNode.flags & ~BD_VALUE_TYPE_MASK) | BD_VALUE_TYPE_STR_IMMEDIATE;
    // Hack: cram one more byte into value, if possible
    if ((newFlags & BE (0xff)) == 0) maximmvalue++;
    if (strlen (nodeData->vstr) < maximmvalue)
    {
      diskNode.value = 0;
      strcpy ((char*)&diskNode.value, nodeData->vstr);
      diskNode.flags = (diskNode.flags & ~BD_VALUE_TYPE_MASK) | 
	BD_VALUE_TYPE_STR_IMMEDIATE;
    }
    else
    {
      diskNode.value = csLittleEndianLong (
	doc->GetOutStringID (nodeData->vstr));
    }
  }

  if (nodeData->flags & BD_NODE_HAS_ATTR)
  {
    diskNode.flags |= BD_NODE_HAS_ATTR;
  }

  if (nodeData->flags & BD_NODE_HAS_CHILDREN)
  {
    diskNode.flags |= BD_NODE_HAS_CHILDREN;
  }

  size_t nodeStart = nodesFile->GetPos ();
  nodesFile->Write ((char*)&diskNode, nodeSize);

  uint32 scratchSize = 0;
  if (nodeData->flags & BD_NODE_HAS_ATTR) scratchSize = nodeData->atNum();
  if (nodeData->flags & BD_NODE_HAS_CHILDREN)
    scratchSize = MAX(scratchSize, nodeData->ctNum());
    
  CS_ALLOC_STACK_ARRAY(uint32, startsScratch, scratchSize);
  
  size_t attrStart = 0;
  uint32 attrCount;
  if (nodeData->flags & BD_NODE_HAS_ATTR)
  {
    attrStart = nodesFile->GetPos();
    attrCount = nodeData->atNum();
    nodesFile->Write ((char*)&attrCount, sizeof(uint32));
    nodesFile->Write ((char*)startsScratch, sizeof(uint32) * attrCount);
  }

  size_t childStart = 0;
  uint32 childCount;
  if (nodeData->flags & BD_NODE_HAS_CHILDREN)
  {
    childStart = nodesFile->GetPos();
    childCount = nodeData->ctNum();
    nodesFile->Write ((char*)&childCount, sizeof(uint32));
    nodesFile->Write ((char*)startsScratch, sizeof(uint32) * childCount);
  }

  if (nodeData->flags & BD_NODE_HAS_ATTR)
  {
    unsigned int i;
    csRef<csBinaryDocAttribute> attr;
    for (i = 0; i < attrCount; i++)
    {
      startsScratch[i] = csLittleEndianLong ((uint32)nodesFile->GetPos() - attrStart);
      attr.AttachNew (doc->GetPoolAttr ());
      attr->SetTo (nodeData->atGetItem (i), this);
      attr->Store (nodesFile);
    }
    size_t attrEnd = nodesFile->GetPos ();
    nodesFile->SetPos (attrStart + sizeof(uint32));
    nodesFile->Write ((char*)startsScratch, sizeof(uint32) * attrCount);
    nodesFile->SetPos (attrEnd);
  }

  if (nodeData->flags & BD_NODE_HAS_CHILDREN)
  {
    unsigned int i;
    csRef<csBinaryDocNode> node;
    for (i = 0; i < childCount; i++)
    {
      startsScratch[i] = csLittleEndianLong ((uint32)nodesFile->GetPos() - childStart);
      node.AttachNew (doc->GetPoolNode());
      node->SetTo (nodeData->ctGetItem (i), this);
      node->Store (nodesFile);
    }
    size_t childEnd = nodesFile->GetPos ();
    nodesFile->SetPos (childStart + sizeof(uint32));
    nodesFile->Write ((char*)startsScratch, sizeof(uint32) * childCount);
    nodesFile->SetPos (childEnd);
  }    

  if (diskNode.flags & (BD_NODE_HAS_ATTR | BD_NODE_HAS_CHILDREN))
  {
    size_t nodeEnd = nodesFile->GetPos();
    nodesFile->SetPos (nodeStart);
    nodesFile->Write ((char*)&diskNode, nodeSize);
    nodesFile->SetPos (nodeEnd);
  }
}

// =================================================
//  csBinaryDocument
// =================================================

SCF_IMPLEMENT_IBASE(csBinaryDocument)
  SCF_IMPLEMENTS_INTERFACE(iDocument)
SCF_IMPLEMENT_IBASE_END

csBinaryDocument::csBinaryDocument ()
{
  SCF_CONSTRUCT_IBASE (0);
  nodePool = 0;
  attrPool = 0;
  root = 0;
  outStrHash = 0;
  attrAlloc = 0;
  nodeAlloc = 0;
}

csBinaryDocument::~csBinaryDocument ()
{
  if (root && (root->flags & BD_NODE_MODIFIED))
    delete root;
  while (nodePool)
  {
    csBinaryDocNode *node = nodePool;
    nodePool = node->PoolNextOrParent;
    delete node;
  }
  while (attrPool)
  {
    csBinaryDocAttribute *attr = attrPool;
    attrPool = attr->pool_next;
    delete attr;
  }
  delete attrAlloc;
  delete nodeAlloc;
  SCF_DESTRUCT_IBASE();
}

csBdAttr* csBinaryDocument::AllocBdAttr ()
{
  if (!attrAlloc) attrAlloc = new csBlockAllocator<csBdAttr> (2000);
  return attrAlloc->Alloc();
}

void csBinaryDocument::FreeBdAttr (csBdAttr* attr)
{
  CS_ASSERT(attrAlloc);
  attrAlloc->Free (attr);
}

csBdNode* csBinaryDocument::AllocBdNode ()
{
  if (!nodeAlloc) nodeAlloc = new csBlockAllocator<csBdNode> (2000);
  return nodeAlloc->Alloc();
}

void csBinaryDocument::FreeBdNode (csBdNode* node)
{
  CS_ASSERT(nodeAlloc);
  nodeAlloc->Free (node);
}

csBinaryDocNode* csBinaryDocument::GetPoolNode ()
{
  if (nodePool)
  {
    csBinaryDocNode *node = nodePool;
    nodePool = nodePool->PoolNextOrParent;
    node->doc = this;
    // the node has to keep a ref on us. But to avoid
    // a virtual function call, we increase our refcount
    // 'manually'.
    scfRefCount++;
    return node;
  }
  else
  {
    csBinaryDocNode *node = new csBinaryDocNode ();
    node->doc = this;
    scfRefCount++;
    return node;
  }
}

void csBinaryDocument::RecyclePoolNode (csBinaryDocNode *node)
{
  csBinaryDocNode *parent;
  parent = node->PoolNextOrParent;
  node->PoolNextOrParent = nodePool;
  nodePool = node;
  if (parent)
  {
    if (parent->scfRefCount == 1)
    {
      RecyclePoolNode (parent);
    }
    else
      parent->scfRefCount--;
  }
  // DecRef us.
  if (scfRefCount == 1)
  {
    delete this;
    return;
  }
  scfRefCount--;
}

csBinaryDocAttribute* csBinaryDocument::GetPoolAttr ()
{
  if (attrPool)
  {
    csBinaryDocAttribute* attr = attrPool;
    attrPool = attrPool->pool_next;
    return attr;
  }
  else
  {
    csBinaryDocAttribute* attr = new csBinaryDocAttribute ();
    return attr;
  }
}

void csBinaryDocument::RecyclePoolAttr (csBinaryDocAttribute *attr)
{
  attr->pool_next = attrPool;
  attrPool = attr;
}

csBinaryDocNode* csBinaryDocument::GetRootNode ()
{
  csBinaryDocNode* rootNode;
  rootNode = GetPoolNode();
  rootNode->SetTo (root, 0);
  return rootNode;
}

uint32 csBinaryDocument::GetOutStringID (const char* str)
{
  CS_ASSERT (outStrStorage);
  CS_ASSERT (outStrHash);
  if (str == 0) return BD_OFFSET_INVALID;
  csStringID val;
  val = outStrHash->Request (str);
  if (val == csInvalidStringID)
  {
    val = (csStringID)(outStrStorage->GetPos() - outStrTabOfs);
    outStrStorage->Write (str, strlen (str) + 1);
    outStrHash->Register (str, val);
  }
  return val;
}

const char* csBinaryDocument::GetInIDString (uint32 ID) const
{
  if (ID == BD_OFFSET_INVALID) return 0;
  return (char*)(dataStart + inStrTabOfs + 
    ID);
}

void csBinaryDocument::Clear ()
{
  if (root && (root->flags & BD_NODE_MODIFIED))
    delete root;
  data = 0; 
  dataStart = 0;
  root = 0;
}

csRef<iDocumentNode> csBinaryDocument::CreateRoot ()
{
  Clear();
  root = new csBdNode (BD_NODE_TYPE_DOCUMENT);
  root->SetDoc (this);
  return csPtr<iDocumentNode> (GetRootNode ());
}

csRef<iDocumentNode> csBinaryDocument::GetRoot ()
{
  if (!root) 
  {
    root = new csBdNode (BD_NODE_TYPE_DOCUMENT);
    root->SetDoc (this);
  }
  return csPtr<iDocumentNode> (GetRootNode ());
}

const char* csBinaryDocument::Parse (iFile* file, bool collapse)
{
  csRef<iDataBuffer> newBuffer = csPtr<iDataBuffer> 
    (file->GetAllData());
  return Parse (newBuffer, collapse);
}

const char* csBinaryDocument::Parse (iDataBuffer* buf, bool /* collapse */)
{
  if (buf->GetSize() < sizeof (bdHeader) + sizeof (bdDocument))
  {
    return "Not enough data";
  }
  bdHeader *head = (bdHeader*)buf->GetData();
  if (head->magic != (uint32)BD_HEADER_MAGIC)
  {
    return "Not a binary CS document";
  }
  if (head->size != buf->GetSize())
  {
    return "Size mismatch";
  }
  bdDocument *bdDoc = (bdDocument*)(buf->GetUint8() +  sizeof(bdHeader));
  if (bdDoc->ofsRoot == BD_OFFSET_INVALID)
  {
    return "No root node";
  }
  
  Clear();
  root = 0;
  data = buf;
  dataStart = data->GetUint8();

  inStrTabOfs =  sizeof(bdHeader) + bdDoc->ofsStr;

  root = (csBdNode*)((uint8*)bdDoc + bdDoc->ofsRoot);

  return 0;
}

const char* csBinaryDocument::Parse (iString* str, bool collapse)
{
  csRef<csDataBuffer> newBuffer = csPtr<csDataBuffer>
    (new csDataBuffer(str->Length()));
  memcpy (newBuffer->GetData(), str->GetData(),
    str->Length());
  return Parse (newBuffer, collapse);
}

const char* csBinaryDocument::Parse (const char* buf, bool collapse)
{
  csRef<csDataBuffer> newBuffer = csPtr<csDataBuffer>
    (new csDataBuffer(strlen (buf)));
  memcpy (newBuffer->GetData(), buf, strlen (buf));
  return Parse (newBuffer, collapse);
}

const char* csBinaryDocument::Write (iFile* out)
{
  bdHeader head;
  head.magic = BD_HEADER_MAGIC;

  out->Write ((char*)&head, sizeof (head));

  size_t docStart = out->GetPos();
  bdDocument doc;
  out->Write ((char*)&doc, sizeof (doc));

  outStrStorage = out;
  outStrHash = new csStringHash (431);
  doc.ofsStr = (uint32)out->GetPos();
  {
    int pad = (4 - doc.ofsStr) & 3;
    if (pad != 0)
    {
      // align to 4 byte boundary, to avoid problems
      char null[4] = {0, 0, 0, 0};
      out->Write (null, pad);
      doc.ofsStr += pad;
    }
    doc.ofsStr -= (uint32)docStart;
  }
  doc.ofsStr = csLittleEndianLong (doc.ofsStr);
  outStrTabOfs = (uint32)out->GetPos();

  csMemFile* outNodes = new csMemFile;
  if (root)
  {
    csRef<csBinaryDocNode> rootNode;
    rootNode.AttachNew (GetRootNode());
    rootNode->Store (outNodes);
  }
  else
  {
    doc.ofsRoot = BD_OFFSET_INVALID;
  }
  delete outStrHash; 
  outStrHash = 0;

  doc.ofsRoot = (uint32)out->GetPos();
  {
    int pad = (4 - doc.ofsRoot) & 3;
    if (pad != 0)
    {
      // align to 4 byte boundary, to avoid problems
      char null[4] = {0, 0, 0, 0};
      out->Write (null, pad);
      doc.ofsRoot += pad;
    }
    doc.ofsRoot -= (uint32)docStart;
  }
  doc.ofsRoot = csLittleEndianLong (doc.ofsRoot);
  out->Write (outNodes->GetData(), outNodes->GetSize());
  delete outNodes;

  head.size = (uint32)out->GetSize();
  out->SetPos (0);
  out->Write ((char*)&head, sizeof (head));
  out->Write ((char*)&doc, sizeof (doc));

  return 0;
}

const char* csBinaryDocument::Write (iString* str)
{
  csMemFile temp;

  const char* ret = Write (&temp);
  str->Clear();
  str->Append (temp.GetData(), temp.GetSize());

  return ret;
}

const char* csBinaryDocument::Write (iVFS* vfs, const char* filename)
{
  csMemFile temp;

  const char* ret = Write (&temp);
  vfs->WriteFile (filename, temp.GetData(), temp.GetSize());

  return ret;
}

int csBinaryDocument::Changeable ()
{
  if (!root || (root->flags & BD_NODE_MODIFIED))
  {
    return CS_CHANGEABLE_YES;
  }
  else
  {
    return CS_CHANGEABLE_NEWROOT;
  }
}
