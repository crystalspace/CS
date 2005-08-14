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

#ifndef __CSUTIL_XMLTINYPRIV_H__
#define __CSUTIL_XMLTINYPRIV_H__

#include "csextern.h"
#include "iutil/document.h"
#include "tinyxml.h"

class csTinyXmlDocument;

/**
 * This is an SCF compatible wrapper for an attribute iterator.
 */
struct CS_CRYSTALSPACE_EXPORT csTinyXmlAttributeIterator :
  public iDocumentAttributeIterator
{
private:
  size_t current;
  size_t count;
  TiXmlElement* parent;

public:
  csTinyXmlAttributeIterator (TiDocumentNode* parent);
  virtual ~csTinyXmlAttributeIterator ();

  SCF_DECLARE_IBASE;

  virtual bool HasNext ();
  virtual csRef<iDocumentAttribute> Next ();
};

/**
 * This is an SCF compatible wrapper for an attribute in TinyXml.
 */
struct CS_CRYSTALSPACE_EXPORT csTinyXmlAttribute : public iDocumentAttribute
{
private:
  TiDocumentAttribute* attr;

public:
  csTinyXmlAttribute ()
  {
    SCF_CONSTRUCT_IBASE (0);
    attr = 0;
  }

  csTinyXmlAttribute (TiDocumentAttribute* attr)
  {
    SCF_CONSTRUCT_IBASE (0);
    csTinyXmlAttribute::attr = attr;
  }

  virtual ~csTinyXmlAttribute ()
  {
    SCF_DESTRUCT_IBASE ();
  }

  SCF_DECLARE_IBASE;

  virtual const char* GetName ()
  {
    return attr->Name ();
  }

  virtual const char* GetValue ()
  {
    return attr->Value ();
  }

  virtual int GetValueAsInt ()
  {
    return attr->IntValue ();
  }

  virtual float GetValueAsFloat ()
  {
    const char* val = attr->Value ();
    float f;
    sscanf (val, "%f", &f);
    return f;
  }

  virtual bool GetValueAsBool ()
  {
    if (!attr || !attr->Value() ) return false;
    if (strcasecmp(attr->Value(),"true")==0 ||
        strcasecmp(attr->Value(),"yes")==0 ||
        atoi(attr->Value())!=0)
    {
      return true;
    }
    else
      return false;
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
    csString buf;
    buf.Format ("%g", f);
    attr->SetValue (buf);
  }
};

/**
 * This is an SCF compatible wrapper for a node iterator.
 */
struct CS_CRYSTALSPACE_EXPORT csTinyXmlNodeIterator : public iDocumentNodeIterator
{
private:
  csTinyXmlDocument* doc;
  TiDocumentNode* current;
  TiDocumentNodeChildren* parent;
  char* value;

public:
  csTinyXmlNodeIterator (csTinyXmlDocument* doc,
	TiDocumentNodeChildren* parent, const char* value);
  virtual ~csTinyXmlNodeIterator ();

  SCF_DECLARE_IBASE;

  virtual bool HasNext ();
  virtual csRef<iDocumentNode> Next ();
};

/**
 * This is an SCF compatible wrapper for a node in TinyXml.
 */
struct CS_CRYSTALSPACE_EXPORT csTinyXmlNode : public iDocumentNode
{
private:
  friend class csTinyXmlDocument;
  TiDocumentNode* node;
  TiDocumentNodeChildren* node_children;
  // We keep a reference to 'doc' to avoid it being cleaned up too early.
  // We need 'doc' for the pool.
  csRef<csTinyXmlDocument> doc;
  csTinyXmlNode* next_pool;	// Next element in pool.

  csTinyXmlNode (csTinyXmlDocument* doc);

  TiDocumentAttribute* GetAttributeInternal (const char* name);

public:
  virtual ~csTinyXmlNode ();

  TiDocumentNode* GetTiNode () { return node; }
  void SetTiNode (TiDocumentNode* node)
  {
    csTinyXmlNode::node = node;
    node_children = node->ToDocumentNodeChildren ();
  }

  SCF_DECLARE_IBASE;

  virtual csDocumentNodeType GetType ();
  virtual bool Equals (iDocumentNode* other);
  virtual const char* GetValue ();
  virtual void SetValue (const char* value);
  virtual void SetValueAsInt (int value);
  virtual void SetValueAsFloat (float value);

  virtual csRef<iDocumentNode> GetParent ();

  virtual csRef<iDocumentNodeIterator> GetNodes ();
  virtual csRef<iDocumentNodeIterator> GetNodes (const char* value);
  virtual csRef<iDocumentNode> GetNode (const char* value);
  virtual void RemoveNode (const csRef<iDocumentNode>& child);
  virtual void RemoveNodes (csRef<iDocumentNodeIterator> children);
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
  virtual bool  GetAttributeValueAsBool (const char* name,
					 bool defaultvalue = false);
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
class CS_CRYSTALSPACE_EXPORT csTinyXmlDocument : public iDocument
{
private:
  TiDocument* root;
  // We keep a reference to 'sys' to avoid it being cleaned up too early.
  csRef<csTinyDocumentSystem> sys;

  csTinyXmlNode* pool;

public:
  csTinyXmlDocument (csTinyDocumentSystem* sys);
  virtual ~csTinyXmlDocument ();

  /// Internal function: don't use!
  csTinyXmlNode* Alloc ();
  /// Internal function: don't use!
  csTinyXmlNode* Alloc (TiDocumentNode*);
  /// Internal function: don't use!
  void Free (csTinyXmlNode* n);

  SCF_DECLARE_IBASE;

  virtual void Clear ();
  virtual csRef<iDocumentNode> CreateRoot ();

  virtual csRef<iDocumentNode> GetRoot ();
  virtual const char* Parse (iFile* file,      bool collapse = false);
  virtual const char* Parse (iDataBuffer* buf, bool collapse = false);
  virtual const char* Parse (iString* str,     bool collapse = false);
  virtual const char* Parse (const char* buf,  bool collapse = false);
  virtual const char* Write (iFile* file);
  virtual const char* Write (iString* str);
  virtual const char* Write (iVFS* vfs, const char* filename);

  virtual int Changeable ();
};

#endif // __CSUTIL_XMLTINYPRIV_H__
