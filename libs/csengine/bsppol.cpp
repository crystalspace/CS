/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#include "sysdef.h"
#include "qint.h"
#include "csengine/bsppol.h"
#include "csengine/polygon.h"

//---------------------------------------------------------------------------

csPolygonBsp::csPolygonBsp ()
{
  max_vertices = 4;
  CHK (vertices = new csVector3 [max_vertices]);
  num_vertices = 0;
  poly3d = NULL;
}

csPolygonBsp::csPolygonBsp (csPolygon3D* orig_poly3d)
{
  max_vertices = num_vertices = orig_poly3d->GetNumVertices ();
  CHK (vertices = new csVector3 [max_vertices]);
  int i;
  for (i = 0 ; i < num_vertices ; i++)
    vertices[i] = orig_poly3d->Vwor (i);
  poly3d = orig_poly3d;
}

csPolygonBsp::~csPolygonBsp ()
{
  CHK (delete [] vertices);
}


void csPolygonBsp::AddVertex (const csVector3& v)
{
  if (num_vertices >= max_vertices)
  {
    max_vertices += 2;
    CHK (csVector3* new_vertices = new csVector3 [max_vertices]);
    memcpy (new_vertices, vertices, sizeof (csVector3)*num_vertices);
    CHK (delete [] vertices);
    vertices = new_vertices;
  }
  vertices[num_vertices++] = v;
}

int csPolygonBsp::Classify (csPolygonInt* spoly)
{
  if (SamePlane (spoly)) return POL_SAME_PLANE;

  int i;
  int front = 0, back = 0;
  csPolygonBsp* poly = (csPolygonBsp*)spoly;
  csPolyPlane* pl = poly->poly3d->GetPlane ();

  for (i = 0 ; i < num_vertices ; i++)
  {
    float dot = pl->Classify (vertices[i]);
    if (ABS (dot) < SMALL_EPSILON) dot = 0;
    if (dot > 0) back++;
    else if (dot < 0) front++;
  }
  if (back == 0) return POL_FRONT;
  if (front == 0) return POL_BACK;
  return POL_SPLIT_NEEDED;
}

void csPolygonBsp::SplitWithPlane (csPolygonInt** poly1, csPolygonInt** poly2,
				   csPlane& plane)
{
  CHK (csPolygonBsp* np1 = new csPolygonBsp ());
  CHK (csPolygonBsp* np2 = new csPolygonBsp ());
  *poly1 = (csPolygonInt*)np1; // Front
  *poly2 = (csPolygonInt*)np2; // Back
  np1->SetPolygon3D (poly3d);
  np2->SetPolygon3D (poly3d);

  csVector3 ptB;
  float sideA, sideB;
  csVector3 ptA = vertices[num_vertices - 1];
  sideA = plane.Classify (ptA);

  for (int i = -1 ; ++i < num_vertices ; )
  {
    ptB = vertices[i];
    sideB = plane.Classify (ptB);
    if (sideB > 0)
    {
      if (sideA < 0)
      {
	// Compute the intersection point of the line
	// from point A to point B with the partition
	// plane. This is a simple ray-plane intersection.
	csVector3 v = ptB; v -= ptA;
	float sect = - plane.Classify (ptA) / ( plane.Normal () * v ) ;
	v *= sect; v += ptA;
	np1->AddVertex (v);
	np2->AddVertex (v);
      }
      np2->AddVertex (ptB);
    }
    else if (sideB < 0)
    {
      if (sideA > 0)
      {
	// Compute the intersection point of the line
	// from point A to point B with the partition
	// plane. This is a simple ray-plane intersection.
	csVector3 v = ptB; v -= ptA;
	float sect = - plane.Classify (ptA) / ( plane.Normal () * v );
	v *= sect; v += ptA;
	np1->AddVertex (v);
	np2->AddVertex (v);
      }
      np1->AddVertex (ptB);
    }
    else
    {
      np1->AddVertex (ptB);
      np2->AddVertex (ptB);
    }
    ptA = ptB;
    sideA = sideB;
  }
}

bool csPolygonBsp::SamePlane (csPolygonInt* p)
{
  if (((csPolygonBsp*)p)->GetPolyPlane () == GetPolyPlane ()) return true;
  return csMath3::PlanesEqual (*((csPolygonBsp*)p)->GetPolyPlane (), *GetPolyPlane ());
}


//---------------------------------------------------------------------------

csPolygonBspContainer::csPolygonBspContainer ()
{
  max_polygon = 10;
  CHK (polygons = new csPolygonInt* [max_polygon]);
  num_polygon = 0;
}

csPolygonBspContainer::~csPolygonBspContainer ()
{
  if (polygons)
  {
    for (int i = 0 ; i < num_polygon ; i++)
      CHKB (delete (csPolygonBsp*)polygons [i]);
    CHK (delete [] polygons);
  }
}

void csPolygonBspContainer::AddPolygon (csPolygonInt* poly)
{
  if (num_polygon >= max_polygon)
  {
    max_polygon += 10;
    CHK (csPolygonInt** new_polygons = new csPolygonInt* [max_polygon]);
    memcpy (new_polygons, polygons, sizeof (csPolygonInt*)*num_polygon);
    CHK (delete [] polygons);
    polygons = new_polygons;
  }

  polygons[num_polygon++] = poly;
}

//---------------------------------------------------------------------------
