/*
    Crystal Space polygon clipping routines
    Copyright (C) 1998 by Jorrit Tyberghein
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

#include "sysdef.h"
#include "csgeom/polyclip.h"

//---------------------------------------------------------------------------

bool csBoxClipper::Clip (csVector2 *Polygon, csVector2* dest_poly, int Count, int &OutCount)
{
  memcpy (dest_poly, Polygon, Count * sizeof (csVector2));
  OutCount = Count;
  csBox bbox = csBox(-100000., -100000., 100000., 100000.);
  if (Clip(dest_poly, OutCount, Count+5, &bbox)) return true;
   else
   {
    OutCount = 0;
    return false;
   }
}

bool csBoxClipper::Clip (csVector2 *Polygon, int& Count, int MaxCount, 
                       csBox *BoundingBox)
{
  csVector2 OutPoly[100];
  csVector2 *InPolygon = Polygon;
  int InVertices = Count;
  csVector2 *OutPolygon = OutPoly;
  int OutVertices = Count;
  int vert; 

  if (!region.Overlap (*BoundingBox)) return false;

  
  if (BoundingBox->MinX () < region.MinX ()) /* clip against left x line */
  {
    float px = InPolygon[0].x, py = InPolygon[0].y;
    bool pVertIn = (px > region.MinX ());
    OutVertices = 0;
    int isectCount = 0;
    for (vert = 1; vert <= InVertices; vert++)
    {
     float cx, cy;
     if (vert < InVertices) 
     { cx = InPolygon[vert].x;  cy = InPolygon[vert].y; }
     else { cx = InPolygon[0].x;  cy = InPolygon[0].y; }
     bool cVertIn = (cx > region.MinX ());
     
     if (pVertIn)
     {
      OutPolygon[OutVertices].x = px;  OutPolygon[OutVertices].y = py;
      OutVertices++;
      if (OutVertices == MaxCount) break;
     }   
     if (pVertIn != cVertIn)
     {
      OutPolygon[OutVertices].x = region.MinX ();
      OutPolygon[OutVertices].y = cy - (cx-region.MinX ())*(cy-py)/(cx-px);
      OutVertices++;  
      if (OutVertices == MaxCount) break;
      isectCount++;
      if (isectCount >= 2)
      {
       if (cVertIn)
       {
        memcpy (&OutPolygon[OutVertices], &InPolygon[vert],
                (InVertices - vert) * sizeof(OutPolygon[0]));
        OutVertices += InVertices - vert;
       }
       break;
      }
     }
     px = cx; py = cy;  pVertIn = cVertIn; 
    } /* for (vert = 1; vert <= InVertices; vert++) */

    if (OutVertices < 3) { Count = 0; return false; }
    InVertices = OutVertices;
    InPolygon = OutPolygon;
    if (OutPolygon == OutPoly) OutPolygon = Polygon;
     else OutPolygon = OutPoly;
  }
  if (BoundingBox->MinY () < region.MinY ()) /* clip against upper y line */
  {
    float px = InPolygon[0].x, py = InPolygon[0].y;
    bool pVertIn = (py > region.MinY ());
    OutVertices = 0;
    int isectCount = 0;
    for (vert = 1; vert <= InVertices; vert++)
    {
     float cx, cy;
     if (vert < InVertices) 
     { cx = InPolygon[vert].x;  cy = InPolygon[vert].y; }
     else { cx = InPolygon[0].x;  cy = InPolygon[0].y; }
     bool cVertIn = (cy > region.MinY ());
     
     if (pVertIn)
     {
      OutPolygon[OutVertices].x = px;  OutPolygon[OutVertices].y = py;
      OutVertices++;
      if (OutVertices == MaxCount) break;
     }   
     if (pVertIn != cVertIn)
     {
      OutPolygon[OutVertices].x = cx - (cy-region.MinY ())*(cx-px)/(cy-py);
      OutPolygon[OutVertices].y = region.MinY ();
      OutVertices++;  
      if (OutVertices == MaxCount) break;
      isectCount++;
      if (isectCount >= 2)
      {
       if (cVertIn)
       {
        memcpy (&OutPolygon[OutVertices], &InPolygon[vert],
                (InVertices - vert) * sizeof(OutPolygon[0]));
        OutVertices += InVertices - vert;
       }
       break;
      }
     }
     px = cx; py = cy;  pVertIn = cVertIn; 
    } /* for (vert = 1; vert <= InVertices; vert++) */

    if (OutVertices < 3) { Count = 0; return false; }
    InVertices = OutVertices;
    InPolygon = OutPolygon;
    if (OutPolygon == OutPoly) OutPolygon = Polygon;
     else OutPolygon = OutPoly;
  }
  if (BoundingBox->MaxX () > region.MaxX ()) /* clip against right x line */
  {
    float px = InPolygon[0].x, py = InPolygon[0].y;
    bool pVertIn = (px < region.MaxX ());
    OutVertices = 0;
    int isectCount = 0;
    for (vert = 1; vert <= InVertices; vert++)
    {
     float cx, cy;
     if (vert < InVertices) 
     { cx = InPolygon[vert].x;  cy = InPolygon[vert].y; }
     else { cx = InPolygon[0].x;  cy = InPolygon[0].y; }
     bool cVertIn = (cx < region.MaxX ());
     
     if (pVertIn)
     {
      OutPolygon[OutVertices].x = px;  OutPolygon[OutVertices].y = py;
      OutVertices++;
      if (OutVertices == MaxCount) break;
     }   
     if (pVertIn != cVertIn)
     {
      OutPolygon[OutVertices].x = region.MaxX ();
      OutPolygon[OutVertices].y = cy - (cx-region.MaxX ())*(cy-py)/(cx-px);
      OutVertices++;  
      if (OutVertices == MaxCount) break;
      isectCount++;
      if (isectCount >= 2)
      {
       if (cVertIn)
       {
        memcpy (&OutPolygon[OutVertices], &InPolygon[vert],
                (InVertices - vert) * sizeof(OutPolygon[0]));
        OutVertices += InVertices - vert;
       }
       break;
      }
     }
     px = cx; py = cy;  pVertIn = cVertIn; 
    } /* for (vert = 1; vert <= InVertices; vert++) */

    if (OutVertices < 3) { Count = 0; return false; }
    InVertices = OutVertices;
    InPolygon = OutPolygon;
    if (OutPolygon == OutPoly) OutPolygon = Polygon;
     else OutPolygon = OutPoly;
  }
  if (BoundingBox->MaxY () > region.MaxY ()) /* clip against lower y line */
  {
    float px = InPolygon[0].x, py = InPolygon[0].y;
    bool pVertIn = (py < region.MaxY ());
    OutVertices = 0;
    int isectCount = 0;
    for (vert = 1; vert <= InVertices; vert++)
    {
     float cx, cy;
     if (vert < InVertices) 
     { cx = InPolygon[vert].x;  cy = InPolygon[vert].y; }
     else { cx = InPolygon[0].x;  cy = InPolygon[0].y; }
     bool cVertIn = (cy < region.MaxY ());
     
     if (pVertIn)
     {
      OutPolygon[OutVertices].x = px;  OutPolygon[OutVertices].y = py;
      OutVertices++;
      if (OutVertices == MaxCount) break;
     }   
     if (pVertIn != cVertIn)
     {
      OutPolygon[OutVertices].x = cx - (cy-region.MaxY ())*(cx-px)/(cy-py);
      OutPolygon[OutVertices].y = region.MaxY ();
      OutVertices++;  
      if (OutVertices == MaxCount) break;
      isectCount++;
      if (isectCount >= 2)
      {
       if (cVertIn)
       {
        memcpy (&OutPolygon[OutVertices], &InPolygon[vert],
                (InVertices - vert) * sizeof(OutPolygon[0]));
        OutVertices += InVertices - vert;
       }
       break;
      }
     }
     px = cx; py = cy;  pVertIn = cVertIn; 
    } /* for (vert = 1; vert <= InVertices; vert++) */

    if (OutVertices < 3) { Count = 0; return false; }
    InVertices = OutVertices;
    InPolygon = OutPolygon;
    if (OutPolygon == OutPoly) OutPolygon = Polygon;
     else OutPolygon = OutPoly;
  }

  Count = OutVertices;
  if (Count < 3) {  Count=0; return false; }
  if (InPolygon != Polygon) 
    memcpy (Polygon, InPolygon, Count * sizeof (csVector2));
  BoundingBox->StartBoundingBox (*InPolygon++);
  while (--OutVertices)
    BoundingBox->AddBoundingVertexSmart (*InPolygon++);
  return true;
}

//---------------------------------------------------------------------------

csPolygonClipper::csPolygonClipper (csVector2 *Clipper, int Count, bool mirror,
  bool copy)
{
  int vert;

  ClipPolyIsACopy = mirror | copy;
  if (mirror)
  {
    CHK (ClipPoly = new csVector2 [Count]);
    for (vert = 0; vert < Count; vert++)
      ClipPoly [Count - 1 - vert] = Clipper [vert];
  } else if (copy)
  {
    CHK (ClipPoly = new csVector2 [Count]);
    for (vert = 0; vert < Count; vert++)
      ClipPoly [vert] = Clipper [vert];
  } else
    ClipPoly = Clipper;

  CHK (ClipData = new SegData [ClipPolyVertices = Count]);
  // Precompute some data for each clipping edge
  for (vert = 0; vert < ClipPolyVertices; vert++)
  {
    int next = (vert == ClipPolyVertices - 1 ? 0 : vert + 1);
    ClipData [vert].dx = ClipPoly [next].x - ClipPoly [vert].x;
    ClipData [vert].dy = ClipPoly [next].y - ClipPoly [vert].y;
  } /* endfor */

  // Initialize bounding box
  ClipBox.StartBoundingBox (ClipPoly [0]);
  for (vert = 1; vert < ClipPolyVertices; vert++)
    ClipBox.AddBoundingVertex (ClipPoly [vert]);
}

csPolygonClipper::~csPolygonClipper ()
{
  if (ClipData)
    CHKB (delete [] ClipData);
  if (ClipPolyIsACopy)
    CHKB (delete [] ClipPoly);
}

bool csPolygonClipper::IsInside (float x, float y)
{
  for (int vert = 0; vert < ClipPolyVertices; vert++)
    if (x * ClipData [vert].dy - y * ClipData [vert].dx < 0)
      return false;
  return true;
}

bool csPolygonClipper::Clip (csVector2 *Polygon, csVector2* dest_poly, int Count, int &OutCount)
{
#  include "polyclip.inc"

  if (OutVertices < 3)
  {
    OutCount = 0;
    return false;
  } else
  {
    memcpy (dest_poly, InPolygon, OutVertices * sizeof (csVector2));
    OutCount = OutVertices;
    return true;
  } /* endif */
}

bool csPolygonClipper::Clip (csVector2 *Polygon, int &Count, int MaxCount,
  csBox *BoundingBox)
{
  if (!ClipBox.Overlap (*BoundingBox))
    return false;

#  include "polyclip.inc"

  if (OutVertices < 3)
  {
    Count = 0;
    return false;
  } else
  {
    if (OutVertices > MaxCount)
      OutVertices = MaxCount;
    Count = OutVertices;
    memcpy (Polygon, InPolygon, Count * sizeof (csVector2));
    BoundingBox->StartBoundingBox (*InPolygon++);
    while (--OutVertices)
      BoundingBox->AddBoundingVertexSmart (*InPolygon++);
    return true;
  } /* endif */
}

