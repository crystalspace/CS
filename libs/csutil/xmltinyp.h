/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CSUTIL_XMLTINYPRIV_H__
#define __CSUTIL_XMLTINYPRIV_H__

#include "iutil/document.h"
#include "csutil/tinyxml.h"

/**
 * This is an SCF compatible wrapper for an attribute iterator.
 */
struct csTinyXmlAttributeIterator : public iDocumentAttributeIterator
{
private:
  TiDocumentAttribute* current;
  TiXmlElement* parent;

public:
  csTinyXmlAttributeIterator (TiDocumentNode* parent);
  virtual ~csTinyXmlAttributeIterator () { }

  SCF_DECLARE_IBASE;

  virtual bool HasNext ();
  virtual csRef<iDocumentAttribute> Next ();
};

/**
 * This is an SCF compatible wrapper for an attribute in TinyXml.
 */
struct csTinyXmlAttribute : public iDocumentAttribute
{
private:
  TiDocumentAttribute* attr;

public:
  csTinyXmlAttribute ()
  {
    SCF_CONSTRUCT_IBASE (NULL);
    attr = NULL;
  }

  csTinyXmlAttribute (TiDocumentAttribute* attr)
  {
    SCF_CONSTRUCT_IBASE (NULL);
    csTinyXmlAttribute::attr = attr;
  }

  virtual ~csTinyXmlAttribute ()
  {
  }

  SCF_DECLARE_IBASE;

  virtual const char* GetName () const
  {
    return attr->Name ();
  }

  virtual const char* GetValue () const
  {
    return attr->Value ();
  }

  virtual int GetValueAsInt () const
  {
    return attr->IntValue ();
  }

  virtual float GetValueAsFloat () const
  {
    const char* val = attr->Value ();
    float f;
    sscanf (val, "%f", &f);
    return f;
  }

  virtual void SetName (const char* name)
  {
    attr->SetName (name);
  }

  virtual void SetValue (const char* value)
  {
    attr->SetValue (value);
  }

  virtual void SetValueAsInt (int v)
  {
    attr->SetIntValue (v);
  }

  virtual void SetValueAsFloat (float f)
  {
    char buf[100];
    sprintf (buf, "%g", f);
    attr->SetValue (buf);
  }
};

/**
 * This is an SCF compatible wrapper for a node iterator.
 */
struct csTinyXmlNodeIterator : public iDocumentNodeIterator
{
private:
  csTinyDocumentSystem* sys;
  TiDocumentNode* current;
  TiDocumentNode* parent;
  char* value;

public:
  csTinyXmlNodeIterator (csTinyDocumentSystem* sys,
	TiDocumentNode* parent, const char* value);
  virtual ~csTinyXmlNodeIterator () { delete[] value; }

  SCF_DECLARE_IBASE;

  virtual bool HasNext ();
  virtual csRef<iDocumentNode> Next ();
};

/**
 * This is an SCF compatible wrapper for a node in TinyXml.
 */
struct csTinyXmlNode : public iDocumentNode
{
private:
  friend class csTinyDocumentSystem;
  TiDocumentNode* node;
  // We keep a reference to 'sys' to avoid it being cleaned up too early.
  // We need 'sys' for the pool.
  csRef<csTinyDocumentSystem> sys;
  csTinyXmlNode* next_pool;	// Next element in pool.

  csTinyXmlNode (csTinyDocumentSystem* sys);

  TiDocumentAttribute* GetAttributeInternal (const char* name);

public:
  virtual ~csTinyXmlNode ();

  TiDocumentNode* GetTiNode () { return node; }
  void SetTiNode (TiDocumentNode* node) { csTinyXmlNode::node = node; }

  SCF_DECLARE_IBASE;

  virtual csDocumentNodeType GetType ();
  virtual const char* GetValue ();
  virtual void SetValue (const char* value);
  virtual void SetValueAsInt (int value);
  virtual void SetValueAsFloat (float value);

  virtual csRef<iDocumentNode> GetParent ();

  virtual csRef<iDocumentNodeIterator> GetNodes ();
  virtual csRef<iDocumentNodeIterator> GetNodes (const char* type);
  virtual csRef<iDocumentNode> GetNode (const char* type);
  virtual void RemoveNode (const csRef<iDocumentNode>& child);
  virtual void RemoveNodes ();
  virtual csRef<iDocumentNode> CreateNodeBefore (csDocumentNodeType type,
  	iDocumentNode* before);

  virtual const char* GetContentsValue ();
  virtual int GetContentsValueAsInt ();
  virtual float GetContentsValueAsFloat ();

  virtual csRef<iDocumentAttributeIterator> GetAttributes ();
  virtual csRef<iDocumentAttribute> GetAttribute (const char* name);
  virtual int GetAttributeValueAsInt (const char* name);
  virtual float GetAttributeValueAsFloat (const char* name);
  virtual const char* GetAttributeValue (const char* name);
  virtual void RemoveAttribute (const csRef<iDocumentAttribute>& attr);
  virtual void RemoveAttributes ();
  virtual void SetAttribute (const char* name, const char* value);
  virtual void SetAttributeAsInt (const char* name, int value);
  virtual void SetAttributeAsFloat (const char* name, float value);
};

/**
 * This is an SCF compatible wrapper for a document in TinyXml.
 */
class csTinyXmlDocument : public iDocument
{
private:
  csRef<iDocumentNode> root;
  // We keep a reference to 'sys' to avoid it being cleaned up too early.
  csRef<csTinyDocumentSystem> sys;

public:
  csTinyXmlDocument (csTinyDocumentSystem* sys);
  virtual ~csTinyXmlDocument ();

  SCF_DECLARE_IBASE;

  virtual void Clear ();
  virtual csRef<iDocumentNode> CreateRoot ();

  virtual csRef<iDocumentNode> GetRoot ();
  virtual const char* Parse (iFile* file);
  virtual const char* Parse (iDataBuffer* buf);
  virtual const char* Parse (iString* str);
  virtual const char* Parse (const char* buf);
  virtual const char* Write (iFile* file);
  virtual const char* Write (iString* str);
  virtual const char* Write (iVFS* vfs, const char* filename);
};

#endif // __CSUTIL_XMLTINYPRIV_H__

