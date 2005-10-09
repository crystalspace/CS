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
#include "csgeom/box.h"
#include "csgeom/poly2d.h"
#include "csgeom/polyclip.h"
#include "csgeom/polypool.h"

#include "csutil/sysfunc.h"

//#define CLIP_DEBUG

#ifdef CLIP_DEBUG
#define CLIP_PRINTF csPrintf
#else
#define CLIP_PRINTF while(0) csPrintf
#endif

//---------------------------------------------------------------------------
CS_IMPLEMENT_STATIC_VAR (GetPolyPool, csPoly2DPool,
	(csPoly2DFactory::SharedFactory()))
csPoly2DPool *csClipper::polypool = 0;

csClipper::csClipper () : scfImplementationType(this)
{
  polypool = GetPolyPool ();
}

csClipper::~csClipper () { } 

csPoly2DPool *csClipper::GetSharedPool ()
{
  polypool = GetPolyPool ();
  return polypool;
}

uint8 csClipper::ClipInPlace (
  csVector2 *InPolygon,
  size_t &InOutCount,
  csBox2 &BoundingBox)
{
  csVector2 TempPoly[MAX_OUTPUT_VERTICES];
  uint8 rc = Clip (InPolygon, InOutCount, TempPoly, InOutCount, BoundingBox);
  if (rc != CS_CLIP_OUTSIDE)
    memcpy (InPolygon, TempPoly, InOutCount * sizeof (csVector2));
  return rc;
}

//---------------------------------------------------------------------------

struct StatusOutputNone
{
  void Flip() {}

  unsigned char GetType (size_t /*vert*/) const { return ~0; }
  float GetPos (size_t /*vert*/) const { return 0; }
  size_t GetVert (size_t /*vert*/) const { return 0; }

  void Copy (size_t /*to*/, size_t /*from*/) {}
  void OnEdge (size_t /*to*/, size_t /*vertex*/, float /*Pos*/) {}
  void Inside (size_t /*to*/) {}

  void Copy (size_t /*outOffs*/, size_t /*count*/, size_t /*inOffs*/) {}
};

struct StatusOutputDefault
{
  csVertexStatus* TempStatus;

  csVertexStatus* OutStatus;
  csVertexStatus *InS;
  csVertexStatus *OutS;

  StatusOutputDefault (csVertexStatus* TempStatus, size_t InCount, 
    csVertexStatus* OutStatus) :
    TempStatus (TempStatus), OutStatus (OutStatus)
  {
    InS = OutStatus;
    OutS = TempStatus;

    for (size_t vs = 0; vs < InCount; vs++)
    {
      OutS[vs].Type = InS[vs].Type = CS_VERTEX_ORIGINAL;
      OutS[vs].Vertex = InS[vs].Vertex = vs;
    }
  }

  void Flip()
  {
    InS = OutS;
    OutS = (OutS == TempStatus) ? OutStatus : TempStatus;
  }

  unsigned char GetType (size_t vert) const 
  { return InS[vert].Type; }
  float GetPos (size_t vert) const 
  { return InS[vert].Pos; }
  size_t GetVert (size_t vert) const 
  { return InS[vert].Vertex; }

  void Copy (size_t to, size_t from) 
  { 
    CS_ASSERT((InS[from].Type == CS_VERTEX_ORIGINAL) 
      || (InS[from].Type == CS_VERTEX_ONEDGE)
      || (InS[from].Type == CS_VERTEX_INSIDE));
    OutS[to] = InS[from];
    CLIP_PRINTF ("%zu: copy %zu\n", to, from);
  }
  void OnEdge (size_t to, size_t vertex, float Pos) 
  {
    OutS[to].Type = CS_VERTEX_ONEDGE;
    OutS[to].Vertex = InS[vertex].Vertex;
    OutS[to].Pos = Pos;
    CLIP_PRINTF ("%zu: edge %zu(%zu) %g\n", to, vertex, InS[vertex].Vertex, Pos);
  }
  void Inside (size_t to) 
  {
    OutS[to].Type = CS_VERTEX_INSIDE;
    CLIP_PRINTF ("%zu: inside\n", to);
  }

  void Copy (size_t outOffs, size_t count, size_t inOffs)
  {
    for (size_t vs = 0; vs < count; vs++)
    {
      CS_ASSERT ((vs+outOffs) >= 0 && (vs+outOffs) < MAX_OUTPUT_VERTICES);
      OutS [vs + outOffs] = InS [inOffs + vs];
      CLIP_PRINTF ("%zu: copy %zu\n", vs + outOffs, inOffs + vs);
    }
  }
};

//---------------------------------------------------------------------------

#include "boxclip.h"

struct BoxTestAll
{
  bool ClipMinX() const { return true; }
  bool ClipMaxX() const { return true; }
  bool ClipMinY() const { return true; }
  bool ClipMaxY() const { return true; }
  size_t GetClipCount() const { return 0; }
};

struct BoxTestBbox
{
  uint8 ClipEdges;
  size_t ClipCount;

  BoxTestBbox (const csBox2& BoundingBox,
    const csBox2& region) : ClipEdges(0), ClipCount (0)
  {
    // Do we need to clip against left x line?
    if (BoundingBox.MinX () < region.MinX ())
      ClipEdges |= 1, ClipCount++;
    // Do we need to clip against right x line?
    if (BoundingBox.MaxX () > region.MaxX ())
      ClipEdges |= 2, ClipCount++;
    // Do we need to clip against bottom y line?
    if (BoundingBox.MinY () < region.MinY ())
      ClipEdges |= 4, ClipCount++;
    // Do we need to clip against top y line?
    if (BoundingBox.MaxY () > region.MaxY ())
      ClipEdges |= 8, ClipCount++;
  }

  bool ClipMinX() const { return ClipEdges & 1; }
  bool ClipMaxX() const { return ClipEdges & 2; }
  bool ClipMinY() const { return ClipEdges & 4; }
  bool ClipMaxY() const { return ClipEdges & 8; }
  size_t GetClipCount() const { return ClipCount; }
};

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
  size_t InCount,
  csVector2 *OutPolygon,
  size_t &OutCount)
{
  CrystalSpace::BoxClipper<BoxTestAll, StatusOutputNone> boxClip
    (BoxTestAll(), StatusOutputNone(), region,
    InPolygon, InCount, OutPolygon);

  uint8 Clipped = boxClip.Clip();
  OutCount = boxClip.GetOutputCount();
  return Clipped;
}

uint8 csBoxClipper::Clip (
  csVector2 *InPolygon,
  size_t InCount,
  csVector2 *OutPolygon,
  size_t &OutCount,
  csVertexStatus *OutStatus)
{
  csVertexStatus TempStatus [MAX_OUTPUT_VERTICES];

  CrystalSpace::BoxClipper<BoxTestAll, StatusOutputDefault> boxClip
    (BoxTestAll(), StatusOutputDefault (TempStatus, InCount, OutStatus), 
    region, InPolygon, InCount, OutPolygon);

  uint8 Clipped = boxClip.Clip();
  OutCount = boxClip.GetOutputCount();
  return Clipped;
}

uint8 csBoxClipper::Clip (
  csVector2 *InPolygon,
  size_t InCount,
  csVector2 *OutPolygon,
  size_t &OutCount,
  csBox2 &BoundingBox)
{
  if (!region.Overlap (BoundingBox)) return false;

  CrystalSpace::BoxClipper<BoxTestBbox, StatusOutputNone> boxClip
    (BoxTestBbox (BoundingBox, region), StatusOutputNone(), region,
    InPolygon, InCount, OutPolygon);

  uint8 Clipped = boxClip.Clip();
  OutCount = boxClip.GetOutputCount();

  BoundingBox.StartBoundingBox (OutPolygon[0]);
  size_t i;
  for (i = 1; i < OutCount; i++)
    BoundingBox.AddBoundingVertexSmart (OutPolygon[i]);
  return Clipped;
}

//---------------------------------------------------------------------------

#include "polyclip.h"

csPolygonClipper::csPolygonClipper (
  csPoly2D *Clipper,
  bool mirror,
  bool copy) :
    csClipper()
{
  size_t Count = Clipper->GetVertexCount ();
  ClipPolyVertices = Count;

  if (mirror || copy)
  {
    ClipPoly2D = polypool->Alloc ();
    ClipPoly2D->MakeRoom (Count * 2);
    ClipPoly = ClipPoly2D->GetVertices ();
    ClipData = ClipPoly + Count;

    size_t vert;
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
  size_t Count,
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

    size_t vert;
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
  size_t vert;
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
  size_t vert;
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
  size_t InCount,
  csVector2 *OutPolygon,
  size_t &OutCount)
{
  CrystalSpace::PolyClipper<StatusOutputNone> polyClip
    (StatusOutputNone(), InPolygon, InCount, OutPolygon,
    ClipPolyVertices, ClipPoly, ClipData);

  uint8 Clipped = polyClip.Clip();
  OutCount = polyClip.GetOutputCount();
  return Clipped;
}

uint8 csPolygonClipper::Clip (
  csVector2 *InPolygon,
  size_t InCount,
  csVector2 *OutPolygon,
  size_t &OutCount,
  csBox2 &BoundingBox)
{
  if (!ClipBox.Overlap (BoundingBox)) return false;

  uint8 rc = Clip (InPolygon, InCount, OutPolygon, OutCount);
  if (rc != CS_CLIP_OUTSIDE)
  {
    BoundingBox.StartBoundingBox (OutPolygon[0]);

    size_t i;
    for (i = 1; i < OutCount; i++)
      BoundingBox.AddBoundingVertexSmart (OutPolygon[i]);
  }

  return rc;
}

uint8 csPolygonClipper::Clip (
  csVector2 *InPolygon,
  size_t InCount,
  csVector2 *OutPolygon,
  size_t &OutCount,
  csVertexStatus *OutStatus)
{
  csVertexStatus TempStatus [MAX_OUTPUT_VERTICES];

  CrystalSpace::PolyClipper<StatusOutputDefault> polyClip
    (StatusOutputDefault (TempStatus, InCount, OutStatus), 
    InPolygon, InCount, OutPolygon,
    ClipPolyVertices, ClipPoly, ClipData);

  uint8 Clipped = polyClip.Clip();
  OutCount = polyClip.GetOutputCount();
  return Clipped;
}
