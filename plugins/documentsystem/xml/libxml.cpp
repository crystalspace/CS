#include "cssysdef.h"

#include "csutil/util.h"
#include "csutil/scf.h"
#include "iutil/document.h"
#include "iutil/databuff.h"
#include "libxml.h"

#include "libxml/tree.h"
#include "libxml/parser.h"

static inline int String2Int (const char* string)
{
  if (!string)
    return 0;

  int val = 0;
  sscanf (string, "%d", &val);
  return val;
}

static inline float String2Float (const char* string)
{
  if (!string)
    return 0.0;
  
  float val = 0.0;
  sscanf (string, "%f", &val);
  return val;
}

static inline const char* GetXmlText (xmlNodePtr node)
{
  // this is not that nice but using the libxml2 functions is not an option
  // because they construct new strings
  for (xmlNodePtr n = node; n!=NULL; n=n->next)
  {
    if (n->type == XML_TEXT_NODE)
      return (const char*) n->content;
  }

  return "";
}

//---------------------------------------------------------------------------

class csXMLAttribute : public iDocumentAttribute
{
public:
  SCF_DECLARE_IBASE;
  csXMLAttribute (xmlNodePtr node, xmlAttrPtr attr);
  virtual ~csXMLAttribute ();

  virtual const char* GetName ()
  { return (const char*) attr->name; }
  virtual const char* GetValue ()
  { 
    return (const char*) GetXmlText (attr->children);
  }
  virtual int GetValueAsInt ()
  { 
    return String2Int(GetValue());
  }
  virtual float GetValueAsFloat ()
  { 
    return String2Float (GetValue());
  }

  virtual void SetValue (const char* value)
  { 
#if 0
    xmlSetProp (node, attributename, (xmlChar*) value);
#endif
  }
  virtual void SetValueAsInt (int value)
  {
    xmlChar buf[40];
    sprintf ((char*) buf, "%d", value);
    //xmlSetProp (node, attributename, buf);
  }
  virtual void SetValueAsFloat (float value)
  {
    xmlChar buf[40];
    sprintf ((char*) buf, "%f", value);
    //xmlSetProp (node, attributename, buf);
  }

  virtual void SetName (const char* newattributename)
  {
#if 0
    xmlChar* oldvalue
      = (xmlChar*) csStrNew ((const char*) xmlGetProp (node, attributename));
    xmlUnsetProp (node, attributename);
    delete[] attributename;
    attributename = (xmlChar*) csStrNew (newattributename);
    xmlSetProp (node, attributename, oldvalue);
    delete[] oldvalue;
#endif
  }
  
private:
  xmlNodePtr node;
  xmlAttrPtr attr;
};

class csXMLAttributeIterator : public iDocumentAttributeIterator
{
public:
  SCF_DECLARE_IBASE;

  csXMLAttributeIterator (xmlNodePtr node, xmlAttrPtr attr);
  virtual ~csXMLAttributeIterator ();

  virtual bool HasNext()
  { return attr != NULL; }
  virtual csRef<iDocumentAttribute> Next()
  {
    xmlAttrPtr csattr = attr;
    
    if (attr)
      attr = attr->next;

    return csPtr<iDocumentAttribute> (new csXMLAttribute (node, csattr));
  }

private:
  xmlNodePtr node;
  xmlAttrPtr attr;
};

csXMLAttributeIterator::csXMLAttributeIterator 
				  (xmlNodePtr newnode, xmlAttrPtr newattr)
  : node(newnode), attr(newattr)
{
  SCF_CONSTRUCT_IBASE(0);
}

csXMLAttributeIterator::~csXMLAttributeIterator ()
{
}

SCF_IMPLEMENT_IBASE (csXMLAttributeIterator)
  SCF_IMPLEMENTS_INTERFACE (iDocumentAttributeIterator)
SCF_IMPLEMENT_IBASE_END

//---------------------------------------------------------------------------

csXMLAttribute::csXMLAttribute (xmlNodePtr newnode, xmlAttrPtr newattr)
  : node(newnode), attr(newattr)
{
  SCF_CONSTRUCT_IBASE(0);
}

csXMLAttribute::~csXMLAttribute ()
{
}

SCF_IMPLEMENT_IBASE (csXMLAttribute)
  SCF_IMPLEMENTS_INTERFACE (iDocumentAttribute)
SCF_IMPLEMENT_IBASE_END

//---------------------------------------------------------------------------

class csXMLDocumentNodeIterator : public iDocumentNodeIterator
{
public:
  SCF_DECLARE_IBASE;
  csXMLDocumentNodeIterator (xmlNodePtr node);
  virtual ~csXMLDocumentNodeIterator ();

  virtual bool HasNext ();
  virtual csRef<iDocumentNode> Next ();
  
private:
  xmlNodePtr node;
};

csXMLDocumentNodeIterator::csXMLDocumentNodeIterator (xmlNodePtr newnode)
  : node(newnode)
{
  SCF_CONSTRUCT_IBASE (0);
}

csXMLDocumentNodeIterator::~csXMLDocumentNodeIterator ()
{
}

SCF_IMPLEMENT_IBASE (csXMLDocumentNodeIterator)
  SCF_IMPLEMENTS_INTERFACE (iDocumentNodeIterator)
SCF_IMPLEMENT_IBASE_END

//---------------------------------------------------------------------------

class csXMLDocumentNode : public iDocumentNode
{
public:
  SCF_DECLARE_IBASE;
  csXMLDocumentNode (xmlNodePtr node);
  virtual ~csXMLDocumentNode ();

  virtual csDocumentNodeType GetType ();
  virtual bool Equals (iDocumentNode* other)
  { 
    return false;
  }
  virtual const char* GetValue ()
  { 
    return node ? (const char*) node->name : NULL ;
  }
  virtual void SetValue (const char* value)
  { }
  virtual void SetValueAsInt (int )
  { }
  virtual void SetValueAsFloat (float)
  { }

  virtual csRef<iDocumentNode> GetParent ()
  { 
    if (!node)
      return NULL;
    
    return csPtr<iDocumentNode> (new csXMLDocumentNode(node->parent));
  }

  virtual csRef<iDocumentNodeIterator> GetNodes ();
  virtual csRef<iDocumentNodeIterator> GetNodes (const char*);

  virtual csRef<iDocumentNode> GetNode (const char* );

  virtual void RemoveNode (const csRef<iDocumentNode>& )
  { }
  virtual void RemoveNodes ()
  { }

  virtual csRef<iDocumentNode> CreateNodeBefore (csDocumentNodeType,
      iDocumentNode*)
  { return NULL; }

  virtual const char* GetContentsValue ()
  { 
    if (!node)
      return NULL;

    for (xmlNodePtr n = node->children; n != NULL; n=n->next)
    {
      if (n->type == XML_TEXT_NODE)
      {
	return (const char*) n->content;
      }
    }

    return NULL;
  }
  virtual int GetContentsValueAsInt ()
  { return String2Int (GetContentsValue()); }
  virtual float GetContentsValueAsFloat ()
  { return String2Float (GetContentsValue ()); }

  virtual csRef<iDocumentAttributeIterator> GetAttributes ();
  virtual csRef<iDocumentAttribute> GetAttribute (const char*);

  virtual const char* GetAttributeValue (const char* name);
  virtual int GetAttributeValueAsInt (const char* name)
  { return String2Int (GetAttributeValue (name)); }
  virtual float GetAttributeValueAsFloat (const char* name)
  { return String2Float (GetAttributeValue (name)); }

  virtual void RemoveAttribute (const csRef<iDocumentAttribute>& attr)
  { 
    xmlUnsetProp (node, (xmlChar*) attr->GetName());
  }
  virtual void RemoveAttributes ()
  { }

  virtual void SetAttribute (const char*, const char*)
  { }
  virtual void SetAttributeAsInt (const char*, int)
  { }
  virtual void SetAttributeAsFloat (const char*, float)
  { }

private:
  xmlNodePtr node;
};

SCF_IMPLEMENT_IBASE (csXMLDocumentNode)
  SCF_IMPLEMENTS_INTERFACE (iDocumentNode)
SCF_IMPLEMENT_IBASE_END

csXMLDocumentNode::csXMLDocumentNode (xmlNodePtr newnode)
    : node(newnode)
{
  SCF_CONSTRUCT_IBASE(0);
}

csXMLDocumentNode::~csXMLDocumentNode ()
{
}

csDocumentNodeType csXMLDocumentNode::GetType ()
{
  switch (node->type)
  {
    case XML_ELEMENT_NODE:
      return CS_NODE_ELEMENT;
    case XML_TEXT_NODE:
      return CS_NODE_TEXT;
    case XML_COMMENT_NODE:
      return CS_NODE_COMMENT;
    case XML_ELEMENT_DECL:
    case XML_ATTRIBUTE_DECL:
      return CS_NODE_DECLARATION;
      
    default:
      return CS_NODE_UNKNOWN;
  }
}

csRef<iDocumentNode> csXMLDocumentNode::GetNode (const char* name)
{
  for (xmlNodePtr p=node->children; p!=NULL; p=p->next)
  {
    if (p->name && strcmp (name, (const char*) p->name) == 0)
      return csPtr<iDocumentNode> (new csXMLDocumentNode(p));
  }

  return NULL;
}

csRef<iDocumentNodeIterator> csXMLDocumentNode::GetNodes (const char* name)
{
  for (xmlNodePtr p=node->children; p!=NULL; p=p->next)
  {                                                            
    if (p->name && strcmp (name, (const char*) p->name) == 0)  
      return csPtr<iDocumentNodeIterator> (new csXMLDocumentNodeIterator (p));
  }

  return csPtr<iDocumentNodeIterator> (new csXMLDocumentNodeIterator (NULL));
}

csRef<iDocumentAttributeIterator> csXMLDocumentNode::GetAttributes ()
{
  return csPtr<iDocumentAttributeIterator> 
    (new csXMLAttributeIterator (node, node->properties));
}

csRef<iDocumentAttribute> csXMLDocumentNode::GetAttribute (const char* name)
{
  for (xmlAttrPtr attr = node->properties; attr != NULL; attr = attr->next)
  {                                                                          
    if (attr->name && strcmp (name, (const char*) attr->name) == 0)
      return csPtr<iDocumentAttribute> (new csXMLAttribute (node, attr));
  }

  return NULL;
}

csRef<iDocumentNodeIterator> csXMLDocumentNode::GetNodes ()
{
  return csPtr<iDocumentNodeIterator> 
	    (new csXMLDocumentNodeIterator(node->children));
}

const char* csXMLDocumentNode::GetAttributeValue (const char* name)
{
  if (!node)
    return NULL;

  for (xmlAttrPtr attr = node->properties; attr!=NULL; attr=attr->next)
  {
    if (attr->name && strcmp(name, (const char*) attr->name) == 0)
    {
      return GetXmlText (attr->children);
    }
  }

  return NULL;
}

//---------------------------------------------------------------------------

bool csXMLDocumentNodeIterator::HasNext ()
{
  return (node != NULL);
}

csRef<iDocumentNode> csXMLDocumentNodeIterator::Next ()
{ 
  if (!node)
    return NULL;
        
  xmlNodePtr temp = node;
  node = node->next;

  return csPtr<iDocumentNode> (new csXMLDocumentNode(temp));
}

//---------------------------------------------------------------------------

class csXMLDocument : public iDocument
{
public:
  SCF_DECLARE_IBASE;
  csXMLDocument ();
  virtual ~csXMLDocument ();

  virtual void Clear ()
  {}

  virtual csRef<iDocumentNode> CreateRoot ()
  { return NULL; }
  virtual csRef<iDocumentNode> GetRoot ();

  virtual const char* Parse (iFile*)
  { return "error"; }
  virtual const char* Parse (iDataBuffer* );
  virtual const char* Parse (iString* )
  { return "error"; }
  virtual const char* Parse (const char*);

  virtual const char* Write (iFile*)
  { return "error"; }
  virtual const char* Write (iString*)
  { return "error"; }
  virtual const char* Write (iVFS*, const char*)
  { return "error"; }

private:
  xmlDocPtr doc;
};

csXMLDocument::csXMLDocument ()
{
  SCF_CONSTRUCT_IBASE (0);

  doc = NULL;
}

csXMLDocument::~csXMLDocument ()
{
  if (doc)
  {
    xmlFreeDoc(doc);
  }
}

csRef<iDocumentNode> csXMLDocument::GetRoot ()
{
  if (!doc->doc)
    return NULL;

  // Not nice but seems this is needed in object oriented C
  xmlNodePtr rootnode = (xmlNodePtr) doc;
  return csPtr<iDocumentNode> (new csXMLDocumentNode (rootnode));
}

const char* csXMLDocument::Parse (const char* filename)
{
  if (doc)
    xmlFreeDoc(doc);

  doc = xmlParseFile (filename);
  // all ok
  return NULL;
}

const char* csXMLDocument::Parse (iDataBuffer* buf)
{
  if (doc)
    xmlFreeDoc (doc);

  doc = xmlParseMemory ((const char*) buf->GetData(), buf->GetSize());

  return NULL;
}

SCF_IMPLEMENT_IBASE (csXMLDocument)
  SCF_IMPLEMENTS_INTERFACE (iDocument)
SCF_IMPLEMENT_IBASE_END

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csXMLDocumentSystem)
  SCF_IMPLEMENTS_INTERFACE(iDocumentSystem)
  SCF_IMPLEMENTS_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END

csXMLDocumentSystem::csXMLDocumentSystem(iBase* parent)
{
  SCF_CONSTRUCT_IBASE(parent);
}

csXMLDocumentSystem::~csXMLDocumentSystem ()
{
}

csRef<iDocument> csXMLDocumentSystem::CreateDocument ()
{
  return csPtr<iDocument> (new csXMLDocument);
}

bool csXMLDocumentSystem::Initialize (iObjectRegistry* objreg)
{
  return true;
}

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csXMLDocumentSystem)

SCF_EXPORT_CLASS_TABLE(xml)
  SCF_EXPORT_CLASS(csXMLDocumentSystem, "crystalspace.documentsystem.xml",
      "libxml2 parser")
SCF_EXPORT_CLASS_TABLE_END

