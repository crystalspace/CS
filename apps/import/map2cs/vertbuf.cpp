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

#include "cssysdef.h"
#include "mapstd.h"
#include "mpoly.h"
#include "mpolyset.h"
#include "iworld.h"
#include "iportal.h"
#include "vertbuf.h"

#include "dochelp.h"

CVertexBuffer::CVertexBuffer()
{
}

CVertexBuffer::~CVertexBuffer()
{
  DELETE_VECTOR_MEMBERS(m_Vertices);
}

void CVertexBuffer::AddVertex(CdVector3 Vertex)
{
  //Check if the vertex is already stored in the Buffer
  if (GetIndex(Vertex) != (size_t)-1) return;

  //It's a new Vertex:
  m_Vertices.Push(new CdVector3(Vertex));
}

void CVertexBuffer::AddVertices(CMapPolygon* pPoly)
{
  //Add all vertices of this polygon
  size_t NumPolyVertices = pPoly->GetVertexCount();
  size_t i;
  for (i=0; i<NumPolyVertices; i++)
  {
    AddVertex(pPoly->GetVertex(i));
  }
}

void CVertexBuffer::AddVertices(CMapPolygonSet* pPolySet)
{
  //Add all vertices of all polygons
  size_t NumPolygons = pPolySet->GetPolygonCount();
  size_t i;
  for (i=0; i<NumPolygons ; i++)
  {
    AddVertices(pPolySet->GetPolygon(i));
  }
}

void CVertexBuffer::AddVertices(CMapPolygonSetVector* pPolySetVector)
{
  size_t i;
  for (i=0; i<pPolySetVector->Length(); i++)
  {
    AddVertices(pPolySetVector->Get(i));
  }
}

void CVertexBuffer::AddVertices(CIPortalVector* pPortalVector)
{
  size_t i;
  for (i=0; i<pPortalVector->Length(); i++)
  {
    AddVertices(pPortalVector->Get(i));
  }
}

size_t CVertexBuffer::GetIndex(CdVector3 Vertex) const
{
  //search for the vertex in the buffer
  size_t i;
  for (i=0; i<m_Vertices.Length(); i++)
  {
    if (Vertex == (*m_Vertices[i]))
    {
      //found the vertex, return the index
      return i;
    }
  }

  //index not found
  return (size_t)-1;
}

bool CVertexBuffer::WriteCS(csRef<iDocumentNode> node, CIWorld* pWorld)
{
  assert(pWorld);

  double ScaleFactor = pWorld->GetScalefactor();

  size_t i;
  for (i=0; i<m_Vertices.Length(); i++)
  {
    DocNode v = CreateNode (node, "v");
    v->SetAttributeAsFloat ("x", m_Vertices[i]->x*ScaleFactor);
    v->SetAttributeAsFloat ("y", m_Vertices[i]->z*ScaleFactor);
    v->SetAttributeAsFloat ("z", m_Vertices[i]->y*ScaleFactor);
  } //for vertex

  return true;
}
