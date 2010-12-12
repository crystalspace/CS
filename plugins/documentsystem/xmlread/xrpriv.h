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

#ifndef __XRPRIV_H__
#define __XRPRIV_H__

#include "iutil/document.h"
#include "csutil/pooledscfclass.h"
#include "csutil/scf_implementation.h"
#include "csutil/scanstr.h"
#include "xr.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLRead)
{

class csXmlReadDocument;

/**
 * This is an SCF compatible wrapper for an attribute iterator.
 */
struct csXmlReadAttributeIterator :
  public scfImplementation1<csXmlReadAttributeIterator,
    iDocumentAttributeIterator>
{
private:
  size_t current;
  size_t count;
  TrXmlElement* parent;

public:
  csXmlReadAttributeIterator (TrDocumentNode* parent);
  virtual ~csXmlReadAttributeIterator ();

  virtual bool HasNext ();
  virtual csRef<iDocumentAttribute> Next ();
};

/**
 * This is an SCF compatible wrapper for an attribute in XmlRead.
 */
struct csXmlReadAttribute :
  public scfImplementation1<csXmlReadAttribute, iDocumentAttribute>
{
private:
  TrDocumentAttribute* attr;

public:
  csXmlReadAttribute () : scfImplementationType(this), attr(0) { }

  csXmlReadAttribute (TrDocumentAttribute* attr) :
    scfImplementationType(this), attr(attr) { }

  virtual ~csXmlReadAttribute () { }

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
    csScanStr (val, "%f", &f);
    return f;
  }
  virtual bool GetValueAsBool ()
  {
    const char* val = attr->Value ();
    if (!val) return false;
    if (strcasecmp(val,"true")==0 ||
        strcasecmp(val,"yes")==0 ||
        atoi(val)!=0)
    {
      return true;
    }
    else
      return false;
  }

  virtual void SetName (const char*) { }
  virtual void SetValue (const char*) { }
  virtual void SetValueAsInt (int) { }
  virtual void SetValueAsFloat (float) { }
};

/**
 * This is an SCF compatible wrapper for a node iterator.
 */
struct csXmlReadNodeIterator :
  public scfImplementation1<csXmlReadNodeIterator, iDocumentNodeIterator>
{
private:
  csXmlReadDocument* doc;
  TrDocumentNode* current;
  bool use_contents_value;
  TrDocumentNodeChildren* parent;
  char* value;

  size_t currentPos, endPos;
public:
  csXmlReadNodeIterator (csXmlReadDocument* doc,
	TrDocumentNodeChildren* parent, const char* value);
  virtual ~csXmlReadNodeIterator ();

  virtual bool HasNext ();
  virtual csRef<iDocumentNode> Next ();

  size_t GetNextPosition () { return currentPos; }
  size_t GetEndPosition ();
};

/**
 * This is an SCF compatible wrapper for a node in XmlRead.
 */
struct csXmlReadNode :
  public scfImplementationPooled<scfImplementation1<csXmlReadNode,
				 iDocumentNode>,
				 CS::Memory::AllocatorMalloc,
				 true>
{
private:
  friend class csXmlReadDocument;
  TrDocumentNode* node;
  bool use_contents_value;	// Optimization: use GetContentsValue().
  TrDocumentNodeChildren* node_children;

  csXmlReadNode ();

  TrDocumentAttribute* GetAttributeInternal (const char* name);

  csXmlReadDocument* GetDoc();
public:
  virtual ~csXmlReadNode ();
  virtual void DecRef ();

  TrDocumentNode* GetTiNode () { return node; }
  void SetTiNode (TrDocumentNode* node, bool use_contents_value)
  {
    csXmlReadNode::node = node;
    csXmlReadNode::use_contents_value = use_contents_value;
    node_children = node->ToDocumentNodeChildren ();
  }

  virtual csDocumentNodeType GetType ();
  virtual bool Equals (iDocumentNode* other);
  virtual const char* GetValue ();
  virtual void SetValue (const char*) { }
  virtual void SetValueAsInt (int) { }
  virtual void SetValueAsFloat (float) { }

  virtual csRef<iDocumentNode> GetParent ();

  virtual csRef<iDocumentNodeIterator> GetNodes ();
  virtual csRef<iDocumentNodeIterator> GetNodes (const char* value);
  virtual csRef<iDocumentNode> GetNode (const char* value);
  virtual void RemoveNode (const csRef<iDocumentNode>&) { }
  virtual void RemoveNodes (csRef<iDocumentNodeIterator> /*children*/) { }
  virtual void RemoveNodes () { }
  virtual csRef<iDocumentNode> CreateNodeBefore (csDocumentNodeType,
  	iDocumentNode*) { return 0; }

  virtual const char* GetContentsValue ();
  virtual int GetContentsValueAsInt ();
  virtual float GetContentsValueAsFloat ();

  virtual csRef<iDocumentAttributeIterator> GetAttributes ();
  virtual csRef<iDocumentAttribute> GetAttribute (const char* name);
  virtual int GetAttributeValueAsInt (const char* name, int defaultValue = 0);
  virtual float GetAttributeValueAsFloat (const char* name, float defaultValue = 0.0f);
  virtual bool  GetAttributeValueAsBool (const char* name,
					 bool defaultValue = false);
  virtual const char* GetAttributeValue (const char* name, const char* defaultValue = 0);
  virtual void RemoveAttribute (const csRef<iDocumentAttribute>&) { }
  virtual void RemoveAttributes () { }
  virtual void SetAttribute (const char*, const char*) { }
  virtual void SetAttributeAsInt (const char*, int) { }
  virtual void SetAttributeAsFloat (const char*, float) { }
};

/**
 * This is an SCF compatible wrapper for a document in XmlRead.
 */
class csXmlReadDocument :
  public csXmlReadNode::Pool,
  public scfImplementation1<csXmlReadDocument, iDocument>
{
private:
  TrDocument* root;
  // We keep a reference to 'sys' to avoid it being cleaned up too early.
  csRef<csXmlReadDocumentSystem> sys;

  friend struct csXmlReadNode;
public:
  csXmlReadDocument (csXmlReadDocumentSystem* sys);
  virtual ~csXmlReadDocument ();

  virtual void Clear ();
  virtual csRef<iDocumentNode> CreateRoot ();

  /// Internal function: don't use!
  csXmlReadNode* Alloc ();
  /// Internal function: don't use!
  csXmlReadNode* Alloc (TrDocumentNode*, bool use_contents_value);

  virtual csRef<iDocumentNode> GetRoot ();
  virtual const char* Parse (iFile* file,      bool collapse = false);
  virtual const char* Parse (iDataBuffer* buf, bool collapse = false);
  virtual const char* Parse (iString* str,     bool collapse = false);
  virtual const char* Parse (const char* buf,  bool collapse = false);
  virtual const char* Write (iFile*);
  virtual const char* Write (iString*);
  virtual const char* Write (iVFS*, const char*);

  csRef<iDocumentNode> CreateRoot (char* buf, size_t bufSize);
  const char* Parse (const char* buf, size_t bufSize, bool collapse);
  const char* ParseInPlace (char* buf, size_t bufSize, bool collapse);

  virtual int Changeable () { return CS_CHANGEABLE_NEVER; }
};

}
CS_PLUGIN_NAMESPACE_END(XMLRead)

#endif // __XRPRIV_H__
