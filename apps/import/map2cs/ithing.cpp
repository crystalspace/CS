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
#include "map.h"
#include "mpoly.h"
#include "mpolyset.h"
#include "texplane.h"
#include "vertbuf.h"
#include "entity.h"
#include "iworld.h"
#include "ithing.h"

CIThing::CIThing(CMapEntity* pEntity)
{
  assert(pEntity);
  m_pOriginalEntity = pEntity;
}

CIThing::~CIThing()
{
  DELETE_VECTOR_MEMBERS(m_Polygon);
}

void CIThing::InsertPolygon(CMapPolygonSet* pPolygon)
{
  m_Polygon.Push(pPolygon);
}

const char* CIThing::GetName()
{
  assert(m_pOriginalEntity);
  return m_pOriginalEntity->GetName();
}

bool CIThing::IsMoveable()
{
  assert(m_pOriginalEntity);
  return m_pOriginalEntity->GetBoolValueOfKey("moveable", false);
}

bool CIThing::IsSky()
{
  bool Sky = false;
  if (m_pOriginalEntity)
    Sky = m_pOriginalEntity->GetBoolValueOfKey("sky", false);

  return Sky;
}

const char* CIThing::GetClassname()
{
  assert(m_pOriginalEntity);
  return m_pOriginalEntity->GetClassname();
}

bool CIThing::ContainsVisiblePolygons()
{
  size_t i, j;
  for (i=0; i<m_Polygon.Length(); i++)
  {
    CMapPolygonSet* pPolySet = m_Polygon[i];
    for (j=0; j<pPolySet->GetPolygonCount(); j++)
    {
      CMapPolygon*             pPolygon = pPolySet->GetPolygon(j);
      const CMapTexturedPlane* pPlane   = pPolygon->GetBaseplane();
      CTextureFile*            pTexture = pPlane->GetTexture();

      if (pTexture->IsVisible())
      {
        return true;
      }
    }
  }

  return false;
}
