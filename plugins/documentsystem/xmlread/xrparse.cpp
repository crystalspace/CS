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

#include "csutil/csstring.h"

#include "xr.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLRead)
{

TrXmlBase::Entity TrXmlBase::entity[ NUM_ENTITY ] = 
{
  { "&amp;",  5, '&' },
  { "&lt;",   4, '<' },
  { "&gt;",   4, '>' },
  { "&quot;", 6, '\"' },
  { "&apos;", 6, '\'' }
};

inline static bool IsSpace (const char c)
{
  return (c == 0x20) || (c == 0x0a) || (c == 0x0d) || (c == 0x09);
}

char* TrXmlBase::SkipWhiteSpace( ParseInfo& parse, char* p )
{
  if ( !p || !*p )
  {
    return 0;
  }
  while ( IsSpace (*p))
  {
    if (*p == '\n')
    {
      parse.linenum++;
      parse.startOfLine = p + 1;
    }
    p++;
  }
  return p;
}

const char* TrXmlBase::SkipWhiteSpace( ParseInfo& parse, const char* p )
{
  if ( !p || !*p )
  {
    return 0;
  }
  while ( IsSpace (*p))
  {
    if (*p == '\n')
    {
      parse.linenum++;
      parse.startOfLine = p + 1;
    }
    p++;
  }
  return p;
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

char* TrXmlBase::ReadText(ParseInfo& parse, char* p,
	char*& buf, int& buflen,
        bool trimWhiteSpace, 
        const char* endTag)
{
  const char tagStart = *endTag;
  buf = p;
  char* out = p;

  if (!trimWhiteSpace)    // certain tags always keep whitespace
  {
    while (true)
    {
      // Keep all the white space.
      while (*p && (*p != tagStart))
      {
	if (*p == '\n')
	{
	  parse.linenum++;
	  parse.startOfLine = p + 1;
	}
        char c;
        p = GetChar( p, &c );
        *out++ = c;
      }
      if (StringEqual (p, endTag)) break;
      if (!*p) return p;
      *out++ = *p++;
    }
  }
  else
  {
    bool whitespace = false;
    bool first = true;

    // Remove leading white space:
    p = SkipWhiteSpace( parse, p );
    buf = p;
    out = p;
    while (true)
    {
      while ( *p && (*p != tagStart))
      {
	if (*p == '\n')
	{
	  parse.linenum++;
	  parse.startOfLine = p + 1;
	}
        if ( IsSpace( *p ) )
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
      if (StringEqual (p, endTag)) break;
      if (!*p) return p;
      *out++ = *p++;
    }
  }
  // *out++ = 0;
  buflen = out-buf;
  // Skip over endTag
  const char* t = endTag;
  while (*p && *t) { p++; t++; }
  return p;
}

char* TrDocument::Parse( ParseInfo& parse,  char* p )
{
  // Parse away, at the document level. Since a document
  // contains nothing but other tags, most of what happens
  // here is skipping white space.
  //
  // In this variant (as opposed to stream and Parse) we
  // read everything we can.


  if ( !p || !*p )
  {
    SetError( TIXML_ERROR_DOCUMENT_EMPTY, 0, 0 );
    return 0;
  }

  p = SkipWhiteSpace( parse, p );
  if ( !p )
  {
    SetError( TIXML_ERROR_DOCUMENT_EMPTY, 0, 0 );
    return 0;
  }

  this->parse.document = this;
  this->parse.ReadName = ReadName_ISO_8859_1;
  this->parse.IsNameStart = IsNameStart_ISO_8859_1;

  TrDocumentNode* lastChild = 0;
  while ( p && *p )
  {
    TrDocumentNode* node = Identify( this->parse, p );
    if ( node )
    {
      p = node->Parse( this->parse, p );
      lastChild = LinkEndChild( lastChild, node );
    }
    else
    {
      break;
    }
    p = SkipWhiteSpace( parse, p );
  }
  // All is well.
  return p;
}


char* TrXmlElement::Parse( ParseInfo& parse,  char* p )
{
  p = SkipWhiteSpace( parse, p );

  if ( !p || !*p || *p != '<' )
  {
    parse.document->SetError( TIXML_ERROR_PARSING_ELEMENT, this, p );
    return 0;
  }

  p = SkipWhiteSpace( parse, p+1 );

  // Read the name.
  value = p;
  p = parse.ReadName( p );
  char* endp = p;

  if ( !p || !*p )
  {
    *endp = 0;
    parse.document->SetError( TIXML_ERROR_FAILED_TO_READ_ELEMENT_NAME, this, p );
    return 0;
  }

  int endTaglen = endp-value;
  csString endTag;
  endTag = "</";

  char endchar = *endp;
  *endp = 0;
  endTag << value;
  *endp = endchar;

  endTag << ">";
  endTaglen += 2+1;

  // Check for and read attributes. Also look for an empty
  // tag or an end tag.
  while ( p && *p )
  {
    p = SkipWhiteSpace( parse, p );
    if ( !p || !*p )
    {
      *endp = 0;
      parse.document->SetError( TIXML_ERROR_READING_ATTRIBUTES, this, p );
      return 0;
    }
    if ( *p == '/' )
    {
      ++p;
      // Empty tag.
      if ( *p  != '>' )
      {
        *endp = 0;
        parse.document->SetError( TIXML_ERROR_PARSING_EMPTY, this, p );    
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
      ++p; *endp = 0;
      p = ReadValue( parse, p );    // Note this is an Element method, and will set the error if one happens.
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
        return p;
      }
      else
      {
        parse.document->SetError( TIXML_ERROR_READING_END_TAG, this, p );
        return 0;
      }
    }
    else
    {
      // Try to read an element:
      TrDocumentAttribute attrib;
      // @@@ OPTIMIZE
      p = attrib.Parse( parse, this, p );

      if ( !p || !*p )
      {
        *endp = 0;
        parse.document->SetError( TIXML_ERROR_PARSING_ELEMENT, this, p );
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


char* TrXmlElement::ReadValue( ParseInfo& parse, char* p )
{
  // Remember original location in stream because text and CDATA nodes decide
  // themselves if leading whitespace should be stripped.
  char* orig_p = p;

  // Read in text and elements in any order.
  p = SkipWhiteSpace( parse, p );
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
	p = ReadText( parse, orig_p, contentsvalue, contentsvalue_len, 
          parse.condenseWhiteSpace, end);
	if ( p ) p--;
      }
      else
      {
	// Take what we have, make a text element.
	TrXmlText* textNode = parse.document->blk_text.Alloc ();
	if ( !textNode )
	{
	  parse.document->SetError( TIXML_ERROR_OUT_OF_MEMORY, this, p );
	  return 0;
	}
	p = textNode->Parse( parse, orig_p );
	lastChild = LinkEndChild( lastChild, textNode );
      }
    } 
    else if ( StringEqual(p, "<![CDATA[") )
    {
      TrXmlCData* cdataNode = new TrXmlCData( );

      if ( !cdataNode )
      {
        parse.document->SetError( TIXML_ERROR_OUT_OF_MEMORY, this, p );
        return 0;
      }

      p = cdataNode->Parse( parse, p ); 
      // don't care about whitespace before <![CDATA[ -> don't use orig_p

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
        TrDocumentNode* node = Identify( parse, p );
        if ( node )
        {
          p = node->Parse( parse, p );
          lastChild = LinkEndChild( lastChild, node );
	  if (!p) return 0;
        }        
        else
        {
          return 0;
        }
      }
    }

    orig_p = p;

    p = SkipWhiteSpace( parse, p );
  }

  if ( !p )
  {
    parse.document->SetError( TIXML_ERROR_READING_ELEMENT_VALUE, this, p );
  }

  return p;
}


char* TrXmlUnknown::Parse( ParseInfo& parse,  char* p )
{
  p = SkipWhiteSpace( parse, p );
  if ( !p || !*p || *p != '<' )
  {
    parse.document->SetError( TIXML_ERROR_PARSING_UNKNOWN, this, p );
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
    parse.document->SetError( TIXML_ERROR_PARSING_UNKNOWN, this, p );
  }
  if ( *p == '>' )
    return p+1;
  return p;
}

char* TrXmlComment::Parse( ParseInfo& parse, char* p )
{
  p = SkipWhiteSpace( parse, p );
  const char* startTag = "<!--";
  const char* endTag   = "-->";

  if ( !StringEqual ( p, startTag) )
  {
    parse.document->SetError( TIXML_ERROR_PARSING_COMMENT, this, p );
    return 0;
  }
  p += strlen( startTag );
  p = ReadText( parse, p, value, vallen, false, endTag);
  return p;
}


char* TrDocumentAttribute::Parse( ParseInfo& parse, TrDocumentNode* node, char* p )
{
  p = TrXmlBase::SkipWhiteSpace( parse, p );
  if ( !p || !*p ) return 0;

  // Read the name, the '=' and the value.
  name = p;
  p = parse.ReadName( p );
  char* endp = p;

  if ( !p || !*p )
  {
    parse.document->SetError( TIXML_ERROR_READING_ATTRIBUTES, node, p );
    return 0;
  }

  p = TrXmlBase::SkipWhiteSpace( parse, p );
  if ( !p || !*p || *p != '=' )
  {
    parse.document->SetError( TIXML_ERROR_READING_ATTRIBUTES, node, p );
    return 0;
  }

  ++p;  // skip '='
  *endp = 0;	// End the name field.
  p = TrXmlBase::SkipWhiteSpace( parse, p );
  if ( !p || !*p )
  {
    parse.document->SetError( TIXML_ERROR_READING_ATTRIBUTES, node, p );
    return 0;
  }
 
  const char* end;

  char* buf;
  int buflen;
  if ( *p == '\'' )
  {
    ++p;
    end = "\'";
    p = TrXmlBase::ReadText( parse, p, buf, buflen, false, end);
  }
  else if ( *p == '"' )
  {
    ++p;
    end = "\"";
    p = TrXmlBase::ReadText( parse, p, buf, buflen, false, end);
  }
  else
  {
    parse.document->SetError( TIXML_ERROR_READING_ATTRIBUTES, node, p );
    return 0;
  }
  value = buf;
  vallen = buflen;
  return p;
}

char* TrXmlText::Parse( ParseInfo& parse, char* p )
{
  //TrDocument* doc = GetDocument();
  //bool ignoreWhite = true;
  //if ( doc && !doc->IgnoreWhiteSpace() ) ignoreWhite = false;

  const char* end = "<";
  p = ReadText( parse, p, value, vallen, parse.condenseWhiteSpace, end);

  if ( p )
    return p-1;  // don't truncate the '<'
  return 0;
}

char* TrXmlCData::Parse( ParseInfo& parse, char* p )
{
  //skip the <![CDATA[ 
  p += 9;
  const char* end = "]]>";
  p = ReadText( parse, p, value, vallen, false, end);

  if ( p )
    return p;
  return 0;
}

char* TrXmlDeclaration::Parse( ParseInfo& parse, char* p )
{
  p = SkipWhiteSpace( parse, p );
  // Find the beginning, find the end, and look for
  // the stuff in-between.
  if ( !p || !*p || !StringEqual( p, "<?xml") )
  {
    parse.document->SetError( TIXML_ERROR_PARSING_DECLARATION, this, p );
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

    p = SkipWhiteSpace( parse, p );
    if ( StringEqual( p, "version") )
    {
//      p += 7;
      TrDocumentAttribute attrib;
      p = attrib.Parse( parse, this, p );    
      version = attrib.Value();
    }
    else if ( StringEqual( p, "encoding") )
    {
//      p += 8;
      TrDocumentAttribute attrib;
      p = attrib.Parse( parse, this, p );    
      encoding = attrib.Value();
    }
    else if ( StringEqual( p, "standalone") )
    {
//      p += 10;
      TrDocumentAttribute attrib;
      p = attrib.Parse( parse, this, p );    
      standalone = attrib.Value();
    }
    else
    {
      // Read over whatever it is.
      while( p && *p && *p != '>' && !IsSpace( *p ) )
        ++p;
    }
  }
  return 0;
}

bool TrXmlText::Blank() const
{
  for ( unsigned int i=0; i<(unsigned int)vallen; i++ )
    if ( !IsSpace( value[i] ) )
      return false;
  return true;
}


}
CS_PLUGIN_NAMESPACE_END(XMLRead)
