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
#include "tinyxml.h"
#include "csutil/csstring.h"
#include <ctype.h>

namespace CS
{
namespace Implementation
{
namespace TinyXml
{

// Note tha "PutString" hardcodes the same list. This
// is less flexible than it appears. Changing the entries
// or order will break putstring.  
const TiXmlBase::Entity TiXmlBase::entity[ NUM_ENTITY ] = 
{
  { "&amp;",  5, '&' },
  { "&lt;",   4, '<' },
  { "&gt;",   4, '>' },
  { "&quot;", 6, '\"' },
  { "&apos;", 6, '\'' }
};


const char* TiXmlBase::SkipWhiteSpace( ParseInfo& parse, const char* p )
{
  if ( !p || !*p )
  {
    return 0;
  }
  while ( isspace ((unsigned char)*p))
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

const char* TiXmlBase::ReadName( const char* p, csString& name)
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
      name << *p++;
    }
    return p;
  }
  return 0;
}

const char* TiXmlBase::GetEntity( const char* p, char* value )
{
  // Presume an entity, and pull it out.
  // TiXmlString ent;
  int i;

  // Ignore the &#x entities.
  if (strncmp( "&#x", p, 3 ) == 0 && *(p+3) && *(p+4) )
  {
    *value = 0;
    
    if ( isalpha( *(p+3) ) ) *value += ( tolower( *(p+3) ) - 'a' + 10 ) * 16;
    else *value += ( *(p+3) - '0' ) * 16;

    if ( isalpha( *(p+4) ) ) *value += ( tolower( *(p+4) ) - 'a' + 10 );
    else *value += ( *(p+4) - '0' );

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


bool TiXmlBase::StringEqual( const char* p, const char* tag)
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

bool TiXmlBase::StringEqualIgnoreCase( const char* p,
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

#define BUFSIZE 2000

/**
 * This is a growing string class that keeps an initial unallocated
 * buffer to avoid memory allocation in the common case where the
 * string is small.
 */
class GrowString
{
private:
  csStringFast<BUFSIZE> buf;

public:
  GrowString() { buf.SetGrowsBy (0); }

  void AddChar (char c)
  {
    buf.Append (c);
  }

  char* GetNewCopy ()
  {
    char* copy = (char*)cs_malloc (buf.Length()+1);
    strcpy (copy, buf.GetDataSafe());
    return copy;
  }

  const char* GetThisCopy () const
  {
    return buf.GetData();
  }
};


const char* TiXmlBase::ReadText(ParseInfo& parse, const char* p, 
        GrowString& buf,
        bool trimWhiteSpace, 
        const char* endTag)
{
  if (!trimWhiteSpace    // certain tags always keep whitespace
      || !parse.condenseWhiteSpace )  // if true, whitespace is always kept
  {
    // Keep all the white space.
    while (*p && !StringEqual ( p, endTag))
    {
      if (*p == '\n')
      {
	parse.linenum++;
	parse.startOfLine = p + 1;
      }
      char c;
      p = GetChar( p, &c );
      buf.AddChar (c);
    }
  }
  else
  {
    bool whitespace = false;

    // Remove leading white space:
    p = SkipWhiteSpace( parse, p );
    while (  *p && !StringEqual ( p, endTag) )
    {
      if (*p == '\n')
      {
	parse.linenum++;
	parse.startOfLine = p + 1;
      }
      if ( isspace( *p ) )
      {
        whitespace = true;
        ++p;
      }
      else
      {
        // If we've found whitespace, add it before the
        // new character. Any whitespace just becomes a space.
        if ( whitespace )
        {
          buf.AddChar (' ');
          whitespace = false;
        }
        char c;
        p = GetChar( p, &c );
        buf.AddChar (c);
      }
    }
  }
  buf.AddChar (0);
  return p + strlen( endTag );
}

const char* TiDocumentNode::Parse( ParseInfo& parse, const char* p )
{
  switch (Type())
  {
    case DOCUMENT:
      return static_cast<TiDocument*> (this)->Parse (parse, p);
    case ELEMENT:
      return static_cast<TiXmlElement*> (this)->Parse (parse, p);
    case COMMENT:
      return static_cast<TiXmlComment*> (this)->Parse (parse, p);
    case UNKNOWN:
      return static_cast<TiXmlUnknown*> (this)->Parse (parse, p);
    case TEXT:
      return static_cast<TiXmlText*> (this)->Parse (parse, p);
    case CDATA:
      return static_cast<TiXmlCData*> (this)->Parse (parse, p);
    case DECLARATION:
      return static_cast<TiXmlDeclaration*> (this)->Parse (parse, p);
    default:
      CS_ASSERT(false);
      return 0;
  }
}

const char* TiDocument::Parse( ParseInfo& parse, const char* p )
{
  // Parse away, at the document level. Since a document
  // contains nothing but other tags, most of what happens
  // here is skipping white space.
  //
  // In this variant (as opposed to stream and Parse) we
  // read everything we can.


  if ( !p || !*p )
  {
    SetError( TIXML_ERROR_DOCUMENT_EMPTY, 0, p );
    return 0;
  }

  p = SkipWhiteSpace( parse, p );
  if ( !p )
  {
    SetError( TIXML_ERROR_DOCUMENT_EMPTY, 0, p );
    return 0;
  }

  TiDocumentNode* lastChild = 0;
  while ( p && *p )
  {
    csRef<TiDocumentNode> node (Identify( parse, p ));
    if ( node )
    {
      p = node->Parse( parse, p );
      InsertAfterChild (lastChild, node);
      lastChild = node;
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


const char* TiXmlElement::Parse( ParseInfo& parse, const char* p )
{
  p = SkipWhiteSpace( parse, p );

  if ( !p || !*p || *p != '<' )
  {
    parse.document->SetError( TIXML_ERROR_PARSING_ELEMENT, this, p );
    return 0;
  }

  p = SkipWhiteSpace( parse, p+1 );

  // Read the name.
  csString inname;  
  p = ReadName( p, inname );
  if ( inname.IsEmpty() )
  {
    parse.document->SetError( TIXML_ERROR_FAILED_TO_READ_ELEMENT_NAME, this, p );
    return 0;
  }
  csStringID name_id = parse.document->strings.Request (inname);
  const char* reg_name = parse.document->strings.Request (name_id);
  SetValueRegistered (reg_name);

  TiXmlString endTag ("</");
  endTag += value;
  endTag += ">";

  // Check for and read attributes. Also look for an empty
  // tag or an end tag.
  while ( p && *p )
  {
    p = SkipWhiteSpace( parse, p );
    if ( !p || !*p )
    {
      parse.document->SetError( TIXML_ERROR_READING_ATTRIBUTES, this, p );
      return 0;
    }
    if ( *p == '/' )
    {
      ++p;
      // Empty tag.
      if ( *p  != '>' )
      {
        parse.document->SetError( TIXML_ERROR_PARSING_EMPTY, this, p );    
        return 0;
      }
      attributeSet.set.ShrinkBestFit ();
      return (p+1);
    }
    else if ( *p == '>' )
    {
      // Done with attributes (if there were any.)
      // Read the value -- which can include other
      // elements -- read the end tag, and return.
      ++p;
      p = ReadValue( parse, p );    // Note this is an Element method,
      				       // and will set the error if one happens.
      if ( !p || !*p )
      {
        attributeSet.set.ShrinkBestFit ();
        return 0;
      }

      // We should find the end tag now
      if ( StringEqualIgnoreCase( p, endTag.c_str()) )
      {
        p += endTag.length();
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
      TiDocumentAttribute attrib;
      // @@@ OPTIMIZE
      p = attrib.Parse( parse, this, p );

      if ( !p || !*p )
      {
        parse.document->SetError( TIXML_ERROR_PARSING_ELEMENT, this, p );
        return 0;
      }
      GetAttributeRegistered (attrib.Name()).
        TakeOverValue (attrib.Value());
      attrib.TakeOverValue (0);
    }
  }
  attributeSet.set.ShrinkBestFit ();
  return p;
}

#include "csutil/custom_new_disable.h"

const char* TiXmlElement::ReadValue( ParseInfo& parse, const char* p )
{
  char const* orig_p;

  // Remember original location in stream because text and CDATA nodes decide
  // themselves if leading whitespace should be stripped.
  orig_p = p;

  TiDocumentNode* lastChild = 0;
  // Read in text and elements in any order.
  p = SkipWhiteSpace( parse, p );
  while ( p && *p )
  {
    if ( *p != '<' )
    {
      // Take what we have, make a text element.
      void* ptr = parse.document->blk_text.Alloc (sizeof (TiXmlText));
      csRef<TiXmlText> textNode;
      textNode.AttachNew (new (ptr) TiXmlText ());

      if ( !textNode )
      {
        parse.document->SetError( TIXML_ERROR_OUT_OF_MEMORY, this, p );
        return 0;
      }

      p = textNode->Parse( parse, orig_p );

      if ( !textNode->Blank() )
      {
        InsertAfterChild (lastChild, textNode);
        lastChild = textNode;
      }
      else
        parse.document->DeleteNode (textNode);
    } 
    else if ( StringEqual(p, "<![CDATA[") )
    {
      csRef<TiXmlCData> cdataNode;
      void* ptr = parse.document->docHeap.Alloc (sizeof (TiXmlCData));
      cdataNode.AttachNew (new (ptr) TiXmlCData( ));

      if ( !cdataNode )
      {
        parse.document->SetError( TIXML_ERROR_OUT_OF_MEMORY, this, p );
        return 0;
      }

      p = cdataNode->Parse( parse, p );
      // don't care about whitespace before <![CDATA[ -> don't use orig_p

      if ( !cdataNode->Blank() )
      {
        InsertAfterChild (lastChild, cdataNode );
        lastChild = cdataNode;
      }
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
        csRef<TiDocumentNode> node (Identify( parse, p ));
        if ( node )
        {
          p = node->Parse( parse, p );
          InsertAfterChild (lastChild, node);
	  if (!p) return 0;
          lastChild = node;
        }        
        else
        {
          return 0;
        }
      }
    }

    // Remember original location in stream because text and CDATA nodes decide
    // themselves if leading whitespace should be stripped.
    orig_p = p;

    p = SkipWhiteSpace( parse, p );
  }

  if ( !p )
  {
    parse.document->SetError( TIXML_ERROR_READING_ELEMENT_VALUE, this, p );
  }  
  return p;
}

#include "csutil/custom_new_disable.h"

const char* TiXmlUnknown::Parse( ParseInfo& parse, const char* p )
{
  p = SkipWhiteSpace( parse, p );
  if ( !p || !*p || *p != '<' )
  {
    parse.document->SetError( TIXML_ERROR_PARSING_UNKNOWN, this, p );
    return 0;
  }
  ++p;
  value = "";

  while ( p && *p && *p != '>' )
  {
    value += *p;
    ++p;
  }

  if ( !p )
  {
    parse.document->SetError( TIXML_ERROR_PARSING_UNKNOWN, this, p );
  }
  if ( *p == '>' )
    return p+1;
  return p;
}

const char* TiXmlComment::Parse( ParseInfo& parse, const char* p )
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
  cs_free (value);
  GrowString buf;
  p = ReadText( parse, p, buf, false, endTag);
  value = buf.GetNewCopy ();
  return p;
}


const char* TiDocumentAttribute::Parse( ParseInfo& parse, TiDocumentNode* node,
                                        const char* p )
{
  p = TiXmlBase::SkipWhiteSpace( parse, p );
  if ( !p || !*p ) return 0;

  // Read the name, the '=' and the value.
  csString inname;
  p = TiXmlBase::ReadName( p, inname );

  if ( inname.IsEmpty() )
  {
    parse.document->SetError( TIXML_ERROR_READING_ATTRIBUTES, node, p );
    return 0;
  }

  csStringID name_id = parse.document->strings.Request (inname);
  name = parse.document->strings.Request (name_id);

  p = TiXmlBase::SkipWhiteSpace( parse, p );
  if ( !p || !*p || *p != '=' )
  {
    parse.document->SetError( TIXML_ERROR_READING_ATTRIBUTES, node, p );
    return 0;
  }

  ++p;  // skip '='
  p = TiXmlBase::SkipWhiteSpace( parse, p );
  if ( !p || !*p )
  {
    parse.document->SetError( TIXML_ERROR_READING_ATTRIBUTES, node, p );
    return 0;
  }
  
  const char* end;

  cs_free (value);
  GrowString buf;
  if ( *p == '\'' )
  {
    ++p;
    end = "\'";
    p = TiXmlBase::ReadText( parse, p, buf, false, end);
  }
  else if ( *p == '"' )
  {
    ++p;
    end = "\"";
    p = TiXmlBase::ReadText( parse, p, buf, false, end);
  }
  else
  {
    parse.document->SetError( TIXML_ERROR_READING_ATTRIBUTES, node, p );
    return 0;
  }
  value = buf.GetNewCopy ();
  return p;
}

const char* TiXmlText::Parse( ParseInfo& parse, const char* p )
{
  //TiDocument* doc = GetDocument();
  bool ignoreWhite = true;
//  if ( doc && !doc->IgnoreWhiteSpace() ) ignoreWhite = false;

  const char* end = "<";
  GrowString buf;
  p = ReadText( parse, p, buf, ignoreWhite, end);

  csStringID value_id = parse.document->strings.Request (buf.GetThisCopy ());
  const char* reg_value = parse.document->strings.Request (value_id);
  SetValueRegistered (reg_value);

  if ( p )
    return p-1;  // don't truncate the '<'
  return 0;
}

const char* TiXmlCData::Parse( ParseInfo& parse, const char* p )
{
  //TiDocument* doc = GetDocument();
  bool ignoreWhite = false;
//  if ( doc && !doc->IgnoreWhiteSpace() ) ignoreWhite = false;
        //skip the <![CDATA[ 
        p += 9;
  const char* end = "]]>";
  GrowString buf;
  p = ReadText( parse, p, buf, ignoreWhite, end);

  csStringID value_id = parse.document->strings.Request (buf.GetThisCopy ());
  const char* reg_value = parse.document->strings.Request (value_id);
  SetValueRegistered (reg_value);

  if ( p )
    return p;
  return 0;
}

const char* TiXmlDeclaration::Parse( ParseInfo& parse, const char* p )
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
      TiDocumentAttribute attrib;
      p = attrib.Parse( parse, this, p );    
      version = attrib.Value();
    }
    else if ( StringEqual( p, "encoding") )
    {
//      p += 8;
      TiDocumentAttribute attrib;
      p = attrib.Parse( parse, this, p );    
      encoding = attrib.Value();
    }
    else if ( StringEqual( p, "standalone") )
    {
//      p += 10;
      TiDocumentAttribute attrib;
      p = attrib.Parse( parse, this, p );    
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

bool TiXmlText::Blank() const
{
  size_t l = strlen (value);
  for ( unsigned i=0; i<l; i++ )
    if ( !isspace( value[i] ) )
      return false;
  return true;
}

} // namespace TinyXml
} // namespace Implementation
} // namespace CS
