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
  bool prevVertexInside = BOXCLIP_INSIDE (px, py);

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
    if (LastVertex)
    {
      cx = InP [0].x;
      cy = InP [0].y;
    }
    else
    {
      cx = InP [vert].x;
      cy = InP [vert].y;
    } /* endif */

    // If starting vertex is visible, put it into output array
    if (prevVertexInside)
      if ((!OutV
        || fabs (px - OutP [OutV - 1].x) > EPSILON
        || fabs (py - OutP [OutV - 1].y) > EPSILON)
       && (!LastVertex
        || fabs (px - OutP [0].x) > EPSILON
        || fabs (py - OutP [0].y) > EPSILON))
    {
      OutP [OutV].x = px;
      OutP [OutV].y = py;
#ifdef OUTPUT_VERTEX_STATUS
      OutS [OutV] = InS [vert - 1];
#endif
      if (++OutV >= MAX_OUTPUT_VERTICES)
        break;
    } /* endif */

    // The "inside/outside polygon" flag for current vertex
    bool curVertexInside = BOXCLIP_INSIDE (cx, cy);

    // If vertices are on different sides of edge,
    // look where we're intersecting
    if (prevVertexInside != curVertexInside)
    {
      // Set the "input has been clipped" flag
      Clipped = true;

      // Check if and where edges intersects
      double t;
      float tx, ty;

      BOXCLIP_INTERSECT

      if ((!OutV
        || fabs (tx - OutP [OutV - 1].x) > EPSILON
        || fabs (ty - OutP [OutV - 1].y) > EPSILON)
       && (!LastVertex
        || fabs (tx - OutP [0].x) > EPSILON
        || fabs (ty - OutP [0].y) > EPSILON))
      {
        OutP [OutV].x = tx;
        OutP [OutV].y = ty;
#ifdef OUTPUT_VERTEX_STATUS
        if (InS [vert - 1].Type == CS_VERTEX_ORIGINAL)
        {
          OutS [OutV].Type = CS_VERTEX_ONEDGE;
          OutS [OutV].Pos = t;
          OutS [OutV].Vertex = InS [vert - 1].Vertex;
        }
        else
          OutS [OutV].Type = CS_VERTEX_INSIDE;
#endif
        if (++OutV >= MAX_OUTPUT_VERTICES)
          break;
      } /* endif */

      if (++IntersectionCount >= 2)
      {
        // Drop out, after adding all vertices left in input polygon
        if (curVertexInside && !LastVertex)
        {
          if (fabs (InP [vert].x - OutP [OutV - 1].x) < EPSILON
           && fabs (InP [vert].y - OutP [OutV - 1].y) < EPSILON)
            vert++;
          int count = InV - vert;
          if (OutV + count > MAX_OUTPUT_VERTICES)
            count = MAX_OUTPUT_VERTICES - OutV;
          memcpy (&OutP [OutV], &InP [vert], count * sizeof (OutP [0]));
#ifdef OUTPUT_VERTEX_STATUS
          for (int vs = 0; vs < count; vs++)
            OutS [vs + OutV] = InS [vert + vs];
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

#undef BOXCLIP_INSIDE
#undef BOXCLIP_INTERSECT
