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

// >>> JORRIT
// @@@ This is an EXTREMELY ugly hack.
// I want to avoid touching the TinyXml sources too much (to ensure we
// can upgrade easily to new versions of TinyXml). But I need a function
// to output XML to an iString. So I do this by redefining FILE and
// all FILE related routines (fprintf) to work on iString instead.
#include "iutil/string.h"
#include "csutil/scfstr.h"
#include "csutil/array.h"
#include "csutil/util.h"
#include "csutil/strset.h"
#undef FILE
#define FILE iString
#define fprintf new_fprintf
#define fputs new_fputs
#define fopen new_fopen
#define fseek new_fseek
#define ftell new_ftell
#define fclose new_fclose
#define fgets new_fgets
#ifdef stdout
#undef stdout
#endif
#define stdout NULL
static void new_fprintf (iString* file, const char* msg, ...)
{
  scfString str;
  va_list args;
  va_start (args, msg);
  str.FormatV (msg, args);
  va_end (args);
  file->SetGrowsExponentially(true);
  file->Append (str);
}
static void new_fputs (const char* msg, iString* file)
{
  file->Append (msg);
}
static iString* new_fopen (const char* /*fn*/, const char* /*m*/)
{ return NULL; }
static void new_fseek (iString*, int, int) { }
static long new_ftell (iString*) { return 0; }
static void new_fclose (iString*) { }
static int new_fgets (const char*, int, iString*) { return 0; }
// <<< JORRIT

// Help out windows:
#if defined( _DEBUG ) && !defined( DEBUG )
#define DEBUG
#endif

#if defined( DEBUG ) && defined( _MSC_VER )
#include <windows.h>
#define TIXML_LOG OutputDebugString
#else
#define TIXML_LOG printf
#endif


#include "tinystr.h"
#define TIXML_STRING	TiXmlString
#define TIXML_OSTREAM	TiXmlOutStream

class TiDocument;
class TiDocumentNodeChildren;
class TiXmlElement;
class TiXmlComment;
class TiXmlUnknown;
class TiDocumentAttribute;
class TiXmlText;
class TiXmlDeclaration;

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
 * TiXmlBase is a base class for every class in TinyXml.
 * It does little except to establish that TinyXml classes
 * can be printed and provide some utility functions.
 *
 * In XML, the document and elements can contain
 * other elements and other types of nodes.
 *
 * @verbatim
 * A Document can contain:	Element	(container or leaf)
 *				Comment (leaf)
 *				Unknown (leaf)
 *				Declaration( leaf )
 *
 * An Element can contain:	Element (container or leaf)
 *				Text	(leaf)
 *				Attributes (not on tree)
 *				Comment (leaf)
 *				Unknown (leaf)
 *
 * A Decleration contains: Attributes (not on tree)
 * @endverbatim
 */
class TiXmlBase
{
  friend class TiDocumentNode;
  friend class TiXmlElement;
  friend class TiDocument;

public:
  TiXmlBase () {}
  virtual ~TiXmlBase () {}

  /**
   * All TinyXml classes can print themselves to a filestream.
   * This is a formatted print, and will insert tabs and newlines.
   * (For an unformatted stream, use the << operator.)
   */
  virtual void Print( FILE* cfile, int depth ) const = 0;

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

  /**
   * Reads an XML name into the string provided. Returns
   * a pointer just past the last character of the name,
   * or 0 if the function has an error.
   */
  static const char* ReadName( const char* p, TIXML_STRING* name );
  static const char* ReadName( const char* p, char* name );

  /**
   * Reads text. Returns a pointer past the given end tag.
   * Wickedly complex options, but it keeps the (sensitive) code in one place.
   */
  static const char* ReadText(	const char* in, TIXML_STRING* text,
				bool ignoreWhiteSpace,
				const char* endTag);
  static const char* ReadText(	const char* in, char** text,
				bool ignoreWhiteSpace,
				const char* endTag);

  /**
   * Puts a string to a stream, expanding entities as it goes.
   * Note this should not contian the '<', '>', etc, or they will be
   * transformed into entities!
   */
  static void PutString( const TIXML_STRING& str, TIXML_OSTREAM* out );
  static void PutString( const TIXML_STRING& str, TIXML_STRING* out );

protected:
  // See STL_STRING_BUG
  // Utility class to overcome a bug.
  class StringToBuffer
  {
  public:
    StringToBuffer( const TIXML_STRING& str );
    ~StringToBuffer();
    char* buffer;
  };

  virtual void StreamOut (TIXML_OSTREAM *) const = 0;
  virtual const char* Parse( TiDocument* document, const char* p ) = 0;

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
 * in a document, or stand on its own. The type of a TiDocumentNode
 * can be queried, and it can be cast to its more defined type.
 */
class TiDocumentNode : public TiXmlBase
{
  friend class TiDocument;
  friend class TiDocumentNodeChildren;
  friend class TiXmlElement;

public:
  // Used internally, not part of the public API.
  friend TIXML_OSTREAM& operator<< (TIXML_OSTREAM& out,
  	const TiDocumentNode& base);

  /**
   * The types of XML nodes supported by TinyXml. (All the
   * unsupported types are picked up by UNKNOWN.)
   */
  enum NodeType
  {
    DOCUMENT, ELEMENT, COMMENT, UNKNOWN, TEXT, DECLARATION, TYPECOUNT
  };

  virtual ~TiDocumentNode();

  /**
   * The meaning of 'value' changes for the specific type of
   * TiDocumentNode.
   * @verbatim
   * Document:	filename of the xml file
   * Element:	name of the element
   * Comment:	the comment text
   * Unknown:	the tag contents
   * Text:		the text string
   * @endverbatim
   * The subclasses will wrap this function.
   */
  virtual const char * Value () const = 0;

  /**
   * Changes the value of the node.
   */
  virtual void SetValue (const char * _value) = 0;

  /// One step up the DOM.
  TiDocumentNodeChildren* Parent() const{ return parent; }

  /// Navigate to a sibling node.
  TiDocumentNode* PreviousSibling() const { return prev; }

  /// Navigate to a sibling node.
  TiDocumentNode* PreviousSibling( const char * ) const;

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
  virtual int Type() const = 0;

  /**
   * Return a pointer to the Document this node lives in.
   * Returns null if not in a document.
   */
  TiDocument* GetDocument() const;

  TiDocumentNodeChildren* ToDocumentNodeChildren() const
  {
    int t = Type ();
    return ( t == DOCUMENT || t == ELEMENT )
    	? (TiDocumentNodeChildren*) this
	: 0;
  }
  TiDocument* ToDocument() const
  { return ( Type () == DOCUMENT ) ? (TiDocument*) this : 0; }
  TiXmlElement*  ToElement() const
  { return ( Type () == ELEMENT  ) ? (TiXmlElement*)  this : 0; }
  TiXmlComment*  ToComment() const
  { return ( Type () == COMMENT  ) ? (TiXmlComment*)  this : 0; }
  TiXmlUnknown*  ToUnknown() const
  { return ( Type () == UNKNOWN  ) ? (TiXmlUnknown*)  this : 0; }
  TiXmlText*   ToText()    const
  { return ( Type () == TEXT     ) ? (TiXmlText*)     this : 0; }
  TiXmlDeclaration* ToDeclaration() const
  { return ( Type () == DECLARATION ) ? (TiXmlDeclaration*) this : 0; }

  virtual TiDocumentNode* Clone() const = 0;

protected:
  TiDocumentNode( );

  void CopyToClone( TiDocumentNode* target ) const
  {
    target->SetValue (Value () );
  }

  TiDocumentNodeChildren* parent;

  TiDocumentNode* prev;
  TiDocumentNode* next;
};

/**
 * A document node with children.
 */
class TiDocumentNodeChildren : public TiDocumentNode
{
public:
  /// Construct an element.
  TiDocumentNodeChildren ();

  virtual ~TiDocumentNodeChildren();

  /// Delete all the children of this node. Does not affect 'this'.
  void Clear();

  /// Returns true if this node has no children.
  bool NoChildren() const { return !firstChild; }

  TiDocumentNode* FirstChild()	const	{ return firstChild; }
  TiDocumentNode* FirstChild( const char * value ) const;

  TiDocumentNode* LastChild() const	{ return lastChild; }
  TiDocumentNode* LastChild( const char * value ) const;

  /**
   * An alternate way to walk the children of a node.
   * One way to iterate over nodes is:
   * @verbatim
   * for( child = parent->FirstChild(); child; child = child->NextSibling() )
   * @endverbatim
   *
   * IterateChildren does the same thing with the syntax:
   * @verbatim
   * child = 0;
   * while( child = parent->IterateChildren( child ) )
   * @endverbatim
   *
   * IterateChildren takes the previous child as input and finds
   * the next one. If the previous child is null, it returns the
   * first. IterateChildren will return null when done.
   */
  TiDocumentNode* IterateChildren( TiDocumentNode* previous ) const;

  /**
   * This flavor of IterateChildren searches for children with a particular
   * 'value'.
   */
  TiDocumentNode* IterateChildren( const char * value,
  	TiDocumentNode* previous ) const;

  /**
   * Add a new node related to this. Adds a child past the LastChild.
   * Returns a pointer to the new object or NULL if an error occured.
   */
  TiDocumentNode* InsertEndChild( const TiDocumentNode& addThis );

  /**
   * Add a new node related to this. Adds a child before the specified child.
   * Returns a pointer to the new object or NULL if an error occured.
   */
  TiDocumentNode* InsertBeforeChild( TiDocumentNode* beforeThis,
  	const TiDocumentNode& addThis );

  /**
   * Add a new node related to this. Adds a child after the specified child.
   * Returns a pointer to the new object or NULL if an error occured.
   */
  TiDocumentNode* InsertAfterChild(  TiDocumentNode* afterThis,
  	const TiDocumentNode& addThis );

  /**
   * Replace a child of this node.
   * Returns a pointer to the new object or NULL if an error occured.
   */
  TiDocumentNode* ReplaceChild( TiDocumentNode* replaceThis,
  	const TiDocumentNode& withThis );

  /// Delete a child of this node.
  bool RemoveChild( TiDocumentNode* removeThis );

  /// Convenience function to get through elements.
  TiXmlElement* FirstChildElement()	const;

  /// Convenience function to get through elements.
  TiXmlElement* FirstChildElement( const char * value ) const;

protected:
  // Figure out what is at *p, and parse it. Returns null if it is not an xml
  // node.
  TiDocumentNode* Identify( TiDocument* document, const char* start );

  // The node is passed in by ownership. This object will delete it.
  TiDocumentNode* LinkEndChild( TiDocumentNode* addThis );

  TiDocumentNode* firstChild;
  TiDocumentNode* lastChild;
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
  TiDocumentAttribute() { name = NULL; value = NULL; }
  ~TiDocumentAttribute () { delete[] value; }

  const char* Name()  const { return name; }
  const char* Value() const { return value; }
  char* Value() { return value; }
  const int IntValue() const;
  const double DoubleValue() const;

  void SetName( const char* _name )	{ name = _name; }
  void SetValue( const char* _value )
  {
    delete[] value;
    value = csStrNew (_value);
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

  /*	[internal use]
   * Attribute parsing starts: first letter of the name
   * returns: the next char after the value end quote
   */
  const char* Parse( TiDocument* document, const char* p );

  // [internal use]
  void Print( FILE* cfile, int depth ) const;

  void StreamOut( TIXML_OSTREAM * out ) const;

private:
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
  csArray<TiDocumentAttribute> set;

  TiDocumentAttributeSet() : set (0, 4) { }
  int Find (const char * name) const;
  int FindExact (const char * reg_name) const;
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

  virtual ~TiXmlElement();

  virtual int Type() const { return ELEMENT; }

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
  int GetAttributeCount () const { return attributeSet.set.Length (); }
  /// Get attribute.
  const TiDocumentAttribute& GetAttribute (int idx) const
  {
    return attributeSet.set[idx];
  }
  /// Get attribute.
  TiDocumentAttribute& GetAttribute (int idx)
  {
    return attributeSet.set[idx];
  }

  /**
   * Deletes an attribute with the given name.
   */
  void RemoveAttribute( const char * name );
  // [internal use] Creates a new Element and returs it.
  virtual TiDocumentNode* Clone() const;
  // [internal use]

  virtual void Print( FILE* cfile, int depth ) const;

  virtual const char * Value () const { return value; }
  void SetValueRegistered (const char * _value)
  {
    value = _value;
  }
  virtual void SetValue (const char * _value);

protected:
  // Used to be public [internal use]
  virtual void StreamOut( TIXML_OSTREAM * out ) const;

  /*	[internal use]
   * Attribtue parsing starts: next char past '<'
   * returns: next char past '>'
   */
  virtual const char* Parse( TiDocument* document, const char* p );

  /*	[internal use]
   * Reads the "value" of the element -- another element, or text.
   * This should terminate with the current end tag.
   */
  const char* ReadValue( TiDocument* document, const char* in );

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
  TiXmlComment() {}
  virtual ~TiXmlComment()	{}
  virtual int Type() const { return COMMENT; }

  // [internal use] Creates a new Element and returs it.
  virtual TiDocumentNode* Clone() const;
  // [internal use]
  virtual void Print( FILE* cfile, int depth ) const;
  virtual const char * Value () const { return value.c_str (); }
  virtual void SetValue (const char * _value) { value = _value;}

protected:
  // used to be public
  virtual void StreamOut( TIXML_OSTREAM * out ) const;
  /*	[internal use]
   * Attribtue parsing starts: at the ! of the !--
   * returns: next char past '>'
   */
  virtual const char* Parse( TiDocument* document, const char* p );

  TIXML_STRING value;
};

/**
 * XML text. Contained in an element.
 */
class TiXmlText : public TiDocumentNode
{
  friend class TiXmlElement;

public:
  /// Constructor.
  TiXmlText (const char * initValue)
  {
    value = NULL;
    SetValue( initValue );
  }
  virtual ~TiXmlText()
  {
    delete[] value;
  }
  virtual int Type() const { return TEXT; }
  virtual const char * Value () const { return value; }
  virtual void SetValue (const char * _value)
  {
    delete[] value;
    if (_value)
      value = csStrNew (_value);
    else
      value = NULL;
  }

protected :
  // [internal use] Creates a new Element and returns it.
  virtual TiDocumentNode* Clone() const;
  // [internal use]
  virtual void Print( FILE* cfile, int depth ) const;
  virtual void StreamOut ( TIXML_OSTREAM * out ) const;
  // [internal use]
  bool Blank() const;	// returns true if all white space and new lines
  /*	[internal use]
   * Attribtue parsing starts: First char of the text
   * returns: next char past '>'
   */
  virtual const char* Parse( TiDocument* document,  const char* p );

  char* value;
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
  TiXmlCData (const char * initValue) : TiXmlText (initValue)
  {
  }
  virtual ~TiXmlCData() {}

protected :
  virtual const char* Parse( TiDocument* document,  const char* p );
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
  TiXmlDeclaration() { }

  /// Construct.
  TiXmlDeclaration::TiXmlDeclaration( const char * _version,
		const char * _encoding, const char * _standalone );

  virtual ~TiXmlDeclaration() {}
  virtual int Type() const { return DECLARATION; }

  /// Version. Will return empty if none was found.
  const char * Version() const { return version.c_str (); }
  /// Encoding. Will return empty if none was found.
  const char * Encoding() const { return encoding.c_str (); }
  /// Is this a standalone document?
  const char * Standalone() const { return standalone.c_str (); }

  // [internal use] Creates a new Element and returs it.
  virtual TiDocumentNode* Clone() const;
  // [internal use]
  virtual void Print( FILE* cfile, int depth ) const;
  virtual const char * Value () const { return value.c_str (); }
  virtual void SetValue (const char * _value) { value = _value;}

protected:
  // used to be public
  virtual void StreamOut ( TIXML_OSTREAM * out) const;
  //	[internal use]
  //	Attribtue parsing starts: next char past '<'
  //					 returns: next char past '>'
  virtual const char* Parse( TiDocument* document,  const char* p );

private:
  TIXML_STRING version;
  TIXML_STRING encoding;
  TIXML_STRING standalone;
  TIXML_STRING value;
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
  TiXmlUnknown() { }
  virtual ~TiXmlUnknown() {}
  virtual int Type() const { return UNKNOWN; }

  // [internal use]
  virtual TiDocumentNode* Clone() const;
  // [internal use]
  virtual void Print( FILE* cfile, int depth ) const;
  virtual const char * Value () const { return value.c_str (); }
  virtual void SetValue (const char * _value) { value = _value;}
protected:
  // used to be public
  virtual void StreamOut ( TIXML_OSTREAM * out ) const;
  /*	[internal use]
   * Attribute parsing starts: First char of the text
   * returns: next char past '>'
   */
  virtual const char* Parse( TiDocument* document,  const char* p );

  TIXML_STRING value;
};


/**
 * Always the top level node. A document binds together all the
 * XML pieces. It can be saved, loaded, and printed to the screen.
 * The 'value' of a document node is the xml file name.
 */
class TiDocument : public TiDocumentNodeChildren
{
public:
  /// Interned strings.
  csStringSet strings;

  /// Create an empty document, that has no name.
  TiDocument();
  /**
   * Create a document with a name. The name of the document is also the
   * filename of the xml.
   */
  TiDocument( const char * documentName );

  virtual ~TiDocument() {}
  virtual int Type() const { return DOCUMENT; }

  virtual const char * Value () const { return value.c_str (); }
  virtual void SetValue (const char * _value) { value = _value;}

  /**
   * Load a file using the current document value.
   * Returns true if successful. Will delete any existing
   * document data before loading.
   */
  bool LoadFile();
  /// Save a file using the current document value. Returns true if successful.
  bool SaveFile() const;
  /// Load a file using the given filename. Returns true if successful.
  bool LoadFile( const char * filename );
  /// Save a file using the given filename. Returns true if successful.
  bool SaveFile( const char * filename ) const;

  /// Parse the given null terminated block of xml data.
  virtual const char* Parse( TiDocument* document,  const char* p );

  /**
   * Get the root element -- the only top level element -- of the document.
   * In well formed XML, there should only be one. TinyXml is tolerant of
   * multiple elements at the document level.
   */
  TiXmlElement* RootElement() const { return FirstChildElement(); }

  /// If, during parsing, a error occurs, Error will be set to true.
  bool Error() const { return error; }

  /// Contains a textual (english) description of the error if one occurs.
  const char * ErrorDesc() const { return errorDesc.c_str (); }

  /**
   * Generally, you probably want the error string ( ErrorDesc() ). But if you
   * prefer the ErrorId, this function will fetch it.
   */
  const int ErrorId() const { return errorId; }

  /// If you have handled the error, it can be reset with this call.
  void ClearError() { error = false; errorId = 0; errorDesc = ""; }

  /**
   * Dump the document to standard out.
   */
  void Print() const { Print( stdout, 0 ); }

  // [internal use]
  virtual void Print( FILE* cfile, int depth = 0 ) const;
  // [internal use]
  void SetError( int err )
  {
    error   = true;
    errorId = err;
    errorDesc = errorString[ errorId ];
  }

protected :
  virtual void StreamOut ( TIXML_OSTREAM * out) const;
  // [internal use]
  virtual TiDocumentNode* Clone() const;

private:
  bool error;
  int  errorId;
  TIXML_STRING errorDesc;
  TIXML_STRING value;
};

#endif

