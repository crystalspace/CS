/*
    Copyright (C) 2004 by Eric Sunshine

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
    
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
    
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CSUTIL_TINYWRAP_H__
#define __CSUTIL_TINYWRAP_H__

//-----------------------------------------------------------------------------
// The TinyXML classes are likely to be embedded into other libraries, in
// addition to being embedded into Crystal Space's csutil.  In order to avoid
// symbolic conflicts when client applications link against both the Crystal
// Space libraries and other libraries which embed TinyXML (such as Cal3D), we
// move our copy of TinXML into a private namespace.
//-----------------------------------------------------------------------------

#define GrowString		csTiGrowString
#define TiDocument		csTiDocument
#define TiDocumentAttribute	csTiDocumentAttribute
#define TiDocumentAttributeSet	csTiDocumentAttributeSet
#define TiDocumentNode		csTiDocumentNode
#define TiDocumentNodeChildren	csTiDocumentNodeChildren
#define TiXmlBase		csTiXmlBase
#define TiXmlCData		csTiXmlCData
#define TiXmlComment		csTiXmlComment
#define TiXmlDeclaration	csTiXmlDeclaration
#define TiXmlElement		csTiXmlElement
#define TiXmlString		csTiXmlString
#define TiXmlText		csTiXmlText
#define TiXmlUnknown		csTiXmlUnknown

#endif // __CSUTIL_TINYWRAP_H__
