/*  -*- Mode: C++; -*- */
/*
    Crystal Space 3D engine
    Copyright (C) 2000 by Jorrit Tyberghein

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

/*
   This is the well-known Southerland-Hodgemen polygon clipping algorythm
   which works by successively clipping target polygon against every clipper
   polygon edge.

   In the following we call the "clipping" (or "clipper") polygon the polygon
   against which we're clipping, and "clipped" polygon the polygon that we
   are clipping against the clipper.

   The algorythm does not care about the vertex order of both polygons
   (clockwise or anti-clockwise). However, the clipping polygon should be
   convex while the clipped polygon can be arbitrary.
*/

  // Temporary storage for intermediate polygons
  csVector2 TempPoly [MAX_OUTPUT_VERTICES];
  // The currently clipped polygon
  csVector2 *InP = InPolygon;
  // Number of input vertices
  size_t InV = InCount;
  // The current output polygon
  csVector2 *OutP = (ClipPolyVertices & 1) ? OutPolygon : TempPoly;
  // Number of output vertices
  size_t OutV = 0;
  // Current clipper edge & input vertex & vertex % num_vertices
  size_t edge, vert, realvert;
  // Flag: was input polygon clipped?
  bool Clipped = false;

#ifdef OUTPUT_VERTEX_STATUS
  // Temporary storage for per-vertex status
  csVertexStatus TempStatus [MAX_OUTPUT_VERTICES];
  // A pointer to output array with vertex status
  csVertexStatus *InS, *OutS;
  if (ClipPolyVertices & 1)
    InS = TempStatus, OutS = OutStatus;
  else
    InS = OutStatus, OutS = TempStatus;
  // Fill the input vertex status array
  for (size_t vs = 0; vs < InCount; vs++)
  {
    InS [vs].Type = CS_VERTEX_ORIGINAL;
    InS [vs].Vertex = vs;
  }
#endif

#define CLIP_INSIDE(xx,yy)				\
  (xx - ClipPoly [edge].x) * ClipData [edge].y -	\
  (yy - ClipPoly [edge].y) * ClipData [edge].x >= 0;
#define CLIP_INTERSECT					\
  /* Check if and where edges intersects */		\
  /* t = - (n * (a - c)) / (n * (b - a)) */		\
  double denom = ((cx - px) * ClipData [edge].y -	\
                  (cy - py) * ClipData [edge].x);	\
  t = (denom == 0) ? 1.0 :				\
      ((py - ClipPoly [edge].y) * ClipData [edge].x	\
     - (px - ClipPoly [edge].x) * ClipData [edge].y)	\
     / denom;						\
							\
  if (t <= 0)						\
  { tx = px; ty = py; }					\
  else if (t >= 1)					\
  { tx = cx; ty = cy; }					\
  else							\
  {							\
    tx = px + t * (cx - px);				\
    ty = py + t * (cy - py);				\
  }

  for (edge = 0; edge < ClipPolyVertices; edge++)
#include "edgeclip.inc"

#undef OUTPUT_VERTEX_STATUS
