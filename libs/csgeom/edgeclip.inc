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
    This include file contains the loop that clips the polygon pointed by
    InP to the output array pointed by OutP against a single edge of clipper
    polygon. This file is used both for clipping against a polygon
    (csPolygonClipper) and against a box (csBoxClipper). It expects several
    macros to be defined prior to including this file.
*/

{
  // First ("previous") point x/y
  float px = InP [0].x, py = InP [0].y;
  // The "inside/outside polygon" flag for previous vertex
  bool prevVertexInside = CLIP_INSIDE (px, py);

  // Empty output polygon
  OutV = 0;
  // A convex polygon cannot be intersected by a line more than twice
  int IntersectionCount = 0;
  for (vert = 1; vert <= InV; vert++)
  {
    // Is this the last vertex being considered?
    bool LastVertex = (vert == InV);
    // Second ("current") point x/y
    float cx, cy;
    realvert = LastVertex ? 0 : vert;
    CS_ASSERT (realvert < InV);
    cx = InP [realvert].x;
    cy = InP [realvert].y;

    // If starting vertex is visible, put it into output array
    if (prevVertexInside)
      if ((!OutV
        || ABS (px - OutP [OutV - 1].x) > EPSILON
        || ABS (py - OutP [OutV - 1].y) > EPSILON)
       && (!LastVertex
        || ABS (px - OutP [0].x) > EPSILON
        || ABS (py - OutP [0].y) > EPSILON))
    {
      CS_ASSERT (OutV >= 0 && OutV < MAX_OUTPUT_VERTICES);
      OutP [OutV].x = px;
      OutP [OutV].y = py;
#ifdef OUTPUT_VERTEX_STATUS
      CS_ASSERT ((vert-1) >= 0 && (vert-1) < InV);
      OutS [OutV] = InS [vert - 1];
#endif
      if (++OutV >= MAX_OUTPUT_VERTICES)
        break;
    } /* endif */

    // The "inside/outside polygon" flag for current vertex
    bool curVertexInside = CLIP_INSIDE (cx, cy);

    // If vertices are on different sides of edge,
    // look where we're intersecting
    if (prevVertexInside != curVertexInside)
    {
      // Set the "input has been clipped" flag
      Clipped = true;

      // Check if and where edges intersects
      double t;
      float tx, ty;

      CLIP_INTERSECT

      if ((!OutV
        || ABS (tx - OutP [OutV - 1].x) > EPSILON
        || ABS (ty - OutP [OutV - 1].y) > EPSILON)
       && (!LastVertex
        || ABS (tx - OutP [0].x) > EPSILON
        || ABS (ty - OutP [0].y) > EPSILON))
      {
        CS_ASSERT (OutV >= 0 && OutV < MAX_OUTPUT_VERTICES);
        OutP [OutV].x = tx;
        OutP [OutV].y = ty;
#ifdef OUTPUT_VERTEX_STATUS
        //
        // Four cases are possible here:
        //
        // (*) prev vertex: CS_VERTEX_ORIGINAL
        //     curr vertex: CS_VERTEX_ORIGINAL
        //       -> status_v = prev_vertex.index
        //       -> status_t = t
        //
        // (*) prev vertex: CS_VERTEX_ORIGINAL
        //     curr vertex: CS_VERTEX_ONEDGE
        //       -> status_v = prev_vertex.index
        //       -> status_t = t * curr_vertex.status_t
        //
        // (*) prev vertex: CS_VERTEX_ONEDGE
        //     curr vertex: CS_VERTEX_ORIGINAL
        //       -> status_v = prev_vertex.status_v
        //       -> status_t = prev_vertex.status_t + t * (1 - prev_vertex.status_t)
        //
        // (*) prev vertex: CS_VERTEX_ONEDGE
        //     curr vertex: CS_VERTEX_ONEDGE
        //       -> status_type = CS_VERTEX_INSIDE
        //
        if (InS [vert - 1].Type == CS_VERTEX_ORIGINAL)
          if (InS [realvert].Type == CS_VERTEX_ORIGINAL)
          {
            CS_ASSERT (OutV >= 0 && OutV < MAX_OUTPUT_VERTICES);
            OutS [OutV].Type = CS_VERTEX_ONEDGE;
            OutS [OutV].Vertex = InS [vert - 1].Vertex;
            OutS [OutV].Pos = t;
          }
          else
          {
            // Current vertex is on edge, it cannot be CS_VERTEX_INSIDE
            // because it is connected with at least one CS_VERTEX_ORIGINAL
            CS_ASSERT (OutV >= 0 && OutV < MAX_OUTPUT_VERTICES);
            OutS [OutV].Type = CS_VERTEX_ONEDGE;
            OutS [OutV].Vertex = InS [vert - 1].Vertex;
            OutS [OutV].Pos = t * InS [realvert].Pos;
          }
        else if (InS [vert - 1].Type == CS_VERTEX_ONEDGE)
          if (InS [realvert].Type == CS_VERTEX_ORIGINAL)
          {
            CS_ASSERT (OutV >= 0 && OutV < MAX_OUTPUT_VERTICES);
            OutS [OutV].Type = CS_VERTEX_ONEDGE;
            OutS [OutV].Vertex = InS [vert - 1].Vertex;
            OutS [OutV].Pos = InS [vert - 1].Pos + t * (1 - InS [vert - 1].Pos);
          }
          else
	  {
            CS_ASSERT (OutV >= 0 && OutV < MAX_OUTPUT_VERTICES);
            OutS [OutV].Type = CS_VERTEX_INSIDE;
	  }
        else
	{
          CS_ASSERT (OutV >= 0 && OutV < MAX_OUTPUT_VERTICES);
          OutS [OutV].Type = CS_VERTEX_INSIDE;
	}
#endif
        if (++OutV >= MAX_OUTPUT_VERTICES)
          break;
      } /* endif */

      if (++IntersectionCount >= 2)
      {
        // Drop out, after adding all vertices left in input polygon
        if (curVertexInside && !LastVertex)
        {
          if (ABS (InP [vert].x - OutP [OutV - 1].x) < EPSILON
           && ABS (InP [vert].y - OutP [OutV - 1].y) < EPSILON)
            vert++;
          size_t count = InV - vert;
          if (OutV + count > MAX_OUTPUT_VERTICES)
            count = MAX_OUTPUT_VERTICES - OutV;
          memcpy (&OutP [OutV], &InP [vert], count * sizeof (OutP [0]));
#ifdef OUTPUT_VERTEX_STATUS
          for (size_t vs = 0; vs < count; vs++)
	  {
            CS_ASSERT ((vs+OutV) >= 0 && (vs+OutV) < MAX_OUTPUT_VERTICES);
            OutS [vs + OutV] = InS [vert + vs];
	  }
#endif
          OutV += count;
          if (OutV >= MAX_OUTPUT_VERTICES)
            break;
        } /* endif */
        break;
      } /* endif */
    } /* endif */

    px = cx; py = cy;
    prevVertexInside = curVertexInside;
  } /* endfor */

  // If polygon is wiped out, break
  if (OutV < 3)
  {
    OutCount = 0;
    return CS_CLIP_OUTSIDE;
  }

  // Switch input/output polys: now we're going to clip
  // the polygon we just created against the next clipper edge
  InV = OutV;
  InP = OutP;
  if (OutP == TempPoly)
    OutP = OutPolygon;
  else
    OutP = TempPoly;
#ifdef OUTPUT_VERTEX_STATUS
  InS = OutS;
  if (OutS == TempStatus)
    OutS = OutStatus;
  else
    OutS = TempStatus;
#endif
}

#undef CLIP_INSIDE
#undef CLIP_INTERSECT
