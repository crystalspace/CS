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
#include <ctype.h>

//#define DEBUG_PARSER

// Note tha "PutString" hardcodes the same list. This
// is less flexible than it appears. Changing the entries
// or order will break putstring.	
TiXmlBase::Entity TiXmlBase::entity[ NUM_ENTITY ] = 
{
	{ "&amp;",  5, '&' },
	{ "&lt;",   4, '<' },
	{ "&gt;",   4, '>' },
	{ "&quot;", 6, '\"' },
	{ "&apos;", 6, '\'' }
};


const char* TiXmlBase::SkipWhiteSpace( const char* p )
{
	if ( !p || !*p )
	{
		return 0;
	}
	while ( isspace (*p)) p++;
	return p;
}

const char* TiXmlBase::ReadName( const char* p, char* name)
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

const char* TiXmlBase::ReadName( const char* p, TIXML_STRING * name )
{
	*name = "";
	//assert( p );

	// Names start with letters or underscores.
	// After that, they can be letters, underscores, numbers,
	// hyphens, or colons. (Colons are valid ony for namespaces,
	// but tinyxml can't tell namespaces from names.)
	if (*p 
		 && ( isalpha( (unsigned char) *p ) || *p == '_' ) )
	{
		while(*p
				&&	(		isalnum( (unsigned char ) *p ) 
						 || *p == '_'
						 || *p == '-'
						 || *p == ':' ) )
		{
			(*name) += *p;
			++p;
		}
		return p;
	}
	return 0;
}

const char* TiXmlBase::GetEntity( const char* p, char* value )
{
	// Presume an entity, and pull it out.
	// TIXML_STRING ent;
	int i;

	// Ignore the &#x entities.
	if (    strncmp( "&#x", p, 3 ) == 0 
	     && *(p+3) 
		 && *(p+4) )
	{
		*value = 0;
		
		if ( isalpha( *(p+3) ) ) *value += ( tolower( *(p+3) ) - 'a' + 10 ) * 16;
		else				     *value += ( *(p+3) - '0' ) * 16;

		if ( isalpha( *(p+4) ) ) *value += ( tolower( *(p+4) ) - 'a' + 10 );
		else				     *value += ( *(p+4) - '0' );

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
	*value = *p;	// Don't put back the last one, since we return it!
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

	if ( *tag == 0 )		// Have we found the end of the tag, and everything equal?
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

const char* TiXmlBase::ReadText(	const char* p, 
					char** text, 
					bool trimWhiteSpace, 
					const char* endTag)
{
    char buf[10000];
    char* ptext = buf;
    *ptext = 0;
    int len = 0;

	if (!trimWhiteSpace		// certain tags always keep whitespace
	    || !condenseWhiteSpace )	// if true, whitespace is always kept
	{
		// Keep all the white space.
		while (*p && !StringEqual ( p, endTag))
		{
			char c;
			p = GetChar( p, &c );
			*ptext++ = c;
			len++;
			//CS_ASSERT (len <= 10000);
		}
	}
	else
	{
		bool whitespace = false;

		// Remove leading white space:
		p = SkipWhiteSpace( p );
		while (	*p && !StringEqual ( p, endTag) )
		{
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
					*ptext++ = ' ';
					len++;
					//CS_ASSERT (len <= 10000);
					whitespace = false;
				}
				char c;
				p = GetChar( p, &c );
				*ptext++ = c;
				len++;
				//CS_ASSERT (len <= 10000);
			}
		}
	}
	*ptext = 0;
	*text = new char[len+1];
	strcpy (*text, buf);
	return p + strlen( endTag );
}

const char* TiXmlBase::ReadText(	const char* p, 
					TIXML_STRING * text, 
					bool trimWhiteSpace, 
					const char* endTag)
{
    *text = "";
	if (!trimWhiteSpace		// certain tags always keep whitespace
	    || !condenseWhiteSpace )	// if true, whitespace is always kept
	{
		// Keep all the white space.
		while (	   p && *p
				&& !StringEqual ( p, endTag)
			  )
		{
			char c;
			p = GetChar( p, &c );
            (* text) += c;
		}
	}
	else
	{
		bool whitespace = false;

		// Remove leading white space:
		p = SkipWhiteSpace( p );
		while (	   p && *p
				&& !StringEqual ( p, endTag) )
		{
			if ( *p == '\r' || *p == '\n' )
			{
				whitespace = true;
				++p;
			}
			else if ( isspace( *p ) )
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
               (* text) += ' ';
					whitespace = false;
				}
				char c;
				p = GetChar( p, &c );
            (* text) += c;
			}
		}
	}
	return p + strlen( endTag );
}

const char* TiDocument::Parse( TiDocument*,  const char* p )
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

	while ( p && *p )
	{
		TiDocumentNode* node = Identify( this, p );
		if ( node )
		{
			p = node->Parse( this, p );
			LinkEndChild( node );
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


const char* TiXmlElement::Parse( TiDocument* document, const char* p )
{
	p = SkipWhiteSpace( p );

	if ( !p || !*p || *p != '<' )
	{
		document->SetError( TIXML_ERROR_PARSING_ELEMENT );
		return 0;
	}

	p = SkipWhiteSpace( p+1 );

	// Read the name.
	char inname[1000];
	p = ReadName( p, inname );
	if ( !p || !*p )
	{
		document->SetError( TIXML_ERROR_FAILED_TO_READ_ELEMENT_NAME );
		return 0;
	}
	csStringID name_id = document->strings.Request (inname);
	const char* reg_name = document->strings.Request (name_id);
	SetValueRegistered (reg_name);

    TIXML_STRING endTag ("</");
	endTag += value;
	endTag += ">";

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
			return (p+1);
		}
		else if ( *p == '>' )
		{
			// Done with attributes (if there were any.)
			// Read the value -- which can include other
			// elements -- read the end tag, and return.
			++p;
			p = ReadValue( document, p );		// Note this is an Element method, and will set the error if one happens.
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
				document->SetError( TIXML_ERROR_READING_END_TAG );
				return 0;
			}
		}
		else
		{
			// Try to read an element:
			TiDocumentAttribute attrib;
			// @@@ OPTIMIZE
			p = attrib.Parse( document, p );

			if ( !p || !*p )
			{
				document->SetError( TIXML_ERROR_PARSING_ELEMENT );
				return 0;
			}
			GetAttributeRegistered (attrib.Name()).
				TakeOverValue (attrib.Value());
			attrib.TakeOverValue (NULL);
		}
	}
	attributeSet.set.ShrinkBestFit ();
	return p;
}


const char* TiXmlElement::ReadValue( TiDocument* document, const char* p )
{
	// Read in text and elements in any order.
	p = SkipWhiteSpace( p );
	while ( p && *p )
	{
		if ( *p != '<' )
		{
			// Take what we have, make a text element.
			TiXmlText* textNode = new TiXmlText( "" );

			if ( !textNode )
			{
				document->SetError( TIXML_ERROR_OUT_OF_MEMORY );
				    return 0;
			}

			p = textNode->Parse( document, p );

			if ( !textNode->Blank() )
				LinkEndChild( textNode );
			else
				delete textNode;
		} 
                else if ( StringEqual(p, "<![CDATA[") )
                {
                        TiXmlCData* cdataNode = new TiXmlCData( "" );

			if ( !cdataNode )
			{
				document->SetError( TIXML_ERROR_OUT_OF_MEMORY );
				    return 0;
			}

			p = cdataNode->Parse( document, p );

			if ( !cdataNode->Blank() )
				LinkEndChild( cdataNode );
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
				TiDocumentNode* node = Identify( document, p );
				if ( node )
				{
					p = node->Parse( document, p );
					LinkEndChild( node );
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


const char* TiXmlUnknown::Parse( TiDocument* document, const char* p )
{
	p = SkipWhiteSpace( p );
	if ( !p || !*p || *p != '<' )
	{
		document->SetError( TIXML_ERROR_PARSING_UNKNOWN );
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
		document->SetError( TIXML_ERROR_PARSING_UNKNOWN );
	}
	if ( *p == '>' )
		return p+1;
	return p;
}

const char* TiXmlComment::Parse( TiDocument* document, const char* p )
{
	value = "";

	p = SkipWhiteSpace( p );
	const char* startTag = "<!--";
	const char* endTag   = "-->";

	if ( !StringEqual ( p, startTag) )
	{
		document->SetError( TIXML_ERROR_PARSING_COMMENT );
		return 0;
	}
	p += strlen( startTag );
	p = ReadText( p, &value, false, endTag);
	return p;
}


const char* TiDocumentAttribute::Parse( TiDocument* document, const char* p )
{
	p = TiXmlBase::SkipWhiteSpace( p );
	if ( !p || !*p ) return 0;

	// Read the name, the '=' and the value.
	char inname[1000];
	p = TiXmlBase::ReadName( p, inname );

	if ( !p || !*p )
	{
		document->SetError( TIXML_ERROR_READING_ATTRIBUTES );
		return 0;
	}

	csStringID name_id = document->strings.Request (inname);
	name = document->strings.Request (name_id);
	//printf ("inname=%s name_id=%d name=%s\n", inname, name_id, name); fflush (stdout);

	p = TiXmlBase::SkipWhiteSpace( p );
	if ( !p || !*p || *p != '=' )
	{
		document->SetError( TIXML_ERROR_READING_ATTRIBUTES );
		return 0;
	}

	++p;	// skip '='
	p = TiXmlBase::SkipWhiteSpace( p );
	if ( !p || !*p )
	{
		document->SetError( TIXML_ERROR_READING_ATTRIBUTES );
		return 0;
	}
	
	const char* end;

	if ( *p == '\'' )
	{
		++p;
		end = "\'";
		char* pvalue;
		p = TiXmlBase::ReadText( p, &pvalue, false, end);
		value = pvalue;
	}
	else if ( *p == '"' )
	{
		++p;
		end = "\"";
		char* pvalue;
		p = TiXmlBase::ReadText( p, &pvalue, false, end);
		value = pvalue;
	}
	else
	{
		// All attribute values should be in single or double quotes.
		// But this is such a common error that the parser will try
		// its best, even without them.
		value = "";
		while (    p && *p										// existence
				&& !isspace( *p ) && *p != '\n' && *p != '\r'	// whitespace
				&& *p != '/' && *p != '>' )						// tag end
		{
			value += *p;
			++p;
		}
	}
	return p;
}

const char* TiXmlText::Parse( TiDocument*, const char* p )
{
	value = "";

	//TiDocument* doc = GetDocument();
	bool ignoreWhite = true;
//	if ( doc && !doc->IgnoreWhiteSpace() ) ignoreWhite = false;

	const char* end = "<";
	p = ReadText( p, &value, ignoreWhite, end);
	if ( p )
		return p-1;	// don't truncate the '<'
	return 0;
}

const char* TiXmlCData::Parse( TiDocument*, const char* p )
{
	value = "";

	//TiDocument* doc = GetDocument();
	bool ignoreWhite = false;
//	if ( doc && !doc->IgnoreWhiteSpace() ) ignoreWhite = false;
        //skip the <![CDATA[ 
        p += 9;
	const char* end = "]]>";
	p = ReadText( p, &value, ignoreWhite, end);
	if ( p )
		return p;
	return 0;
}

const char* TiXmlDeclaration::Parse( TiDocument* document, const char* p )
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
//	const char* start = p+5;
//	const char* end  = strstr( start, "?>" );

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
//			p += 7;
			TiDocumentAttribute attrib;
			p = attrib.Parse( document, p );		
			version = attrib.Value();
		}
		else if ( StringEqual( p, "encoding") )
		{
//			p += 8;
			TiDocumentAttribute attrib;
			p = attrib.Parse( document, p );		
			encoding = attrib.Value();
		}
		else if ( StringEqual( p, "standalone") )
		{
//			p += 10;
			TiDocumentAttribute attrib;
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

bool TiXmlText::Blank() const
{
	for ( unsigned i=0; i<value.length(); i++ )
		if ( !isspace( value[i] ) )
			return false;
	return true;
}

