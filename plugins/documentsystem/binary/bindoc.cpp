/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"

#include "cssys/csendian.h"
#include "iutil/document.h"
#include "iutil/databuff.h"
#include "iutil/vfs.h"
#include "iutil/string.h"
#include "csutil/databuf.h"
#include "csutil/csstring.h"
#include "csutil/util.h"
#include "csutil/memfile.h"
#include "csutil/hashmap.h"
#include "csutil/hashmapr.h"

#include "bindoc.h"

// =================================================
//  csBinaryDocAttributeIterator
// =================================================

SCF_IMPLEMENT_IBASE(csBinaryDocAttributeIterator)
  SCF_IMPLEMENTS_INTERFACE(iDocumentAttributeIterator)
SCF_IMPLEMENT_IBASE_END

csBinaryDocAttributeIterator::csBinaryDocAttributeIterator (
    csBinaryDocNode *node)
{
  SCF_CONSTRUCT_IBASE (NULL);

  csBinaryDocAttributeIterator::node = node;
  pos = 0;
}

csBinaryDocAttributeIterator::~csBinaryDocAttributeIterator()
{
}

bool csBinaryDocAttributeIterator::HasNext ()
{
  return ((node->attributes) && (pos < node->attributes->Length()));
}

csRef<iDocumentAttribute> csBinaryDocAttributeIterator::Next ()
{
  return (iDocumentAttribute*) (*node->attributes)[pos++];
}

// =================================================
//  csBinaryDocAttribute
// =================================================

SCF_IMPLEMENT_IBASE(csBinaryDocAttribute)
  SCF_IMPLEMENTS_INTERFACE(iDocumentAttribute)
SCF_IMPLEMENT_IBASE_END

csBinaryDocAttribute::csBinaryDocAttribute (csBinaryDocument* document,
					    csBinaryDocNode* node,
					    uint32 attrType)
{
  SCF_CONSTRUCT_IBASE (NULL);

  doc = document;

  flags = attrType | BD_NODE_MODIFIED;
  name = NULL;
  value = 0;
  vstr = NULL;
  csBinaryDocAttribute::node = node;
}

csBinaryDocAttribute::csBinaryDocAttribute (csBinaryDocument* document,
    csBinaryDocNode* node,
    bdNodeAttribute* data)
{
  SCF_CONSTRUCT_IBASE (NULL);

  doc = document;
  flags = data->flags;
  if (flags & BD_ATTR_NAME_IMMEDIATE)
  {
    name = (char*)&data->nameID;
  }
  else
  {
    name = doc->GetInIDString (
      little_endian_long (data->nameID));
  }
  value = data->value;
  if ((flags & BD_VALUE_TYPE_MASK) == BD_VALUE_TYPE_STR)
  {
    if (flags & BD_VALUE_STR_IMMEDIATE)
    {
      vstr = (char*)&value;
    }
    else
    {
      vstr = doc->GetInIDString (little_endian_long (value));
    }
  }
  else
  {
    vstr = NULL;
  }
  csBinaryDocAttribute::node = node;
}

csBinaryDocAttribute::~csBinaryDocAttribute ()
{
  if ((flags & (BD_NODE_MODIFIED | BD_NODE_TYPE_MASK)) !=
    BD_VALUE_TYPE_STR)
  {
    delete[] name;
  }
  if ((flags & (BD_NODE_MODIFIED | BD_NODE_TYPE_MASK)) !=
    BD_VALUE_TYPE_STR)
  {
    delete[] vstr;
  }
}

void csBinaryDocAttribute::Modify ()
{
  node->Modify ();
}

const char* csBinaryDocAttribute::GetName ()
{
  return name;
}

const char* csBinaryDocAttribute::GetValue ()
{
  switch (flags & BD_VALUE_TYPE_MASK)
  {
    case BD_VALUE_TYPE_STR:
      {
	return vstr;
      }
    case BD_VALUE_TYPE_INT:
      {
	if (!vstr)
	{
	  vstr = csStrNew (csString().Format ("%d", 
	    little_endian_long (value)));
	}
	return vstr;
      }
    case BD_VALUE_TYPE_FLOAT:
      {
	if (!vstr)
	{
	  vstr = csStrNew (csString().Format ("%g", 
	    long2float (little_endian_long (value))));
	}
	return vstr;
      }
    default:
      return NULL;
  }
}

int csBinaryDocAttribute::GetValueAsInt ()
{
  switch (flags & BD_VALUE_TYPE_MASK)
  {
    case BD_VALUE_TYPE_STR:
      {
	if (!vstr) return 0;
	int val = 0;
	sscanf (vstr, "%d", &val);
	return val;
      }
    case BD_VALUE_TYPE_INT:
      {
	return little_endian_long (value);
      }
    case BD_VALUE_TYPE_FLOAT:
      {
	return (int)long2float (little_endian_long (value));
      }
    default:
      return 0;
  }
}

float csBinaryDocAttribute::GetValueAsFloat ()
{
  switch (flags & BD_VALUE_TYPE_MASK)
  {
    case BD_VALUE_TYPE_STR:
      {
	if (!vstr) return 0.0f;
	float val = 0.0f;
	sscanf (vstr, "%g", &val);
	return val;
      }
    case BD_VALUE_TYPE_INT:
      {
	return (float)little_endian_long (value);
      }
    case BD_VALUE_TYPE_FLOAT:
      {
	return long2float (little_endian_long (value));
      }
    default:
      return 0.0f;
  }
}

void csBinaryDocAttribute::SetName (const char* name)
{
  Modify ();
  delete[] csBinaryDocAttribute::name;
  csBinaryDocAttribute::name = csStrNew (name);
}

static inline bool checkInt (const char* str, int &v)
{
  const char *c = str;
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
  while (*c)
  {
    if (!(isdigit (*c)) && (*c != '-') && (*c != '.') && (*c != 'e'))
      return false;
    c++;
  }
  sscanf (str, "%f", &v);
  return true;
}

void csBinaryDocAttribute::SetValue (const char* val)
{
  Modify ();
  delete vstr;
  int v;
  float f;
  if (checkInt (val, v))
  {
    flags = (flags & ~BD_VALUE_TYPE_MASK) | BD_VALUE_TYPE_INT;
    value = little_endian_long (v);
  }
  else if (checkFloat (val, f))
  {
    flags = (flags & ~BD_VALUE_TYPE_MASK) | BD_VALUE_TYPE_FLOAT;
    value = little_endian_long (float2long (f));
  }
  else 
  {
    flags = (flags & ~BD_VALUE_TYPE_MASK) | BD_VALUE_TYPE_STR;
    vstr = csStrNew (val);
  }
}

void csBinaryDocAttribute::SetValueAsInt (int v)
{
  Modify ();
  delete vstr;
  flags = (flags & ~BD_VALUE_TYPE_MASK) | BD_VALUE_TYPE_INT;
  value = little_endian_long (v);
}

void csBinaryDocAttribute::SetValueAsFloat (float f)
{
  Modify ();
  delete vstr;
  flags = (flags & ~BD_VALUE_TYPE_MASK) | BD_VALUE_TYPE_FLOAT;
  value = little_endian_long (float2long (f));
}

void csBinaryDocAttribute::Store (csMemFile* nodesFile)
{
  bdNodeAttribute diskAttr;

  diskAttr.flags = flags & BD_VALUE_TYPE_MASK;
  if (strlen (name) < 4)
  {
    diskAttr.flags |= BD_ATTR_NAME_IMMEDIATE;
    diskAttr.nameID = 0;
    strcpy ((char*)&diskAttr.nameID, name);
  }
  else
  {
    diskAttr.nameID = little_endian_long (
      doc->GetOutStringID (name));
  }
  if ((diskAttr.flags & BD_VALUE_TYPE_MASK) == BD_VALUE_TYPE_STR)
  {
    if (strlen (vstr) < 4)
    {
      diskAttr.flags |= BD_VALUE_STR_IMMEDIATE;
      diskAttr.value = 0;
      strcpy ((char*)&diskAttr.value, vstr);
    }
    else
    {
      diskAttr.value = little_endian_long (
	doc->GetOutStringID (vstr));
    }
  }
  else
    diskAttr.value = value;
  nodesFile->Write ((char*)&diskAttr, sizeof (diskAttr));
}

// =================================================
//  csBinaryDocNodeIterator
// =================================================

SCF_IMPLEMENT_IBASE(csBinaryDocNodeIterator)
  SCF_IMPLEMENTS_INTERFACE(iDocumentNodeIterator)
SCF_IMPLEMENT_IBASE_END

csBinaryDocNodeIterator::csBinaryDocNodeIterator (
    csBinaryDocNode *node, const char* onlyval)
{
  SCF_CONSTRUCT_IBASE (NULL);

  csBinaryDocNodeIterator::node = node;
  pos = 0;
  if (onlyval) 
  {
    value = csStrNew (onlyval);
  }
  else
  {
    value = NULL;
  }
  FastForward();
}

csBinaryDocNodeIterator::~csBinaryDocNodeIterator ()
{
  delete[] value;
}

void csBinaryDocNodeIterator::FastForward()
{
  if (value && node->children)
  {
    while ((pos < node->children->Length()) &&
      (strcmp ((*node->children)[pos]->GetValue(),
      value) == 0))
    {
      pos++;
    }
  }
}

bool csBinaryDocNodeIterator::HasNext ()
{
  return ((node->children) && 
    (pos < node->children->Length()));
}

csRef<iDocumentNode> csBinaryDocNodeIterator::Next ()
{
  csRef<iDocumentNode> retNode = (iDocumentNode*) ((*node->children)[pos++]);
  FastForward();
  return retNode;
}

// =================================================
//  csBinaryDocNode
// =================================================

SCF_IMPLEMENT_IBASE(csBinaryDocNode)
  SCF_IMPLEMENTS_INTERFACE(iDocumentNode)
SCF_IMPLEMENT_IBASE_END

csBinaryDocNode::csBinaryDocNode (csBinaryDocument* document,
				  csBinaryDocNode* parent,
				  uint32 nodeType)
{
  SCF_CONSTRUCT_IBASE (NULL);

  doc = document;
  Parent = parent;
  attributes = new csRefArray<csBinaryDocAttribute>;
  children = new csRefArray<csBinaryDocNode>;

  nodeData = new bdNode;
  nodeData->flags = nodeType | BD_VALUE_TYPE_INT | BD_NODE_MODIFIED;
  nodeData->offsets[0] = BD_OFFSET_INVALID;
  nodeData->offsets[1] = BD_OFFSET_INVALID;
  nodeData->value = 0;

  vstr = NULL;
}

csBinaryDocNode::csBinaryDocNode (csBinaryDocument* document,
				  csBinaryDocNode* parent,
				  bdNode* data)
{
  SCF_CONSTRUCT_IBASE (NULL);

  doc = document;
  Parent = parent;
  attributes = NULL;
  children = NULL;

  nodeData = data;
  if ((nodeData->flags & BD_VALUE_TYPE_MASK) == BD_VALUE_TYPE_STR)
  {
    if (nodeData->flags & BD_VALUE_STR_IMMEDIATE)
    {
      vstr = (char*)&nodeData->value;
    }
    else
    {
      vstr = doc->GetInIDString (little_endian_long (nodeData->value));
    }
  }
  else
  {
    vstr = NULL;
  }
}

csBinaryDocNode::~csBinaryDocNode ()
{
  if (nodeData->flags & BD_NODE_MODIFIED)
    delete nodeData;

  if ((nodeData->flags & (BD_NODE_MODIFIED | BD_NODE_TYPE_MASK)) !=
    BD_VALUE_TYPE_STR)
  {
    delete[] vstr;
  }
  delete attributes;
  delete children;
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
  return (this == other);
}

const char* csBinaryDocNode::GetValue ()
{
  switch (nodeData->flags & BD_VALUE_TYPE_MASK)
  {
    case BD_VALUE_TYPE_STR:
      {
	return vstr;
      }
    case BD_VALUE_TYPE_INT:
      {
	if (!vstr)
	{
	  vstr = csStrNew (csString().Format ("%d", 
	    little_endian_long (nodeData->value)));
	}
	return vstr;
      }
    case BD_VALUE_TYPE_FLOAT:
      {
	if (!vstr)
	{
	  vstr = csStrNew (csString().Format ("%g", 
	    long2float (little_endian_long (nodeData->value))));
	}
	return vstr;
      }
    default:
      return NULL;
  }
}

int csBinaryDocNode::GetValueAsInt ()
{
  switch (nodeData->flags & BD_VALUE_TYPE_MASK)
  {
    case BD_VALUE_TYPE_STR:
      {
	if (!vstr) return 0;
	int val = 0;
	sscanf (vstr, "%d", &val);
	return val;
      }
    case BD_VALUE_TYPE_INT:
      {
	return little_endian_long (nodeData->value);
      }
    case BD_VALUE_TYPE_FLOAT:
      {
	return (int)long2float (little_endian_long (nodeData->value));
      }
    default:
      return 0;
  }
}

float csBinaryDocNode::GetValueAsFloat ()
{
  switch (nodeData->flags & BD_VALUE_TYPE_MASK)
  {
    case BD_VALUE_TYPE_STR:
      {
	if (!vstr) return 0.0f;
	float val = 0.0f;
	sscanf (vstr, "%g", &val);
	return val;
      }
    case BD_VALUE_TYPE_INT:
      {
	return (float)little_endian_long (nodeData->value);
      }
    case BD_VALUE_TYPE_FLOAT:
      {
	return long2float (little_endian_long (nodeData->value));
      }
    default:
      return 0.0f;
  }
}

int csBinaryDocNode::attrCompareName (csBinaryDocAttribute* item1, 
				    csBinaryDocAttribute* item2)
{
  return strcmp (item1->name, item2->name);
}

int csBinaryDocNode::attrCompareKeyName (csBinaryDocAttribute* item, 
					 void* key)
{
  return (strcmp (item->name, (char*)key));
}

void csBinaryDocNode::Modify()
{
  if (!(nodeData->flags & BD_NODE_MODIFIED))
  {
    ReadChildren();
    ReadAttrs();

    if (!children) children = new csRefArray<csBinaryDocNode>;
    if (!attributes) attributes = new csRefArray<csBinaryDocAttribute>;

    bdNode* newNodeData = new bdNode;
    memcpy (newNodeData, nodeData, sizeof (bdNode));
    newNodeData->flags |= BD_NODE_MODIFIED;
    if (((nodeData->flags & BD_VALUE_TYPE_MASK) == BD_VALUE_TYPE_STR))
    {
      vstr = csStrNew (vstr);
    }
    nodeData = newNodeData;

    int i;
    for (i = 0; i < attributes->Length(); i++)
    {
      csRef<csBinaryDocAttribute> attr = (*attributes)[i];
      attr->name = csStrNew (attr->name);
      if ((attr->flags & BD_VALUE_TYPE_MASK) == BD_VALUE_TYPE_STR)
      {
	attr->vstr = csStrNew (attr->vstr);
      }
      attr->flags |= BD_NODE_MODIFIED;
    }
  }
}

int csBinaryDocNode::IndexOfAttr (const char* attr)
{
  ReadAttrs();
  if (!attributes) return -1;
  return attributes->FindSortedKey ((void*) attr,
    attrCompareKeyName);
}

void csBinaryDocNode::InsertNewAttr (csRef<csBinaryDocAttribute> attrib)
{
  Modify ();
  attributes->InsertSorted (attrib, attrCompareName);  
}

void csBinaryDocNode::ReadChildren ()
{
  if (((nodeData->flags & (BD_NODE_HAS_CHILDREN | BD_NODE_MODIFIED))
    == BD_NODE_HAS_CHILDREN) && (!children))
  {
    bdNodeChildTab *childTabStart = (bdNodeChildTab*)
      ((uint8*)nodeData + 
      nodeData->offsets[nodeData->flags & BD_NODE_HAS_ATTR]);
    uint32 *childTab = (uint32*)childTabStart + 1;
    uint32 i;

    children = new csRefArray<csBinaryDocNode> (childTabStart->num);
    for (i = 0; i < childTabStart->num; i++)
    {
      children->Insert (children->Length(), new csBinaryDocNode (
	  doc, this, (bdNode*)((uint8*)nodeData + *(childTab + i))));
    }
  }
}

void csBinaryDocNode::ReadAttrs ()
{
  if (((nodeData->flags & (BD_NODE_HAS_ATTR | BD_NODE_MODIFIED))
    == BD_NODE_HAS_ATTR) && (!attributes))
  {
    bdNodeAttrTab *attrTabStart = (bdNodeAttrTab*)
      ((uint8*)nodeData + 
      nodeData->offsets[0]);
    uint32 *attrTab = (uint32*)attrTabStart + 1;
    uint32 i;

    attributes = new csRefArray<csBinaryDocAttribute> (attrTabStart->num);
    for (i = 0; i < attrTabStart->num; i++)
    {
      attributes->Insert (attributes->Length(), new csBinaryDocAttribute (
	doc, this, (bdNodeAttribute*)((uint8*)nodeData + *(attrTab + i))));
    }
  }
}

void csBinaryDocNode::SetValue (const char* value)
{
  Modify ();
  delete vstr;
  int v;
  float f;
  if (checkInt (value, v))
  {
    nodeData->flags = (nodeData->flags & ~BD_VALUE_TYPE_MASK) | 
      BD_VALUE_TYPE_INT;
    nodeData->value = little_endian_long (v);
  }
  else if (checkFloat (value, f))
  {
    nodeData->flags = (nodeData->flags & ~BD_VALUE_TYPE_MASK) | 
      BD_VALUE_TYPE_FLOAT;
    nodeData->value = little_endian_long (float2long (f));
  }
  else 
  {
    nodeData->flags = (nodeData->flags & ~BD_VALUE_TYPE_MASK) | 
      BD_VALUE_TYPE_STR;
    vstr = csStrNew (value);
  }
}

void csBinaryDocNode::SetValueAsInt (int value)
{
  Modify ();
  delete vstr;
  vstr = NULL;
  nodeData->flags = (nodeData->flags & ~BD_VALUE_TYPE_MASK) |
    BD_VALUE_TYPE_INT;
  nodeData->value = little_endian_long (value);
}

void csBinaryDocNode::SetValueAsFloat (float value)
{
  Modify ();
  delete vstr;
  vstr = NULL;
  nodeData->flags = (nodeData->flags & ~BD_VALUE_TYPE_MASK) |
    BD_VALUE_TYPE_FLOAT;
  nodeData->value = little_endian_long (float2long (value));
}

csRef<iDocumentNode> csBinaryDocNode::GetParent ()
{
  return Parent;
}

csRef<iDocumentNodeIterator> csBinaryDocNode::GetNodes ()
{
  ReadChildren();
  return (new csBinaryDocNodeIterator (this));
}

csRef<iDocumentNodeIterator> csBinaryDocNode::GetNodes (const char* value)
{
  ReadChildren();
  return (new csBinaryDocNodeIterator (this, value));
}

csRef<iDocumentNode> csBinaryDocNode::GetNode (const char* value)
{
  ReadChildren();
  if (!children) return NULL;
  int i;
  for (i = 0; i < children->Length(); i++)
  {
    if (strcmp ((*children)[i]->GetValue(), value) == 0)
      return (iDocumentNode*) ((*children)[i]);
  }
  return NULL;
}

void csBinaryDocNode::RemoveNode (const csRef<iDocumentNode>& child)
{
  Modify();
  children->Delete ((csRef<csBinaryDocNode>&)child);
}

void csBinaryDocNode::RemoveNodes ()
{
  Modify();
  children->DeleteAll();
}

csRef<iDocumentNode> csBinaryDocNode::CreateNodeBefore (csDocumentNodeType type,
      iDocumentNode* before)
{
  Modify();
  uint8 newType;
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

    default:
      return NULL;
  }

  csRef<csBinaryDocNode> newNode = csPtr<csBinaryDocNode>
    (new csBinaryDocNode (doc, this, newType));
  int newPos = -1;
  if (before)
  {
    newPos = children->Find ((csBinaryDocNode*)before);
  }
  if (newPos < 0) newPos = children->Length();
  children->Insert (newPos, newNode);

  return (iDocumentNode*) newNode;
}

const char* csBinaryDocNode::GetContentsValue ()
{
  ReadChildren();
  if (!children) return NULL;
  int i;
  for (i = 0; i < children->Length(); i++)
  {
    if (((*children)[i]->nodeData->flags & BD_NODE_TYPE_MASK) == 
      BD_NODE_TYPE_TEXT)
    {
      return (*children)[i]->GetValue();
    }
  }
  return NULL;
}

int csBinaryDocNode::GetContentsValueAsInt ()
{
  ReadChildren();
  if (!children) return 0;
  int i;
  for (i = 0; i < children->Length(); i++)
  {
    if (((*children)[i]->nodeData->flags & BD_NODE_TYPE_MASK) == 
      BD_NODE_TYPE_TEXT)
    {
      return (*children)[i]->GetValueAsInt();
    }
  }
  return 0;
}

float csBinaryDocNode::GetContentsValueAsFloat ()
{
  ReadChildren();
  if (!children) return 0.0f;
  int i;
  for (i = 0; i < children->Length(); i++)
  {
    if (((*children)[i]->nodeData->flags & BD_NODE_TYPE_MASK) == 
      BD_NODE_TYPE_TEXT)
    {
      return (*children)[i]->GetValueAsFloat();
    }
  }
  return 0.0f;
}

csRef<iDocumentAttributeIterator> csBinaryDocNode::GetAttributes ()
{
  ReadAttrs();
  return (new csBinaryDocAttributeIterator (this));
}

csRef<iDocumentAttribute> csBinaryDocNode::GetAttribute (const char* name)
{
  ReadAttrs();
  int attrIndex = IndexOfAttr (name);
  if (attrIndex >= 0)
  {
    return (iDocumentAttribute*) ((*attributes)[attrIndex]);
  }
  return NULL;
}

const char* csBinaryDocNode::GetAttributeValue (const char* name)
{
  int attrIndex = IndexOfAttr (name);
  csRef<csBinaryDocAttribute> attr;
  if (attrIndex >= 0)
  {
    attr = (*attributes)[attrIndex];
    return attr->GetValue();
  }
  else
  {
    return NULL;
  }
}

int csBinaryDocNode::GetAttributeValueAsInt (const char* name)
{
  int attrIndex = IndexOfAttr (name);
  csRef<csBinaryDocAttribute> attr;
  if (attrIndex >= 0)
  {
    attr = (*attributes)[attrIndex];
    return attr->GetValueAsInt();
  }
  else
  {
    return 0;
  }
}

float csBinaryDocNode::GetAttributeValueAsFloat (const char* name)
{
  int attrIndex = IndexOfAttr (name);
  csRef<csBinaryDocAttribute> attr;
  if (attrIndex >= 0)
  {
    attr = (*attributes)[attrIndex];
    return attr->GetValueAsFloat();
  }
  else
  {
    return 0.0f;
  }
}

void csBinaryDocNode::RemoveAttribute (const csRef<iDocumentAttribute>& attr)
{
  Modify();
  attributes->Delete ((csRef<csBinaryDocAttribute>&)attr);
}

void csBinaryDocNode::RemoveAttributes ()
{
  Modify();
  attributes->DeleteAll ();
}

void csBinaryDocNode::SetAttribute (const char* name, const char* value)
{
  Modify();
  int attrIndex = IndexOfAttr (name);
  csRef<csBinaryDocAttribute> attr;
  if (attrIndex < 0)
  {
    attr = new csBinaryDocAttribute (doc, this, (uint32)BD_VALUE_TYPE_STR);
    attr->SetName (name);
    InsertNewAttr (attr);
  }
  else
  {
    attr = (*attributes)[attrIndex];
  }
  attr->SetValue (value);
}

void csBinaryDocNode::SetAttributeAsInt (const char* name, int value)
{
  Modify();
  int attrIndex = IndexOfAttr (name);
  csRef<csBinaryDocAttribute> attr;
  if (attrIndex < 0)
  {
    attr = new csBinaryDocAttribute (doc, this, (uint32)BD_VALUE_TYPE_STR);
    InsertNewAttr (attr);
  }
  else
  {
    attr = (*attributes)[attrIndex];
  }
  attr->SetValueAsInt (value);
}

void csBinaryDocNode::SetAttributeAsFloat (const char* name, float value)
{
  Modify();
  int attrIndex = IndexOfAttr (name);
  csRef<csBinaryDocAttribute> attr;
  if (attrIndex < 0)
  {
    attr = new csBinaryDocAttribute (doc, this, (uint32)BD_VALUE_TYPE_STR);
    InsertNewAttr (attr);
  }
  else
  {
    attr = (*attributes)[attrIndex];
  }
  attr->SetValueAsFloat (value);
}

void csBinaryDocNode::Store (csMemFile* nodesFile)
{
  bdNode diskNode;
  size_t nodeSize = sizeof(bdNode) - 2*sizeof(uint32);

  ReadChildren();
  ReadAttrs();

  memcpy (&diskNode, nodeData, nodeSize);
  diskNode.flags &= ~(BD_NODE_MODIFIED | BD_NODE_HAS_ATTR | 
    BD_NODE_HAS_CHILDREN | BD_VALUE_STR_IMMEDIATE);

  if ((nodeData->flags & BD_VALUE_TYPE_MASK) == BD_VALUE_TYPE_STR) 
  {
    if (strlen (vstr) < 4)
    {
      diskNode.value = 0;
      strcpy ((char*)&diskNode.value, vstr);
      diskNode.flags |= BD_VALUE_STR_IMMEDIATE;
    }
    else
    {
      diskNode.value = little_endian_long (
	doc->GetOutStringID (vstr));
    }
  }

  if (attributes->Length() > 0)
  {
    diskNode.flags |= BD_NODE_HAS_ATTR;
    nodeSize += sizeof (uint32);
  }

  if (children->Length() > 0)
  {
    diskNode.flags |= BD_NODE_HAS_CHILDREN;
    nodeSize += sizeof (uint32);
  }

  size_t nodeStart = nodesFile->GetPos ();
  nodesFile->Write ((char*)&diskNode, nodeSize);

  if (attributes->Length() > 0)
  {
    size_t attrStart = nodesFile->GetPos();
    uint32 attrCount = attributes->Length();
    uint32* attrStarts = new uint32[attrCount];
    nodesFile->Write ((char*)&attrCount, sizeof(uint32));
    nodesFile->Write ((char*)attrStarts, sizeof(uint32) * attrCount);

    unsigned int i;
    for (i = 0; i < attrCount; i++)
    {
      attrStarts[i] = nodesFile->GetPos () - nodeStart;
      (*attributes)[i]->Store (nodesFile);
    }
    size_t attrEnd = nodesFile->GetPos ();
    nodesFile->SetPos (attrStart + sizeof(uint32));
    nodesFile->Write ((char*)attrStarts, sizeof(uint32) * attrCount);
    diskNode.offsets[0] = attrStart - nodeStart;
    nodesFile->SetPos (attrEnd);
  }

  if (children->Length() > 0)
  {
    size_t childStart = nodesFile->GetPos();
    uint32 childCount = children->Length();
    uint32* childStarts = new uint32[childCount];
    nodesFile->Write ((char*)&childCount, sizeof(uint32));
    nodesFile->Write ((char*)childStarts, sizeof(uint32) * childCount);

    unsigned int i;
    for (i = 0; i < childCount; i++)
    {
      childStarts[i] = nodesFile->GetPos () - nodeStart;
      (*children)[i]->Store (nodesFile);
    }
    size_t childEnd = nodesFile->GetPos ();
    nodesFile->SetPos (childStart + sizeof(uint32));
    nodesFile->Write ((char*)childStarts, sizeof(uint32) * childCount);
    diskNode.offsets[little_endian_long (diskNode.flags & BD_NODE_HAS_ATTR)] = 
      childStart - nodeStart;
    nodesFile->SetPos (childEnd);
  }

  if ((attributes->Length() > 0) || (children->Length() > 0))
  {
    size_t nodeEnd = nodesFile->GetPos();
    nodesFile->SetPos (nodeStart);
    nodesFile->Write ((char*)&diskNode, nodeSize);
    nodesFile->SetPos (nodeEnd);
  }
}

// =================================================
//  csNoCopyStringHash
// =================================================

struct csRegisteredString
{
  csStringID ID;
  char *String;
  csRegisteredString()
  { String = NULL; }
  ~csRegisteredString()
  {  }
};

csNoCopyStringHash::csNoCopyStringHash (uint32 size) : 
  Registry (size), RevRegistry(size)  
{
}

csNoCopyStringHash::~csNoCopyStringHash()
{
  Clear ();
}

const char* csNoCopyStringHash::Register (char *Name, csStringID id)
{
  csRegisteredString *itf;
  csHashKey hkey = csHashCompute (Name);

  csHashIterator it (&Registry, hkey);
  while (it.HasNext ())
  {
    itf = (csRegisteredString*) it.Next ();
    if (strcmp (itf->String, Name)==0)
    {
      itf->ID = id;
      return itf->String;
    }
  }

  itf = new csRegisteredString ();
  itf->String = Name;
  itf->ID = id;

  Registry.Put (hkey, itf);
  RevRegistry.Put (id, itf);
  return itf->String;
}

csStringID csNoCopyStringHash::Request (const char *Name)
{
  csRegisteredString *itf;
  csHashKey hkey = csHashCompute (Name);

  csHashIterator it (&Registry, hkey);
  while (it.HasNext ())
  {
    itf = (csRegisteredString*) it.Next ();
    if (strcmp (itf->String, Name)==0)
      return itf->ID;
  }

  return csInvalidStringID;
}

char* csNoCopyStringHash::Request (csStringID id)
{
  csRegisteredString *itf;

  itf = (csRegisteredString*)RevRegistry.Get (id);
  return itf->String;

/*  csHashIterator it (&Registry);
  while (it.HasNext ())
  {
    itf = (csRegisteredString*) it.Next ();
    if (itf->ID == id)
      return itf->String;
  }

  return NULL;*/
}

void csNoCopyStringHash::Clear ()
{
  csHashIterator it (&Registry);

  while (it.HasNext ())
  {
    csRegisteredString *s = (csRegisteredString*) it.Next ();
    delete s;
  }
  Registry.DeleteAll ();
}

// =================================================
//  csBinaryDocument
// =================================================

SCF_IMPLEMENT_IBASE(csBinaryDocument)
  SCF_IMPLEMENTS_INTERFACE(iDocument)
SCF_IMPLEMENT_IBASE_END

csBinaryDocument::csBinaryDocument ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  root = NULL;
  outStrSet = NULL;
  inStrHash = NULL;
}

csBinaryDocument::~csBinaryDocument ()
{
  delete root;
  delete outStrSet;
  delete inStrHash;
}

uint32 csBinaryDocument::GetOutStringID (const char* str)
{
  if (!outStrSet)
  {
    outStrSet = new csStringSet (431);
  }
  uint32 val;
  if (str)
  {
    val = outStrSet->Request (str);
  }
  else
  {
    val = BD_OFFSET_INVALID; 
  }
  return val;
}

const char* csBinaryDocument::GetOutIDString (uint32 ID)
{
  if (!outStrSet || (ID == BD_OFFSET_INVALID))
  {
    return NULL;
  }
  return outStrSet->Request (ID);
}

char* csBinaryDocument::GetInIDString (uint32 ID)
{
  if (!inStrHash || (ID == BD_OFFSET_INVALID))
  {
    return NULL;
  }
  return inStrHash->Request (ID);
}

uint32 csBinaryDocument::GetInStringID (const char* str)
{
  uint32 val;
  if (str)
  {
    val = inStrHash->Request (str);
  }
  else
  {
    val = BD_OFFSET_INVALID; 
  }
  return val;
}

void csBinaryDocument::Clear ()
{
  delete outStrSet; outStrSet = NULL;
  delete inStrHash; inStrHash = NULL;
  data = NULL;
  if (root)
  {
    root->RemoveNodes();
    root->RemoveAttributes();
    root->SetValue (NULL);
  }
}

csRef<iDocumentNode> csBinaryDocument::CreateRoot ()
{
  delete root;
  Clear();
  root = new csBinaryDocNode (this, NULL, BD_NODE_TYPE_DOCUMENT);
  return root;
}

csRef<iDocumentNode> csBinaryDocument::GetRoot ()
{
  return root;
}

const char* csBinaryDocument::Parse (iFile* file)
{
  csRef<csDataBuffer> newBuffer = csPtr<csDataBuffer> 
    (new csDataBuffer(file->GetSize()));
  file->Read ((char*)newBuffer->GetData(), file->GetSize());
  return Parse (newBuffer);
}

const char* csBinaryDocument::Parse (iDataBuffer* buf)
{
  if (buf->GetSize() < sizeof (bdHeader) + sizeof (bdDocument))
  {
    return "Not enough data";
  }
  bdHeader *head = (bdHeader*)buf->GetData();
  if (head->magic != BD_HEADER_MAGIC)
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
  delete root;
  data = buf;

  inStrHash = new csNoCopyStringHash (431);
  char *strTab = (char*)bdDoc + bdDoc->ofsStr;
  while (*strTab)
  {
    uint32 *ID = (uint32*)((uint8*)strTab + strlen (strTab) + 1);
    inStrHash->Register (strTab, *ID);
    strTab = (char*)ID + sizeof (uint32);
  }

  root = new csBinaryDocNode (this, NULL,
    (bdNode*)((uint8*)bdDoc + bdDoc->ofsRoot));

  return NULL;
}

const char* csBinaryDocument::Parse (iString* str)
{
  csRef<csDataBuffer> newBuffer = csPtr<csDataBuffer> 
    (new csDataBuffer(str->Length()));
  memcpy (newBuffer->GetData(), str->GetData(),
    str->Length());
  data = (iDataBuffer*) newBuffer;
  return Parse (newBuffer);
}

const char* csBinaryDocument::Parse (const char* buf)
{
  csRef<csDataBuffer> newBuffer = csPtr<csDataBuffer> 
    (new csDataBuffer(strlen (buf)));
  memcpy (newBuffer->GetData(), buf, strlen (buf));
  data = (iDataBuffer*) newBuffer;
  return Parse (newBuffer);
}

const char* csBinaryDocument::Write (csMemFile& out)
{
  bdHeader head;
  head.magic = BD_HEADER_MAGIC;

  out.Write ((char*)&head, sizeof (head));

  size_t docStart = out.GetPos();
  bdDocument doc;
  out.Write ((char*)&doc, sizeof (doc));

  if (root)
  {
    doc.ofsRoot = out.GetPos();
    
    int pad = (4 - doc.ofsRoot) & 3;
    if (pad)
    {
      // align to 4 byte boundary, to avoid problems
      char null[4] = {0, 0, 0, 0};
      out.Write (null, pad);
      doc.ofsRoot += pad;
    }
    doc.ofsRoot -= docStart;
    root->Store (&out);
  }
  else
  {
    doc.ofsRoot = BD_OFFSET_INVALID;
  }

  doc.ofsStr = out.GetPos();
  {
    int pad = (4 - doc.ofsStr) & 3;
    if (pad)
    {
      // align to 4 byte boundary, to avoid problems
      char null[4] = {0, 0, 0, 0};
      out.Write (null, pad);
      doc.ofsStr += pad;
    }
    doc.ofsStr -= docStart;
  }

  csStringSetIterator* hashiter = new csStringSetIterator (outStrSet);
  while (hashiter->HasNext())
  {
    uint32 ID;   
    const char* key;
    
    ID = little_endian_long ((uint32)hashiter->Next());
    key = outStrSet->Request (ID);
    out.Write (key, strlen (key) + 1);
    out.Write ((char*)&ID, sizeof(ID));
  }

  {
    char null = 0;
    out.Write (&null, 1);
  }

  head.size = out.GetSize();
  out.SetPos (0);
  out.Write ((char*)&head, sizeof (head));
  out.Write ((char*)&doc, sizeof (doc));

  return NULL;
}

const char* csBinaryDocument::Write (iFile* file)
{
  csMemFile out;
  const char* ret = Write(out);
  file->Write (out.GetData(), out.GetSize());

  return ret;
}

const char* csBinaryDocument::Write (iString* str)
{
  csMemFile temp;

  const char* ret = Write (temp);
  str->Clear();
  str->Append (temp.GetData(), temp.GetSize());

  return ret;
}

const char* csBinaryDocument::Write (iVFS* vfs, const char* filename)
{
  csMemFile temp;

  const char* ret = Write (temp);
  vfs->WriteFile (filename, temp.GetData(), temp.GetSize());

  return ret;
}

