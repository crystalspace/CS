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
 * This is an SCF compatible wrapper for an attribute in TinyXml.
 */
struct csTinyXmlAttribute : public iXmlNode
{
private:
  char* name;
  char* value;

public:
  csTinyXmlAttribute ()
  {
    SCF_CONSTRUCT_IBASE (NULL);
    name = NULL;
    value = NULL;
  }

  virtual ~csTinyXmlAttribute ()
  {
    delete[] name;
    delete[] value;
  }

  SCF_DECLARE_IBASE;

  virtual const char* GetName () const
  {
    return name;
  }

  virtual const char* GetValue () const
  {
    return value;
  }

  virtual void SetName (const char* name)
  {
    delete[] csTinyXmlAttribute::name;
    csTinyXmlAttribute::name = csStrNew (name);
  }

  virtual void SetValue (const char* value)
  {
    delete[] csTinyXmlAttribute::value;
    csTinyXmlAttribute::value = csStrNew (value);
  }
};

/**
 * This is an SCF compatible wrapper for a node in TinyXml.
 */
struct csTinyXmlNode : public iXmlNode
{
private:
  TiXmlNode* node;

public:
  csTinyXmlNode ();
  csTinyXmlNode (TiXmlNode* node);
  virtual ~csTinyXmlNode ();

  TiXmlNode* GetTiNode () { return node; }

  SCF_DECLARE_IBASE;

  virtual const char* GetType ();
  virtual void SetType (const char* type);
  virtual csRef<iXmlNode> GetParent ();
  virtual csRef<iXmlNodeIterator> GetChildren ();
  virtual csRef<iXmlNodeIterator> GetChildren (const char* type);
  virtual void RemoveChild (const csRef<iXmlNode>& child);
  virtual void RemoveChildren ();
  virtual csRef<iXmlNode> CreateNode (const char* type);
  virtual csRef<iXmlNode> CreateNodeBefore (const char* type,
  	const csRef<iXmlNode>& node);
  virtual csRef<iXmlNode> CreateNodeAfter (const char* type,
  	const csRef<iXmlNode>& node);
  virtual void MoveNodeBefore (const csRef<iXmlNode>& node,
  	const csRef<iXmlNode>& before);
  virtual void MoveNodeAfter (const csRef<iXmlNode>& node,
  	const csRef<iXmlNode>& after);
  virtual csRef<iXmlAttributeIterator> GetAttributes ();
  virtual csRef<iXmlAttributeIterator> GetAttributes (const char* name);
  virtual void RemoveAttribute (const csRef<iXmlAttribute>& attr);
  virtual void RemoveAttributes ();
  virtual void SetAttribute (const char* name, const char* value);
  virtual csRef<iXmlAttribute> CreateAttribute ();
  virtual csRef<iXmlAttribute> CreateAttributeBefore (
  	const csRef<iXmlAttribute>& attr);
  virtual csRef<iXmlAttribute> CreateAttributeAfter (
  	const csRef<iXmlAttribute>& attr);
  virtual void MoveAttributeBefore (const csRef<iXmlAttribute>& attr,
  	const csRef<iXmlAttribute>& before);
  virtual void MoveAttributeAfter (const csRef<iXmlAttribute>& attr,
  	const csRef<iXmlAttribute>& after);
};

/**
 * This is an SCF compatible wrapper for a document in TinyXml.
 */
class csTinyXmlDocument : public iXmlDocument
{
private:
  csRef<iXmlNode> root;

public:
  csTinyXmlDocument ();
  virtual ~csTinyXmlDocument ();

  SCF_DECLARE_IBASE;

  virtual void Clear ();
  virtual csRef<iXmlNode> CreateRoot ();
  virtual csRef<iXmlNode> GetRoot ();
  virtual const char* ParseXML (iFile* file);
  virtual const char* ParseXML (iDataBuffer* buf);
  virtual const char* ParseXML (iString* str);
  virtual const char* ParseXML (const char* buf);
  virtual const char* WriteXML (iFile* file);
  virtual const char* WriteXML (iString& str);
};

#endif // __CSUTIL_XMLTINYPRIV_H__

