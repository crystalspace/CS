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

#include "cssysdef.h"
#include <ctype.h>
#include "tinyxml.h"
#include "csutil/scfstr.h"

const char* TiXmlBase::errorString[ TIXML_ERROR_STRING_COUNT ] =
{
  "No error",
  "Error",
  "Failed to open file",
  "Memory allocation failed.",
  "Error parsing Element.",
  "Failed to read Element name",
  "Error reading Element value.",
  "Error reading Attributes.",
  "Error: empty tag.",
  "Error reading end tag.",
  "Error parsing Unknown.",
  "Error parsing Comment.",
  "Error parsing Declaration.",
  "Error document empty."
};

bool TiXmlBase::condenseWhiteSpace = true;

void TiXmlBase::PutString( const TiXmlString& str, TiXmlString* outString )
{
  int i=0;

  while( i<(int)str.length() )
  {
    int c = str[i];

    if (    c == '&' 
         && i < ( (int)str.length() - 2 )
       && str[i+1] == '#'
       && str[i+2] == 'x' )
    {
      // Hexadecimal character reference.
      // Pass through unchanged.
      // &#xA9;  -- copyright symbol, for example.
      while ( i<(int)str.length() )
      {
        outString->append( str.c_str() + i, 1 );
        ++i;
        if ( str[i] == ';' )
          break;
      }
    }
    else if ( c == '&' )
    {
      outString->append( entity[0].str, entity[0].strLength );
      ++i;
    }
    else if ( c == '<' )
    {
      outString->append( entity[1].str, entity[1].strLength );
      ++i;
    }
    else if ( c == '>' )
    {
      outString->append( entity[2].str, entity[2].strLength );
      ++i;
    }
    else if ( c == '\"' )
    {
      outString->append( entity[3].str, entity[3].strLength );
      ++i;
    }
    else if ( c == '\'' )
    {
      outString->append( entity[4].str, entity[4].strLength );
      ++i;
    }
    else if ( c < 32 || c > 126 )
    {
      // Easy pass at non-alpha/numeric/symbol
      // 127 is the delete key. Below 32 is symbolic.
      csString buf;
      buf.Format ("&#x%02X;", (unsigned) ( c & 0xff ) );
      outString->append( buf, strlen( buf ) );
      ++i;
    }
    else
    {
      char realc = (char) c;
      outString->append( &realc, 1 );
      ++i;
    }
  }
}


TiDocumentNode::TiDocumentNode( )
{
  parent = 0;
  prev = 0;
  next = 0;
}


TiDocumentNode::~TiDocumentNode()
{
}


TiDocumentNode* TiDocumentNode::NextSibling( const char * value ) const
{
  TiDocumentNode* node;
  for ( node = next; node; node = node->next )
  {
    const char* node_val = node->Value ();
    if (node_val && strcmp (node_val, value) == 0)
      return node;
  }
  return 0;
}


TiDocumentNode* TiDocumentNode::PreviousSibling( const char * value ) const
{
  TiDocumentNode* node;
  for ( node = prev; node; node = node->prev )
  {
    const char* node_val = node->Value ();
    if (node_val && strcmp (node_val, value) == 0)
      return node;
  }
  return 0;
}

TiDocumentNode* TiDocumentNodeChildren::Identify( TiDocument* document, const char* p )
{
  TiDocumentNode* returnNode = 0;

  p = SkipWhiteSpace( p );
  if( !p || !*p || *p != '<' )
  {
    return 0;
  }

  p = SkipWhiteSpace( p );

  if ( !p || !*p )
  {
    return 0;
  }

  // What is this thing? 
  // - Elements start with a letter or underscore, but xml is reserved.
  // - Comments: <!--
  // - Decleration: <?xml
  // - Everthing else is unknown to tinyxml.
  //

  const char* xmlHeader = { "<?xml" };
  const char* commentHeader = { "<!--" };

  if ( StringEqual( p, xmlHeader) )
  {
    returnNode = new TiXmlDeclaration();
  }
  else if (    isalpha( *(p+1) )
        || *(p+1) == '_' )
  {
    returnNode = document->blk_element.Alloc ();
  }
  else if ( StringEqual ( p, commentHeader) )
  {
    returnNode = new TiXmlComment();
  }
  else
  {
    returnNode = new TiXmlUnknown();
  }

  if ( returnNode )
  {
    // Set the parent, so it can report errors
    returnNode->parent = this;
    //p = returnNode->Parse( p );
  }
  else
  {
    document->SetError( TIXML_ERROR_OUT_OF_MEMORY );
  }
  return returnNode;
}

// -------------------------------------------------------------------------
TiDocumentNodeChildren::TiDocumentNodeChildren ()
{
  firstChild = 0;
  lastChild = 0;
}

TiDocumentNodeChildren::~TiDocumentNodeChildren()
{
  TiDocumentNode* node = firstChild;
  TiDocumentNode* temp = 0;

  TiDocument* doc = GetDocument ();
  while ( node )
  {
    temp = node;
    node = node->next;
    doc->DeleteNode (temp);
  }  
}

void TiDocumentNodeChildren::Clear()
{
  TiDocumentNode* node = firstChild;
  TiDocumentNode* temp = 0;

  TiDocument* doc = GetDocument ();
  while ( node )
  {
    temp = node;
    node = node->next;
    doc->DeleteNode (temp);
  }

  firstChild = 0;
  lastChild = 0;
}

TiDocumentNode* TiDocumentNodeChildren::LinkEndChild( TiDocumentNode* node )
{
  node->parent = this;

  node->prev = lastChild;
  node->next = 0;

  if ( lastChild )
    lastChild->next = node;
  else
    firstChild = node;      // it was an empty list.

  lastChild = node;
  return node;
}


TiDocumentNode* TiDocumentNodeChildren::InsertEndChild( const TiDocumentNode& addThis )
{
  TiDocumentNode* node = addThis.Clone(GetDocument ());
  if ( !node )
    return 0;

  return LinkEndChild( node );
}


TiDocumentNode* TiDocumentNodeChildren::InsertBeforeChild(
  TiDocumentNode* beforeThis, const TiDocumentNode& addThis )
{  
  if ( !beforeThis || beforeThis->parent != this )
    return 0;

  TiDocumentNode* node = addThis.Clone(GetDocument ());
  if ( !node )
    return 0;
  node->parent = this;

  node->next = beforeThis;
  node->prev = beforeThis->prev;
  if ( beforeThis->prev )
  {
    beforeThis->prev->next = node;
  }
  else
  {
    assert( firstChild == beforeThis );
    firstChild = node;
  }
  beforeThis->prev = node;
  return node;
}


bool TiDocumentNodeChildren::RemoveChild( TiDocumentNode* removeThis )
{
  if ( removeThis->parent != this )
  {  
    assert( 0 );
    return false;
  }

  if ( removeThis->next )
    removeThis->next->prev = removeThis->prev;
  else
    lastChild = removeThis->prev;

  if ( removeThis->prev )
    removeThis->prev->next = removeThis->next;
  else
    firstChild = removeThis->next;

  GetDocument ()->DeleteNode (removeThis);

  return true;
}

TiDocumentNode* TiDocumentNodeChildren::FirstChild( const char * value ) const
{
  TiDocumentNode* node;
  for ( node = firstChild; node; node = node->next )
  {
    const char* node_val = node->Value ();
    if (node_val && strcmp (node_val, value) == 0)
      return node;
  }
  return 0;
}

// -------------------------------------------------------------------------

void TiXmlElement::RemoveAttribute( const char * name )
{
  size_t nodeidx = attributeSet.Find (name);
  if ( nodeidx != (size_t)-1 )
  {
    attributeSet.set.DeleteIndex (nodeidx);
  }
}

TiXmlElement* TiDocumentNode::NextSiblingElement() const
{
  TiDocumentNode* node;

  for (  node = NextSibling();
  node;
  node = node->NextSibling() )
  {
    if ( node->ToElement() )
      return node->ToElement();
  }
  return 0;
}

TiXmlElement* TiDocumentNode::NextSiblingElement( const char * value ) const
{
  TiDocumentNode* node;

  for (  node = NextSibling( value );
  node;
  node = node->NextSibling( value ) )
  {
    if ( node->ToElement() )
      return node->ToElement();
  }
  return 0;
}



TiDocument* TiDocumentNode::GetDocument() const
{
  const TiDocumentNode* node;

  for( node = this; node; node = node->parent )
  {
    if ( node->ToDocument() )
      return node->ToDocument();
  }
  return 0;
}


TiXmlElement::TiXmlElement ()
{
  value = 0;
  type = ELEMENT;
}

TiXmlElement::~TiXmlElement()
{
}


void TiXmlElement::SetValue (const char * name)
{
  if (name == 0)
  {
    value = 0;
  }
  else
  {
    TiDocument* document = GetDocument ();
    csStringID name_id = document->strings.Request (name);
    const char* reg_name = document->strings.Request (name_id);
    value = reg_name;
  }
}

const char * TiXmlElement::Attribute( const char * name ) const
{
  size_t nodeidx = attributeSet.Find (name);

  if (nodeidx != (size_t)-1)
    return attributeSet.set[nodeidx].Value ();

  return 0;
}


const char * TiXmlElement::Attribute( const char * name, int* i ) const
{
  const char * s = Attribute( name );
  if ( i )
  {
    if ( s )
      *i = atoi( s );
    else
      *i = 0;
  }
  return s;
}


void TiXmlElement::SetAttribute( TiDocument* document,
  const char * name, int val )
{  
  csString buf;
  buf.Format ("%d", val);
  SetAttribute( document, name, buf );
}


TiDocumentAttribute& TiXmlElement::GetAttributeRegistered (
  const char * reg_name)
{
  size_t nodeidx = attributeSet.FindExact (reg_name);
  if (nodeidx != (size_t)-1)
  {
    return attributeSet.set[nodeidx];
  }

  TiDocumentAttribute at;
  size_t idx = attributeSet.set.Push (at);
  attributeSet.set[idx].SetName (reg_name);
  return attributeSet.set[idx];
}

void TiXmlElement::SetAttribute (TiDocument* document,
  const char * name, const char * value)
{
  csStringID name_id = document->strings.Request (name);
  const char* reg_name = document->strings.Request (name_id);
  GetAttributeRegistered (reg_name).SetValue (value);
}

static void StrPrintf (iString* file, const char* msg, ...)
{
  scfString str;
  va_list args;
  va_start (args, msg);
  str.FormatV (msg, args);
  va_end (args);
  file->SetGrowsBy(0);
  file->Append (str);
}

static void StrPuts (const char* msg, iString* file)
{
  file->Append (msg);
}

void TiXmlElement::Print( iString* cfile, int depth ) const
{
  int d;
  for ( d=0; d<depth; d++ )
  {
    StrPrintf ( cfile, "    " );
  }

  StrPrintf ( cfile, "<%s", value );

  for (size_t i = 0 ; i < attributeSet.set.Length () ; i++)
  {
    const TiDocumentAttribute& attrib = attributeSet.set[i];
    StrPrintf ( cfile, " " );
    attrib.Print( cfile, depth );
  }

  // There are 3 different formatting approaches:
  // 1) An element without children is printed as a <foo /> node
  // 2) An element with only a text child is printed as <foo> text </foo>
  // 3) An element with children is printed on multiple lines.
  TiDocumentNode* node;
  if ( !firstChild )
  {
    StrPrintf ( cfile, " />" );
  }
  else if ( firstChild == lastChild && firstChild->ToText() )
  {
    StrPrintf ( cfile, ">" );
    firstChild->Print( cfile, depth + 1 );
    StrPrintf ( cfile, "</%s>", value );
  }
  else
  {
    StrPrintf ( cfile, ">" );

    for ( node = firstChild; node; node=node->NextSibling() )
    {
      if ( !node->ToText() )
      {
        StrPrintf ( cfile, "\n" );
      }
      node->Print( cfile, depth+1 );
    }
    StrPrintf ( cfile, "\n" );
    for( d=0; d<depth; ++d )
    StrPrintf ( cfile, "    " );
    StrPrintf ( cfile, "</%s>", value );
  }
}

TiDocumentNode* TiXmlElement::Clone(TiDocument* document) const
{
  TiXmlElement* clone = document->blk_element.Alloc ();
  if ( !clone )
    return 0;

  clone->SetValueRegistered (Value ());
  CopyToClone( clone );

  // Clone the attributes, then clone the children.
  size_t i;
  for (i = 0 ; i < attributeSet.set.Length () ; i++)
  {
    const TiDocumentAttribute& attrib = attributeSet.set[i];
    clone->GetAttributeRegistered (attrib.Name ()).
      SetValue (attrib.Value ());
  }

  TiDocumentNode* node = 0;
  for ( node = firstChild; node; node = node->NextSibling() )
  {
    clone->LinkEndChild( node->Clone(document) );
  }
  return clone;
}


TiDocument::TiDocument() :
  strings (3541),
  blk_element (1000),
  blk_text (1000)
{
  error = false;
  //  ignoreWhiteSpace = true;
  type = DOCUMENT;
}

TiDocument::TiDocument( const char * documentName ) :
  strings (3541),
  blk_element (1000),
  blk_text (1000)
{
  //  ignoreWhiteSpace = true;
  value = documentName;
  error = false;
  type = DOCUMENT;
}

TiDocument::~TiDocument ()
{
  // Call explicit clear so that all children are destroyed
  // before 'blk_element' and 'blk_text' are destroyed.
  Clear ();
}

TiDocumentNode* TiDocument::Clone(TiDocument* document) const
{
  TiDocument* clone = new TiDocument();
  if ( !clone )
    return 0;

  CopyToClone( clone );
  clone->error = error;
  clone->errorDesc = errorDesc.c_str ();

  TiDocumentNode* node = 0;
  for ( node = firstChild; node; node = node->NextSibling() )
  {
    clone->LinkEndChild( node->Clone(document) );
  }
  return clone;
}


void TiDocument::Print( iString* cfile, int depth ) const
{
  TiDocumentNode* node;
  for ( node=FirstChild(); node; node=node->NextSibling() )
  {
    node->Print( cfile, depth );
    StrPrintf ( cfile, "\n" );
  }
}

void TiDocumentAttribute::Print( iString* cfile, int /*depth*/ ) const
{
  TiXmlString n, v;

  TiXmlBase::PutString( Name(), &n );
  TiXmlBase::PutString( Value(), &v );

  char* idx = strchr (value, '\"');
  if (idx == 0)
    StrPrintf  (cfile, "%s=\"%s\"", n.c_str(), v.c_str() );
  else
    StrPrintf  (cfile, "%s='%s'", n.c_str(), v.c_str() );
}


void TiDocumentAttribute::SetIntValue( int value )
{
  csString buf;
  buf.Format ("%d", value);
  SetValue (buf);
}

void TiDocumentAttribute::SetDoubleValue( double value )
{
  csString buf;
  buf.Format ("%f", value);
  SetValue (buf);
}

int TiDocumentAttribute::IntValue() const
{
  return atoi (value);
}

double  TiDocumentAttribute::DoubleValue() const
{
  return atof (value);
}

void TiXmlComment::Print( iString* cfile, int depth ) const
{
  for ( int i=0; i<depth; i++ )
  {
    StrPuts ( "    ", cfile );
  }
  StrPrintf ( cfile, "<!--%s-->", value );
}

TiDocumentNode* TiXmlComment::Clone(TiDocument* /*document*/) const
{
  TiXmlComment* clone = new TiXmlComment();

  if ( !clone )
    return 0;

  CopyToClone( clone );
  return clone;
}


void TiXmlText::SetValue (const char * name)
{
  if (name == 0)
  {
    value = 0;
  }
  else
  {
    TiDocument* document = GetDocument ();
    csStringID name_id = document->strings.Request (name);
    const char* reg_name = document->strings.Request (name_id);
    value = reg_name;
  }
}

void TiXmlText::Print( iString* cfile, int /*depth*/ ) const
{
  bool printCData = 
    (strchr (value, '\r') != 0) || (strchr (value, '\n') != 0);

  if (printCData)
  {
    StrPrintf ( cfile, "<![CDATA[%s]]>", value );
  }
  else
  {
    TiXmlString buffer;
    PutString( value, &buffer );
    StrPrintf ( cfile, "%s", buffer.c_str() );
  }
}


TiDocumentNode* TiXmlText::Clone(TiDocument* document) const
{  
  TiXmlText* clone = 0;
  clone = document->blk_text.Alloc ();

  if ( !clone )
    return 0;

  CopyToClone( clone );
  return clone;
}


TiXmlDeclaration::TiXmlDeclaration( const char * _version,
  const char * _encoding,
  const char * _standalone )
{
  version = _version;
  encoding = _encoding;
  standalone = _standalone;
  type = DECLARATION;
}


void TiXmlDeclaration::Print( iString* cfile, int /*depth*/ ) const
{
  StrPrintf  (cfile, "<?xml ");

  if ( !version.empty() )
    StrPrintf  (cfile, "version=\"%s\" ", version.c_str ());
  if ( !encoding.empty() )
    StrPrintf  (cfile, "encoding=\"%s\" ", encoding.c_str ());
  if ( !standalone.empty() )
    StrPrintf  (cfile, "standalone=\"%s\" ", standalone.c_str ());
  StrPrintf  (cfile, "?>");
}

TiDocumentNode* TiXmlDeclaration::Clone(TiDocument* /*document*/) const
{  
  TiXmlDeclaration* clone = new TiXmlDeclaration();

  if ( !clone )
    return 0;

  CopyToClone( clone );
  clone->version = version;
  clone->encoding = encoding;
  clone->standalone = standalone;
  return clone;
}


void TiXmlUnknown::Print( iString* cfile, int depth ) const
{
  for ( int i=0; i<depth; i++ )
    StrPrintf ( cfile, "    " );
  StrPrintf ( cfile, "<%s>", value.c_str() );
}

TiDocumentNode* TiXmlUnknown::Clone(TiDocument* /*document*/) const
{
  TiXmlUnknown* clone = new TiXmlUnknown();

  if ( !clone )
    return 0;

  CopyToClone( clone );
  return clone;
}


size_t TiDocumentAttributeSet::Find (const char * name) const
{
  size_t i;
  for (i = 0 ; i < set.Length () ; i++)
  {
    if (strcmp (set[i].name, name) == 0) return i;
  }
  return (size_t)-1;
}

size_t TiDocumentAttributeSet::FindExact (const char * reg_name) const
{
  size_t i;
  for (i = 0 ; i < set.Length () ; i++)
  {
    if (set[i].name == reg_name) return i;
  }
  return (size_t)-1;
}

