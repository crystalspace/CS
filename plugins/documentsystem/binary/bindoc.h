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

#include "iutil/document.h"
#include "csutil/ptrarr.h"
#include "csutil/refarr.h"
#include "csutil/strset.h"

struct iDataBuffer;
class csMemFile;

/*
   Designed to be loadable with minimal processing.
   Offsets are from the beginning of the respective
   structure, unless noted otherwise.

   All integers are stored in little endian.

   Wanted to use uint8 for flags, but I guess uint32
   causes less trouble (alignment etc.)

   @@@ uses 32bit integers throughout... 
   possibly causes problems on 64 bit machines
 */

/// conveniance macro
#ifdef CS_LITTLE_ENDIAN
#  define LE(x)	  (x)
#else
#  define LE(x)	  ((x >> 24) | ((x >> 8) & 0xff00) | \
  ((x << 8) & 0xff0000) | (x << 24))
#endif

/// The Binary Document magic ID
#define BD_HEADER_MAGIC	      LE (0x83190420)

/// Binary document file header
typedef struct 
{
  /// magic ID
  uint32 magic;
  /// file size, for sanity checking
  uint32 size; 
} bdHeader;

/// Binary document header
typedef struct 
{
  /// offset from start of the struct to string table
  uint32 ofsStr;
  /// offset from start of the struct to root node
  uint32 ofsRoot;
  /* 
    string table 

    format:
      ASCIIZ string, uint32 ID
     until string is empty
   */
  /* root */
} bdDocument;

/// mask for a node/attr value type
#define BD_VALUE_TYPE_MASK	    LE (0x00c0)
/// node/attr value is string
#define BD_VALUE_TYPE_STR	    LE (0x0000)
/// node/attr value is integer
#define BD_VALUE_TYPE_INT	    LE (0x0040)
/// node/attr value is float
#define BD_VALUE_TYPE_FLOAT	    LE (0x0080)
/**
 * node/attr value is a string and shorter than 4 chars and
 * cranked into the 'value' field
 */
#define BD_VALUE_STR_IMMEDIATE	    LE (0x0100)

/// mask for a node type
#define BD_NODE_TYPE_MASK	    LE (0x001c)
/// node is an element
#define BD_NODE_TYPE_ELEMENT	    LE (0x0000)
/// node is a comment
#define BD_NODE_TYPE_COMMENT	    LE (0x0004)
/// node is some text
#define BD_NODE_TYPE_TEXT   	    LE (0x0008)
/// node is the document
#define BD_NODE_TYPE_DOCUMENT	    LE (0x000c)
/// node is of unknown type
#define BD_NODE_TYPE_UNKNOWN	    LE (0x0010)
/// node is a declaration
#define BD_NODE_TYPE_DECLARATION    LE (0x0014)

/// node has attributes
#define BD_NODE_HAS_ATTR	    LE (0x0001)
/// node has children
#define BD_NODE_HAS_CHILDREN	    LE (0x0002)

/**
 * attr name is a string and shorter than 4 chars and
 * cranked into the 'value' field
 */
#define BD_ATTR_NAME_IMMEDIATE	    LE (0x0200)

/**
 * node has been changed. must be always 0 on disk.
 */
#define BD_NODE_MODIFIED	    LE (0x0020)

/// used to save NULL strings
#define BD_OFFSET_INVALID	    0xffffffff

/// Binary document node
typedef struct
{
  /// Flags of this node
  uint32 flags;
  /**
   * Value of this nose
   *  str: ID of string or immediate string
   *  int: value
   *  float: value converted using float2long
   */
  uint32 value;
  /**
   * Offsets to children/attribute tables
   *  [0] - attr 
   *  [flags & NODE_HAS_ATTR] - children
   */
  uint32 offsets[2]; 
} bdNode;

/// Binary document node child table
typedef struct
{
  /// number of children
  uint32 num;
  /* uint32 offsets to children */
} bdNodeChildTab;

/// Binary document node attribute
typedef struct
{
  /// Attribute flags
  uint32 flags;
  /// ID of the name
  uint32 nameID;
  /// Value, same as in node value
  uint32 value;
} bdNodeAttribute;

/// Binary document node attribute table
typedef struct
{
  /// number of attributes
  uint32 num;
  /* uint32 offsets to attributes */
} bdNodeAttrTab;

struct csBinaryDocument;
struct csBinaryDocAttribute;
struct csBinaryDocNode;

struct csBinaryDocAttributeIterator : public iDocumentAttributeIterator
{
private:
  /**
   * Where we are in the attribute list.
   */
  int pos;
  /// The node whose attributes we're iterating.
  csBinaryDocNode* node;
public:
  SCF_DECLARE_IBASE;

  csBinaryDocAttributeIterator (csBinaryDocNode *node);
  virtual ~csBinaryDocAttributeIterator();

  virtual bool HasNext ();
  virtual csRef<iDocumentAttribute> Next ();
};

struct csBinaryDocAttribute : public iDocumentAttribute
{
private:
  friend struct csBinaryDocNode;

  csBinaryDocument* doc;
  csBinaryDocNode* node;

  char* name;
  uint32 flags;
  uint32 value;
  char* vstr;

  /// Switch into "modified" state
  inline void Modify();
  /// Store into a file
  void Store (csMemFile* nodesFile);
public:
  SCF_DECLARE_IBASE;

  /// Create new
  csBinaryDocAttribute (csBinaryDocument* document,
    csBinaryDocNode* node,
    uint32 attrType);
  /// Create from data
  csBinaryDocAttribute (csBinaryDocument* document,
    csBinaryDocNode* node,
    bdNodeAttribute* data);
  virtual ~csBinaryDocAttribute ();

  virtual const char* GetName ();
  virtual const char* GetValue ();
  virtual int GetValueAsInt ();
  virtual float GetValueAsFloat ();
  virtual void SetName (const char* name);
  virtual void SetValue (const char* value);
  virtual void SetValueAsInt (int v);
  virtual void SetValueAsFloat (float f);
};

struct csBinaryDocNodeIterator : iDocumentNodeIterator
{
private:
  /**
   * Where we are in the children list.
   */
  int pos;
  /// The node whose children we're iterating.
  csBinaryDocNode* node;
  /// Only iterate through nodes w/ this name
  char* value;

  inline void FastForward();
public:
  SCF_DECLARE_IBASE;

  csBinaryDocNodeIterator (csBinaryDocNode* node,
    const char* onlyval = NULL);
  virtual ~csBinaryDocNodeIterator ();

  virtual bool HasNext ();
  virtual csRef<iDocumentNode> Next ();
};

struct csBinaryDocNode: public iDocumentNode
{
private:
  friend struct csBinaryDocument;
  friend struct csBinaryDocAttribute;
  friend struct csBinaryDocAttributeIterator;
  friend struct csBinaryDocNodeIterator;

  /**
   * Pointer to the node data.
   *  Modified node - new'd bdNode*.
   *  Unmodified n. - Pointer to structure in data buffer.
   */
  bdNode* nodeData;
  csBinaryDocument* doc;
  char* vstr;
  csBinaryDocNode* Parent; 
  csRefArray<csBinaryDocAttribute> *attributes;
  csRefArray<csBinaryDocNode> *children;

  static int attrCompareName (csBinaryDocAttribute* item1, 
    csBinaryDocAttribute* item2);
  static int attrCompareKeyName (csBinaryDocAttribute* item, void* key);
  inline void Modify();
  void Store (csMemFile* nodesFile);
  inline int IndexOfAttr (const char* attr);
  inline void InsertNewAttr (csRef<csBinaryDocAttribute> attrib);
  inline void ReadChildren();
  inline void ReadAttrs();
public:
  SCF_DECLARE_IBASE;

  csBinaryDocNode (csBinaryDocument* document,
    csBinaryDocNode* parent,
    uint32 nodeType);
  csBinaryDocNode (csBinaryDocument* document,
    csBinaryDocNode* parent,
    bdNode* data);
  virtual ~csBinaryDocNode ();

  virtual csDocumentNodeType GetType ();
  virtual bool Equals (iDocumentNode* other);
  virtual const char* GetValue ();
  virtual int GetValueAsInt ();
  virtual float GetValueAsFloat ();
  virtual void SetValue (const char* value);
  virtual void SetValueAsInt (int value);
  virtual void SetValueAsFloat (float value);
  virtual csRef<iDocumentNode> GetParent ();
  virtual csRef<iDocumentNodeIterator> GetNodes ();
  virtual csRef<iDocumentNodeIterator> GetNodes (const char* value);
  virtual csRef<iDocumentNode> GetNode (const char* value);
  virtual void RemoveNode (const csRef<iDocumentNode>& child);
  virtual void RemoveNodes ();
  virtual csRef<iDocumentNode> CreateNodeBefore (csDocumentNodeType type,
  	iDocumentNode* before = NULL);
  virtual const char* GetContentsValue ();
  virtual int GetContentsValueAsInt ();
  virtual float GetContentsValueAsFloat ();
  virtual csRef<iDocumentAttributeIterator> GetAttributes ();
  virtual csRef<iDocumentAttribute> GetAttribute (const char* name);
  virtual const char* GetAttributeValue (const char* name);
  virtual int GetAttributeValueAsInt (const char* name);
  virtual float GetAttributeValueAsFloat (const char* name);
  virtual void RemoveAttribute (const csRef<iDocumentAttribute>& attr);
  virtual void RemoveAttributes ();
  virtual void SetAttribute (const char* name, const char* value);
  virtual void SetAttributeAsInt (const char* name, int value);
  virtual void SetAttributeAsFloat (const char* name, float value);
};

/**
 * Special version of csStringHash which doesn't copy the registered
 * strings via csStrNew() but only keeps a pointer to them.
 */
class csNoCopyStringHash
{
private:
  csHashMap Registry;
  csHashMap RevRegistry;
public:
  /// Constructor
  csNoCopyStringHash (uint32 size = 211);
  /// Destructor
  ~csNoCopyStringHash ();

  /**
   * Register a string with an id. Returns the pointer to the copy
   * of the string in this hash.
   */
  const char* Register (char *s, csStringID id);

  /**
   * Request the ID for the given string. Return csInvalidStringID
   * if the string was never requested before.
   */
  csStringID Request (const char *s);

  /**
   * Request the string for a given ID. Return NULL if the string
   * has not been requested (yet).
   */
  char* Request (csStringID id);

  /**
   * Delete all stored strings.
   */
  void Clear ();
};

struct csBinaryDocument : public iDocument
{
private:
  friend struct csBinaryDocNode;
  friend struct csBinaryDocAttribute;

  csRef<iDataBuffer> data;
  csBinaryDocNode* root;
  
  csStringSet* outStrSet;
  csNoCopyStringHash* inStrHash;

  /// Get an ID for a string in the output string map
  uint32 GetOutStringID (const char* str);
  /// Get an ID for a string in the input string map
  uint32 GetInStringID (const char* str);
  /// Get a string for an ID in the output string map
  const char* GetOutIDString (uint32 ID);
  /// Get a string for an ID in the input string map
  char* GetInIDString (uint32 ID);
  inline const char* Write (csMemFile& out);
public:
  SCF_DECLARE_IBASE;

  csBinaryDocument ();
  virtual ~csBinaryDocument ();

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

