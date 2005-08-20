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


#ifndef XMLREAD_INCLUDED
#define XMLREAD_INCLUDED

#ifdef _MSC_VER
#pragma warning( disable : 4530 )
#pragma warning( disable : 4786 )
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <iutil/string.h>
#include <csutil/util.h>
#include <csutil/array.h>
#include <csutil/blockallocator.h>

class TrDocument;
class TrDocumentNodeChildren;
class TrXmlElement;
class TrXmlComment;
class TrXmlUnknown;
class TrDocumentAttribute;
class TrXmlText;
class TrXmlDeclaration;

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

/**
 * TrXmlBase is a base class for every class in TinyXml.
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
class TrXmlBase
{
  friend class TrDocumentNode;
  friend class TrXmlElement;
  friend class TrDocument;

public:
  TrXmlBase () {}
  virtual ~TrXmlBase () {}

  /**
   * The world does not agree on whether white space should be kept or
   * not. In order to make everyone happy, these global, static functions
   * are provided to set whether or not TinyXml will condense all white space
   * into a single space or not. The default is to condense. Note changing these
   * values is not thread safe.
   */
  static void SetCondenseWhiteSpace( bool condense )
  { condenseWhiteSpace = condense; }

  /// Return the current white space setting.
  static bool IsWhiteSpaceCondensed()
  { return condenseWhiteSpace; }

  static const char* SkipWhiteSpace( const char* );
  static char* SkipWhiteSpace( char* );

  /**
   * Reads an XML name into the string provided. Returns
   * a pointer just past the last character of the name,
   * or 0 if the function has an error.
   * This version reads the name in place.
   */
  static char* ReadName( char* p );

  /**
   * Reads text. Returns a pointer past the given end tag.
   * This version parses in place (i.e. it modifies the in buffer and
   * returns a pointer inside that).
   */
  static char* ReadText(  char* in, char*& buf, int& buflen,
        bool ignoreWhiteSpace,
        const char* endTag);

protected:
  virtual char* Parse( TrDocument* document, char* p ) = 0;

  // If an entity has been found, transform it into a character.
  static char* GetEntity( char* in, char* value );

  // Get a character, while interpreting entities.
  inline static char* GetChar( char* p, char* value )
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

  static const char* errorString[ TIXML_ERROR_STRING_COUNT ];

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
  static Entity entity[ NUM_ENTITY ];
  static bool condenseWhiteSpace;
};


/**
 * The parent class for everything in the Document Object Model.
 * (Except for attributes, which are contained in elements.)
 * Nodes have siblings, a parent, and children. A node can be
 * in a document, or stand on its own. The type of a TrDocumentNode
 * can be queried, and it can be cast to its more defined type.
 */
class TrDocumentNode : public TrXmlBase
{
  friend class TrDocument;
  friend class TrDocumentNodeChildren;
  friend class TrXmlElement;

public:
  /**
   * The types of XML nodes supported by TinyXml. (All the
   * unsupported types are picked up by UNKNOWN.)
   */
  enum NodeType
  {
    DOCUMENT, ELEMENT, COMMENT, UNKNOWN, TEXT, CDATA, DECLARATION, TYPECOUNT
  };

  virtual ~TrDocumentNode();

  /**
   * The meaning of 'value' changes for the specific type of
   * TrDocumentNode.
   * @verbatim
   * Document:  filename of the xml file
   * Element:  name of the element
   * Comment:  the comment text
   * Unknown:  the tag contents
   * Text:    the text string
   * @endverbatim
   * The subclasses will wrap this function.
   */
  virtual const char * Value () = 0;

  /// One step up the DOM.
  TrDocumentNodeChildren* Parent() const{ return parent; }

  /// Navigate to a sibling node.
  TrDocumentNode* NextSibling() const { return next; }

  /// Navigate to a sibling node with the given 'value'.
  TrDocumentNode* NextSibling( const char * ) const;

  /// Query the type (as an enumerated value, above) of this node.
  NodeType Type() const { return type; }

  /**
   * Return a pointer to the Document this node lives in.
   * Returns null if not in a document.
   */
  TrDocument* GetDocument() const;

  TrDocumentNodeChildren* ToDocumentNodeChildren() const
  {
    int t = Type ();
    return ( t == DOCUMENT || t == ELEMENT )
      ? (TrDocumentNodeChildren*) this
  : 0;
  }
  TrDocument* ToDocument() const
  { return ( Type () == DOCUMENT ) ? (TrDocument*) this : 0; }
  TrXmlElement*  ToElement() const
  { return ( Type () == ELEMENT  ) ? (TrXmlElement*)  this : 0; }
  TrXmlComment*  ToComment() const
  { return ( Type () == COMMENT  ) ? (TrXmlComment*)  this : 0; }
  TrXmlUnknown*  ToUnknown() const
  { return ( Type () == UNKNOWN  ) ? (TrXmlUnknown*)  this : 0; }
  TrXmlText*   ToText()    const
  {
    int t = Type ();
    return ( t == TEXT || t == CDATA )
    	? (TrXmlText*)     this : 0;
  }
  TrXmlDeclaration* ToDeclaration() const
  { return ( Type () == DECLARATION ) ? (TrXmlDeclaration*) this : 0; }

protected:
  TrDocumentNode( );

  NodeType type;
  TrDocumentNodeChildren* parent;
  TrDocumentNode* next;
};

/**
 * A document node with children.
 */
class TrDocumentNodeChildren : public TrDocumentNode
{
public:
  /// Construct an element.
  TrDocumentNodeChildren ();

  virtual ~TrDocumentNodeChildren();

  /// Delete all the children of this node. Does not affect 'this'.
  void Clear();

  /// Returns true if this node has no children.
  bool NoChildren() const { return !firstChild; }

  TrDocumentNode* FirstChild()  const  { return firstChild; }
  TrDocumentNode* FirstChild( const char * value ) const;

protected:
  // Figure out what is at *p, and parse it. Returns null if it is not an xml
  // node.
  TrDocumentNode* Identify( TrDocument* document, const char* start );

  // The node is passed in by ownership. This object will delete it.
  TrDocumentNode* LinkEndChild( TrDocumentNode* lastChild,
  	TrDocumentNode* addThis );

  TrDocumentNode* firstChild;
};


/**
 * An attribute is a name-value pair. Elements have an arbitrary
 * number of attributes, each with a unique name.
 *
 * @note The attributes are not TrDocumentNodes, since they are not
 * part of the tinyXML document object model. There are other
 * suggested ways to look at this problem.
 */
class TrDocumentAttribute
{
  friend class TrDocumentAttributeSet;

public:
  /// Construct an empty attribute.
  TrDocumentAttribute() { name = 0; value = 0; }
  ~TrDocumentAttribute () { }

  const char* Name()  const { return name; }
  const char* Value() { value[vallen] = 0; return value; }
  const int IntValue() const;
  const double DoubleValue() const;
  int GetValueLength () const { return vallen; }

  void SetName( const char* _name )  { name = _name; }
  /// Take over value so that this attribute has ownership.
  void TakeOverValue( char* _value, int _vallen )
  {
    value = _value;
    vallen = _vallen;
  }

  bool operator==( const TrDocumentAttribute& rhs ) const
  {
    return strcmp (rhs.name, name) == 0;
  }
  bool operator<( const TrDocumentAttribute& rhs ) const
  {
    return strcmp (name, rhs.name) > 0;
  }
  bool operator>( const TrDocumentAttribute& rhs ) const
  {
    return strcmp (name, rhs.name) < 0;
  }

  /*  [internal use]
   * Attribute parsing starts: first letter of the name
   * returns: the next char after the value end quote
   */
  char* Parse( TrDocument* document, char* p );

private:
  const char* name;
  char* value;
  int vallen;
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
class TrDocumentAttributeSet
{
public:
  csArray<TrDocumentAttribute> set;

  TrDocumentAttributeSet() : set (0, 4) { }
  size_t Find (const char * name) const;
  size_t FindExact (const char * reg_name) const;
};


/**
 * The element is a container class. It has a value, the element name,
 * and can contain other elements, text, comments, and unknowns.
 * Elements also contain an arbitrary number of attributes.
 */
class TrXmlElement : public TrDocumentNodeChildren
{
public:
  /// Construct an element.
  TrXmlElement ();

  virtual ~TrXmlElement();

  /**
   * Given an attribute name, attribute returns the value
   * for the attribute of that name, or null if none exists.
   */
  const char* Attribute( const char* name );

  /**
   * Given an attribute name, attribute returns the value
   * for the attribute of that name, or null if none exists.
   * If the attribute exists and can be converted to an integer,
   * the integer value will be put in the return 'i', if 'i'
   * is non-null.
   */
  const char* Attribute( const char* name, int* i );

  /// Get attribute with registered name.
  TrDocumentAttribute& GetAttributeRegistered (const char * reg_name);

  /// Get number of attributes.
  size_t GetAttributeCount () const { return attributeSet.set.Length (); }
  /// Get attribute.
  const TrDocumentAttribute& GetAttribute (size_t idx) const
  {
    return attributeSet.set[idx];
  }
  /// Get attribute.
  TrDocumentAttribute& GetAttribute (size_t idx)
  {
    return attributeSet.set[idx];
  }

  virtual const char * Value () { return value; }
  void SetValueRegistered (const char * _value)
  {
    value = _value;
  }
  const char* GetContentsValue ()
  {
    if (contentsvalue) contentsvalue[contentsvalue_len] = 0;
    return contentsvalue;
  }

protected:
  /*  [internal use]
   * Attribtue parsing starts: next char past '<'
   * returns: next char past '>'
   */
  virtual char* Parse( TrDocument* document, char* p );

  /*  [internal use]
   * Reads the "value" of the element -- another element, or text.
   * This should terminate with the current end tag.
   */
  char* ReadValue( TrDocument* document, char* in );

private:
  TrDocumentAttributeSet attributeSet;
  const char* value;

  // If not null then this represents the first TEXT node. This
  // is an optimization.
  char* contentsvalue;
  int contentsvalue_len;
};


/**
 * An XML comment.
 */
class TrXmlComment : public TrDocumentNode
{
public:
  /// Constructs an empty comment.
  TrXmlComment() { value = 0; type = COMMENT; }
  virtual ~TrXmlComment() { }

  virtual const char * Value () { value[vallen] = 0; return value; }

protected:
  /*  [internal use]
   * Attribtue parsing starts: at the ! of the !--
   * returns: next char past '>'
   */
  virtual char* Parse( TrDocument* document, char* p );

  char* value;
  int vallen;
};

/**
 * XML text. Contained in an element.
 */
class TrXmlText : public TrDocumentNode
{
  friend class TrXmlElement;

public:
  /// Constructor.
  TrXmlText ()
  {
    value = 0;
    type = TEXT;
  }
  virtual ~TrXmlText()
  {
  }
  virtual const char * Value () { value[vallen] = 0; return value; }

protected :
  // [internal use]
  bool Blank() const;  // returns true if all white space and new lines
  /*  [internal use]
   * Attribtue parsing starts: First char of the text
   * returns: next char past '>'
   */
  virtual char* Parse( TrDocument* document,  char* p );

  char* value;
  int vallen;
};

/**
 * XML Cdata section. Contained in an element.
 * Always start with <![CDATA[  and end with ]]>
 */
class TrXmlCData : public TrXmlText
{
  friend class TrXmlElement;

public:
  /// Constructor.
  TrXmlCData () : TrXmlText ()
  {
    type = CDATA;
  }
  virtual ~TrXmlCData() {}

protected :
  virtual char* Parse( TrDocument* document,  char* p );
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
class TrXmlDeclaration : public TrDocumentNode
{
public:
  /// Construct an empty declaration.
  TrXmlDeclaration()
  {
    version = 0;
    encoding = 0;
    standalone = 0;
    value = 0;
    type = DECLARATION;
  }

  /// Construct.
  TrXmlDeclaration (const char * _version,
    const char * _encoding, const char * _standalone );

  virtual ~TrXmlDeclaration() {}

  /// Version. Will return empty if none was found.
  const char * Version() const { return version; }
  /// Encoding. Will return empty if none was found.
  const char * Encoding() const { return encoding; }
  /// Is this a standalone document?
  const char * Standalone() const { return standalone; }

  virtual const char * Value () { return value; }

protected:
  //  [internal use]
  //  Attribtue parsing starts: next char past '<'
  //           returns: next char past '>'
  virtual char* Parse( TrDocument* document,  char* p );

private:
  const char* version;
  const char* encoding;
  const char* standalone;
  const char* value;
};


/**
 * Any tag that tinyXml doesn't recognize is save as an
 * unknown. It is a tag of text, but should not be modified.
 * It will be written back to the XML, unchanged, when the file
 * is saved.
 */
class TrXmlUnknown : public TrDocumentNode
{
public:
  TrXmlUnknown() { value = 0; type = UNKNOWN; }
  virtual ~TrXmlUnknown() {}

  virtual const char * Value () { value[vallen] = 0; return value; }
protected:
  /*  [internal use]
   * Attribute parsing starts: First char of the text
   * returns: next char past '>'
   */
  virtual char* Parse( TrDocument* document,  char* p );

  char* value;
  int vallen;
};


/**
 * Always the top level node. A document binds together all the
 * XML pieces. It can be saved, loaded, and printed to the screen.
 * The 'value' of a document node is the xml file name.
 */
class TrDocument : public TrDocumentNodeChildren
{
public:
  /// Block allocator for elements.
  csBlockAllocator<TrXmlElement> blk_element;
  /// Block allocator for text.
  csBlockAllocator<TrXmlText> blk_text;
  /// Copy of the input data.
  char* input_data;

  /// Create an empty document. Optional buf is given as input data.
  TrDocument(char* buf = 0);

  virtual ~TrDocument();

  /**
   * Correctly delete a node. This will take care to use the correct
   * memory block allocator.
   */
  void DeleteNode (TrDocumentNode* node)
  {
    switch (node->Type ())
    {
      case ELEMENT: blk_element.Free ((TrXmlElement*)node); break;
      case TEXT: blk_text.Free ((TrXmlText*)node); break;
      default: delete node;
    }
  }

  virtual const char * Value () { return 0; }

  /// Parse the given null terminated block of xml data.
  virtual char* Parse( TrDocument* document,  char* p );

  /// If, during parsing, a error occurs, Error will be set to true.
  bool Error() const { return error; }

  /// Contains a textual (english) description of the error if one occurs.
  const char * ErrorDesc() const { return errorDesc; }

  /**
   * Generally, you probably want the error string ( ErrorDesc() ). But if you
   * prefer the ErrorId, this function will fetch it.
   */
  const int ErrorId() const { return errorId; }

  /// If you have handled the error, it can be reset with this call.
  void ClearError() { error = false; errorId = 0; errorDesc = ""; }

  // [internal use]
  void SetError( int err )
  {
    error   = true;
    errorId = err;
    errorDesc = errorString[ errorId ];
  }

private:
  bool error;
  int  errorId;
  const char* errorDesc;
};

#endif

