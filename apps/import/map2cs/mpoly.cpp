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
#include "texplane.h"
#include "mpoly.h"

CMapPolygon::CMapPolygon()
{
  m_pBaseplane      = 0;
  m_pBrush          = 0;
  m_BrushLineNumber = 0;
  m_PlaneNumber     = 0;
}

CMapPolygon::CMapPolygon(const CMapPolygon& poly)
{
  m_pBaseplane      = poly.m_pBaseplane;
  m_pBrush          = poly.m_pBrush;
  m_BrushLineNumber = poly.m_BrushLineNumber;
  m_PlaneNumber     = poly.m_PlaneNumber;
  size_t i;
  for (i=0; i<poly.m_Planes.Length(); i++)
  {
    m_Planes.Push(poly.m_Planes[i]); //only stored by reference
  }

  //The vertices need to be copied!
  COPY_VECTOR_MEMBERS(m_Vertices, poly.m_Vertices, CdVector3);
}

CMapPolygon::~CMapPolygon()
{
  //Do the final cleanup.
  Clear();
}

void CMapPolygon::Create(CMapTexturedPlane*              pBaseplane,
                          const CMapTexturedPlaneVector& planes,
                          CMapBrush*                     pBrush)
{
  assert(pBaseplane);
  assert(planes.Length()>2);

  //Remove all previous contents
  Clear();

  //Start to create a new polygon. We start by remembering the baseplane.
  m_pBaseplane = pBaseplane;
  m_pBrush     = pBrush;

  //for the upcoming handling, we copy planes into a reduced array of planes
  //this will help us to change order and to remove already used planes.
  //Also we will remove all planes that don't create an edge right here.
  size_t p=0;
  CMapTexturedPlaneVector RedplanesStage1;
  for (p=0; p<planes.Length(); p++)
  {
    CMapTexturedPlane* pPlane = planes[p];
    if (pPlane->IsSameGeometry(pBaseplane) ||
        pPlane->IsSameGeometry(pBaseplane->GetMirror())) continue;

    RedplanesStage1.Push(pPlane);
  }


  CMapTexturedPlaneVector redplanes;
  for (p=0; p<RedplanesStage1.Length(); p++)
  {
    CMapTexturedPlane* pPlane = RedplanesStage1[p];

    int NumValVertex = GetNumberOfValidVertices(pPlane, RedplanesStage1);
    if (NumValVertex == 2)
    {
      redplanes.Push(pPlane);
    }
  }

  //Now we try to find a starting position for our search for edges and points.
  //This position will be a vertex, that is defined by the baseplane and two
  //other planes. The point is guaranteed to be part of the polygon and not
  //to be outside of it.
  //they will also be sorted, so that one can contiue from plane1 to plane2 and
  //one will get a proper orientation of the polygon for backface culling.
  CMapTexturedPlane* pPlane1 = 0;
  CMapTexturedPlane* pPlane2 = 0;
  CdVector3          point(0,0,0);
  GetStartplanes(redplanes, pPlane1, pPlane2, point);

  if (!pPlane1 || !pPlane2)
  {
    //The polygon is empty!
    return;
  }

  //Ok, the first point and planes are done
  m_Planes.Push(pPlane1);
  m_Planes.Push(pPlane2);
  m_Vertices.Push(new CdVector3(point));

  //Remove plane1 and plane2 from the array
  redplanes.DeleteIndex(redplanes.Find(pPlane1));
  redplanes.DeleteIndex(redplanes.Find(pPlane2));

  //Remove the baseplane too.
  size_t BaseplaneIndex = redplanes.Find(pBaseplane);
  if (BaseplaneIndex != csArrayItemNotFound) redplanes.DeleteIndex(BaseplaneIndex);

  //Insert pPlane1 as first element of redplanes. This will ensure, that
  //this plane is preferred to close the polygon again. Otherwise there
  //is a small chance the polygon is closed by another plane and we can't
  //detect this.
  redplanes.Insert(0, pPlane1);

  //DumpPolyinfo(m_pBaseplane, redplanes);

  //loop as long the polygon is not closed (pPlane2 != m_Planes[0])
  //and as long as new vertices are being found
  bool FoundNewVertex  = true;
  while ((pPlane2 != m_Planes[0]) && FoundNewVertex)
  {
    FoundNewVertex = false;
    pPlane1 = m_Planes[m_Planes.Length()-1];
    //pPlane1 = pPlane2;

    size_t NumPlanes = redplanes.Length();

    //Check all planes, if they will form a new legal vertex
    //with the baseplane and plane1 (the last found plane)
    for (p=0; p<NumPlanes; p++)
    {
      pPlane2 = redplanes[p];

      //Only after three planes have been inserted, we can check if
      //m_Planes[0] will close the polygon.
      //if (m_Planes.Length() < 3 && i==0) continue;

      //If this plane is to be processed, and both planes will form
      //a vertex with the baseplane, we need to do further checks.
      if (CdIntersect3::Planes(*m_pBaseplane, *pPlane1,
                               *pPlane2, point))
      {
        //Check if the newly found vertex is identical to one of the two last
        //vertices. If this is the case, we will ignore that point, because
        //this is an identitcal edge to a previous edge, and would close the
        //polygon in an illegal way.
        if ((((*m_Vertices[m_Vertices.Length()-1])-point).Norm()<SMALL_EPSILON) ||
            (m_Vertices.Length() > 1 &&
             ((*m_Vertices[m_Vertices.Length()-2])-point).Norm()<SMALL_EPSILON))
        {
          //This would close the polygon too early. We wont handle this plane.
          //If there is no alternative found later on, then we declare this polygon
          //to be empty.
          continue; //Continue with processing the next plane.
        }

        //See if the new Vertex is a valid vertex. For this we use the original
        //planes array, and not the reduced array!
        //assert(point != *(m_Vertices[m_Vertices.Length()-1]));
        if (CheckIfInside(point, planes, m_pBaseplane, pPlane1, pPlane2))
        {
          //Ok, this is a good next plane
          if (pPlane2 != m_Planes[0])
          {
            //We only push that plane, if it is not closing the polygon.
            //Otherwise, m_Planes[0] would be added twice.
            m_Planes.Push(pPlane2);

            //Remove this plane, so further processing will be faster.
            redplanes.DeleteIndex(redplanes.Find(pPlane2));
          }
          //In any case, we need the vertex. (which _is_ new)
          m_Vertices.Push(new CdVector3(point));

          //Remember that we found a new vertex
          FoundNewVertex = true;

          //leave the for loop across all planes and prepare search for
          //the next plane.
          break;
        }
      }
    } //for(i)
  } //while(poly not closed)

  if (!FoundNewVertex)
  {
    // This polygon is degenerated. This may happen, if you are doing some
    // weird splits. (I have come to think now, that this is a rather commom
    // case instead of an exception)

    ////csPrintf("Create() failed!\n");
    ////DumpPolyinfo(pBaseplane, planes);
    ////Create(pBaseplane, planes);

    Clear();
    return;
  }

  //The following check would be nice to have, but test with some maps showed, that
  //even polygons got removed that had an area _much_ larger than one pixel.
  //In future, I need to either fix GetArea or get the final polygon merger step
  //before exporting to work. (Probably the second, because that will cure other
  //problems too.)

  //if (GetArea()<1)
  //{
    //csPrintf("removed polygon due to too small area! (%g)\n", GetArea());
    //Clear();
  //}
}

void CMapPolygon::Clear()
{
  //Remove previous vertices
  DELETE_VECTOR_MEMBERS(m_Vertices);
  m_Planes.DeleteAll();
  m_pBaseplane = 0;
  m_pBrush     = 0;
}

bool CMapPolygon::IsEmpty()
{
  assert(m_Vertices.Length() == m_Planes.Length());
  assert(m_Vertices.Length() >= 0);

  return m_Vertices.Length() == 0;
}

double CMapPolygon::GetArea()
{
  // 2 A(P) = abs(N . (sum_{i=0}^{n-1} (v_i x v_{i+1})))
  //
  // where N is a unit vector normal to the plane. The `.' represents the
  // dot product operator, the `x' represents the cross product operator,
  // and abs() is the absolute value function.

  //Attention: Obviously this method is broken. I don't know why, but sometimes it
  //will report sizes<1, where the polygon is in fact larger than that.
  //Thomas Hieber Jan 08th 2000
  assert(false);

  CdVector3 Normal = m_pBaseplane->GetNormal().Unit();
  CdVector3 Sum(0,0,0);

  size_t i;
  for (i=0; i<(m_Vertices.Length()-1); i++)
  {
    Sum += (*(m_Vertices[i])) % (*(m_Vertices[i+1]));
  }

  double Area = (Normal * Sum)/2;
  return ABS(Area);
}

void CMapPolygon::Split(CMapTexturedPlane* pSplitplane,
                        CMapPolygon*       pTargetPoly)
{
  assert(pSplitplane);
  assert(pTargetPoly);

  //If this polygon is empty, we can return at once.
  if (IsEmpty()) return;

  //To create a new Polygon is really simple. We just create a new polygon, based
  //on all current planes plus the splitplane. This will automatically remove all
  //unneeded planes and vertices.
  CMapTexturedPlaneVector NewPlanes;

  size_t NumPlanes = m_Planes.Length();
  size_t i;
  for (i=0; i<NumPlanes; i++)
  {
    //Use all current planes
    NewPlanes.Push(m_Planes[i]);
  }
  //Also take into accont the splitplane
  NewPlanes.Push(pSplitplane);

  //Create a new polygon from these planes on the current baseplane.
  pTargetPoly->Create(m_pBaseplane, NewPlanes, m_pBrush);
}

/// Assign an existing polygon.
void CMapPolygon::SetPolygon(CMapPolygon* pPoly)
{
  m_pBaseplane = pPoly->m_pBaseplane;;

  size_t i=0;
  for (i=0; i<pPoly->m_Vertices.Length(); i++)
  {
    m_Vertices.Push(new CdVector3(*pPoly->m_Vertices[i]));
  }

  for (i=0; i<pPoly->m_Planes.Length(); i++)
  {
    m_Planes.Push(pPoly->m_Planes[i]);
  }
}

void CMapPolygon::FlipSide()
{
  m_pBaseplane = m_pBaseplane->GetMirror();

  size_t i=0;
  for (i=0; i<m_Vertices.Length()/2; i++)
  {
    CdVector3* pHelp                    = m_Vertices[i];
    m_Vertices[i]                       = m_Vertices[m_Vertices.Length()-i-1];
    m_Vertices[m_Vertices.Length()-i-1] = pHelp;
  }

  for (i=0; i<m_Planes.Length()/2; i++)
  {
    CMapTexturedPlane* pHelp        = m_Planes[i];
    m_Planes[i]                     = m_Planes[m_Planes.Length()-i-1];
    m_Planes[m_Planes.Length()-i-1] = pHelp;
  }
}

CMapEntity* CMapPolygon::GetEntity()
{
  assert(m_pBrush);
  return m_pBrush->GetEntity();
}

void CMapPolygon::GetStartplanes(const CMapTexturedPlaneVector& planes,
                                 CMapTexturedPlane*&            pPlane1,
                                 CMapTexturedPlane*&            pPlane2,
                                 CdVector3&                     point)
{
  size_t NumPlanes = planes.Length();

  //Try the follwing for all possible pairs of planes.
  size_t i, j;
  for (i=0; i<NumPlanes; i++)
  {
    pPlane1 = planes[i];
    for (j=i+1; j<NumPlanes; j++)
    {
      pPlane2 = planes[j];
      if (CdIntersect3::Planes(*m_pBaseplane, *pPlane1, *pPlane2, point))
      {
        if (CheckIfInside(point, planes, m_pBaseplane, pPlane1, pPlane2))
        {
          //OK, these planes might be a good start

          //To get proper orientation (backface culling!) of the polygon, we
          //need to decide where to start. To do this, we take the cross product
          //of the normals of the two planes, and check if this points to
          //the same direction as the baseplane. If not, we need to swap planes
          double direction = (pPlane1->Normal()%pPlane2->Normal())*
                              m_pBaseplane->Normal();

          if (direction<0)
          {
            //Ok, now we need to swap planes.
            CMapTexturedPlane* p = pPlane1;
            pPlane1 = pPlane2;
            pPlane2 = p;
          }
          return;
        }
      } //if (intersect)
    } //for (j)
  } //for (i)

  //if we arrive here, then there is a problem finding any starting pint
  //for the poly
  pPlane1 = 0;
  pPlane2 = 0;

}

int  CMapPolygon::GetNumberOfValidVertices(CMapTexturedPlane*             pPlane,
                                           const CMapTexturedPlaneVector& planes)
{
  int       GetVertexCount = 0;
  CdVector3 Vertex[3]; // I expect this array to be filled with only two values.
                       // getting three values happens only in case of an error,
                       // that will be explicitly tracked down.

  size_t NumPlanes = planes.Length();

  //Try the follwing for all planes.
  size_t i;
  for (i=0; i<NumPlanes; i++)
  {
    CMapTexturedPlane* pOtherPlane = planes[i];
    if (pOtherPlane == pPlane || pOtherPlane == m_pBaseplane) continue;

    CdVector3 point(0,0,0);
    if (CdIntersect3::Planes(*m_pBaseplane, *pPlane, *pOtherPlane, point))
    {
      if (CheckIfInside(point, planes, m_pBaseplane, pPlane, pOtherPlane))
      {
        //OK, this vertex is the inside the polygon. Now we check, if this vertex
        //was already found. (possible, if there are several planes cutting through
        //the same vertex.
        bool DuplicateVertex = false;
        int k;
        for (k=0; k<GetVertexCount; k++)
        {
          // check if the point is the same (or almost the same) as another point
          if ((point-Vertex[k])<0.00001)
          {
            DuplicateVertex = true;
          }
        }
        if (!DuplicateVertex)
        {
          //A new original vertex has been found.
          Vertex[GetVertexCount] = point;
          GetVertexCount++;
          if (GetVertexCount>=3)
          {
            //DumpPolyinfo(pPlane, planes);
            //@@@@@@@@@@@assert(false);
            return 3;
          }
        } //if not DuplicateVertex;
      } //if vertex is inside
    } //if (intersect)
  } //for (i)

  return GetVertexCount;
}

bool CMapPolygon::CheckIfInside(const CdVector3&               point,
                                const CMapTexturedPlaneVector& planes,
                                CMapTexturedPlane const*       pIgnorePlane1,
                                CMapTexturedPlane const*       pIgnorePlane2,
                                CMapTexturedPlane const*       pIgnorePlane3)
{
  //All thre planes create at least a single point. Now we need to
  //determine, if that point is inside or outside the given polygons

  size_t k, NumPlanes = planes.Length();
  for (k=0; k<NumPlanes; k++)
  {
    CMapTexturedPlane* pPlane = planes[k];

    //we will not check the point against the planes forming it.
    //this will avoid problems related to precision loss.
    if (pPlane == pIgnorePlane1 ||
        pPlane == pIgnorePlane2 ||
        pPlane == pIgnorePlane3) continue;

    if (pPlane->Classify(point) < (-SMALL_EPSILON))
    {
      return false;
    }
  }  //for (k)

  return true;
}

void CMapPolygon::SetErrorInfo(int BrushLineNumber, size_t PlaneNumber)
{
  m_BrushLineNumber = BrushLineNumber;
  m_PlaneNumber     = PlaneNumber;
}

void CMapPolygon::DumpPolyinfo(CMapTexturedPlane*             pBaseplane,
                               const CMapTexturedPlaneVector& planes)
{
  size_t j, i, y, c, x;
  for (j=0; j<m_Planes.Length(); j++)
  {
    for (i=0; i<planes.Length(); i++)
    {
      if (m_Planes[j] == planes[i])
      {
        csPrintf("%zu ", i);
      }
    }
  }
  csPrintf("\n");

  for (y=0; y<planes.Length(); y++)
  {
    for (c=0; c<3; c++)
    {
      for (x=0; x<planes.Length(); x++)
      {
        CdVector3 point;
        if (x==y)
        {
          csPrintf("        * "); //10 char "No check!"
        }
        else
        {
          if (CdIntersect3::Planes(*pBaseplane, *planes[x], *planes[y], point))
          {
            double v=(c==0)?point.x:(c==1)?point.y:point.z;

            if (CheckIfInside(point, planes, pBaseplane, planes[x], planes[y]))
            {
              csPrintf("%8.3fI ",v); //10 char "Inside"
            }
            else
            {
              csPrintf("%8.3fO ",v); //10 char "Outside"
            }
          }
          else
          {
            csPrintf("        M "); //10 char "Miss"
          }
        }
      }
      csPrintf("\n");
    }
    csPrintf("\n");
  }
  csPrintf("Dump done\n");
}
