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
#include "xr.h"
#include "csutil/scfstr.h"

const char* TrXmlBase::errorString[ TIXML_ERROR_STRING_COUNT ] =
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

bool TrXmlBase::condenseWhiteSpace = true;

TrDocumentNode::TrDocumentNode( )
{
  parent = 0;
  next = 0;
}


TrDocumentNode::~TrDocumentNode()
{
}


TrDocumentNode* TrDocumentNode::NextSibling( const char * value ) const
{
  TrDocumentNode* node;
  for ( node = next; node; node = node->next )
  {
    const char* node_val = node->Value ();
    if (node_val && strcmp (node_val, value) == 0)
      return node;
  }
  return 0;
}


TrDocumentNode* TrDocumentNodeChildren::Identify (TrDocument* document,
	const char* p)
{
  TrDocumentNode* returnNode = 0;

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

  if (StringEqual (p, xmlHeader))
  {
    returnNode = new TrXmlDeclaration();
  }
  else if (isalpha (*(p+1))
        || *(p+1) == '_' )
  {
    returnNode = document->blk_element.Alloc ();
  }
  else if (StringEqual (p, commentHeader))
  {
    returnNode = new TrXmlComment();
  }
  else
  {
    returnNode = new TrXmlUnknown();
  }

  if (returnNode)
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
TrDocumentNodeChildren::TrDocumentNodeChildren ()
{
  firstChild = 0;
}

TrDocumentNodeChildren::~TrDocumentNodeChildren()
{
  TrDocumentNode* node = firstChild;
  TrDocumentNode* temp = 0;

  TrDocument* doc = GetDocument ();
  while ( node )
  {
    temp = node;
    node = node->next;
    doc->DeleteNode (temp);
  }  
}

void TrDocumentNodeChildren::Clear()
{
  TrDocumentNode* node = firstChild;
  TrDocumentNode* temp = 0;

  TrDocument* doc = GetDocument ();
  while ( node )
  {
    temp = node;
    node = node->next;
    doc->DeleteNode (temp);
  }

  firstChild = 0;
}

TrDocumentNode* TrDocumentNodeChildren::LinkEndChild( TrDocumentNode* lastChild,
	TrDocumentNode* node )
{
  node->parent = this;

  node->next = 0;

  if ( lastChild )
    lastChild->next = node;
  else
    firstChild = node;      // it was an empty list.

  lastChild = node;
  return node;
}


TrDocumentNode* TrDocumentNodeChildren::FirstChild( const char * value ) const
{
  TrDocumentNode* node;
  for ( node = firstChild; node; node = node->next )
  {
    const char* node_val = node->Value ();
    if (node_val && strcmp (node_val, value) == 0)
      return node;
  }
  return 0;
}

// -------------------------------------------------------------------------

TrDocument* TrDocumentNode::GetDocument() const
{
  const TrDocumentNode* node;

  for( node = this; node; node = node->parent )
  {
    if ( node->ToDocument() )
      return node->ToDocument();
  }
  return 0;
}


TrXmlElement::TrXmlElement ()
{
  value = 0;
  type = ELEMENT;
  contentsvalue = 0;
}

TrXmlElement::~TrXmlElement()
{
}


const char * TrXmlElement::Attribute( const char * name )
{
  int nodeidx = attributeSet.Find (name);

  if (nodeidx != -1)
    return attributeSet.set[nodeidx].Value ();

  return 0;
}


const char * TrXmlElement::Attribute( const char * name, int* i )
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


TrDocumentAttribute& TrXmlElement::GetAttributeRegistered (
  const char * reg_name)
{
  int nodeidx = attributeSet.FindExact (reg_name);
  if (nodeidx != -1)
  {
    return attributeSet.set[nodeidx];
  }

  TrDocumentAttribute at;
  int idx = attributeSet.set.Push (at);
  attributeSet.set[idx].SetName (reg_name);
  return attributeSet.set[idx];
}

TrDocument::TrDocument(char* buf) :
  blk_element (7000),
  blk_text (100)
{
  error = false;
  //  ignoreWhiteSpace = true;
  type = DOCUMENT;
  input_data = buf;
}

TrDocument::~TrDocument ()
{
  // Call explicit clear so that all children are destroyed
  // before 'blk_element' and 'blk_text' are destroyed.
  Clear ();
  delete[] input_data;
}

const int TrDocumentAttribute::IntValue() const
{
  value[vallen] = 0;
  return atoi (value);
}

const double  TrDocumentAttribute::DoubleValue() const
{
  value[vallen] = 0;
  return atof (value);
}

TrXmlDeclaration::TrXmlDeclaration( const char * _version,
  const char * _encoding,
  const char * _standalone )
{
  version = _version;
  encoding = _encoding;
  standalone = _standalone;
  value = 0;
  type = DECLARATION;
}


int TrDocumentAttributeSet::Find (const char * name) const
{
  int i;
  for (i = 0 ; i < set.Length () ; i++)
  {
    if (strcmp (set[i].name, name) == 0) return i;
  }
  return -1;
}

int TrDocumentAttributeSet::FindExact (const char * reg_name) const
{
  int i;
  for (i = 0 ; i < set.Length () ; i++)
  {
    if (set[i].name == reg_name) return i;
  }
  return -1;
}

