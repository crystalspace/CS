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
#include "texplane.h"

CMapPolygonSet::CMapPolygonSet()
{
}

CMapPolygonSet::CMapPolygonSet(const CMapPolygonSet& Set)
{
  COPY_VECTOR_MEMBERS(m_Polygon, Set.m_Polygon, CMapPolygon);
}

CMapPolygonSet::CMapPolygonSet(const CMapPolygon& Poly)
{
  m_Polygon.Push(new CMapPolygon(Poly));
}

CMapPolygonSet::~CMapPolygonSet()
{
  DELETE_VECTOR_MEMBERS(m_Polygon);
}

CMapPolygonSet& CMapPolygonSet::operator=(const CMapPolygonSet& Other)
{
  DELETE_VECTOR_MEMBERS(m_Polygon);

  AddPolygons(Other);

  return *this;
}

void CMapPolygonSet::AddPolygons(const CMapPolygonSet& Other)
{
  size_t i;
  for (i=0; i<Other.m_Polygon.Length(); i++)
  {
    m_Polygon.Push(new CMapPolygon(*Other.m_Polygon.Get(i)));
  }
}

void CMapPolygonSet::FlipSide()
{
  size_t i;
  for (i=0; i<m_Polygon.Length(); i++)
  {
    m_Polygon[i]->FlipSide();
  }
}

void CMapPolygonSet::ReduceToCommonParts(const CMapPolygonSet& Other, bool optimise)
{
  CMapPolygonVector NewPoly;
  size_t i, j, k;

  //We handle every pair of convex Polygon and Other convex Polygon
  for (i=0; i<m_Polygon.Length(); i++)
  {
    if (optimise)
    {
      CMapPolygonSet Tempset(*m_Polygon[i]);
      Tempset.RemoveCommonParts(Other, false);
      if (Tempset.IsEmpty())
      {
        //This polygon is completely within the other polygon, so we can
        //add it without splitting
        NewPoly.Push(new CMapPolygon(*m_Polygon[i]));
        continue;
      }
    }

    //We need to split the polygon into smaller parts.
    for (j=0; j<Other.m_Polygon.Length(); j++)
    {
      CMapPolygon  Poly(*(m_Polygon[i]));
      CMapPolygon* pClipPoly = Other.m_Polygon[j];

      //We clip them against each other, to get the common parts
      for (k=0; k<pClipPoly->GetVertexCount(); k++)
      {
        Poly.Split(pClipPoly->GetPlane(k), &Poly);
        if (Poly.IsEmpty()) break;
      }

      //If there is a common part, we add this to the Polygon set.
      if (!Poly.IsEmpty())
      {
        NewPoly.Push(new CMapPolygon(Poly));
      }
    }
  }

  //As a last step, we clear the current list of polygons and replace
  //it by a new list.
  DELETE_VECTOR_MEMBERS(m_Polygon);
  size_t p;
  for (p=0; p<NewPoly.Length(); p++)
  {
    m_Polygon.Push(NewPoly.Get(p));
  }
}

void CMapPolygonSet::RemoveCommonParts(const CMapPolygonSet& Other, bool optimise)
{
  size_t i;
  for (i=0; i<Other.m_Polygon.Length(); i++)
  {
    if (optimise)
    {
      CMapPolygonSet Tempset(*Other.m_Polygon[i]);
      Tempset.ReduceToCommonParts(*this, false);
      if (Tempset.IsEmpty()) continue;
    }

    RemovePolygon(*Other.m_Polygon[i]);
  }
}

CMapTexturedPlane* CMapPolygonSet::GetBaseplane()
{
  if (GetPolygonCount()==0) return 0;

  CMapPolygon* pPoly1 = GetPolygon(0);
  assert(pPoly1);

  return pPoly1->GetBaseplane();
}

void CMapPolygonSet::RemovePolygon(const CMapPolygon& Other)
{
  CMapPolygonVector NewPoly;

  size_t i, k;
  for (i=0; i<m_Polygon.Length(); i++)
  {
    CMapPolygon Poly(*(m_Polygon[i]));

    for (k=0; k<Other.GetVertexCount(); k++)
    {
      //Get the area outside of this plane
      CMapPolygon Outside;
      Poly.Split(Other.GetPlane(k)->GetMirror(), &Outside);

      if (!Outside.IsEmpty())
      {
        //If there is some area outside:
        NewPoly.Push(new CMapPolygon(Outside));

        //Reduce the polygon. Remove the Outside part.
        Poly.Split(Other.GetPlane(k), &Poly);

        //If there is nothing left, stop this loop
        if (Poly.IsEmpty()) break;
      }
    }
  }

  //As a last step, we clear the current list of polygons and replace
  //it by a new list.
  DELETE_VECTOR_MEMBERS(m_Polygon);
  size_t p;
  for (p=0; p<NewPoly.Length(); p++)
  {
    m_Polygon.Push(NewPoly.Get(p));
  }
}
