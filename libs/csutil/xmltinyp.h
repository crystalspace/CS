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

#include "iutil/xml.h"
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
  TiDocumentNode* current;
  TiDocumentNode* parent;
  char* value;

public:
  csTinyXmlNodeIterator (TiDocumentNode* parent, const char* value);
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
  TiDocumentNode* node;

public:
  csTinyXmlNode ();
  csTinyXmlNode (TiDocumentNode* node);
  virtual ~csTinyXmlNode ();

  TiDocumentNode* GetTiNode () { return node; }

  SCF_DECLARE_IBASE;

  virtual csXmlNodeType GetType ();
  virtual const char* GetValue ();
  virtual void SetValue (const char* value);
  virtual csRef<iDocumentNode> GetParent ();

  virtual csRef<iDocumentNodeIterator> GetNodes ();
  virtual csRef<iDocumentNodeIterator> GetNodes (const char* type);
  virtual csRef<iDocumentNode> GetNode (const char* type);
  virtual void RemoveNode (const csRef<iDocumentNode>& child);
  virtual void RemoveNodes ();
  virtual csRef<iDocumentNode> CreateNode (const char* type);
  virtual csRef<iDocumentNode> CreateNodeBefore (const char* type,
  	const csRef<iDocumentNode>& node);
  virtual csRef<iDocumentNode> CreateNodeAfter (const char* type,
  	const csRef<iDocumentNode>& node);
  virtual void MoveNodeBefore (const csRef<iDocumentNode>& node,
  	const csRef<iDocumentNode>& before);
  virtual void MoveNodeAfter (const csRef<iDocumentNode>& node,
  	const csRef<iDocumentNode>& after);

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
  virtual csRef<iDocumentAttribute> CreateAttribute ();
  virtual csRef<iDocumentAttribute> CreateAttributeBefore (
  	const csRef<iDocumentAttribute>& attr);
  virtual csRef<iDocumentAttribute> CreateAttributeAfter (
  	const csRef<iDocumentAttribute>& attr);
  virtual void MoveAttributeBefore (const csRef<iDocumentAttribute>& attr,
  	const csRef<iDocumentAttribute>& before);
  virtual void MoveAttributeAfter (const csRef<iDocumentAttribute>& attr,
  	const csRef<iDocumentAttribute>& after);
};

/**
 * This is an SCF compatible wrapper for a document in TinyXml.
 */
class csTinyXmlDocument : public iDocument
{
private:
  csRef<iDocumentNode> root;

public:
  csTinyXmlDocument ();
  virtual ~csTinyXmlDocument ();

  SCF_DECLARE_IBASE;

  virtual void Clear ();
  virtual csRef<iDocumentNode> CreateRoot ();
  virtual csRef<iDocumentNode> GetRoot ();
  virtual const char* ParseXML (iFile* file);
  virtual const char* ParseXML (iDataBuffer* buf);
  virtual const char* ParseXML (iString* str);
  virtual const char* ParseXML (const char* buf);
  virtual const char* WriteXML (iFile* file);
  virtual const char* WriteXML (iString& str);
};

#endif // __CSUTIL_XMLTINYPRIV_H__

