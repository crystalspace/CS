/*
    Copyright (C) 1998 by Jorrit Tyberghein

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
#include "qint.h"
#include "csengine/cbuffer.h"
#include "csengine/engine.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"

//---------------------------------------------------------------------------

csCBufferLine::csCBufferLine ()
{
  first_span = NULL;
  last_span = NULL;
  parent = NULL;
}

void csCBufferLine::Initialize (int startx, int endx)
{
  if (last_span)
  {
    // Append the current unused list to the last span
    // and set the new unused list to the first span.
    // This moves all used spans to the unused list.

    last_span->next = parent->first_unused;
    parent->first_unused = first_span;
  }

  // Allocate an empty span for the whole scanline.
  first_span = parent->AllocSpan ();
  first_span->startx = startx;
  first_span->endx = endx;
  first_span->next = NULL;
  last_span = first_span;
}

csCBufferLine::~csCBufferLine ()
{
  csCBufferSpan* n;
  while (first_span)
  {
    n = first_span->next;
    delete first_span;
    first_span = n;
  }
}

void csCBufferLine::MakeFull ()
{
  while (first_span)
  {
    csCBufferSpan* n = first_span->next;
    parent->FreeSpan (first_span);
    first_span = n;
  }
  last_span = NULL;
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
    // If the start of the empty span is after the end of the full span
    // then we can stop scanning.
    if (s->startx > endx) break;

    if (s->endx >= startx)
    {
      // We have found a span which ends after the start of the full span.

      // We know this empty span is going to modified somehow.
      // So the full span is visible.
      vis = true;

      // If empty span is fully enclosed in full span then we have to remove
      // empty span.
      if (s->startx >= startx && s->endx <= endx)
      {
	csCBufferSpan* sn = s->next;
        if (ps) ps->next = sn;
	else first_span = sn;
	if (!sn) last_span = ps;
	parent->FreeSpan (s);
	s = sn;
	continue;
      }

      // If start of empty span is before start of full span then we
      // have to shrink the empty span.
      else if (s->startx < startx && s->endx <= endx)
      {
        s->endx = startx-1;
      }

      // If end of empty span is after end of full span then we have
      // to shrink the empty span.
      else if (s->endx > endx && s->startx >= startx)
      {
	s->startx = endx+1;
      }

      // Else the empty span is totally covering the full scan. We
      // have to split the empty span.
      else
      {
        csCBufferSpan* new_span = parent->AllocSpan ();
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

void csCBufferLine::Dump ()
{
  csCBufferSpan* span = first_span;
  while (span)
  {
    printf ("  empty = %d-%d\n", span->startx, span->endx);
    span = span->next;
  }
}

//---------------------------------------------------------------------------

csCBuffer::csCBuffer (int sx, int ex, int n_lines)
{
  num_lines = n_lines;
  startx = sx;
  endx = ex;
  lines = new csCBufferLine [num_lines];
  first_unused = NULL;
  int i;
  for (i = 0 ; i < num_lines ; i++)
    lines[i].SetParent (this);
  vert_line.SetParent (this);
}

csCBuffer::~csCBuffer ()
{
  while (first_unused)
  {
    csCBufferSpan* n = first_unused->next;
    delete first_unused;
    first_unused = n;
  }
  delete [] lines;
}

void csCBuffer::Initialize ()
{
  int y;
  for (y = 0 ; y < num_lines ; y++)
    lines[y].Initialize (startx, endx);
  vert_line.Initialize (0, num_lines-1);
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
    if ((ABS (verts [i].x - verts [i - 1].x)
       + ABS (verts [i].y - verts [i - 1].y)) > VERTEX_NEAR_THRESHOLD)
      num_vertices++;
  }

  // if this is a 'degenerate' polygon, skip it (i.e. not visible).
  if (num_vertices < 3) return false;

  int scanL1, scanL2, scanR1, scanR2;   // scan vertex left/right start/final
  float sxL, sxR, dxL, dxR;             // scanline X left/right and deltas
  int sy, fyL, fyR;                     // scanline Y, final Y left and right
  int xL, xR;

  sxL = sxR = dxL = dxR = 0;
  scanL2 = scanR2 = max_i;
  sy = fyL = fyR = QRound (verts [scanL2].y);

  // First test if the min/max y range isn't already full in the
  // c-buffer. In that case we don't need to do anything.
  int miny = QRound (verts [min_i].y);
  if (!vert_line.TestSpan (miny, sy)) return false;

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

    // Make sure we work correctly for both orientations of the polygon.
    bool swap;
    if (sxL+dxL > sxR+dxR)
    {
      float xs = sxR; sxR = sxL; sxL = xs;
      float dxs = dxR; dxR = dxL; dxL = dxs;
      swap = true;
    }
    else swap = false;

    while (sy > fin_y)
    {
      // Compute the rounded screen coordinates of horizontal strip
      xL = QRound (sxL);
      xR = QRound (sxR);
      if (TestSpan (xL, xR-1, sy)) return true;

      sxL += dxL;
      sxR += dxR;
      sy--;
    }
    if (swap)
    {
      float xs = sxR; sxR = sxL; sxL = xs;
      float dxs = dxR; dxR = dxL; dxL = dxs;
    }
  }

finish:
  // If we come here polygon was not visible.
  return false;
}

bool csCBuffer::InsertPolygon (csVector2* verts, int num_verts, bool negative)
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
    if ((ABS (verts [i].x - verts [i - 1].x)
       + ABS (verts [i].y - verts [i - 1].y)) > VERTEX_NEAR_THRESHOLD)
      num_vertices++;
  }

  // if this is a 'degenerate' polygon, skip it (i.e. not visible).
  if (num_vertices < 3) return false;

  int scanL1, scanL2, scanR1, scanR2;   // scan vertex left/right start/final
  float sxL, sxR, dxL, dxR;             // scanline X left/right and deltas
  int sy, fyL, fyR;                     // scanline Y, final Y left and right
  int xL, xR;

  sxL = sxR = dxL = dxR = 0;
  scanL2 = scanR2 = max_i;
  sy = fyL = fyR = QRound (verts [scanL2].y);

  // First test if the min/max y range isn't already full in the
  // c-buffer. In that case we don't need to do anything.
  int miny = QRound (verts [min_i].y);
  if (!vert_line.TestSpan (miny, sy)) return false;

  // If in_full is true we are in a full region.
  bool in_full = false;
  int start_full = sy;

  // If we are in 'negative' mode then we make the lines
  // above and below the polygon full.
  if (negative)
  {
    int yy;
    for (yy = 0 ; yy < miny ; yy++) lines[yy].MakeFull ();
    vert_line.InsertSpan (0, miny-1);
    for (yy = sy+1 ; yy < num_lines ; yy++) lines[yy].MakeFull ();
    vert_line.InsertSpan (sy+1, num_lines-1);
  }

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

    // Make sure we work correctly for both orientations of the polygon.
    // To avoid rounding errors we test sxL+dxL with sxR+dxR. In other words
    // we already advance one step. This avoids a problem with the two top
    // x coordinates being almost equal and rounding errors causing the
    // wrong swap here.
    // @@@ INVESTIGATE IF THIS IS THE RIGHT FIX!
    bool swap;
    if (sxL+dxL > sxR+dxR)
    {
      float xs = sxR; sxR = sxL; sxL = xs;
      float dxs = dxR; dxR = dxL; dxL = dxs;
      swap = true;
    }
    else swap = false;

    while (sy > fin_y)
    {
      // Compute the rounded screen coordinates of horizontal strip
      xL = QRound (sxL);
      xR = QRound (sxR);
      if (negative)
      {
        if (InsertSpan (startx, xL-1, sy)) vis = true;
        if (InsertSpan (xR+1, endx, sy)) vis = true;
      }
      else
        if (InsertSpan (xL, xR-1, sy)) vis = true;

      // See if we need to update the full-line buffer.
      if (IsFull (sy))
      {
        if (!in_full)
	{
	  // We are full but we were not full in the previous line.
	  // In that case we start a new full span.
	  in_full = true;
	  start_full = sy;
	}
      }
      else
      {
        if (in_full)
	{
	  // We are not full but we were full in the previous line.
	  // In that case we end the previous full span.
	  vert_line.InsertSpan (sy-1, start_full);
	  in_full = false;
	}
      }

      sxL += dxL;
      sxR += dxR;
      sy--;
    }

    if (swap)
    {
      float xs = sxR; sxR = sxL; sxL = xs;
      float dxs = dxR; dxR = dxL; dxL = dxs;
    }
  }

finish:
  if (in_full)
  {
    // We are not full but we were full in the previous line.
    // In that case we end the previous full span.
    vert_line.InsertSpan (miny, start_full);
    in_full = false;
  }

  // If we come here polygon was not visible.
  return vis;
}

bool csCBuffer::TestPoint (const csVector2& point)
{
  return lines[QRound (point.y)].TestSpan (QRound (point.x), QRound (point.x));
}

void csCBuffer::GfxDump (iGraphics2D* ig2d, iGraphics3D* ig3d)
{
  iTextureManager* txtmgr = ig3d->GetTextureManager ();
  int col = txtmgr->FindRGB (255, 255, 0);
  int y, yy;
  for (y = 0 ; y < num_lines ; y++)
  {
    yy = ig2d->GetHeight ()-y;
    csCBufferSpan* span = lines[y].first_span;
    while (span)
    {
      ig2d->DrawLine (span->startx, yy, span->endx, yy, col);
      span = span->next;
    }
  }
}
