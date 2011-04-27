/*
www.sourceforge.net/projects/tinyxml
Original code (2.0 and earlier )copyright (c) 2000-2002 Lee Thomason (www.grinninglizard.com)

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#ifndef TINYXML_INCLUDED
#define TINYXML_INCLUDED

#ifdef _MSC_VER
#pragma warning( disable : 4530 )
#pragma warning( disable : 4786 )
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "iutil/string.h"
#include "csutil/array.h"
#include "csutil/csstring.h"
#include "csutil/fifo.h"
#include "csutil/fixedsizeallocator.h"
#include "csutil/memheap.h"
#include "csutil/reftrackeraccess.h"
#include "csutil/strset.h"
#include "csutil/threading/atomicops.h"
#include "csutil/util.h"
#include "csgeom/math.h"

#include "tinystr.h"

struct iFile;

namespace CS
{
namespace Implementation
{
namespace TinyXml
{

class TiDocument;
class TiDocumentNodeChildren;
class TiXmlElement;
class TiXmlComment;
class TiXmlUnknown;
class TiDocumentAttribute;
class TiXmlText;
class TiXmlDeclaration;
class GrowString;

enum
{
  TIXML_NO_ERROR = 0,
  TIXML_ERROR,
  TIXML_ERROR_OPENING_FILE,
  TIXML_ERROR_OUT_OF_MEMORY,
  TIXML_ERROR_PARSING_ELEMENT,
  TIXML_ERROR_FAILED_TO_READ_ELEMENT_NAME,
  TIXML_ERROR_READING_ELEMENT_VALUE,
  TIXML_ERROR_READING_ATTRIBUTES,
  TIXML_ERROR_PARSING_EMPTY,
  TIXML_ERROR_READING_END_TAG,
  TIXML_ERROR_PARSING_UNKNOWN,
  TIXML_ERROR_PARSING_COMMENT,
  TIXML_ERROR_PARSING_DECLARATION,
  TIXML_ERROR_DOCUMENT_EMPTY,

  TIXML_ERROR_STRING_COUNT
};

struct ParseInfo
{
  TiDocument* document;
  bool condenseWhiteSpace;
  
  const char* startOfLine;
  int linenum;
  
  void BeginParse (const char* p)
  { startOfLine = p; linenum = 1; }
};

struct iPrintOutput
{
  virtual ~iPrintOutput() {}

  virtual void Init (char*& bufPtr, size_t& bufRemaining) = 0;
  virtual bool FlushBuffer (char*& bufPtr, size_t& bufRemaining) = 0;
};

class PrintState
{
  iPrintOutput* output;
  char* bufPtr;
  size_t bufRemaining;

  csString fmtBuf;
public:
  PrintState (iPrintOutput* output) : output (output)
  {
    output->Init (bufPtr, bufRemaining);
  }
  
  bool AppendFmtV (const char* fmt, va_list args)
  {
    fmtBuf.FormatV (fmt, args);
    return Append (fmtBuf.GetData(), fmtBuf.Length ());
  }

  bool Append (const char* data, size_t size)
  {
    while (size > 0)
    {
      if (bufRemaining == 0)
      {
        if (!output->FlushBuffer (bufPtr, bufRemaining)) return false;
      }

      size_t copySize = csMin (size, bufRemaining);
      memcpy (bufPtr, data, copySize);
      bufRemaining -= copySize;
      bufPtr += copySize;
      data += copySize;
      size -= copySize;
    }

    return true;
  }

  bool Flush ()
  {
    return output->FlushBuffer (bufPtr, bufRemaining);
  }
};

/**
 * TiXmlBase is a base class for every class in TinyXml.
 * It does little except to establish that TinyXml classes
 * can be printed and provide some utility functions.
 *
 * In XML, the document and elements can contain
 * other elements and other types of nodes.
 *
 * @verbatim
 * A Document can contain:  Element  (container or leaf)
 *        Comment (leaf)
 *        Unknown (leaf)
 *        Declaration( leaf )
 *
 * An Element can contain:  Element (container or leaf)
 *        Text  (leaf)
 *        Attributes (not on tree)
 *        Comment (leaf)
 *        Unknown (leaf)
 *
 * A Decleration contains: Attributes (not on tree)
 * @endverbatim
 */
class TiXmlBase : public CS::Memory::CustomAllocated
{
  friend class TiDocumentNode;
  friend class TiXmlElement;
  friend class TiDocument;

public:
  TiXmlBase () {}
  ~TiXmlBase () {}

  static const char* SkipWhiteSpace( ParseInfo& parse, const char* );

  /**
   * Reads an XML name into the string provided. Returns
   * a pointer just past the last character of the name,
   * or 0 if the function has an error.
   */
  static const char* ReadName( const char* p, csString& name );

  /**
   * Reads text. Returns a pointer past the given end tag.
   * Wickedly complex options, but it keeps the (sensitive) code in one place.
   */
  static const char* ReadText( ParseInfo& parse, 
        const char* in, GrowString& buf,
        bool ignoreWhiteSpace,
        const char* endTag);

  /**
   * Puts a string to a stream, expanding entities as it goes.
   * Note this should not contian the '<', '>', etc, or they will be
   * transformed into entities!
   */
  static void PutString( const TiXmlString& str, TiXmlString* out );

protected:
  // If an entity has been found, transform it into a character.
  static const char* GetEntity( const char* in, char* value );

  // Get a character, while interpreting entities.
  inline static const char* GetChar( const char* p, char* value )
  {
    if ( *p == '&' )
    {
      return GetEntity( p, value );
    }
    else
    {
      *value = *p;
      return p+1;
    }
  }

  // Return true if the next characters in the stream are any of the endTag
  // sequences.
  static bool StringEqual(const char* p, const char* endTag);
  static bool StringEqualIgnoreCase(const char* p, const char* endTag);

  static const char* const errorString[ TIXML_ERROR_STRING_COUNT ];

private:
  struct Entity
  {
    const char* str;
    unsigned int strLength;
    char chr;
  };
  enum
  {
    NUM_ENTITY = 5,
    MAX_ENTITY_LENGTH = 6
  };
  static const Entity entity[ NUM_ENTITY ];
};


/**
 * The parent class for everything in the Document Object Model.
 * (Except for attributes, which are contained in elements.)
 * Nodes have siblings, a parent, and children. A node can be
 * in a document, or stand on its own. The type of a TiDocumentNode
 * can be queried, and it can be cast to its more defined type.
 */
class TiDocumentNode : public TiXmlBase
{
  friend class TiDocument;
  friend class TiDocumentNodeChildren;
  friend class TiXmlElement;

public:
  /**
   * The types of XML nodes supported by TinyXml. (All the
   * unsupported types are picked up by UNKNOWN.)
   */
  enum NodeType
  {
    DOCUMENT, ELEMENT, COMMENT, UNKNOWN, TEXT, CDATA, DECLARATION, TYPECOUNT
  };

  ~TiDocumentNode();

  void IncRef () 
  { 
    csRefTrackerAccess::TrackIncRef (this, int16 (typeAndRefCount & 0xffff)); 
    CS::Threading::AtomicOperations::Increment (&typeAndRefCount);
  }
  void DecRef ();
  int GetRefCount () const
  { return int16 (CS::Threading::AtomicOperations::Read (&typeAndRefCount) & 0xffff); }

  /**
   * All TinyXml classes can print themselves to a filestream.
   * This is a formatted print, and will insert tabs and newlines.
   * (For an unformatted stream, use the << operator.)
   */
  const char* Print( PrintState& print, int depth ) const;
  const char* Parse( ParseInfo& parse, const char* p );

  /**
   * The meaning of 'value' changes for the specific type of
   * TiDocumentNode.
   * @verbatim
   * Document:  filename of the xml file
   * Element:  name of the element
   * Comment:  the comment text
   * Unknown:  the tag contents
   * Text:    the text string
   * @endverbatim
   * The subclasses will wrap this function.
   */
  const char * Value () const;

  /**
   * Changes the value of the node.
   */
  void SetValue (const char * _value);

  /// One step up the DOM.
  TiDocumentNodeChildren* Parent() const{ return parent; }

  /// Navigate to a sibling node.
  TiDocumentNode* NextSibling() const { return next; }

  /// Navigate to a sibling node with the given 'value'.
  TiDocumentNode* NextSibling( const char * ) const;

  /**
   * Convenience function to get through elements.
   * Calls NextSibling and ToElement. Will skip all non-Element
   * nodes. Returns 0 if there is not another element.
   */
  TiXmlElement* NextSiblingElement() const;

  /**
   * Convenience function to get through elements.
   * Calls NextSibling and ToElement. Will skip all non-Element
   * nodes. Returns 0 if there is not another element.
   */
  TiXmlElement* NextSiblingElement( const char * ) const;

  /// Query the type (as an enumerated value, above) of this node.
  NodeType Type() const
  { return (NodeType)(CS::Threading::AtomicOperations::Read (&typeAndRefCount) >> 16); }

  /**
   * Return a pointer to the Document this node lives in.
   * Returns null if not in a document.
   */
  TiDocument* GetDocument() const;

  TiDocument* ToDocument() const
  { return ( Type () == DOCUMENT ) ? (TiDocument*) this : 0; }
  TiXmlElement*  ToElement() const
  { return ( Type () == ELEMENT  ) ? (TiXmlElement*)  this : 0; }
  TiXmlComment*  ToComment() const
  { return ( Type () == COMMENT  ) ? (TiXmlComment*)  this : 0; }
  TiXmlUnknown*  ToUnknown() const
  { return ( Type () == UNKNOWN  ) ? (TiXmlUnknown*)  this : 0; }
  TiXmlText*   ToText()    const
  { return ((Type () == TEXT) || (Type () == CDATA)) ? (TiXmlText*)this : 0; }
  TiXmlDeclaration* ToDeclaration() const
  { return ( Type () == DECLARATION ) ? (TiXmlDeclaration*) this : 0; }

  csPtr<TiDocumentNode> Clone(TiDocument* document) const;

protected:
  int32 typeAndRefCount;
  void SetType (NodeType type)
  {
    typeAndRefCount = (typeAndRefCount & 0xffff) | (type << 16);
  }

  TiDocumentNode( );

  void CopyToClone( TiDocumentNode* target ) const
  {
    target->SetValue (Value () );
  }

  TiDocumentNodeChildren* parent;

  csRef<TiDocumentNode> next;
};

/**
 * A document node with children.
 */
class TiDocumentNodeChildren : public TiDocumentNode
{
public:
  /// Construct an element.
  TiDocumentNodeChildren ();

  ~TiDocumentNodeChildren();

  /// Delete all the children of this node. Does not affect 'this'.
  void Clear();

  /// Returns true if this node has no children.
  bool NoChildren() const { return !firstChild; }

  TiDocumentNode* FirstChild()  const  { return firstChild; }
  TiDocumentNode* FirstChild( const char * value ) const;

  /**
   * Add a new node related to this. Adds a child before the specified child.
   * Returns a pointer to the new object or 0 if an error occured.
   */
  TiDocumentNode* InsertBeforeChild( TiDocumentNode* beforeThis,
    const TiDocumentNode& addThis );
  TiDocumentNode* InsertAfterChild( TiDocumentNode* beforeThis,
    const TiDocumentNode& addThis );

  /// Delete a child of this node.
  bool RemoveChild( TiDocumentNode* removeThis );

  TiDocumentNode* Previous (TiDocumentNode* child);
  TiDocumentNode* LastChild ();
protected:
  // Figure out what is at *p, and parse it. Returns null if it is not an xml
  // node.
  csPtr<TiDocumentNode> Identify( ParseInfo& parse, const char* start );

  /**
   * Add a new node related to this. Adds a child past afterThis.
   * Returns a pointer to the new object or 0 if an error occured.
   */
  void InsertAfterChild( TiDocumentNode* afterThis,
    TiDocumentNode* addThis );

  csRef<TiDocumentNode> firstChild;
};


/**
 * An attribute is a name-value pair. Elements have an arbitrary
 * number of attributes, each with a unique name.
 *
 * @note The attributes are not TiDocumentNodes, since they are not
 * part of the tinyXML document object model. There are other
 * suggested ways to look at this problem.
 */
class TiDocumentAttribute
{
  friend class TiDocumentAttributeSet;

public:
  /// Construct an empty attribute.
  TiDocumentAttribute() { name = 0; value = 0; }
  ~TiDocumentAttribute () { cs_free (value); }

  const char* Name()  const { return name; }
  const char* Value() const { return value; }
  char* Value() { return value; }
  int IntValue() const;
  double DoubleValue() const;

  void SetName( const char* _name )  { name = _name; }
  void SetValue( const char* _value )
  {
    cs_free (value);
    value = CS::StrDup (_value);
  }
  /// Take over value so that this attribute has ownership.
  void TakeOverValue( char* _value )
  {
    value = _value;
  }

  void SetIntValue( int value );
  void SetDoubleValue( double value );

  bool operator==( const TiDocumentAttribute& rhs ) const
  {
    return strcmp (rhs.name, name) == 0;
  }
  bool operator<( const TiDocumentAttribute& rhs ) const
  {
    return strcmp (name, rhs.name) > 0;
  }
  bool operator>( const TiDocumentAttribute& rhs ) const
  {
    return strcmp (name, rhs.name) < 0;
  }

  /*  [internal use]
   * Attribute parsing starts: first letter of the name
   * returns: the next char after the value end quote
   */
  const char* Parse( ParseInfo& parse, TiDocumentNode* node, const char* p );

private:
  friend class TiXmlElement;

  // [internal use]
  const char* Print( PrintState& print, int depth ) const;

  const char* name;
  char* value;
};


/**
 * A class used to manage a group of attributes.
 * It is only used internally, both by the ELEMENT and the DECLARATION.
 *
 * The set can be changed transparent to the Element and Declaration
 * classes that use it, but NOT transparent to the Attribute
 * which has to implement a next() and previous() method. Which makes
 * it a bit problematic and prevents the use of STL.
 */
class TiDocumentAttributeSet
{
public:
  csArray<TiDocumentAttribute,
    csArrayElementHandler<TiDocumentAttribute>,
    CS::Memory::AllocatorMalloc,
    csArrayCapacityLinear<csArrayThresholdFixed<4> > > set;

  TiDocumentAttributeSet() : set (0) { }
  size_t Find (const char * name) const;
  size_t FindExact (const char * reg_name) const;
};


/**
 * The element is a container class. It has a value, the element name,
 * and can contain other elements, text, comments, and unknowns.
 * Elements also contain an arbitrary number of attributes.
 */
class TiXmlElement : public TiDocumentNodeChildren
{
public:
  /// Construct an element.
  TiXmlElement ();

  ~TiXmlElement();

  /**
   * Given an attribute name, attribute returns the value
   * for the attribute of that name, or null if none exists.
   */
  const char* Attribute( const char* name ) const;

  /**
   * Given an attribute name, attribute returns the value
   * for the attribute of that name, or null if none exists.
   * If the attribute exists and can be converted to an integer,
   * the integer value will be put in the return 'i', if 'i'
   * is non-null.
   */
  const char* Attribute( const char* name, int* i ) const;

  /**
   * Sets an attribute of name to a given value. The attribute
   * will be created if it does not exist, or changed if it does.
   */
  void SetAttribute(TiDocument* document,
    const char* reg_name, const char * value );
  /// Get attribute with registered name.
  TiDocumentAttribute& GetAttributeRegistered (const char * reg_name);

  /**
   * Sets an attribute of name to a given value. The attribute
   * will be created if it does not exist, or changed if it does.
   */
  void SetAttribute( TiDocument* document, const char * name, int value );

  /// Get number of attributes.
  size_t GetAttributeCount () const { return attributeSet.set.GetSize (); }
  /// Get attribute.
  const TiDocumentAttribute& GetAttribute (size_t idx) const
  {
    return attributeSet.set[idx];
  }
  /// Get attribute.
  TiDocumentAttribute& GetAttribute (size_t idx)
  {
    return attributeSet.set[idx];
  }

  /**
   * Deletes an attribute with the given name.
   */
  void RemoveAttribute( const char * name );
  // [internal use] Creates a new Element and returs it.
  csPtr<TiDocumentNode> Clone(TiDocument* document) const;
  // [internal use]

  const char * Value () const { return value; }
  void SetValueRegistered (const char * _value)
  {
    value = _value;
  }
  void SetValue (const char * _value);

  void ShrinkAttributes ()
  {
    attributeSet.set.ShrinkBestFit ();
  }
protected:
  friend class TiDocumentNode;

  const char* Print( PrintState& print, int depth ) const;

  /*  [internal use]
   * Attribtue parsing starts: next char past '<'
   * returns: next char past '>'
   */
  const char* Parse( ParseInfo& parse, const char* p );

  /*  [internal use]
   * Reads the "value" of the element -- another element, or text.
   * This should terminate with the current end tag.
   */
  const char* ReadValue( ParseInfo& parse, const char* in );

private:
  TiDocumentAttributeSet attributeSet;
  const char* value;
};


/**
 * An XML comment.
 */
class TiXmlComment : public TiDocumentNode
{
public:
  /// Constructs an empty comment.
  TiXmlComment() { value = 0; SetType (COMMENT); }
  ~TiXmlComment() { cs_free (value); }

  // [internal use] Creates a new Element and returs it.
  csPtr<TiDocumentNode> Clone(TiDocument* document) const;
  const char * Value () const { return value; }
  void SetValue (const char * _value)
  {
    cs_free (value);
    value = CS::StrDup (_value);
  }

protected:
  friend class TiDocumentNode;

  // [internal use]
  const char* Print( PrintState& print, int depth ) const;

  /*  [internal use]
   * Attribtue parsing starts: at the ! of the !--
   * returns: next char past '>'
   */
  const char* Parse( ParseInfo& parse, const char* p );

  char* value;
};

/**
 * XML text. Contained in an element.
 */
class TiXmlText : public TiDocumentNode
{
  friend class TiXmlElement;

public:
  /// Constructor.
  TiXmlText ()
  {
    value = 0;
    SetType (TEXT);
  }
  ~TiXmlText()
  {
  }
  const char * Value () const { return value; }
  void SetValueRegistered (const char * _value)
  {
    value = _value;
  }
  void SetValue (const char * _value);
protected :
  friend class TiDocumentNode;

  // [internal use]
  const char* Print( PrintState& print, int depth ) const;

  // [internal use] Creates a new Element and returns it.
  csPtr<TiDocumentNode> Clone(TiDocument* document) const;
  // [internal use]
  bool Blank() const;  // returns true if all white space and new lines
  /*  [internal use]
   * Attribtue parsing starts: First char of the text
   * returns: next char past '>'
   */
  const char* Parse( ParseInfo& parse, const char* p );

  const char* value;
};

/**
 * XML Cdata section. Contained in an element.
 * Always start with <![CDATA[  and end with ]]>
 */
class TiXmlCData : public TiXmlText
{
  friend class TiXmlElement;

public:
  /// Constructor.
  TiXmlCData () : TiXmlText ()
  {
    SetType (CDATA);
  }
  ~TiXmlCData() {}

protected :
  friend class TiDocumentNode;

  const char* Parse( ParseInfo& parse, const char* p );
};

/**
 * In correct XML the declaration is the first entry in the file.
 * @verbatim
 * <?xml version="1.0" standalone="yes"?>
 * @endverbatim
 *
 * TinyXml will happily read or write files without a declaration,
 * however. There are 3 possible attributes to the declaration:
 * version, encoding, and standalone.
 *
 * Note: In this version of the code, the attributes are
 * handled as special cases, not generic attributes, simply
 * because there can only be at most 3 and they are always the same.
 */
class TiXmlDeclaration : public TiDocumentNode
{
public:
  /// Construct an empty declaration.
  TiXmlDeclaration() { SetType (DECLARATION); }

  /// Construct.
  TiXmlDeclaration (const char * _version,
    const char * _encoding, const char * _standalone );

  ~TiXmlDeclaration() {}

  /// Version. Will return empty if none was found.
  const char * Version() const { return version.c_str (); }
  /// Encoding. Will return empty if none was found.
  const char * Encoding() const { return encoding.c_str (); }
  /// Is this a standalone document?
  const char * Standalone() const { return standalone.c_str (); }

  // [internal use] Creates a new Element and returs it.
  csPtr<TiDocumentNode> Clone(TiDocument* document) const;
  const char * Value () const { return value.c_str (); }
  void SetValue (const char * _value) { value = _value;}

protected:
  friend class TiDocumentNode;

  // [internal use]
  const char* Print( PrintState& print, int depth ) const;

  //  [internal use]
  //  Attribtue parsing starts: next char past '<'
  //           returns: next char past '>'
  const char* Parse( ParseInfo& parse, const char* p );

private:
  TiXmlString version;
  TiXmlString encoding;
  TiXmlString standalone;
  TiXmlString value;
};


/**
 * Any tag that tinyXml doesn't recognize is save as an
 * unknown. It is a tag of text, but should not be modified.
 * It will be written back to the XML, unchanged, when the file
 * is saved.
 */
class TiXmlUnknown : public TiDocumentNode
{
public:
  TiXmlUnknown() { SetType (UNKNOWN); }
  ~TiXmlUnknown() {}

  // [internal use]
  csPtr<TiDocumentNode> Clone(TiDocument* document) const;
  const char * Value () const { return value.c_str (); }
  void SetValue (const char * _value) { value = _value;}
protected:
  friend class TiDocumentNode;

  // [internal use]
  const char* Print( PrintState& print, int depth ) const;

  /*  [internal use]
   * Attribute parsing starts: First char of the text
   * returns: next char past '>'
   */
  const char* Parse( ParseInfo& parse, const char* p );

  TiXmlString value;
};


/**
 * Always the top level node. A document binds together all the
 * XML pieces. It can be saved, loaded, and printed to the screen.
 * The 'value' of a document node is the xml file name.
 */
  class TiDocument : public TiDocumentNodeChildren
{
  /* When e.g. the root node is deleted with DeleteNode() an avalanche of
     node releases may follow, even to the extent that the stack is blown.
     To work around that catch reentrants into DeleteNode() and enqueue such
     nodes to delete instead.
   */
  int deleteNest;
  csFIFO<TiDocumentNode*> destroyQueue;
  struct FreeQueueEntry
  {
    int type;
    void* ptr;
  };

  void ActualDeleteNode (TiDocumentNode* node)
  {
    switch (node->Type())
    {
      case ELEMENT: static_cast<TiXmlElement*> (node)->~TiXmlElement(); break;
      case TEXT: static_cast<TiXmlText*> (node)->~TiXmlText(); break;
      case DOCUMENT: static_cast<TiDocument*> (node)->~TiDocument(); break;
      case COMMENT: static_cast<TiXmlComment*> (node)->~TiXmlComment(); break;
      case CDATA: static_cast<TiXmlCData*> (node)->~TiXmlCData(); break;
      case DECLARATION: 
        static_cast<TiXmlDeclaration*> (node)->~TiXmlDeclaration();
        break;
      case UNKNOWN: static_cast<TiXmlUnknown*> (node)->~TiXmlUnknown(); break;
      default: 
        CS_ASSERT(false);
    }
  }
  void ActualFreeNode (const FreeQueueEntry& node)
  {
    switch (node.type)
    {
      case ELEMENT: 
        blk_element.Free (node.ptr); 
        break;
      case TEXT: 
        blk_text.Free (node.ptr); 
        break;
      case COMMENT:
      case CDATA:
      case DECLARATION:
      case UNKNOWN:
        docHeap.Free (node.ptr);
        break;
      case DOCUMENT:
        CS::Memory::CustomAllocated::operator delete (node.ptr);
        break;
      default: 
        CS_ASSERT(false);
    }
  }
  /**
   * Array capacity. Grows exponentially until a cap is hit.
   */
  class FreeQueueArrayCapacity
  {
    enum { maxGrow = 256*1024 };
  public:
    FreeQueueArrayCapacity () {}
  
    bool IsCapacityExcessive (size_t capacity, size_t count) const
    {
      size_t threshold = csMin (size_t (csFindNearestPowerOf2 (int (count))),
	size_t (maxGrow));
      return (capacity > threshold && count < capacity - threshold);
    }
    size_t GetCapacity (size_t count) const
    {
      size_t threshold = csMin (size_t (csFindNearestPowerOf2 (int (count))), 
	size_t (maxGrow));
      return ((count + threshold - 1) / threshold) * threshold;
    }
  };
  void EmptyDestroyQueue()
  {
    csArray<FreeQueueEntry, csArrayElementHandler<FreeQueueEntry>,
      CS::Memory::AllocatorMalloc, FreeQueueArrayCapacity> freeQueue;
    freeQueue.SetCapacity (destroyQueue.GetSize());
    while (destroyQueue.GetSize() > 0)
    {
      TiDocumentNode* node = destroyQueue.PopTop ();
      // If we delete ourselves here the queue will disappear!
      CS_ASSERT(node != this);
      FreeQueueEntry fqe;
      fqe.ptr = node;
      fqe.type = node->Type();
      freeQueue.Push (fqe);
      ActualDeleteNode (node);
    }
    for (size_t n = freeQueue.GetSize(); n-- > 0; )
    {
      ActualFreeNode (freeQueue[n]);
    }
  }
public:
  /// Heap used for document allocations.
  CS::Memory::Heap docHeap;
  typedef CS::Memory::AllocatorHeap<CS::Memory::Heap*> DocHeapAlloc;
  /// Interned strings.
  csStringSet strings;
  /// Block allocator for elements.
  CS::Memory::FixedSizeAllocatorSafe<sizeof(TiXmlElement), DocHeapAlloc> blk_element;
  /// Block allocator for text.
  CS::Memory::FixedSizeAllocatorSafe<sizeof(TiXmlText), DocHeapAlloc> blk_text;

  /// Create an empty document, that has no name.
  TiDocument();
  /**
   * Create a document with a name. The name of the document is also the
   * filename of the xml.
   */
  TiDocument( const char * documentName );

  ~TiDocument();

  /**
   * Correctly delete a node. This will take care to use the correct
   * memory block allocator.
   */
  void DeleteNode (TiDocumentNode* node)
  {
    if (deleteNest == 0)
    {
      // No nesting yet:
      deleteNest++;
      FreeQueueEntry fqe;
      fqe.ptr = node;
      fqe.type = node->Type();
      // Delete the node...
      ActualDeleteNode (node);
      // ... and all that have been enqueued by that delete.
      if (node != this)
        EmptyDestroyQueue();
      else
        CS_ASSERT (destroyQueue.GetSize() == 0);
      deleteNest--;
      ActualFreeNode (fqe);
      return;
    }
    // Nesting: enqueue for later deletion.
    destroyQueue.Push (node);
  }

  const char * Value () const { return value.c_str (); }
  void SetValue (const char * _value) { value = _value;}

private:
  /// Parse the given null terminated block of xml data.
  const char* Parse( ParseInfo& parse, const char* p );
  const char* Print( PrintState& print, int depth = 0 ) const;
public:
  const char* Parse( const char* p )
  {
    parse.BeginParse (p);
    return Parse (parse, p);
  }

  /// If, during parsing, a error occurs, Error will be set to true.
  bool Error() const { return errorId != TIXML_NO_ERROR; }

  /// Contains a textual (english) description of the error if one occurs.
  const char * ErrorDesc() const { return errorDesc.c_str (); }

  /**
   * Generally, you probably want the error string ( ErrorDesc() ). But if you
   * prefer the ErrorId, this function will fetch it.
   */
  int ErrorId() const { return errorId; }

  /// If you have handled the error, it can be reset with this call.
  void ClearError() { errorId = TIXML_NO_ERROR; errorDesc = ""; }

  // [internal use]
  const char* Print( iString* cfile ) const;
  const char* Print( iFile* cfile ) const;
  // [internal use]
  void SetError( int err, TiDocumentNode* errorNode, const char* errorPos )
  {
    errorId = err;
    errorDesc = errorString[ errorId ];
    if (errorNode != 0)
    {
      csString errorPath;
      
      while (errorNode != 0)
      {
        const char* nodeVal;
        if ((errorNode->Type() == ELEMENT)
          && (nodeVal = errorNode->Value()) && *nodeVal)
        {
          if (!errorPath.IsEmpty ())
            errorPath.Insert (0, " -> ");
	  errorPath.Insert (0, nodeVal);
        }
        errorNode = errorNode->parent;
      }
      
      errorDesc += " (in: ";
      csString location;
      location.Format ("line %d", parse.linenum);
      if (errorPos != 0)
        location.AppendFmt (":%zu",
          static_cast<size_t>(errorPos - parse.startOfLine + 1));
      errorDesc += location.GetDataSafe();
      if (!errorPath.IsEmpty())
      {
        errorDesc += "; ";
        errorDesc += errorPath.GetDataSafe();
      }
      errorDesc += ")";
    }
  }


  /**
   * The world does not agree on whether white space should be kept or
   * not. In order to make everyone happy, these functions are provided 
   * to set whether or not TinyXml will condense all white space into a 
   * single space or not. The default is to condense. 
   */
  void SetCondenseWhiteSpace( bool condense )
  { parse.condenseWhiteSpace = condense; }

  /// Return the current white space setting.
  bool IsWhiteSpaceCondensed()
  { return parse.condenseWhiteSpace; }
protected :
  friend class TiDocumentNode;

  // [internal use]
  csPtr<TiDocumentNode> Clone(TiDocument* document) const;

private:
  int  errorId;
  ParseInfo parse;
  TiXmlString errorDesc;
  TiXmlString value;
};

} // namespace TinyXml
} // namespace Implementation
} // namespace CS

#endif
