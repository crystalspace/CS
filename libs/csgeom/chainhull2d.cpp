/*
    Copyright (C) 2005 by Jorrit Tyberghein

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
    The code has been adapted from other code with the following
    copyright message:
    Copyright 2001, softSurfer (www.softsurfer.com)
    This code may be freely used and modified for any purpose
    providing that this copyright notice is included with it.
    SoftSurfer makes no warranty for this code, and cannot be held
    liable for any real or imagined damage resulting from its use.
    Users of this code must verify correctness for their application.
*/

#include "cssysdef.h"
#include "csgeom/chainhull2d.h"

static int compare_points_on_xy (void const* p1, void const* p2)
{
  const csVector2* vp1 = (const csVector2*)p1;
  const csVector2* vp2 = (const csVector2*)p2;
  if (vp1->x < vp2->x) return -1;
  else if (vp1->x > vp2->x) return 1;
  else if (vp1->y < vp2->y) return -1;
  else if (vp1->y > vp2->y) return 1;
  else return 0;
}

void csChainHull2D::SortXY (csVector2* points, int n)
{
  qsort (points, n, sizeof (csVector2), compare_points_on_xy);
}

int csChainHull2D::CalculatePresorted (csVector2* points, int n,
	csVector2* hull)
{
  // The output array hull[] will be used as the stack.
  int bot = 0, top = -1;  // Indices for bottom and top of the stack.
  int i;

  // Get the indices of points with min x-coord and min|max y-coord.
  int minmin = 0, minmax;
  float xmin = points[0].x;
  for (i=1 ; i<n ; i++)
    if (points[i].x != xmin) break;
  minmax = i-1;
  if (minmax == n-1)
  {
    // Degenerate case: all x-coords == xmin
    hull[++top] = points[minmin];
    if (points[minmax].y != points[minmin].y) // A nontrivial segment
      hull[++top] = points[minmax];
    hull[++top] = points[minmin];             // Add polygon endpoint
    return top+1;
  }

  // Get the indices of points with max x-coord and min|max y-coord.
  int maxmin, maxmax = n-1;
  float xmax = points[n-1].x;
  for (i=n-2 ; i>=0 ; i--)
    if (points[i].x != xmax) break;
  maxmin = i+1;

  // Compute the lower hull on the stack 'hull'.
  hull[++top] = points[minmin]; // Push minmin point onto stack.
  i = minmax;
  while (++i <= maxmin)
  {
    // The lower line joins points[minmin] with points[maxmin].
    if (points[i].IsLeft (points[minmin], points[maxmin]) >= 0 && i < maxmin)
      continue;                 // Ignore points[i] above or on the lower line.

    while (top > 0)             // There are at least 2 points on the stack.
    {
      // Test if points[i] is left of the line at the stack top.
      if (points[i].IsLeft (hull[top-1], hull[top]) > 0)
	break;                  // points[i] is a new hull vertex.
      else
	top--;                  // Pop top point off stack.
    }
    hull[++top] = points[i];    // Push points[i] onto stack.
  }

  // Next, compute the upper hull on the stack H above the bottom hull.
  if (maxmax != maxmin)         // If distinct xmax points push maxmax on stack.
    hull[++top] = points[maxmax];
  bot = top;                    // The bottom point of the upper hull stack.
  i = maxmin;
  while (--i >= minmax)
  {
    // The upper line joins points[maxmax] with points[minmax].
    if (points[i].IsLeft (points[maxmax], points[minmax]) >= 0 && i > minmax)
      continue;                 // Ignore points[i] below or on the upper line.

    while (top > bot)           // At least 2 points on the upper stack.
    {
      // Test if points[i] is left of the line at the stack top.
      if (points[i].IsLeft (hull[top-1], hull[top]) > 0)
	break;                 // points[i] is a new hull vertex.
      else
	top--;                 // Pop top point off stack.
    }
    hull[++top] = points[i];   // Push points[i] onto stack.
  }
  if (minmax != minmin)        // Push joining endpoint onto stack.
    hull[++top] = points[minmin];

  return top+1;
}

