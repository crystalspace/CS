/*
    Crystal Space Windowing System: Polygon drawing support
    Copyright (C) 2000 by Andrew Zabolotny <bit@eltech.ru>

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
#include "csws/cscomp.h"
#include "csws/csapp.h"
#include "csgeom/polyclip.h"
#include "csqint.h"

/*
 * Since polygon drawing in windowing system is kind of complicated, it is
 * separated into this file, to clobber less cscomp.cpp. Polygon clipping
 * is implemented as usual (e.g. for lines, boxes etc) - the only difference
 * is that we should adjust per-vertex data (r/g/b/u/v etc) after each
 * clipping. Also since this is the only place in CSWS where we use polygon
 * clipper from csgeom if you don't use this function, you won't get
 * the clipper linked into your application.
 */

#define INTERPOLATE1_S(var) \
  poly.var [j] = orig_poly_##var [vt] + \
    t * (orig_poly_##var [vt2] - orig_poly_ ##var [vt]);

#define INTERPOLATE1(var,component) \
  poly.var [j].component = orig_poly_##var [vt].component + \
    t * (orig_poly_##var [vt2].component - orig_poly_##var [vt].component);

#define INTERPOLATE_S(var) \
{ \
  float v1 = orig_poly_##var [edge_from [0]] + \
    t1 * (orig_poly_##var [edge_to [0]] - orig_poly_##var [edge_from [0]]); \
  float v2 = orig_poly_##var [edge_from [1]] + \
    t2 * (orig_poly_##var [edge_to [1]] - orig_poly_##var [edge_from [1]]); \
  poly.var [j] = v1 + t * (v2 - v1); \
}

#define INTERPOLATE(var,component) \
{ \
  float v1 = orig_poly_##var [edge_from [0]].component + \
    t1 * (orig_poly_##var [edge_to [0]].component - orig_poly_##var [edge_from [0]].component); \
  float v2 = orig_poly_##var [edge_from [1]].component + \
    t2 * (orig_poly_##var [edge_to [1]].component - orig_poly_##var [edge_from [1]].component); \
  poly.var [j].component = v1 + t * (v2 - v1); \
}

//@@@REIMPLEMENT THIS FOR NR
#if 0
void csComponent::Polygon3D (G3DPolygonDPFX &poly, uint mode)
{
 /* Do clipping as follows: create a minimal rectangle which fits the polygon,
  * clip the rectangle against children & parents, then clip the poly against
  * all resulting rectangles.
  */
  cswsRectVector rect (8, 4);
  int x = csQint (poly.vertices[0].x), y = csQint (poly.vertices[0].y);
  int p;
  csRect *lb = new csRect (x, y, x, y);
  for (p = 1; p < poly.num; p++)
    lb->Extend (csQint (poly.vertices[p].x), csQint (poly.vertices[p].y));

  lb->xmax++;
  lb->ymax++;
  if (!clip.IsEmpty ())
    lb->Intersect (clip);
  rect.Push (lb);
  FastClip (rect);

  int dx = 0; int dy = 0;
  LocalToGlobal (dx, dy);

  // Store the original polygon aside (for clipping)
  int orig_num_vert = poly.num;
  CS_ALLOC_STACK_ARRAY (csVector2, orig_vert, orig_num_vert);
  CS_ALLOC_STACK_ARRAY (csVector2, orig_poly_vertices, orig_num_vert);
  CS_ALLOC_STACK_ARRAY (csVector2, orig_poly_texels, orig_num_vert);
  CS_ALLOC_STACK_ARRAY (csColor, orig_poly_colors, orig_num_vert);
  CS_ALLOC_STACK_ARRAY (float, orig_poly_z, orig_num_vert);
  for (p = 0; p < orig_num_vert; p++)
  {
    orig_vert [p].x = dx + poly.vertices [p].x;
    orig_vert [p].y = dy + poly.vertices [p].y;
    orig_poly_vertices [p] = poly.vertices [p];
    orig_poly_texels [p] = poly.texels [p];
    orig_poly_colors [p] = poly.colors [p];
    orig_poly_z [p] = poly.z [p];
  }

  size_t i;
  for (i = rect.Length (); i-- > 0;)
  {
    csRect *cur = (csRect *)rect[i];

    // Create the clipper object
    csBoxClipper clipper (cur->xmin, cur->ymin, cur->xmax, cur->ymax);
    // The resulting clipped polygon
    csVector2 clipped_vert [MAX_OUTPUT_VERTICES];
    // Status of every output vertex
    csVertexStatus vstats [MAX_OUTPUT_VERTICES];
    // Number of vertices in clipped polygon
    int clipped_num_vert;
    // Okay, now do the actual clipping
    int rc = clipper.Clip (orig_vert, orig_num_vert, clipped_vert,
      clipped_num_vert, vstats);
    if (rc == CS_CLIP_OUTSIDE) continue;

    // Now process every polygon vertex and create the accompanying
    // values (r/g/b/u/v) according to status of every vertex.
    poly.num = clipped_num_vert;
    int j, vt, vt2;
    float t;
    for (j = 0; j < clipped_num_vert; j++)
    {
      poly.vertices [j].x = clipped_vert [j].x;
      // The Y coordinate is oriented upside
      poly.vertices [j].y = app->ScreenHeight - clipped_vert [j].y;
      if (rc != CS_CLIP_INSIDE)
        switch (vstats [j].Type)
        {
          case CS_VERTEX_ORIGINAL:
            vt = vstats  [j].Vertex;
            poly.z [j] = orig_poly_z [vt];
            poly.texels [j] = orig_poly_texels [vt];
            poly.colors [j] = orig_poly_colors [vt];
            break;
          case CS_VERTEX_ONEDGE:
            vt = vstats [j].Vertex;
            vt2 = vt + 1; if (vt2 >= orig_num_vert) vt2 = 0;
            t = vstats [j].Pos;
            INTERPOLATE1_S (z);
            INTERPOLATE1 (texels,x);
            INTERPOLATE1 (texels,y);
            if (!(mode & CS_FX_FLAT))
            {
              INTERPOLATE1 (colors,red);
              INTERPOLATE1 (colors,green);
              INTERPOLATE1 (colors,blue);
            }
            break;
          case CS_VERTEX_INSIDE:
            float x = clipped_vert [j].x;
            float y = clipped_vert [j].y;
            int edge_from [2], edge_to [2];
            int edge = 0;
            int et, ef;
            ef = orig_num_vert - 1;
            for (et = 0; et < orig_num_vert; et++)
            {
              if ((y >= orig_vert [et].y && y <= orig_vert [ef].y) ||
                  (y <= orig_vert [et].y && y >= orig_vert [ef].y))
              {
                edge_from [edge] = et;
                edge_to [edge] = ef;
                edge++;
                if (edge >= 2) break;
              }
              ef = et;
            }
            if (edge == 1)
            {
              // Safety if we only found one edge.
              edge_from [1] = edge_from [0];
              edge_to [1] = edge_to [0];
            }
            csVector2& A = orig_vert [edge_from [0]];
            csVector2& B = orig_vert [edge_to [0]];
            csVector2& C = orig_vert [edge_from [1]];
            csVector2& D = orig_vert [edge_to [1]];
            float t1 = (y - A.y) / (B.y - A.y);
            float t2 = (y - C.y) / (D.y - C.y);
            float x1 = A.x + t1 * (B.x - A.x);
            float x2 = C.x + t2 * (D.x - C.x);
            t = (x - x1) / (x2 - x1);
            INTERPOLATE_S (z);
            INTERPOLATE (texels,x);
            INTERPOLATE (texels,y);
            if (!(mode & CS_FX_FLAT))
            {
              INTERPOLATE (colors,red);
              INTERPOLATE (colors,green);
              INTERPOLATE (colors,blue);
            }
            break;
        }
    }

    app->pplPolygon3D (poly, mode);
  }
}
#endif

void csComponent::ClearZbuffer (int x1, int y1, int x2, int y2)
{
  cswsRectVector rect (8, 4);
  csRect *lb = new csRect (x1, y1, x2, y2);
  if (!clip.IsEmpty ())
    lb->Intersect (clip);
  rect.Push (lb);
  FastClip (rect);

  size_t i = rect.Length ();
  while (i > 0)
  {
    i--;
    csRect *cur = (csRect *)rect[i];
    app->pplClearZbuffer (cur->xmin, cur->ymin, cur->xmax, cur->ymax);
  }
}
