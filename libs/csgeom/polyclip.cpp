/*
    Crystal Space polygon clipping routines
    Copyright (C) 1998-2000 by Jorrit Tyberghein
    Contributed by Ivan Avramovic <ivan@avramovic.com> and
                   Andrew Zabolotny <bit@eltech.ru>

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
#include "cssysdef.h"
#include "csgeom/polyclip.h"
#include "csgeom/poly2d.h"
#include "csgeom/box.h"

//---------------------------------------------------------------------------
CS_IMPLEMENT_STATIC_VAR (GetPolyPool, csPoly2DPool,
	(csPoly2DFactory::SharedFactory()))
csPoly2DPool *csClipper::polypool = 0;

SCF_IMPLEMENT_IBASE(csClipper)
  SCF_IMPLEMENTS_INTERFACE(iClipper2D)
SCF_IMPLEMENT_IBASE_END

csClipper::csClipper ()
{
  SCF_CONSTRUCT_IBASE (0);
  polypool = GetPolyPool ();
}

csClipper::~csClipper ()
{
  SCF_DESTRUCT_IBASE ();
}

csPoly2DPool *csClipper::GetSharedPool ()
{
  polypool = GetPolyPool ();
  return polypool;
}

uint8 csClipper::ClipInPlace (
  csVector2 *InPolygon,
  int &InOutCount,
  csBox2 &BoundingBox)
{
  csVector2 TempPoly[MAX_OUTPUT_VERTICES];
  uint8 rc = Clip (InPolygon, InOutCount, TempPoly, InOutCount, BoundingBox);
  if (rc != CS_CLIP_OUTSIDE)
    memcpy (InPolygon, TempPoly, InOutCount * sizeof (csVector2));
  return rc;
}

//---------------------------------------------------------------------------
int csBoxClipper::ClassifyBox (const csBox2 &box)
{
  if (!region.Overlap (box)) return -1;
  if (
    box.MinX () >= region.MinX () &&
    box.MaxX () <= region.MaxX () &&
    box.MinY () >= region.MinY () &&
    box.MaxY () <= region.MaxY ())
    return 1;
  return 0;
}

uint8 csBoxClipper::Clip (
  csVector2 *InPolygon,
  int InCount,
  csVector2 *OutPolygon,
  int &OutCount)
{
#include "boxclip.inc"
  OutCount = OutV;
  return Clipped ? CS_CLIP_CLIPPED : CS_CLIP_INSIDE;
}

uint8 csBoxClipper::Clip (
  csVector2 *InPolygon,
  int InCount,
  csVector2 *OutPolygon,
  int &OutCount,
  csVertexStatus *OutStatus)
{
#define OUTPUT_VERTEX_STATUS
#include "boxclip.inc"
  OutCount = OutV;
  return Clipped ? CS_CLIP_CLIPPED : CS_CLIP_INSIDE;
}

uint8 csBoxClipper::Clip (
  csVector2 *InPolygon,
  int InCount,
  csVector2 *OutPolygon,
  int &OutCount,
  csBox2 &BoundingBox)
{
  if (!region.Overlap (BoundingBox)) return false;

#define BOXCLIP_HAVEBOX
#include "boxclip.inc"
  OutCount = OutV;
  BoundingBox.StartBoundingBox (OutPolygon[0]);

  int i;
  for (i = 1; i < OutCount; i++)
    BoundingBox.AddBoundingVertexSmart (OutPolygon[i]);
  return Clipped ? CS_CLIP_CLIPPED : CS_CLIP_INSIDE;
}

//---------------------------------------------------------------------------
csPolygonClipper::csPolygonClipper (
  csPoly2D *Clipper,
  bool mirror,
  bool copy) :
    csClipper()
{
  int Count = Clipper->GetVertexCount ();
  ClipPolyVertices = Count;

  if (mirror || copy)
  {
    ClipPoly2D = polypool->Alloc ();
    ClipPoly2D->MakeRoom (Count * 2);
    ClipPoly = ClipPoly2D->GetVertices ();
    ClipData = ClipPoly + Count;

    int vert;
    if (mirror)
      for (vert = 0; vert < Count; vert++)
        ClipPoly[Count - 1 - vert] = (*Clipper)[vert];
    else
      for (vert = 0; vert < Count; vert++)
        ClipPoly[vert] = (*Clipper)[vert];
  }
  else
  {
    ClipPoly2D = 0;
    ClipPoly = Clipper->GetVertices ();
    ClipData = new csVector2[ClipPolyVertices];
  }

  Prepare ();
}

csPolygonClipper::csPolygonClipper (
  csVector2 *Clipper,
  int Count,
  bool mirror,
  bool copy) :
    csClipper()
{
  ClipPolyVertices = Count;

  if (mirror || copy)
  {
    ClipPoly2D = polypool->Alloc ();
    ClipPoly2D->MakeRoom (Count * 2);
    ClipPoly = ClipPoly2D->GetVertices ();
    ClipData = ClipPoly + Count;

    int vert;
    if (mirror)
      for (vert = 0; vert < Count; vert++)
        ClipPoly[Count - 1 - vert] = Clipper[vert];
    else
      for (vert = 0; vert < Count; vert++) ClipPoly[vert] = Clipper[vert];
  }
  else
  {
    ClipPoly2D = 0;
    ClipPoly = Clipper;
    ClipData = new csVector2[ClipPolyVertices];
  }

  Prepare ();
}

csPolygonClipper::~csPolygonClipper ()
{
  if (ClipPoly2D)
    polypool->Free (ClipPoly2D);
  else
    delete[] ClipData;
}

void csPolygonClipper::Prepare ()
{
  // Precompute some data for each clipping edge
  int vert;
  ClipBox.StartBoundingBox (ClipPoly[0]);
  for (vert = 0; vert < ClipPolyVertices; vert++)
  {
    size_t next = (size_t)(vert == ClipPolyVertices - 1 ? 0 : vert + 1);
    ClipData[vert].x = ClipPoly[next].x - ClipPoly[vert].x;
    ClipData[vert].y = ClipPoly[next].y - ClipPoly[vert].y;
    if (vert) ClipBox.AddBoundingVertex (ClipPoly[vert]);
  }
}

int csPolygonClipper::ClassifyBox (const csBox2 &box)
{
  if (!ClipBox.Overlap (box)) return -1;
  if (!IsInside (box.GetCorner (0))) return 0;
  if (!IsInside (box.GetCorner (1))) return 0;
  if (!IsInside (box.GetCorner (2))) return 0;
  if (!IsInside (box.GetCorner (3))) return 0;
  return 1;
}

bool csPolygonClipper::IsInside (const csVector2 &v)
{
  // Quick test
  if (!ClipBox.In (v.x, v.y)) return false;

  // Detailed test
  int vert;
  for (vert = 0; vert < ClipPolyVertices; vert++)
    if ((v.x - ClipPoly[vert].x) *
          ClipData[vert].y -
          (v.y - ClipPoly[vert].y) *
          ClipData[vert].x < 0)
      return false;
  return true;
}

uint8 csPolygonClipper::Clip (
  csVector2 *InPolygon,
  int InCount,
  csVector2 *OutPolygon,
  int &OutCount)
{
#include "polyclip.inc"
  OutCount = OutV;
  return Clipped ? CS_CLIP_CLIPPED : CS_CLIP_INSIDE;
}

uint8 csPolygonClipper::Clip (
  csVector2 *InPolygon,
  int InCount,
  csVector2 *OutPolygon,
  int &OutCount,
  csBox2 &BoundingBox)
{
  if (!ClipBox.Overlap (BoundingBox)) return false;

  uint8 rc = Clip (InPolygon, InCount, OutPolygon, OutCount);
  if (rc != CS_CLIP_OUTSIDE)
  {
    BoundingBox.StartBoundingBox (OutPolygon[0]);

    int i;
    for (i = 1; i < OutCount; i++)
      BoundingBox.AddBoundingVertexSmart (OutPolygon[i]);
  }

  return rc;
}

uint8 csPolygonClipper::Clip (
  csVector2 *InPolygon,
  int InCount,
  csVector2 *OutPolygon,
  int &OutCount,
  csVertexStatus *OutStatus)
{
#define OUTPUT_VERTEX_STATUS
#include "polyclip.inc"
  OutCount = OutV;
  return Clipped ? CS_CLIP_CLIPPED : CS_CLIP_INSIDE;
}
