/*
    Copyright (C) 1999 by Denis Dmitriev
    Antialiased polygon splitting

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
#include <math.h>
#include "csqint.h"
#include "csgeom/vector2.h"
#include "csgeom/polyaa.h"

static float __calc_area (int n, csVector2 *p)
{
  if (n <= 1) return 0.0;

  float area = 0;
  int i = 1;
  do
  {
    area += (p[i - 1].y + p[i].y) * (p[i - 1].x - p[i].x);
    i++;
  } while (i < n);
  area += (p[n - 1].y + p[0].y) * (p[n - 1].x - p[0].x);

  return ABS (area / 2.0f);
}

// No doubt static vars are !!!EVIL!!! :-) but for
// the sake of performance's we still use them ...

CS_IMPLEMENT_STATIC_VAR(GetAAGrid, csRect, ())
static csAAPFCBPixel PutPixel;
static csAAPFCBBox DrawBox;
static void *Arg;

void __poly_fill (csVector2 *iverts, int ivertscount)
{
  // Calculate the complete are of visible rectangle.
  static csRect *Grid = GetAAGrid ();
  int height = Grid->Height ();
  int width = Grid->Width ();
  int visarea = width * height;

  // Sanity check (prevents infinite looping).
  if (visarea <= 0) return ;

  // Calculate the complete area of the polygon.
  float a = __calc_area (ivertscount, iverts);

  // Check if polygon is hollow.
  if (a < EPSILON)  // this area is hollow
    return ;

  // Check if current rectangle equals a single grid cell.
  if (height == 1 && width == 1)
  {
    PutPixel (Grid->xmin, Grid->ymin, a, Arg);
    return ;
  }

  // Check if polygon surface equals the visible rectangle surface.
  if (ABS (a - visarea) < EPSILON)
  {
    // This area is completely covered.
    if (DrawBox)
      DrawBox (Grid->xmin, Grid->ymin, width, height, Arg);
    else
    {
      int i, j;
      for (i = 0; i < height; i++)
      {
        for (j = 0; j < width; j++)
          PutPixel (Grid->xmin + j, Grid->ymin + i, 1.0, Arg);
      }
    }

    return ;
  }

  // Allocate space for both polygons that result after split.
  int n2[2] = { 0, 0 };
  csVector2 *p2[2];

  CS_ALLOC_STACK_ARRAY (csVector2, p2_0, ivertscount  + 1);
  p2[0] = p2_0;
  CS_ALLOC_STACK_ARRAY (csVector2, p2_1, ivertscount  + 1);
  p2[1] = p2_1;

  if (width > height)
  {
    // Split the polygon vertically by the line "x = sub_x"
    // (p2 [0] -- left poly, p2 [1] -- right poly).
    int sub_x = Grid->xmin + width / 2;
    int where_are_we = iverts[0].x > sub_x;
    p2[where_are_we][n2[where_are_we]++] = iverts[0];

    int v, prev;
    for (v = 1, prev = 0; v <= ivertscount; v++)
    {
      // Check whenever current vertex is on left or right side of divider
      int cur = (v == ivertscount) ? 0 : v;
      int now_we_are = iverts[cur].x > sub_x;

      if (now_we_are == where_are_we)
      {
        // Do not add the first point since it will be added at the end.
        if (cur) p2[where_are_we][n2[where_are_we]++] = iverts[cur];
      }
      else
      {
        // The most complex case: find the Y at intersection point.
        float y = iverts[prev].y + (iverts[cur].y - iverts[prev].y) *
          (sub_x - iverts[prev].x) / (iverts[cur].x - iverts[prev].x);

        // Add the intersection point to both polygons.
        p2[0][n2[0]++] = p2[1][n2[1]++] = csVector2 (sub_x, y);

        if (cur) p2[now_we_are][n2[now_we_are]++] = iverts[cur];
      }

      where_are_we = now_we_are;
      prev = cur;
    }

    int tmp = Grid->xmax;
    Grid->xmax = sub_x;
    __poly_fill (p2[0], n2[0]);
    Grid->xmax = tmp;

    tmp = Grid->xmin;
    Grid->xmin = sub_x;
    __poly_fill (p2[1], n2[1]);
    Grid->xmin = tmp;
  }
  else
  {
    // Split the polygon horizontally by the line "y = sub_y".
    // (p[0] -- top poly, p[1] -- bottom poly).
    int sub_y = Grid->ymin + height / 2;
    int where_are_we = iverts[0].y > sub_y;
    p2[where_are_we][n2[where_are_we]++] = iverts[0];

    int v, prev;
    for (v = 1, prev = 0; v <= ivertscount; v++)
    {
      // Check whenever current vertex is on top or down side of divider.
      int cur = (v == ivertscount) ? 0 : v;
      int now_we_are = iverts[cur].y > sub_y;

      if (now_we_are == where_are_we)
      {
        // Do not add the first point since it will be added at the end.
        if (cur) p2[where_are_we][n2[where_are_we]++] = iverts[cur];
      }
      else
      {
        // The most complex case: find the X at intersection point.
        float x = iverts[prev].x + (iverts[cur].x - iverts[prev].x) *
          (sub_y - iverts[prev].y) / (iverts[cur].y - iverts[prev].y);

        // Add the intersection point to both polygons.
        p2[0][n2[0]++] = p2[1][n2[1]++] = csVector2 (x, sub_y);

        if (cur) p2[now_we_are][n2[now_we_are]++] = iverts[cur];
      }

      where_are_we = now_we_are;
      prev = cur;
    }

    int tmp = Grid->ymax;
    Grid->ymax = sub_y;
    __poly_fill (p2[0], n2[0]);
    Grid->ymax = tmp;

    tmp = Grid->ymin;
    Grid->ymin = sub_y;
    __poly_fill (p2[1], n2[1]);
    Grid->ymin = tmp;
  }
}

void csAntialiasedPolyFill (
  csVector2 *iverts,
  int ivertscount,
  void *iArg,
  csAAPFCBPixel iPutPixel,
  csAAPFCBBox iDrawBox)
{
  static csRect *Grid = GetAAGrid();
  // If nothing to do, exit.
  if (ivertscount <= 0) return ;

  PutPixel = iPutPixel;
  DrawBox = iDrawBox;
  Arg = iArg;

  // Find the bounding box first.
  Grid->Set (999999, 999999, -999999, -999999);

  int i;
  for (i = 0; i < ivertscount; i++)
  {
    int x = csQint (iverts[i].x);
    int y = csQint (iverts[i].y);
    if (Grid->xmin > x) Grid->xmin = x;
    if (Grid->ymin > y) Grid->ymin = y;
    double ceilx = ceil (iverts[i].x);
    double ceily = ceil (iverts[i].y);
    x = csQround (ceilx);
    y = csQround (ceily);
    if (Grid->xmax < x) Grid->xmax = x;
    if (Grid->ymax < y) Grid->ymax = y;
  }

  __poly_fill (iverts, ivertscount);
}

