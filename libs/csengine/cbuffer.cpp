/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Camera code written by Ivan Avramovic <ivan@avramovic.com>
  
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
#include "qint.h"
#include "csengine/cbuffer.h"

//---------------------------------------------------------------------------

csCBufferLine::csCBufferLine ()
{
  first_span = NULL;
  last_span = NULL;
  // We already place one span in the unused buffer
  // so that Initialize() can use that.
  CHK (first_unused = new csCBufferSpan ());
  first_unused->next = NULL;
}

void csCBufferLine::Initialize (int startx, int endx)
{
  if (last_span)
  {
    // Append the current unused list to the last span
    // and set the new unused list to the first span.
    // This moves all used spans to the unused list.
    last_span->next = first_unused;
    first_unused = first_span;
  }
  // We know there is at least one span in the unused list
  // because a c-buffer line is initialized with one span.
  first_span = first_unused;
  first_span->startx = startx;
  first_span->endx = endx;
  first_span->next = NULL;
  last_span = first_span;
  first_unused = first_unused->next;
}

csCBufferLine::~csCBufferLine ()
{
  csCBufferSpan* n;
  while (first_span)
  {
    n = first_span->next;
    CHK (delete first_span);
    first_span = n;
  }
  while (first_unused)
  {
    n = first_unused->next;
    CHK (delete first_unused);
    first_unused = n;
  }
}

bool csCBufferLine::TestSpan (int startx, int endx)
{
  csCBufferSpan* s = first_span;
  while (s)
  {
    if (startx <= s->endx)
    {
      // We have found a span which ends after the start of the full span.

      // If the start of the empty span is after the end of the full span
      // then the span is not visible. Otherwise it is visible.
      if (s->startx > endx) return false;

      return true;
    }
    s = s->next;
  }

  // There are no empty spans which overlap with the given full span.
  // so span is not visible.
  return false;
}

bool csCBufferLine::InsertSpan (int startx, int endx)
{
  bool vis = false;
  csCBufferSpan* s = first_span;
  csCBufferSpan* ps = NULL;	// 'ps' holds the previous span.
  while (s)
  {
    // If the start of the empty span is after then end of the full span
    // then we can stop scanning.
    if (s->startx > endx) break;

    if (s->endx >= startx)
    {
      // We have found a span which ends after the start of the full span.

      // If the start of the empty span is after the end of the full span
      // then we can stop.
      if (s->startx > endx) return vis;

      // We know this empty span is going to modified somehow.
      // So the full span is visible.
      vis = true;

      // If empty span is fully enclosed in full span then we have to remove empty span.
      if (s->startx >= startx && s->endx <= endx)
      {
        if (ps) ps->next = s->next;
	else first_span = s->next;
	s->next = first_unused;
	first_unused = s;
      }

      // If start of empty span is before start of full span then we 
      // have to shrink the empty span.
      else if (s->startx < startx)
      {
        s->endx = startx-1;
      }

      // If end of empty span is after end of full span then we have
      // to shrink the empty span.
      else if (s->endx > endx)
      {
	s->startx = endx+1;
      }

      // Else the empty span is totally covering the full scan. We
      // have to split the empty span.
      else
      {
        csCBufferSpan* new_span;
        if (first_unused)
	{
	  new_span = first_unused;
	  first_unused = first_unused->next;
	}
	else
	{
	  CHK (new_span = new csCBufferSpan ());
	}
	new_span->next = s->next;
	s->next = new_span;
	if (new_span->next == NULL) last_span = new_span;
	new_span->endx = s->endx;
	s->endx = startx-1;
	new_span->startx = endx+1;
      }
    }
    ps = s;
    s = s->next;
  }

  // There are no more empty spans which overlap with the given full span.
  return vis;
}

//---------------------------------------------------------------------------

csCBuffer::csCBuffer (int sx, int ex, int n_lines)
{
  num_lines = n_lines;
  startx = sx;
  endx = ex;
  CHK (lines = new csCBufferLine [num_lines]);
  CHK (full = new bool [num_lines]);
}

csCBuffer::~csCBuffer ()
{
  CHK (delete [] lines);
  CHK (delete [] full);
}

void csCBuffer::Initialize ()
{
  int y;
  for (y = 0 ; y < num_lines ; y++)
  {
    lines[y].Initialize (startx, endx);
    full[y] = false;
  }
  not_full_lines = num_lines;
}

bool csCBuffer::InsertSpan (int s_spanx, int e_spanx, int y)
{
  bool rc = lines[y].InsertSpan (s_spanx, e_spanx);
  if (rc)
  {
    // If the line is full now, it could not have been full previously.
    // So we know we have a new full line and we can decrease 'not_full_lines'.
    if (lines[y].IsFull ()) { full[y] = true; not_full_lines--; }
  }
  return rc;
}

#define VERTEX_NEAR_THRESHOLD   0.001
bool csCBuffer::TestPolygon (csVector2* verts, int num_verts)
{
  // Compute the min_y and max_y for this polygon in screen space coordinates.
  // We are going to use these to scan the polygon from top to bottom.
  int min_i = 0, max_i = 0;
  float min_y, max_y;
  min_y = max_y = verts[0].y;
  // count 'real' number of vertices
  int num_vertices = 1;
  int i;
  for (i = 1 ; i < num_verts ; i++)
  {
    if (verts[i].y > max_y)
    {
      max_y = verts[i].y;
      max_i = i;
    }
    else if (verts[i].y < min_y)
    {
      min_y = verts[i].y;
      min_i = i;
    }
    // theoretically we should do sqrt(dx^2+dy^2) here, but
    // we can approximate it by just abs(dx)+abs(dy)
    if ((fabs (verts [i].x - verts [i - 1].x)
       + fabs (verts [i].y - verts [i - 1].y)) > VERTEX_NEAR_THRESHOLD)
      num_vertices++;
  }

  // if this is a 'degenerate' polygon, skip it (i.e. not visible).
  if (num_vertices < 3) return false;

  int scanL1, scanL2, scanR1, scanR2;   // scan vertex left/right start/final
  float sxL, sxR, dxL, dxR;             // scanline X left/right and deltas
  int sy, fyL, fyR;                     // scanline Y, final Y left, final Y right
  int xL, xR;

  sxL = sxR = dxL = dxR = 0;            // avoid GCC warnings about "uninitialized variables"
  scanL2 = scanR2 = max_i;
  sy = fyL = fyR = QRound (verts [scanL2].y);

  for ( ; ; )
  {
    //-----
    // We have reached the next segment. Recalculate the slopes.
    //-----
    bool leave;
    do
    {
      leave = true;
      if (sy <= fyR)
      {
        // Check first if polygon has been finished
        if (scanR2 == min_i)
	  goto finish;
        scanR1 = scanR2;
	if (++scanR2 >= num_verts)
	  scanR2 = 0;

        leave = false;
        fyR = QRound (verts [scanR2].y);
        if (sy <= fyR)
          continue;

        float dyR = (verts [scanR1].y - verts [scanR2].y);
        if (dyR > 0)
        {
          sxR = verts [scanR1].x;
          dxR = (verts [scanR2].x - sxR) / dyR;
          // horizontal pixel correction
          sxR += dxR * (verts [scanR1].y - (float (sy) - 0.5));
        }
      }
      if (sy <= fyL)
      {
        scanL1 = scanL2;
	if (--scanL2 < 0)
	  scanL2 = num_verts - 1;

        leave = false;
        fyL = QRound (verts [scanL2].y);
        if (sy <= fyL)
          continue;

        float dyL = (verts [scanL1].y - verts [scanL2].y);
        if (dyL)
        {
          sxL = verts [scanL1].x;
          dxL = (verts [scanL2].x - sxL) / dyL;
          // horizontal pixel correction
          sxL += dxL * (verts [scanL1].y - (float (sy) - 0.5));
        }
      }
    } while (!leave);

    // Find the trapezoid top (or bottom in inverted Y coordinates)
    int fin_y;
    if (fyL > fyR)
      fin_y = fyL;
    else
      fin_y = fyR;

    while (sy > fin_y)
    {
      // Compute the rounded screen coordinates of horizontal strip
      xL = QRound (sxL);
      xR = QRound (sxR);
      if (TestSpan (xL, xR, sy)) return true;

      sxL += dxL;
      sxR += dxR;
      sy--;
    }
  }

finish:
  // If we come here polygon was not visible.
  return false;
}

bool csCBuffer::InsertPolygon (csVector2* verts, int num_verts)
{
  bool vis = false;

  // Compute the min_y and max_y for this polygon in screen space coordinates.
  // We are going to use these to scan the polygon from top to bottom.
  int min_i = 0, max_i = 0;
  float min_y, max_y;
  min_y = max_y = verts[0].y;
  // count 'real' number of vertices
  int num_vertices = 1;
  int i;
  for (i = 1 ; i < num_verts ; i++)
  {
    if (verts[i].y > max_y)
    {
      max_y = verts[i].y;
      max_i = i;
    }
    else if (verts[i].y < min_y)
    {
      min_y = verts[i].y;
      min_i = i;
    }
    // theoretically we should do sqrt(dx^2+dy^2) here, but
    // we can approximate it by just abs(dx)+abs(dy)
    if ((fabs (verts [i].x - verts [i - 1].x)
       + fabs (verts [i].y - verts [i - 1].y)) > VERTEX_NEAR_THRESHOLD)
      num_vertices++;
  }

  // if this is a 'degenerate' polygon, skip it (i.e. not visible).
  if (num_vertices < 3) return false;

  int scanL1, scanL2, scanR1, scanR2;   // scan vertex left/right start/final
  float sxL, sxR, dxL, dxR;             // scanline X left/right and deltas
  int sy, fyL, fyR;                     // scanline Y, final Y left, final Y right
  int xL, xR;

  sxL = sxR = dxL = dxR = 0;            // avoid GCC warnings about "uninitialized variables"
  scanL2 = scanR2 = max_i;
  sy = fyL = fyR = QRound (verts [scanL2].y);

  for ( ; ; )
  {
    //-----
    // We have reached the next segment. Recalculate the slopes.
    //-----
    bool leave;
    do
    {
      leave = true;
      if (sy <= fyR)
      {
        // Check first if polygon has been finished
        if (scanR2 == min_i)
	  goto finish;
        scanR1 = scanR2;
	if (++scanR2 >= num_verts)
	  scanR2 = 0;

        leave = false;
        fyR = QRound (verts [scanR2].y);
        if (sy <= fyR)
          continue;

        float dyR = (verts [scanR1].y - verts [scanR2].y);
        if (dyR > 0)
        {
          sxR = verts [scanR1].x;
          dxR = (verts [scanR2].x - sxR) / dyR;
          // horizontal pixel correction
          sxR += dxR * (verts [scanR1].y - (float (sy) - 0.5));
        }
      }
      if (sy <= fyL)
      {
        scanL1 = scanL2;
	if (--scanL2 < 0)
	  scanL2 = num_verts - 1;

        leave = false;
        fyL = QRound (verts [scanL2].y);
        if (sy <= fyL)
          continue;

        float dyL = (verts [scanL1].y - verts [scanL2].y);
        if (dyL)
        {
          sxL = verts [scanL1].x;
          dxL = (verts [scanL2].x - sxL) / dyL;
          // horizontal pixel correction
          sxL += dxL * (verts [scanL1].y - (float (sy) - 0.5));
        }
      }
    } while (!leave);

    // Find the trapezoid top (or bottom in inverted Y coordinates)
    int fin_y;
    if (fyL > fyR)
      fin_y = fyL;
    else
      fin_y = fyR;

    while (sy > fin_y)
    {
      // Compute the rounded screen coordinates of horizontal strip
      xL = QRound (sxL);
      xR = QRound (sxR);
      if (InsertSpan (xL, xR, sy)) vis = true;

      sxL += dxL;
      sxR += dxR;
      sy--;
    }
  }

finish:
  // If we come here polygon was not visible.
  return vis;
}


//---------------------------------------------------------------------------

