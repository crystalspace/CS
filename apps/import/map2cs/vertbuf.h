/*
    Map2cs: a convertor to convert the frequently used MAP format, into
    something, that can be directly understood by Crystal Space.

    Copyright (C) 1999 Thomas Hieber (thieber@gmx.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __VERTBUF_H__
#define __VERTBUF_H__

#include "contain.h"

#include "csutil/ref.h"

class CMapPolygon;
class CIWorld;
struct iDocumentNode;

/**
  */
class CVertexBuffer
{
public:
  /// The constructor (as usual)
  CVertexBuffer();

  /// The destuctor
  ~CVertexBuffer();

  /// Add a single vertex
  void AddVertex(CdVector3 Vertex);

  /// Adds the vertices from the given polygon to this buffer
  void AddVertices(CMapPolygon* pPoly);

  /// Add the vertices from a whole polygonset to this buffer
  void AddVertices(CMapPolygonSet* pPolySet);

  /// Add the vertices from an array of polysets to this buffer
  void AddVertices(CMapPolygonSetVector* pPolySetVector);

  /// Add the vertices from an array of portalsto this buffer
  void AddVertices(CIPortalVector* pPortalVector);

  /**
    * returns the index of the given Vertex, or -1 if the vertex is
    * not yet stored in this VertexBuffer
    */
  size_t GetIndex(CdVector3 Vertex) const;

  /**
    * Write all the vertices that have been previously added by
    * AddVertex and AddVertices to the worldfile in standard ASCII
    * CS worldformat. (Maybe we should split up CVertexBuffer into
    * different classes for different formats too.)
    */
  bool WriteCS(csRef<iDocumentNode> node, CIWorld* pWorld);

protected:
  /// Array containing all vertices
  CVector3Vector    m_Vertices;
};

#endif // __VERTBUF_H__

