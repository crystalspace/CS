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

#include "iutil/document.h"
#include "csutil/csendian.h"
#include "csutil/parray.h"
#include "csutil/strset.h"
#include "csutil/blockallocator.h"

struct iDataBuffer;
class csMemFile;

/*
   Designed to be loadable with minimal processing.
   Offsets are from the beginning of the respective
   structure, unless noted otherwise.

   All integers are stored in little endian.
   Flags are big endian. Currently 8bits are used, should
   not exceed 16bit. 

   Strings are saved within a table, or if possible,
   directly in the 32bit sized value field. By saving the
   value in front of the flags, having those use at max
   16bit and in BE, actually up to 6 bytes can be crammed into
   the 32bit value ;) - and quite a lot of tags & attribute names
   in CS are shorter than this. Saves a few bytes.
 */

/// convenience macro
#define ENDIANSHUFFLE(x) ((x >> 24) | ((x >> 8) & 0xff00) | \
  ((x << 8) & 0xff0000) | (x << 24))
#ifdef CS_LITTLE_ENDIAN
#  define LE(x)	  (x)
#  define BE(x)	  ENDIANSHUFFLE(x)
#else
#  define LE(x)	  ENDIANSHUFFLE(x)
#  define BE(x)	  (x)
#endif

/// The Binary Document magic ID
#define BD_HEADER_MAGIC	      LE (0x7ada70fa)

/*
   bd* structures: data as it appears on disk.
 */

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
      ASCIIZ string ... ASCIIZ string ...
      * There's *no* special terminator. 
   */
  /* root element */
} bdDocument;

/// mask for a node/attr value type
#define BD_VALUE_TYPE_MASK	    BE (0x00000003)
/// node/attr value is string
#define BD_VALUE_TYPE_STR	    BE (0x00000001)
/// node/attr value is 32 bit integer
#define BD_VALUE_TYPE_INT	    BE (0x00000002)
/// node/attr value is float
#define BD_VALUE_TYPE_FLOAT	    BE (0x00000003)
/**
 * node/attr value is a string and shorter than 4 chars and
 * cranked into the 'value' field
 */
#define BD_VALUE_TYPE_STR_IMMEDIATE BE (0x00000000)
/// maximum length of an immediate node value, incl. 0
#define MAX_IMM_NODE_VALUE_STR	    7

/// mask for a node type
#define BD_NODE_TYPE_MASK	    BE (0x0000001c)
/// node is some text
#define BD_NODE_TYPE_TEXT   	    BE (0x00000000)
/// node is a comment
#define BD_NODE_TYPE_COMMENT	    BE (0x00000004)
/// node is an element
#define BD_NODE_TYPE_ELEMENT	    BE (0x00000008)
/// node is the document
#define BD_NODE_TYPE_DOCUMENT	    BE (0x0000000c)
/// node is of unknown type
#define BD_NODE_TYPE_UNKNOWN	    BE (0x00000010)
/// node is a declaration
#define BD_NODE_TYPE_DECLARATION    BE (0x00000014)

/// node has attributes
#define BD_NODE_HAS_ATTR	    BE (0x00000020)
/// node has children
#define BD_NODE_HAS_CHILDREN	    BE (0x00000040)

/**
 * attr name is a string and shorter than 4 chars and
 * cranked into the 'value' field
 */
#define BD_ATTR_NAME_IMMEDIATE	    BE (0x00000004)
#define MAX_IMM_ATTR_VALUE_STR	    3
#define MAX_IMM_ATTR_NAME_STR	    7
#define MAX_IMM_ATTR_NAME_STR_W_FLAGS	    3

/**
 * Bit in the attribute name that's set when the attributes
 * are stored in the name, too
 */
#define BD_ATTR_FLAGS_IN_NAME	    BE (0x00000080)
#define BD_ATTR_FLAGS_IN_NAME_MASK  BE (0x00000070)
/*
 * NB: with the current value, endiannes can be ignored when shifting around
 * the flags. Pay attention if you change the 'SHIFT' value.
 */
#define BD_ATTR_FLAGS_IN_NAME_SHIFT 4
/**
 * Bitmasks to obtain real name/attribute flags from an attr name.
 * Attention! Host endianness!
 */
#define BD_ATTR_MAX_NAME_ID_WITH_FLAGS	0x0fffffff
#define BD_ATTR_MAX_NAME_ID	    0x7fffffff
#define BD_ATTR_NAME_ID_WITH_FLAGS_MASK	    0x0fffffff

/// mask of the flags that can be stored on disk.
#define BD_DISK_FLAGS_MASK	    BE (0x000000ff)

/**
 * node has been changed. must be always 0 on disk.
 */
#define BD_NODE_MODIFIED	    BE (0x80000000)
#define BD_ATTR_MODIFIED	    BD_NODE_MODIFIED

/// used to save 0 strings
#define BD_OFFSET_INVALID	    0xffffffff

/// Binary document node
typedef struct
{
  /**
   * Value of this node
   *  str: offset of string into string table or 'immediate' string
   *  int: value
   *  float: value converted using csFloatToLong
   */
  uint32 value;
  /// Flags of this node
  uint32 flags;
  /*
   * After this struct, the attribute offset table (if the attr flag is set)
   * and child offset table (if the child flag is set) follow.
   */
} bdNode;

/// Binary document node child table
typedef struct
{
  /// number of children
  uint32 num;
  /* 
    uint32 offsets to children 
	   from beg. of this struct
  */
} bdNodeChildTab;

/// Binary document node attribute
typedef struct
{
  /// Value, same as in node value
  uint32 value;
  /// string table offset of the name or immediate
  uint32 nameID;
  /// Attribute flags
  uint32 flags;
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

/*
  csBd* structures: descendants from bd* structs.
  Contain some convenience methods to navigate through
  attrs and childs. 
  Also contains some extra fields used when creating a document.

  Because editing requires extra structures, modifying a doc
  loaded from disk would need a new set of csBd*s created to be
  created based up on the data loaded from disk. Due to the way
  everything works, some extra overhead would be needed, which counters
  the intention of maximum loading speed. Thus modifying a loaded doc
  is not supported.
 */
struct csBdNode;
struct csBdAttr;

struct csBinaryDocAttributeIterator : public iDocumentAttributeIterator
{
private:
  friend struct csBinaryDocument;

  /**
   * Where we are in the attribute list.
   */
  uint pos;
  /// The node whose attributes we're iterating.
  csBdNode* iteratedNode;
  /// Owning node.
  csBinaryDocNode* parentNode;

public:
  SCF_DECLARE_IBASE;

  csBinaryDocAttributeIterator ();
  virtual ~csBinaryDocAttributeIterator();
  void SetTo (csBdNode* node,
    csBinaryDocNode* parent);

  virtual bool HasNext ();
  virtual csRef<iDocumentAttribute> Next ();
};

struct csBdAttr : public bdNodeAttribute
{
public:
  // fields below only exist in modified attrs!
  /// name
  char* nstr;
  /// value
  char* vstr;
private:
  uint32 GetRealNameID() const
  {
    return (nameID & BD_ATTR_FLAGS_IN_NAME) ? 
      (csLittleEndianLong (nameID) & BD_ATTR_NAME_ID_WITH_FLAGS_MASK) : 
      csLittleEndianLong (nameID);
  }
public:
  uint32 GetRealFlags() const
  { 
    return (nameID & BD_ATTR_FLAGS_IN_NAME) ? 
      (nameID & BD_ATTR_FLAGS_IN_NAME_MASK) >> BD_ATTR_FLAGS_IN_NAME_SHIFT :
      flags;
  }

  csBdAttr (const char* name);
  csBdAttr ();
  ~csBdAttr ();

  void SetName (const char* name);

  const char* GetValueStr (csBinaryDocument* doc) const;
  const char* GetNameStr (csBinaryDocument* doc) const;
};

struct csBinaryDocAttribute : public iDocumentAttribute
{
private:
  friend struct csBinaryDocument;
  friend struct csBinaryDocNode;

  /// Owning node.
  csBinaryDocNode* node;

  /// Pointer to data
  csBdAttr* attrPtr;
  /// buffer for int/float values requested as strings
  char* vstr;
  /// attrPtr for which vstr is valid
  csBdAttr* vsptr;

  csBinaryDocAttribute* pool_next;

  /// Store into a file
  void Store (csMemFile* nodesFile);

  void CleanData ();
public:
  SCF_DECLARE_IBASE;

  csBinaryDocAttribute ();
  virtual ~csBinaryDocAttribute ();

  void SetTo (csBdAttr* ptr,
	      csBinaryDocNode* owner);

  virtual const char* GetName ();
  virtual const char* GetValue ();
  virtual int GetValueAsInt ();
  virtual float GetValueAsFloat ();
  virtual bool  GetValueAsBool ();
  virtual void SetName (const char* name);
  virtual void SetValue (const char* value);
  virtual void SetValueAsInt (int v);
  virtual void SetValueAsFloat (float f);
};

struct csBinaryDocNodeIterator : iDocumentNodeIterator
{
private:
  friend struct csBinaryDocument;

  /// Owning node.
  csBinaryDocNode* parentNode;
  /**
   * Where we are in the children list.
   */
  uint pos;
  /// Only iterate through nodes w/ this name
  char* value;
  /// Node whose childen we're iterating.
  csBdNode* iteratedNode;

  // fill up struct size to 32(64)
  // may sound like voodoo - but seems to help performance
  int pad[1];

  /// Skip to next node with value 'value'.
  void FastForward();
public:
  SCF_DECLARE_IBASE;

  csBinaryDocNodeIterator ();
  virtual ~csBinaryDocNodeIterator ();
  void SetTo (csBdNode* node,
    csBinaryDocNode* parent,
    const char* onlyval = 0);

  virtual bool HasNext ();
  virtual csRef<iDocumentNode> Next ();
};

struct csBdNode : public bdNode
{
public:
  // fields below only exist in modified attrs!
  /// value of type 'string' for mod. nodes
  char* vstr;
  /// attributes
  //csPDelArray<csBdAttr>* attrs;
  csArray<csBdAttr*>* attrs;
  /// children
  //csPDelArray<csBdNode>* nodes;
  csArray<csBdNode*>* nodes;
  csBinaryDocument* doc;
private:
  void* GetFromOffset (uint32 offset) const
  { return (void*)((uint8*)this + offset); }
  bdNodeAttrTab* GetAttrTab () const;
  bdNodeChildTab* GetChildTab () const;
  uint32 GetAttrTabOffset() const
  { return (sizeof (bdNode)); }
  uint32 GetChildTabOffset() const
  {
    if (flags & BD_NODE_HAS_ATTR)
      return (sizeof (bdNode) + sizeof (bdNodeAttrTab) + 
	(csLittleEndianLong (GetAttrTab()->num) * sizeof (uint32)));
    else
      return (sizeof (bdNode));
  }
public:
  csBdNode (uint32 newType);
  csBdNode (csBdNode* copyFrom);
  csBdNode ();
  ~csBdNode ();

  void SetType (uint32 newType);
  void SetDoc (csBinaryDocument* doc);

  const char* GetValueStr (csBinaryDocument* doc) const;

  csBdAttr* atGetItem (int pos);
  void atSetItem (csBdAttr* item, int pos);
  int atItemPos (csBdAttr* item);
  void atInsertBefore (csBdAttr* item, int pos);
  void atRemove (int pos);
  uint atNum ();

  csBdNode* ctGetItem (int pos);
  void ctSetItem (csBdNode* item, int pos);
  int ctItemPos (csBdNode* item);
  void ctInsertBefore (csBdNode* item, int pos);
  void ctRemove (int pos);
  uint ctNum ();
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
   *  Modified node - new'd csBdNode*.
   *  Unmodified n. - Pointer to structure in data buffer.
   */
  csBdNode* nodeData;
  csBinaryDocument* doc;
  /// buffer for int/float values requested as strings
  char* vstr;
  /// nodeData for which vstr is valid
  csBdNode* vsptr;
  // to save a few bytes the 'Parent' pointer also serves as
  // 'pool_next'.
  csBinaryDocNode* PoolNextOrParent; 

  void Store (csMemFile* nodesFile);
  int IndexOfAttr (const char* attr);
  void CleanData();
  void ResortAttrs();
  void ResortAttrs(int a, int b);
  
  inline const char* nodeValueStr (csBdNode* nodeData);
  inline int nodeValueInt (csBdNode* nodeData);
  inline float nodeValueFloat (csBdNode* nodeData);
public:
  SCF_DECLARE_IBASE;

  csBinaryDocNode ();
  virtual ~csBinaryDocNode ();

  void SetTo (csBdNode* ptr,
	      csBinaryDocNode* parent);

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
  virtual void RemoveNodes (csRef<iDocumentNodeIterator> children);
  virtual void RemoveNodes ();
  virtual csRef<iDocumentNode> CreateNodeBefore (csDocumentNodeType type,
  	iDocumentNode* before = 0);
  virtual const char* GetContentsValue ();
  virtual int GetContentsValueAsInt ();
  virtual float GetContentsValueAsFloat ();
  virtual csRef<iDocumentAttributeIterator> GetAttributes ();
  virtual csRef<iDocumentAttribute> GetAttribute (const char* name);
  virtual const char* GetAttributeValue (const char* name);
  virtual int GetAttributeValueAsInt (const char* name);
  virtual float GetAttributeValueAsFloat (const char* name);
  virtual bool GetAttributeValueAsBool (const char* name,
					bool defaultvalue = false);
  virtual void RemoveAttribute (const csRef<iDocumentAttribute>& attr);
  virtual void RemoveAttributes ();
  virtual void SetAttribute (const char* name, const char* value);
  virtual void SetAttributeAsInt (const char* name, int value);
  virtual void SetAttributeAsFloat (const char* name, float value);
};

struct csBinaryDocument : public iDocument
{
private:
  friend struct csBinaryDocNode;
  friend struct csBinaryDocNodeIterator;
  friend struct csBinaryDocAttribute;
  friend struct csBinaryDocAttributeIterator;

  csRef<iDataBuffer> data;
  uint8* dataStart;
  csBdNode* root;	
  csBinaryDocNode* nodePool;
  csBinaryDocAttribute* attrPool;
  
  csBlockAllocator<csBdAttr>* attrAlloc;
  csBlockAllocator<csBdNode>* nodeAlloc;

  csStringHash* outStrHash;
  iFile* outStrStorage;
  uint32 outStrTabOfs;
  uint32 inStrTabOfs;

  csBinaryDocNode* GetPoolNode ();
  void RecyclePoolNode (csBinaryDocNode *node);
  csBinaryDocAttribute* GetPoolAttr ();
  void RecyclePoolAttr (csBinaryDocAttribute *attr);

  csBinaryDocNode* GetRootNode ();
public:
  SCF_DECLARE_IBASE;

  csBinaryDocument ();
  virtual ~csBinaryDocument ();

  csBdAttr* AllocBdAttr ();
  void FreeBdAttr (csBdAttr* attr);
  csBdNode* AllocBdNode ();
  void FreeBdNode (csBdNode* node);

  /**
   * Get an ID for a string in the output string table.
   * Only call while writing.
   */
  uint32 GetOutStringID (const char* str);
  /// Get a string for an ID in the input string table
  const char* GetInIDString (uint32 ID) const;

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
