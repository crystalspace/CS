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

bool TiXmlBase::condenseWhiteSpace = true;

void TiXmlBase::PutString( const TIXML_STRING& str, TIXML_OSTREAM* stream )
{
	TIXML_STRING buffer;
	PutString( str, &buffer );
	(*stream) << buffer;
}

void TiXmlBase::PutString( const TIXML_STRING& str, TIXML_STRING* outString )
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
			// &#xA9;	-- copyright symbol, for example.
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
			char buf[ 32 ];
			sprintf( buf, "&#x%02X;", (unsigned) ( c & 0xff ) );
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


// <-- Strange class for a bug fix. Search for STL_STRING_BUG
TiXmlBase::StringToBuffer::StringToBuffer( const TIXML_STRING& str )
{
	buffer = new char[ str.length()+1 ];
	if ( buffer )
	{
		strcpy( buffer, str.c_str() );
	}
}


TiXmlBase::StringToBuffer::~StringToBuffer()
{
	delete [] buffer;
}
// End strange bug fix. -->


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
		#ifdef DEBUG_PARSER
			TIXML_LOG( "XML parsing Declaration\n" );
		#endif
		returnNode = new TiXmlDeclaration();
	}
	else if (    isalpha( *(p+1) )
			  || *(p+1) == '_' )
	{
		#ifdef DEBUG_PARSER
			TIXML_LOG( "XML parsing Element\n" );
		#endif
		returnNode = new TiXmlElement( );
	}
	else if ( StringEqual ( p, commentHeader) )
	{
		#ifdef DEBUG_PARSER
			TIXML_LOG( "XML parsing Comment\n" );
		#endif
		returnNode = new TiXmlComment();
	}
	else
	{
		#ifdef DEBUG_PARSER
			TIXML_LOG( "XML parsing Unknown\n" );
		#endif
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

	while ( node )
	{
		temp = node;
		node = node->next;
		delete temp;
	}	
}

void TiDocumentNodeChildren::Clear()
{
	TiDocumentNode* node = firstChild;
	TiDocumentNode* temp = 0;

	while ( node )
	{
		temp = node;
		node = node->next;
		delete temp;
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
		firstChild = node;			// it was an empty list.

	lastChild = node;
	return node;
}


TiDocumentNode* TiDocumentNodeChildren::InsertEndChild( const TiDocumentNode& addThis )
{
	TiDocumentNode* node = addThis.Clone();
	if ( !node )
		return 0;

	return LinkEndChild( node );
}


TiDocumentNode* TiDocumentNodeChildren::InsertBeforeChild( TiDocumentNode* beforeThis, const TiDocumentNode& addThis )
{	
	if ( !beforeThis || beforeThis->parent != this )
		return 0;

	TiDocumentNode* node = addThis.Clone();
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


TiDocumentNode* TiDocumentNodeChildren::InsertAfterChild( TiDocumentNode* afterThis, const TiDocumentNode& addThis )
{
	if ( !afterThis || afterThis->parent != this )
		return 0;

	TiDocumentNode* node = addThis.Clone();
	if ( !node )
		return 0;
	node->parent = this;

	node->prev = afterThis;
	node->next = afterThis->next;
	if ( afterThis->next )
	{
		afterThis->next->prev = node;
	}
	else
	{
		assert( lastChild == afterThis );
		lastChild = node;
	}
	afterThis->next = node;
	return node;
}


TiDocumentNode* TiDocumentNodeChildren::ReplaceChild( TiDocumentNode* replaceThis, const TiDocumentNode& withThis )
{
	if ( replaceThis->parent != this )
		return 0;

	TiDocumentNode* node = withThis.Clone();
	if ( !node )
		return 0;

	node->next = replaceThis->next;
	node->prev = replaceThis->prev;

	if ( replaceThis->next )
		replaceThis->next->prev = node;
	else
		lastChild = node;

	if ( replaceThis->prev )
		replaceThis->prev->next = node;
	else
		firstChild = node;

	delete replaceThis;
	node->parent = this;
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

	delete removeThis;
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

TiDocumentNode* TiDocumentNodeChildren::LastChild( const char * value ) const
{
	TiDocumentNode* node;
	for ( node = lastChild; node; node = node->prev )
	{
		const char* node_val = node->Value ();
		if (node_val && strcmp (node_val, value) == 0)
			return node;
	}
	return 0;
}

TiDocumentNode* TiDocumentNodeChildren::IterateChildren( TiDocumentNode* previous ) const
{
	if ( !previous )
	{
		return FirstChild();
	}
	else
	{
		return previous->NextSibling();
	}
}

TiDocumentNode* TiDocumentNodeChildren::IterateChildren( const char * val, TiDocumentNode* previous ) const
{
	if ( !previous )
	{
		return FirstChild( val );
	}
	else
	{
		return previous->NextSibling( val );
	}
}

TiXmlElement* TiDocumentNodeChildren::FirstChildElement() const
{
	TiDocumentNode* node;

	for (	node = FirstChild(); node; node = node->NextSibling() )
	{
		if ( node->ToElement() )
			return node->ToElement();
	}
	return 0;
}

TiXmlElement* TiDocumentNodeChildren::FirstChildElement( const char * value ) const
{
	TiDocumentNode* node;

	for (	node = FirstChild( value ); node;
		node = node->NextSibling( value ) )
	{
		if ( node->ToElement() )
			return node->ToElement();
	}
	return 0;
}


// -------------------------------------------------------------------------

void TiXmlElement::RemoveAttribute( const char * name )
{
	int nodeidx = attributeSet.Find (name);
	if ( nodeidx != -1 )
	{
		attributeSet.set.DeleteIndex (nodeidx);
	}
}

TiXmlElement* TiDocumentNode::NextSiblingElement() const
{
	TiDocumentNode* node;

	for (	node = NextSibling();
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

	for (	node = NextSibling( value );
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
	value = NULL;
}

TiXmlElement::~TiXmlElement()
{
}


void TiXmlElement::SetValue (const char * name)
{
  if (name == NULL)
  {
    value = NULL;
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
	int nodeidx = attributeSet.Find (name);

	if (nodeidx != -1)
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
	char buf[64];
	sprintf( buf, "%d", val );
	SetAttribute( document, name, buf );
}


TiDocumentAttribute& TiXmlElement::GetAttributeRegistered (
	const char * reg_name)
{
	int nodeidx = attributeSet.FindExact (reg_name);
	if (nodeidx != -1)
	{
		return attributeSet.set[nodeidx];
	}

	TiDocumentAttribute at;
	int idx = attributeSet.set.Push (at);
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

void TiXmlElement::Print( FILE* cfile, int depth ) const
{
	int i;
	for ( i=0; i<depth; i++ )
	{
		fprintf( cfile, "    " );
	}

	fprintf( cfile, "<%s", value );

	for (i = 0 ; i < attributeSet.set.Length () ; i++)
	{
	  const TiDocumentAttribute& attrib = attributeSet.set[i];
	  fprintf( cfile, " " );
	  attrib.Print( cfile, depth );
	}

	// There are 3 different formatting approaches:
	// 1) An element without children is printed as a <foo /> node
	// 2) An element with only a text child is printed as <foo> text </foo>
	// 3) An element with children is printed on multiple lines.
	TiDocumentNode* node;
	if ( !firstChild )
	{
		fprintf( cfile, " />" );
	}
	else if ( firstChild == lastChild && firstChild->ToText() )
	{
		fprintf( cfile, ">" );
		firstChild->Print( cfile, depth + 1 );
		fprintf( cfile, "</%s>", value );
	}
	else
	{
		fprintf( cfile, ">" );

		for ( node = firstChild; node; node=node->NextSibling() )
		{
			if ( !node->ToText() )
			{
				fprintf( cfile, "\n" );
			}
			node->Print( cfile, depth+1 );
		}
		fprintf( cfile, "\n" );
		for( i=0; i<depth; ++i )
		fprintf( cfile, "    " );
		fprintf( cfile, "</%s>", value );
	}
}

void TiXmlElement::StreamOut( TIXML_OSTREAM * stream ) const
{
	(*stream) << "<" << value;

	int i;
	for (i = 0 ; i < attributeSet.set.Length () ; i++)
	{
	  const TiDocumentAttribute& attrib = attributeSet.set[i];
	  (*stream) << " ";
	  attrib.StreamOut( stream );
	}

	// If this node has children, give it a closing tag. Else
	// make it an empty tag.
	TiDocumentNode* node;
	if ( firstChild )
	{ 		
		(*stream) << ">";

		for ( node = firstChild; node; node=node->NextSibling() )
		{
			node->StreamOut( stream );
		}
		(*stream) << "</" << value << ">";
	}
	else
	{
		(*stream) << " />";
	}
}

TiDocumentNode* TiXmlElement::Clone() const
{
	TiXmlElement* clone = new TiXmlElement( );
	if ( !clone )
		return 0;

	clone->SetValueRegistered (Value ());
	CopyToClone( clone );

	// Clone the attributes, then clone the children.
	int i;
	for (i = 0 ; i < attributeSet.set.Length () ; i++)
	{
	  const TiDocumentAttribute& attrib = attributeSet.set[i];
	  clone->GetAttributeRegistered (attrib.Name ()).
	  	SetValue (attrib.Value ());
	}

	TiDocumentNode* node = 0;
	for ( node = firstChild; node; node = node->NextSibling() )
	{
		clone->LinkEndChild( node->Clone() );
	}
	return clone;
}


TiDocument::TiDocument() : strings (431)
{
	error = false;
	//	ignoreWhiteSpace = true;
}

TiDocument::TiDocument( const char * documentName ) : strings (431)
{
	//	ignoreWhiteSpace = true;
	value = documentName;
	error = false;
}

bool TiDocument::LoadFile()
{
	// See STL_STRING_BUG below.
	StringToBuffer buf( value );

	if ( buf.buffer && LoadFile( buf.buffer ) )
		return true;

	return false;
}


bool TiDocument::SaveFile() const
{
	// See STL_STRING_BUG below.
	StringToBuffer buf( value );

	if ( buf.buffer && SaveFile( buf.buffer ) )
		return true;

	return false;
}

bool TiDocument::LoadFile( const char* filename )
{
	// Delete the existing data:
	Clear();

	// There was a really terrifying little bug here. The code:
	//		value = filename
	// in the STL case, cause the assignment method of the std::string to
	// be called. What is strange, is that the std::string had the same
	// address as it's c_str() method, and so bad things happen. Looks
	// like a bug in the Microsoft STL implementation.
	// See STL_STRING_BUG above.
	// Fixed with the StringToBuffer class.
	value = filename;

	FILE* file = fopen( value.c_str (), "r" );

	if ( file )
	{
		// Get the file size, so we can pre-allocate the string. HUGE speed impact.
		long length = 0;
		fseek( file, 0, SEEK_END );
		length = ftell( file );
		fseek( file, 0, SEEK_SET );

		// Strange case, but good to handle up front.
		if ( length == 0 )
		{
			fclose( file );
			return false;
		}

		// If we have a file, assume it is all one big XML file, and read it in.
		// The document parser may decide the document ends sooner than the entire file, however.
		TIXML_STRING data;
		data.reserve( length );

		const int BUF_SIZE = 2048;
		char buf[BUF_SIZE];

		while( fgets( buf, BUF_SIZE, file ) )
		{
			data += buf;
		}
		fclose( file );

		Parse( this, data.c_str() );
		if (  !Error() )
		{
			return true;
		}
	}
	SetError( TIXML_ERROR_OPENING_FILE );
	return false;
}

bool TiDocument::SaveFile( const char * filename ) const
{
	// The old c stuff lives on...
	FILE* fp = fopen( filename, "w" );
	if ( fp )
	{
		Print( fp, 0 );
		fclose( fp );
		return true;
	}
	return false;
}


TiDocumentNode* TiDocument::Clone() const
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
		clone->LinkEndChild( node->Clone() );
	}
	return clone;
}


void TiDocument::Print( FILE* cfile, int depth ) const
{
	TiDocumentNode* node;
	for ( node=FirstChild(); node; node=node->NextSibling() )
	{
		node->Print( cfile, depth );
		fprintf( cfile, "\n" );
	}
}

void TiDocument::StreamOut( TIXML_OSTREAM * out ) const
{
	TiDocumentNode* node;
	for ( node=FirstChild(); node; node=node->NextSibling() )
	{
		node->StreamOut( out );

		// Special rule for streams: stop after the root element.
		// The stream in code will only read one element, so don't
		// write more than one.
		if ( node->ToElement() )
			break;
	}
}

void TiDocumentAttribute::Print( FILE* cfile, int /*depth*/ ) const
{
	TIXML_STRING n, v;

	TiXmlBase::PutString( Name(), &n );
	TiXmlBase::PutString( Value(), &v );

	char* idx = strchr (value, '\"');
	if (idx == NULL)
		fprintf (cfile, "%s=\"%s\"", n.c_str(), v.c_str() );
	else
		fprintf (cfile, "%s='%s'", n.c_str(), v.c_str() );
}


void TiDocumentAttribute::StreamOut( TIXML_OSTREAM * stream ) const
{
	char* idx = strchr (value, '\"');
	if (idx != NULL)
	{
		TiXmlBase::PutString( name, stream );
		(*stream) << "=" << "'";
		TiXmlBase::PutString( value, stream );
		(*stream) << "'";
	}
	else
	{
		TiXmlBase::PutString( name, stream );
		(*stream) << "=" << "\"";
		TiXmlBase::PutString( value, stream );
		(*stream) << "\"";
	}
}

void TiDocumentAttribute::SetIntValue( int value )
{
	char buf [64];
	sprintf (buf, "%d", value);
	SetValue (buf);
}

void TiDocumentAttribute::SetDoubleValue( double value )
{
	char buf [64];
	sprintf (buf, "%f", value);
	SetValue (buf);
}

const int TiDocumentAttribute::IntValue() const
{
	return atoi (value);
}

const double  TiDocumentAttribute::DoubleValue() const
{
	return atof (value);
}

void TiXmlComment::Print( FILE* cfile, int depth ) const
{
	for ( int i=0; i<depth; i++ )
	{
		fputs( "    ", cfile );
	}
	fprintf( cfile, "<!--%s-->", value.c_str () );
}

void TiXmlComment::StreamOut( TIXML_OSTREAM * stream ) const
{
	(*stream) << "<!--";
	PutString( value, stream );
	(*stream) << "-->";
}

TiDocumentNode* TiXmlComment::Clone() const
{
	TiXmlComment* clone = new TiXmlComment();

	if ( !clone )
		return 0;

	CopyToClone( clone );
	return clone;
}


void TiXmlText::Print( FILE* cfile, int /*depth*/ ) const
{
	TIXML_STRING buffer;
	PutString( value, &buffer );
	fprintf( cfile, "%s", buffer.c_str() );
}


void TiXmlText::StreamOut( TIXML_OSTREAM * stream ) const
{
	PutString( value, stream );
}


TiDocumentNode* TiXmlText::Clone() const
{	
	TiXmlText* clone = 0;
	clone = new TiXmlText( "" );

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
}


void TiXmlDeclaration::Print( FILE* cfile, int /*depth*/ ) const
{
	fprintf (cfile, "<?xml ");

	if ( !version.empty() )
		fprintf (cfile, "version=\"%s\" ", version.c_str ());
	if ( !encoding.empty() )
		fprintf (cfile, "encoding=\"%s\" ", encoding.c_str ());
	if ( !standalone.empty() )
		fprintf (cfile, "standalone=\"%s\" ", standalone.c_str ());
	fprintf (cfile, "?>");
}

void TiXmlDeclaration::StreamOut( TIXML_OSTREAM * stream ) const
{
	(*stream) << "<?xml ";

	if ( !version.empty() )
	{
		(*stream) << "version=\"";
		PutString( version, stream );
		(*stream) << "\" ";
	}
	if ( !encoding.empty() )
	{
		(*stream) << "encoding=\"";
		PutString( encoding, stream );
		(*stream ) << "\" ";
	}
	if ( !standalone.empty() )
	{
		(*stream) << "standalone=\"";
		PutString( standalone, stream );
		(*stream) << "\" ";
	}
	(*stream) << "?>";
}

TiDocumentNode* TiXmlDeclaration::Clone() const
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


void TiXmlUnknown::Print( FILE* cfile, int depth ) const
{
	for ( int i=0; i<depth; i++ )
		fprintf( cfile, "    " );
	fprintf( cfile, "%s", value.c_str() );
}

void TiXmlUnknown::StreamOut( TIXML_OSTREAM * stream ) const
{
	(*stream) << "<" << value << ">";		// Don't use entities hear! It is unknown.
}

TiDocumentNode* TiXmlUnknown::Clone() const
{
	TiXmlUnknown* clone = new TiXmlUnknown();

	if ( !clone )
		return 0;

	CopyToClone( clone );
	return clone;
}


int TiDocumentAttributeSet::Find (const char * name) const
{
  int i;
  for (i = 0 ; i < set.Length () ; i++)
  {
    if (strcmp (set[i].name, name) == 0) return i;
  }
  return -1;
}

int TiDocumentAttributeSet::FindExact (const char * reg_name) const
{
  int i;
  for (i = 0 ; i < set.Length () ; i++)
  {
    if (set[i].name == reg_name) return i;
  }
  return -1;
}

TIXML_OSTREAM & operator<< (TIXML_OSTREAM & out, const TiDocumentNode & base)
{
	base.StreamOut (& out);
	return out;
}
