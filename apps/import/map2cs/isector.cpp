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
#include "brush.h"
#include "mpoly.h"
#include "mpolyset.h"
#include "mcurve.h"
#include "map.h"
#include "texplane.h"
#include "entity.h"
#include "vertbuf.h"
#include "iportal.h"
#include "ithing.h"
#include "iworld.h"
#include "isector.h"

CISector::CISector(CMapBrush* pBrush)
{
  assert(pBrush);

  m_pOriginalBrush  = pBrush;

  // If the brush has no assigned entity, then this is only
  // a default sector
  m_IsDefaultsector = (pBrush->GetEntity() == 0);

  // Create wall polygon sets for all sides of the brush
  size_t i;
  for (i=0; i<pBrush->GetPolygonCount(); i++)
  {
    CMapPolygon* pPolygon = pBrush->GetPolygon(i);
    m_Walls.Push(new CMapPolygonSet(*pPolygon));
  } // for poly
}

CISector::~CISector()
{
  DELETE_VECTOR_MEMBERS(m_Walls);
  DELETE_VECTOR_MEMBERS(m_Things);
}

const char* CISector::GetName()
{
  assert(m_pOriginalBrush);
  CMapEntity* pEntity = m_pOriginalBrush->GetEntity();
  if (!pEntity) return "room";
  return pEntity->GetName();
}


bool CISector::IsInside(CdVector3& v)
{
  assert(m_pOriginalBrush);

  return m_pOriginalBrush->IsInside(v);
}

void CISector::CreatePortal(CISector* pOtherSector)
{
  assert(pOtherSector != this);
  size_t i;
  size_t j;

  //Check all pairs of this sectors walls and the other sectors walls. We are using
  //the original brush for this operation, to avoid having portals that are already
  //split. That would be the worst case scenario, because that would mean, the engine
  //must draw the sector multiple times, and this would be a major performance loss!
  for (i=0; i<m_pOriginalBrush->GetPolygonCount(); i++)
  {
    CMapPolygonSet*    pOwnWall  = new CMapPolygonSet(*m_pOriginalBrush->GetPolygon(i));
    CMapTexturedPlane* pOwnPlane = pOwnWall->GetBaseplane();

    for (j=0; j<pOtherSector->m_Walls.Length(); j++)
    {
      CMapPolygonSet*    pOtherWall  = 
	new CMapPolygonSet(*pOtherSector->m_pOriginalBrush->GetPolygon((int)j));
      CMapTexturedPlane* pOtherPlane = pOtherWall->GetBaseplane()->GetMirror();

      //Tf both walls are on the same plane, they might be an intersection, we
      //need to turn into a portal
      if (pOwnPlane->IsSameGeometry(pOtherPlane))
      {
        //Create a portal and reduce it to the area occupied by both sectors
        CIPortal* pNewPortal = new CIPortal(*pOwnWall);
        pNewPortal->ReduceToCommonParts(*pOtherWall);
        if (!pNewPortal->IsEmpty())
        {
          //There is indeed a portal!
          pNewPortal->SetTargetSector(pOtherSector);
          m_Portals.Push(pNewPortal);
          //Make a hole in the wall, we can walk through
          RemoveWallPolygon(pNewPortal, AllOrientations);
        }
        else
        {
          //This portal is leading nowhere
          delete pNewPortal;
        }
      }
      delete pOtherWall;
    } //for j
    delete pOwnWall;
  } // for i
}

void CISector::RemoveWallPolygon(CMapPolygonSet* pRemovePoly, WallOrientation Orientation)
{
  CMapPolygonSet* pWall = GetCorrespondingWall(pRemovePoly->GetBaseplane(), Orientation);
  if (pWall)
  {
    pWall->RemoveCommonParts(*pRemovePoly);
  }
}

CMapPolygonSet* CISector::GetCorrespondingWall(CMapTexturedPlane* pPlane,
                                               WallOrientation Orientation)
{
  if (!pPlane) return 0;

  size_t i;
  for (i=0; i<m_Walls.Length(); i++)
  {
    CMapPolygonSet*    pOwnWall  = m_Walls[i];
    CMapTexturedPlane* pOwnPlane = pOwnWall->GetBaseplane();
    if (pOwnPlane)
    {
      if (pOwnPlane->IsSameGeometry(pPlane) &&
          ((Orientation == SameOrientation) || (Orientation == AllOrientations)))
      {
        return pOwnWall;
      }

      if (pOwnPlane->IsSameGeometry(pPlane->GetMirror()) &&
          ((Orientation == MirroredOrientation) || (Orientation == AllOrientations)))
      {
        return pOwnWall;
      }
    }
  }

  return 0;
}

void CISector::TextureWalls(CIWorld* pWorld)
{
  assert(pWorld);

  size_t j, k;
  size_t i;

  //iterate all entities, brushes, polygons
  for (i=0; i<pWorld->GetNumEntities(); i++)
  {
    CMapEntity* pEntity = pWorld->GetEntity((int)i);

    //the entity is the regular world brush, and no special brush
    //we only texture walls with brushes of that entity, to avoid
    //problems with unnecessarily split polygons.
    for (j=0; j<pEntity->GetNumBrushes(); j++)
    {
      CMapBrush* pBrush = pEntity->GetBrush(j);
      for (k=0; k<pBrush->GetPolygonCount(); k++)
      {
        CMapPolygon* pPolygon = pBrush->GetPolygon(k);
        CMapPolygonSet* pWall = GetCorrespondingWall(pPolygon->GetBaseplane(),
                                                     MirroredOrientation);

        if (pWall)
        {
          // The polygon shares a plane with a sector wall. Now we need to
          // examine, if they occupy the same space. In that case we will
          // replace the wall texture by the polygon texture.
          CMapPolygonSet Polyset(*pPolygon);
          Polyset.ReduceToCommonParts(*pWall);
          if (!Polyset.IsEmpty())
          {
            //There is indeed a common area!
            RemoveWallPolygon(&Polyset, AllOrientations);
            Polyset.FlipSide();
            pWall->AddPolygons(Polyset);
          }
        }
      } //for poly
    } //for brush
  } //for entity
}

void CISector::InsertThings(CIWorld* pWorld)
{
  assert(pWorld);

  size_t i, j, k, p, r;

  size_t BrushesProcessed = 0;
  size_t BrushesToProcess = pWorld->GetMap()->GetNumBrushes() - pWorld->GetNumSectors();

  //iterate all entities, brushes, polygons
  for (i=0; i<pWorld->GetNumEntities(); i++)
  {
    CMapEntity* pEntity = pWorld->GetEntity((int)i);
    if (pEntity->GetNumBrushes() == 0 &&
        pEntity->GetCurveCount() == 0)
    {
      CdVector3 Origin(0,0,0);
      if (pEntity->GetOrigin(Origin))
      {
        if (IsInside(Origin))
        {
          m_Nodes.Push(pEntity);
        }
      }
    }
    else
    {
      CIThing*   pThing  = pWorld->CreateNewThing(pEntity);
      for (j=0; j<pEntity->GetNumBrushes(); j++)
      {
        CMapBrush* pBrush = pEntity->GetBrush(j);
        BrushesProcessed++;
        csPrintf("Adding Things: %.1f%%\r", BrushesProcessed*100.0/BrushesToProcess);

        if (pBrush->GetBoundingBox()->Intersects(m_pOriginalBrush->GetBoundingBox()))
        {
          for (k=0; k<pBrush->GetPolygonCount(); k++)
          {
            CMapPolygon        Polygon    = *pBrush->GetPolygon(k);
            CMapTexturedPlane* pPolyPlane = Polygon.GetBaseplane();

            for (p=0; p<m_pOriginalBrush->GetPolygonCount(); p++)
            {
              CMapTexturedPlane* pPlane = m_pOriginalBrush->GetPolygon(p)->GetBaseplane();

              if (pPlane->IsSameGeometry(pPolyPlane))
              {
                //Check, if the polygon is lying directly on a sector plane.
                //in that case, we do not add the polygon to the sector as a thing.
                //That would only lead to visual problems.
                Polygon.Clear();
              }
              else
              {
                //Clip this polygon against all bounding planes of the sector
                Polygon.Split(m_pOriginalBrush->GetPolygon(p)->GetBaseplane(),
                              &Polygon);
              }
            } // for Polygons

            if (!Polygon.IsEmpty())
            {
              //if some part of the polygon is inside the sector
              CMapPolygonSet* pThingPoly = new CMapPolygonSet(Polygon);

              if (pWorld->GetMap()->GetConfigInt("Map2CS.General.RemoveHidden", 0))
              {
                CMapBrushBoundingBox PolygonBoundingBox;
                PolygonBoundingBox.Extend(&Polygon);

                CMapPolygon RemovePoly;

                //check if this polygon is inside some other part of the thing.
                //To do so, we intersect the baseplane of the polygon with every
                //other brush of the same entity. If the baseplane intersects,
                //we remove the common area, because that area is inside the brush.
                for (r=0; r<pEntity->GetNumBrushes(); r++)
                {
                  if (r==j) continue;

                  CMapBrush* pBrush = pEntity->GetBrush(r);

                  if (!pBrush->IsVisible()) continue;

                  if (!PolygonBoundingBox.Intersects(pBrush->GetBoundingBox())) continue;

                  pBrush->IntersectWithPlane(Polygon.GetBaseplane(), RemovePoly);
                  if (!RemovePoly.IsEmpty())
                  {
                    pThingPoly->RemoveCommonParts(RemovePoly);
                  }
                }
              } //if (RemoveHiddenPolys)

              pThing->InsertPolygon(pThingPoly);
            }
          } //for poly
        } //if (Check only, if brushes intersect)
      } //for brush

      if (pThing->IsEmpty())
      {
        delete pThing;
      }
      else
      {
        m_Things.Push(pThing);
      }
    } //if entity has brushes or curves
  } //for entity

  csPrintf("                      \r");
}



