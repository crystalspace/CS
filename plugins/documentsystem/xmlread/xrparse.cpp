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
#include "xr.h"
#include <ctype.h>

TrXmlBase::Entity TrXmlBase::entity[ NUM_ENTITY ] = 
{
  { "&amp;",  5, '&' },
  { "&lt;",   4, '<' },
  { "&gt;",   4, '>' },
  { "&quot;", 6, '\"' },
  { "&apos;", 6, '\'' }
};


char* TrXmlBase::SkipWhiteSpace( char* p )
{
  if ( !p || !*p )
  {
    return 0;
  }
  while ( isspace ((unsigned char)*p)) p++;
  return p;
}

const char* TrXmlBase::SkipWhiteSpace( const char* p )
{
  if ( !p || !*p )
  {
    return 0;
  }
  while ( isspace ((unsigned char)*p)) p++;
  return p;
}

char* TrXmlBase::ReadName( char* p)
{
  // Names start with letters or underscores.
  // After that, they can be letters, underscores, numbers,
  // hyphens, or colons. (Colons are valid ony for namespaces,
  // but tinyxml can't tell namespaces from names.)
  if (p && *p && (isalpha ((unsigned char) *p) || *p == '_'))
  {
    while ((isalnum ((unsigned char) *p) 
	|| *p == '_' || *p == '-' || *p == ':'))
    {
      p++;
    }
    return p;
  }
  return 0;
}

char* TrXmlBase::ReadName( char* p, char* name)
{
  // Names start with letters or underscores.
  // After that, they can be letters, underscores, numbers,
  // hyphens, or colons. (Colons are valid ony for namespaces,
  // but tinyxml can't tell namespaces from names.)
  if (p && *p && (isalpha ((unsigned char) *p) || *p == '_'))
  {
    char* pname = name;
    while ((isalnum ((unsigned char) *p) 
	|| *p == '_' || *p == '-' || *p == ':'))
    {
      *pname++ = *p++;
    }
    *pname = 0;
    return p;
  }
  *name = 0;
  return 0;
}

char* TrXmlBase::GetEntity( char* p, char* value )
{
  // Presume an entity, and pull it out.
  int i;

  // Ignore the &#x entities.
  if (    strncmp( "&#x", p, 3 ) == 0 
       && *(p+3) 
     && *(p+4) )
  {
    *value = 0;
    
    if ( isalpha( *(p+3) ) ) *value += ( tolower( *(p+3) ) - 'a' + 10 ) * 16;
    else             *value += ( *(p+3) - '0' ) * 16;

    if ( isalpha( *(p+4) ) ) *value += ( tolower( *(p+4) ) - 'a' + 10 );
    else             *value += ( *(p+4) - '0' );

    return p+6;
  }

  // Now try to match it.
  for( i=0; i<NUM_ENTITY; ++i )
  {
    if ( strncmp( entity[i].str, p, entity[i].strLength ) == 0 )
    {
      //assert( strlen( entity[i].str ) == entity[i].strLength );
      *value = entity[i].chr;
      return ( p + entity[i].strLength );
    }
  }

  // So it wasn't an entity, its unrecognized, or something like that.
  *value = *p;  // Don't put back the last one, since we return it!
  return p+1;
}

bool TrXmlBase::StringEqual( const char* p, const char* tag)
{
  //assert( p );
  if ( !p)
  {
    //assert( 0 );
    return false;
  }

  while ( *p == *tag && *p )
  {
    ++p;
    ++tag;
  }

  if ( *tag == 0 )    // Have we found the end of the tag, and everything equal?
  {
    return true;
  }
  return false;
}

bool TrXmlBase::StringEqualIgnoreCase( const char* p,
         const char* tag)
{
  //assert( p );
  if ( !p)
  {
    //assert( 0 );
    return false;
  }

  while ( tolower( *(p) ) == tolower( *tag ) && *p )
  {
    ++p;
    ++tag;
  }

  if ( *tag == 0 )
  {
    return true;
  }
  return false;
}

char* TrXmlBase::ReadText(char* p,
	char*& buf, int& buflen,
        bool trimWhiteSpace, 
        const char* endTag)
{
  buf = p;
  char* out = p;

  if (!trimWhiteSpace    // certain tags always keep whitespace
      || !condenseWhiteSpace )  // if true, whitespace is always kept
  {
    // Keep all the white space.
    while (*p && !StringEqual ( p, endTag))
    {
      char c;
      p = GetChar( p, &c );
      *out++ = c;
    }
  }
  else
  {
    bool whitespace = false;
    bool first = true;

    // Remove leading white space:
    p = SkipWhiteSpace( p );
    buf = p;
    out = p;
    while ( *p && !StringEqual ( p, endTag) )
    {
      if ( isspace( *p ) )
      {
        whitespace = true;
        ++p;
	if (first) { buf = p; out = p; }
      }
      else
      {
        // If we've found whitespace, add it before the
        // new character. Any whitespace just becomes a space.
        if ( whitespace )
        {
	  *out++ = ' ';
          whitespace = false;
        }
        char c;
        p = GetChar( p, &c );
	*out++ = c;
	first = false;
      }
    }
  }
  // *out++ = 0;
  buflen = out-buf;
  return p + strlen( endTag );
}

char* TrDocument::Parse( TrDocument*,  char* p )
{
  // Parse away, at the document level. Since a document
  // contains nothing but other tags, most of what happens
  // here is skipping white space.
  //
  // In this variant (as opposed to stream and Parse) we
  // read everything we can.


  if ( !p || !*p )
  {
    SetError( TIXML_ERROR_DOCUMENT_EMPTY );
    return 0;
  }

  p = SkipWhiteSpace( p );
  if ( !p )
  {
    SetError( TIXML_ERROR_DOCUMENT_EMPTY );
    return 0;
  }

  TrDocumentNode* lastChild = 0;
  while ( p && *p )
  {
    TrDocumentNode* node = Identify( this, p );
    if ( node )
    {
      p = node->Parse( this, p );
      lastChild = LinkEndChild( lastChild, node );
    }
    else
    {
      break;
    }
    p = SkipWhiteSpace( p );
  }
  // All is well.
  return p;
}


char* TrXmlElement::Parse( TrDocument* document, char* p )
{
  p = SkipWhiteSpace( p );

  if ( !p || !*p || *p != '<' )
  {
    document->SetError( TIXML_ERROR_PARSING_ELEMENT );
    return 0;
  }

  p = SkipWhiteSpace( p+1 );

  // Read the name.
  value = p;
  p = ReadName( p );
  char* endp = p;

  if ( !p || !*p )
  {
    document->SetError( TIXML_ERROR_FAILED_TO_READ_ELEMENT_NAME );
    return 0;
  }

  int endTaglen = endp-value;
  char endTag[1000];
  strcpy (endTag, "</");

  char endchar = *endp;
  *endp = 0;
  strcpy (endTag+2, value);
  *endp = endchar;

  strcpy (endTag+2+endTaglen, ">");
  endTaglen += 2+1;

  // Check for and read attributes. Also look for an empty
  // tag or an end tag.
  while ( p && *p )
  {
    p = SkipWhiteSpace( p );
    if ( !p || !*p )
    {
      document->SetError( TIXML_ERROR_READING_ATTRIBUTES );
      return 0;
    }
    if ( *p == '/' )
    {
      ++p;
      // Empty tag.
      if ( *p  != '>' )
      {
        document->SetError( TIXML_ERROR_PARSING_EMPTY );    
        return 0;
      }
      attributeSet.set.ShrinkBestFit ();
      *endp = 0;
      return (p+1);
    }
    else if ( *p == '>' )
    {
      // Done with attributes (if there were any.)
      // Read the value -- which can include other
      // elements -- read the end tag, and return.
      ++p;
      p = ReadValue( document, p );    // Note this is an Element method, and will set the error if one happens.
      if ( !p || !*p )
      {
        attributeSet.set.ShrinkBestFit ();
        return 0;
      }

      // We should find the end tag now
      if ( StringEqualIgnoreCase( p, endTag) )
      {
        p += endTaglen;
        attributeSet.set.ShrinkBestFit ();
        *endp = 0;
        return p;
      }
      else
      {
        document->SetError( TIXML_ERROR_READING_END_TAG );
        return 0;
      }
    }
    else
    {
      // Try to read an element:
      TrDocumentAttribute attrib;
      // @@@ OPTIMIZE
      p = attrib.Parse( document, p );

      if ( !p || !*p )
      {
        document->SetError( TIXML_ERROR_PARSING_ELEMENT );
        return 0;
      }
      GetAttributeRegistered (attrib.Name()).
        TakeOverValue ((char*)attrib.Value(), attrib.GetValueLength ());
      attrib.TakeOverValue (0, 0);
    }
  }
  attributeSet.set.ShrinkBestFit ();
  *endp = 0;
  return p;
}


char* TrXmlElement::ReadValue( TrDocument* document, char* p )
{
  // Remember original location in stream because text and CDATA nodes decide
  // themselves if leading whitespace should be stripped.
  char* orig_p = p;

  // Read in text and elements in any order.
  p = SkipWhiteSpace( p );
  bool first_text = false;
  TrDocumentNode* lastChild = 0;
  while ( p && *p )
  {
    if ( *p != '<' )
    {
      // If we are parsing the first child we use an optimization
      // to store the text in this node instead of a child.
      if (lastChild == 0 && !first_text)
      {
	first_text = true;
	const char* end = "<";
	p = ReadText( orig_p, contentsvalue, contentsvalue_len, true, end);
	if ( p ) p--;
      }
      else
      {
	// Take what we have, make a text element.
	TrXmlText* textNode = document->blk_text.Alloc ();
	if ( !textNode )
	{
	  document->SetError( TIXML_ERROR_OUT_OF_MEMORY );
	  return 0;
	}
	p = textNode->Parse( document, orig_p );
	lastChild = LinkEndChild( lastChild, textNode );
      }
    } 
    else if ( StringEqual(p, "<![CDATA[") )
    {
      TrXmlCData* cdataNode = new TrXmlCData( );

      if ( !cdataNode )
      {
        document->SetError( TIXML_ERROR_OUT_OF_MEMORY );
        return 0;
      }

      p = cdataNode->Parse( document, orig_p );

      if ( !cdataNode->Blank() )
        lastChild = LinkEndChild( lastChild, cdataNode );
      else
        delete cdataNode;
    }
    else 
    {
      // We hit a '<'
      // Have we hit a new element or an end tag?
      if ( StringEqual ( p, "</") )
      {
        return p;
      }
      else
      {
        TrDocumentNode* node = Identify( document, p );
        if ( node )
        {
          p = node->Parse( document, p );
          lastChild = LinkEndChild( lastChild, node );
        }        
        else
        {
          return 0;
        }
      }
    }
    p = SkipWhiteSpace( p );
  }

  if ( !p )
  {
    document->SetError( TIXML_ERROR_READING_ELEMENT_VALUE );
  }

  return p;
}


char* TrXmlUnknown::Parse( TrDocument* document, char* p )
{
  p = SkipWhiteSpace( p );
  if ( !p || !*p || *p != '<' )
  {
    document->SetError( TIXML_ERROR_PARSING_UNKNOWN );
    return 0;
  }
  ++p;
  value = p;

  while ( p && *p && *p != '>' )
  {
    ++p;
  }
  vallen = p-value;

  if ( !p )
  {
    document->SetError( TIXML_ERROR_PARSING_UNKNOWN );
  }
  if ( *p == '>' )
    return p+1;
  return p;
}

char* TrXmlComment::Parse( TrDocument* document, char* p )
{
  p = SkipWhiteSpace( p );
  const char* startTag = "<!--";
  const char* endTag   = "-->";

  if ( !StringEqual ( p, startTag) )
  {
    document->SetError( TIXML_ERROR_PARSING_COMMENT );
    return 0;
  }
  p += strlen( startTag );
  p = ReadText( p, value, vallen, false, endTag);
  return p;
}


char* TrDocumentAttribute::Parse( TrDocument* document, char* p )
{
  p = TrXmlBase::SkipWhiteSpace( p );
  if ( !p || !*p ) return 0;

  // Read the name, the '=' and the value.
  name = p;
  p = TrXmlBase::ReadName( p );
  char* endp = p;

  if ( !p || !*p )
  {
    document->SetError( TIXML_ERROR_READING_ATTRIBUTES );
    return 0;
  }

  p = TrXmlBase::SkipWhiteSpace( p );
  if ( !p || !*p || *p != '=' )
  {
    document->SetError( TIXML_ERROR_READING_ATTRIBUTES );
    return 0;
  }

  ++p;  // skip '='
  *endp = 0;	// End the name field.
  p = TrXmlBase::SkipWhiteSpace( p );
  if ( !p || !*p )
  {
    document->SetError( TIXML_ERROR_READING_ATTRIBUTES );
    return 0;
  }
  
  const char* end;

  char* buf;
  int buflen;
  if ( *p == '\'' )
  {
    ++p;
    end = "\'";
    p = TrXmlBase::ReadText( p, buf, buflen, false, end);
  }
  else if ( *p == '"' )
  {
    ++p;
    end = "\"";
    p = TrXmlBase::ReadText( p, buf, buflen, false, end);
  }
  else
  {
    // All attribute values should be in single or double quotes.
    // But this is such a common error that the parser will try
    // its best, even without them.
    buf = p;
    char* out = buf;
    while (    p && *p                    // existence
        && !isspace( *p ) && *p != '/' && *p != '>' )            // tag end
    {
      *out++ = *p++;
    }
    buflen = out-buf;
  }
  value = buf;
  vallen = buflen;
  return p;
}

char* TrXmlText::Parse( TrDocument* document, char* p )
{
  //TrDocument* doc = GetDocument();
  bool ignoreWhite = true;
//  if ( doc && !doc->IgnoreWhiteSpace() ) ignoreWhite = false;

  const char* end = "<";
  p = ReadText( p, value, vallen, ignoreWhite, end);

  if ( p )
    return p-1;  // don't truncate the '<'
  return 0;
}

char* TrXmlCData::Parse( TrDocument* document, char* p )
{
  //TrDocument* doc = GetDocument();
  bool ignoreWhite = false;
//  if ( doc && !doc->IgnoreWhiteSpace() ) ignoreWhite = false;
        //skip the <![CDATA[ 
        p += 9;
  const char* end = "]]>";
  p = ReadText( p, value, vallen, ignoreWhite, end);

  if ( p )
    return p;
  return 0;
}

char* TrXmlDeclaration::Parse( TrDocument* document, char* p )
{
  p = SkipWhiteSpace( p );
  // Find the beginning, find the end, and look for
  // the stuff in-between.
  if ( !p || !*p || !StringEqual( p, "<?xml") )
  {
    document->SetError( TIXML_ERROR_PARSING_DECLARATION );
    return 0;
  }

  p += 5;
//  const char* start = p+5;
//  const char* end  = strstr( start, "?>" );

  version = "";
  encoding = "";
  standalone = "";

  while ( p && *p )
  {
    if ( *p == '>' )
    {
      ++p;
      return p;
    }

    p = SkipWhiteSpace( p );
    if ( StringEqual( p, "version") )
    {
//      p += 7;
      TrDocumentAttribute attrib;
      p = attrib.Parse( document, p );    
      version = attrib.Value();
    }
    else if ( StringEqual( p, "encoding") )
    {
//      p += 8;
      TrDocumentAttribute attrib;
      p = attrib.Parse( document, p );    
      encoding = attrib.Value();
    }
    else if ( StringEqual( p, "standalone") )
    {
//      p += 10;
      TrDocumentAttribute attrib;
      p = attrib.Parse( document, p );    
      standalone = attrib.Value();
    }
    else
    {
      // Read over whatever it is.
      while( p && *p && *p != '>' && !isspace( *p ) )
        ++p;
    }
  }
  return 0;
}

bool TrXmlText::Blank() const
{
  for ( unsigned int i=0; i<(unsigned int)vallen; i++ )
    if ( !isspace( value[i] ) )
      return false;
  return true;
}

