/*
   This is the well-known Southerland-Hodgemen polygon clipping algorythm
   which works by clipping target polygon against every clipper polygon's
   edge.

   This clipping algorythm does not care about the direction in which polygon
   is described (clockwise or anti-clockwise). The clipping polygon should be
   convex while the clipped polygon is arbitrary.
 */

/*
   This included file is used twice to form two differently suited clipping
   routines: one for sprites and one for portals. It expects OUTPUT_VERTEX
   macro to contain all the code needed to output a vertex and perform other
   tasks such as enlarging bounding box.
   All initialization and finalization routines should be placed before and
   after including this file.
 */
#ifndef USE_POLYCLIP_OPTIM
#define USE_POLYCLIP_OPTIM
#endif

  csVector2 OutPoly1 [100];             // Two temporary storage arrays
  csVector2 OutPoly2 [100];             //    for intermediate polygons
  csVector2 *InPolygon = Polygon;       // The currently clipped polygon
  int InVertices = Count;               // Number of input vertices
  csVector2 *OutPolygon = OutPoly1;     // The current output polygon
  int OutVertices = 0;                  // Number of output vertices
  int edge, vert;                       // Current clipper edge & input vertex

  for (edge = 0; edge < ClipPolyVertices; edge++)
  {
    // Is this the last vertex being considered?
    bool LastVertex = false;
    // First ("previous") point x/y
    float px = InPolygon [0].x, py = InPolygon [0].y;
    // The "inside/outside polygon" flag for previous vertex
    bool prevVertexInside =
      (px - ClipPoly [edge].x) * ClipData [edge].dy -
      (py - ClipPoly [edge].y) * ClipData [edge].dx > 0;

    // Empty output polygon
    OutVertices = 0;
    // A convex polygon cannot be intersected by a line more than twice
    int IntersectionCount = 0;
    for (vert = 1; vert <= InVertices; vert++)
    {
      // second ("current") point x,y
      float cx, cy;
      if (vert < InVertices)
      {
        cx = InPolygon [vert].x;
        cy = InPolygon [vert].y;
      } else
      {
        cx = InPolygon [0].x;
        cy = InPolygon [0].y;
        LastVertex = true;
      } /* endif */

      // If starting vertex is visible, put it into output array
      if (prevVertexInside)
        if ( OutVertices == 0 ||
             ( ABS(px-OutPolygon[OutVertices-1].x) > EPSILON ||
               ABS(py-OutPolygon[OutVertices-1].y) > EPSILON ) &&
             ( !LastVertex || ABS(px-OutPolygon[0].x) > EPSILON ||
                              ABS(py-OutPolygon[0].y) > EPSILON ) )
      {
        OutPolygon [OutVertices].x = px;
        OutPolygon [OutVertices].y = py;
        OutVertices++;
      } /* endif */

      // The "inside/outside polygon" flag for current vertex
      bool curVertexInside =
        (cx - ClipPoly [edge].x) * ClipData [edge].dy -
        (cy - ClipPoly [edge].y) * ClipData [edge].dx > 0;

      // If vertices are on different sides of edge,
      // look where we're intersecting
      if (prevVertexInside != curVertexInside)
      {
        // Check if and where edges intersects
        // t = - (n * (a - c)) / (n * (b - a))
        double denominator =
          (cx - px) * ClipData [edge].dy - (cy - py) * ClipData [edge].dx;

        if (denominator)
        {
          double t = ((py - ClipPoly [edge].y) * ClipData [edge].dx
                    - (px - ClipPoly [edge].x) * ClipData [edge].dy)
                    / denominator;
          float tx, ty;

          if (t <= 0) { tx = px;  ty = py; }
          else if (t >= 1) {tx = cx;  ty = cy; }
          else
          {
            tx = px + t * (cx - px);
            ty = py + t * (cy - py);
          }

          if ( OutVertices == 0 ||
               ( ABS(tx-OutPolygon[OutVertices-1].x) > EPSILON ||
                 ABS(ty-OutPolygon[OutVertices-1].y) > EPSILON ) &&
               ( !LastVertex || ABS(tx-OutPolygon[0].x) > EPSILON ||
                                ABS(ty-OutPolygon[0].y) > EPSILON ) )
          {
            OutPolygon [OutVertices].x = tx;
            OutPolygon [OutVertices].y = ty;
            OutVertices++;
          }

        } /* endif */
        IntersectionCount++;
#ifdef USE_POLYCLIP_OPTIM
        if (IntersectionCount >= 2)
        {
          if (ABS(denominator) < 1) IntersectionCount=0;
          else
          {
            // Drop out, after adding all vertices left in input polygon
            if (curVertexInside && !LastVertex)
            {
              if (ABS(InPolygon[vert].x-OutPolygon[OutVertices-1].x) < EPSILON 
               && ABS(InPolygon[vert].y-OutPolygon[OutVertices-1].y) < EPSILON) vert++;

              memcpy (&OutPolygon [OutVertices], &InPolygon [vert],
                (InVertices - vert) * sizeof (OutPolygon[0]));
              OutVertices += InVertices - vert;
            } /* endif */
            break; 
          }
        } /* endif */
#endif
      } /* endif */

      px = cx; py = cy;
      prevVertexInside = curVertexInside;
    } /* endfor */

    // If polygon is wiped out, break
    if (OutVertices < 3)
      break;

    // Switch input/output polys
    InVertices = OutVertices;
    InPolygon = OutPolygon;
    if (OutPolygon == OutPoly2)
      OutPolygon = OutPoly1;
    else
      OutPolygon = OutPoly2;
  } /* endfor */


