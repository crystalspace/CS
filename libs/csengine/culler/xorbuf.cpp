/*
    Copyright (C) 2002 by Jorrit Tyberghein

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
#include "qsqrt.h"
#include "csgeom/math3d.h"
#include "csengine/xorbuf.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"

//---------------------------------------------------------------------------

csXORBuffer::csXORBuffer (int w, int h)
{
  width = w;
  height = h;
  numrows = (h+31)/32;

  width_po2 = 1;
  w_shift = 0;
  while (width_po2 < width)
  {
    width_po2 <<= 1;
    w_shift++;
  }

  bufsize = width_po2 * numrows;
  printf ("width=%d height=%d width_po2=%d w_shift=%d bufsize=%d numrows=%d\n",
  width, height, width_po2, w_shift, bufsize, numrows); fflush (stdout);

  buffer = new uint32[bufsize];
  scr_buffer = new uint32[bufsize];

  debug_mode = false;
}

csXORBuffer::~csXORBuffer ()
{
  delete[] buffer;
  delete[] scr_buffer;
}

void csXORBuffer::InitializePolygonBuffer ()
{
  memset (buffer, 0, bufsize << 2);
}

void csXORBuffer::Initialize ()
{
  memset (scr_buffer, 0, bufsize << 2);
}

void csXORBuffer::DrawLeftLine (int x1, int y1, int x2, int y2)
{
  if (y2 < 0 || y1 >= height)
  {
    //------
    // Totally outside screen vertically.
    //------
    return;
  }

  if (x1 < 0 && x2 < 0)
  {
    //------
    // Totally on the left side. Just clamp.
    //------

    // First we need to clip vertically. This is easy to do
    // in this particular case since x=0 all the time.
    if (y1 < 0) y1 = 0;
    if (y2 >= height) y2 = height-1;

    int dy = y2-y1;
    int y = y1;
    while (dy > 0)
    {
      // @@@ Optimize!
      uint32* buf = &buffer[(y>>5) << w_shift];
      buf[0] ^= 1 << (y & 0x1f);
      y++;
      dy--;
    }
    return;
  }
  else if (x1 >= width && x2 >= width)
  {
    //------
    // Lines on the far right can just be dropped since they
    // will have no effect on the XOR buffer.
    //------
    return;
  }
  else if (x1 == x2)
  {
    //------
    // If line is fully vertical we also have a special case that
    // is easier to resolve.
    //------
    // First we need to clip vertically. This is easy to do
    // in this particular case since x=0 all the time.
    if (y1 < 0) y1 = 0;
    if (y2 >= height) y2 = height-1;

    int dy = y2-y1;
    int y = y1;
    while (dy > 0)
    {
      // @@@ Optimize!
      uint32* buf = &buffer[(y>>5) << w_shift];
      buf[x1] ^= 1 << (y & 0x1f);
      y++;
      dy--;
    }
    return;
  }

  //------
  // We don't have any of the trivial horizontal cases.
  // So we must clip vertically first.
  //------
  if (y1 < 0)
  {
    x1 = x1 + ( (0-y1) * (x2-x1) ) / (y2-y1);
    y1 = 0;
  }
  if (y2 >= height)
  {
    x2 = x1 + ( (height-1-y1) * (x2-x1) ) / (y2-y1);
    y2 = height-1;
  }

  if (x1 >= 0 && x2 >= 0 && x1 < width && x2 < width)
  {
    //------
    // The normal case, no clipping needed at all.
    //------
    int dy = (y2-y1)+1;
    int x = x1<<16;
    int y = y1;
    int dx = ((x2-x1)<<16) / dy;
    dy--;
    while (dy > 0)
    {
      uint32* buf = &buffer[(y>>5) << w_shift];
      buf[x>>16] ^= 1 << (y & 0x1f);
      x += dx;
      y++;
      dy--;
    }
  }
  else
  {
    //------
    // In this case we need to clip horizontally.
    //------
    int dy = (y2-y1)+1;
    int x = x1<<16;
    int y = y1;
    int dx = ((x2-x1)<<16) / dy;
    dy--;
    while (dy > 0)
    {
      uint32* buf = &buffer[(y>>5) << w_shift];
      if (x < 0)
      {
        buf[0] ^= 1 << (y & 0x1f);
      }
      else
      {
        int xn = x>>16;
	if (xn < width)
          buf[xn] ^= 1 << (y & 0x1f);
        else
	{
	  if (dx > 0) break;	// We can stop here.
	}
      }
      x += dx;
      y++;
      dy--;
    }
  }
}

void csXORBuffer::DrawRightLine (int x1, int y1, int x2, int y2)
{
  if (y2 < 0 || y1 >= height)
  {
    //------
    // Totally outside screen vertically.
    //------
    return;
  }

  if (x1 < 0 && x2 < 0)
  {
    //------
    // Totally on the left side. Just clamp.
    //------

    // First we need to clip vertically. This is easy to do
    // in this particular case since x=0 all the time.
    if (y1 < 0) y1 = 0;
    if (y2 >= height) y2 = height-1;

    int dy = y2-y1;
    int y = y1;
    while (dy > 0)
    {
      // @@@ Optimize!
      uint32* buf = &buffer[(y>>5) << w_shift];
      buf[0] ^= 1 << (y & 0x1f);
      y++;
      dy--;
    }
    return;
  }
  else if (x1 >= width && x2 >= width)
  {
    //------
    // Lines on the far right can just be dropped since they
    // will have no effect on the XOR buffer.
    //------
    return;
  }
  else if (x1 == x2)
  {
    //------
    // If line is fully vertical we also have a special case that
    // is easier to resolve.
    //------
    // First we need to clip vertically. This is easy to do
    // in this particular case since x=0 all the time.
    if (y1 < 0) y1 = 0;
    if (y2 >= height) y2 = height-1;

    int dy = y2-y1;
    int y = y1;
    while (dy > 0)
    {
      // @@@ Optimize!
      uint32* buf = &buffer[(y>>5) << w_shift];
      buf[x1] ^= 1 << (y & 0x1f);
      y++;
      dy--;
    }
    return;
  }

  //------
  // We don't have any of the trivial horizontal cases.
  // So we must clip vertically first.
  //------
  if (y1 < 0)
  {
    x1 = x1 + ( (0-y1) * (x2-x1) ) / (y2-y1);
    y1 = 0;
  }
  if (y2 >= height)
  {
    x2 = x1 + ( (height-1-y1) * (x2-x1) ) / (y2-y1);
    y2 = height-1;
  }

  if (x1 >= 0 && x2 >= 0 && x1 < width && x2 < width)
  {
    //------
    // The normal case, no clipping needed at all.
    //------
    int dy = (y2-y1)+1;
    int x = x1<<16;
    int y = y1;
    int dx = ((x2-x1)<<16) / dy;
    int dx2 = dx>>1;
    x += dx2;
    dy--;
    while (dy > 0)
    {
      uint32* buf = &buffer[(y>>5) << w_shift];
      buf[x>>16] ^= 1 << (y & 0x1f);
      x += dx;
      y++;
      dy--;
    }
  }
  else
  {
    //------
    // In this case we need to clip horizontally.
    //------
    int dy = (y2-y1)+1;
    int x = x1<<16;
    int y = y1;
    int dx = ((x2-x1)<<16) / dy;
    int dx2 = dx>>1;
    x += dx2;
    dy--;
    while (dy > 0)
    {
      uint32* buf = &buffer[(y>>5) << w_shift];
      if (x < 0)
      {
        buf[0] ^= 1 << (y & 0x1f);
      }
      else
      {
        int xn = x>>16;
	if (xn < width)
          buf[xn] ^= 1 << (y & 0x1f);
        else
	{
	  if (dx > 0) break;	// We can stop here.
	}
      }
      x += dx;
      y++;
      dy--;
    }
  }
}

void csXORBuffer::DrawPolygon (csVector2* verts, int num_verts)
{
  int i, j;

  //---------
  // First we copy the vertices to xa/ya. In the mean time
  // we convert to integer and also search for the top vertex (lowest
  // y coordinate) and bottom vertex.
  //@@@ TODO: pre-shift x with 16
  //---------
  int xa[128], ya[128];
  int top_vt = 0;
  int bot_vt = 0;
  xa[0] = QInt (verts[0].x);
  ya[0] = QInt (verts[0].y);
  int top_y = ya[0];
  int bot_y = ya[0];
  for (i = 1 ; i < num_verts ; i++)
  {
    xa[i] = QInt (verts[i].x);
    ya[i] = QInt (verts[i].y);
    if (ya[i] < top_y)
    {
      top_y = ya[i];
      top_vt = i;
    }
    else if (ya[i] > bot_y)
    {
      bot_y = ya[i];
      bot_vt = i;
    }
  }

  //---------
  // First find out in which direction the 'right' lines go.
  //---------
  //@@@@ THIS IS NOT VERY OPTIMAL!
  int top_vt_right = (top_vt+1)%num_verts;
  int top_vt_left = (top_vt-1+num_verts)%num_verts;
  int dxdyR = ((xa[top_vt_right]-xa[top_vt])<<16)
  	/ (ya[top_vt_right]-ya[top_vt]+1);
  int dxdyL = ((xa[top_vt_left]-xa[top_vt])<<16)
  	/ (ya[top_vt_left]-ya[top_vt]+1);
  int dir_right = (dxdyR > dxdyL) ? 1 : -1;

  //---------
  // Draw all right lines.
  //---------
  int dir = dir_right;
  i = top_vt;
  j = (i+num_verts+dir)%num_verts;

  while (i != bot_vt)
  {
    if (ya[i] != ya[j])
    {
      if (debug_mode)
      {
        printf ("R{%d,%d} (%d,%d) - (%d,%d)\n",
	  i, j, xa[i], ya[i], xa[j], ya[j]);
      }
      DrawRightLine (xa[i], ya[i], xa[j], ya[j]);
    }
    i = j;
    j = (j+num_verts+dir)%num_verts;
  }

  //---------
  // Draw all left lines.
  //---------
  dir = -dir_right;
  i = top_vt;
  j = (i+num_verts+dir)%num_verts;

  while (i != bot_vt)
  {
    if (ya[i] != ya[j])
    {
      if (debug_mode)
      {
        printf ("L{%d,%d} (%d,%d) - (%d,%d)\n",
	  i, j, xa[i], ya[i], xa[j], ya[j]);
      }
      DrawLeftLine (xa[i], ya[i], xa[j], ya[j]);
    }
    i = j;
    j = (j+num_verts+dir)%num_verts;
  }
}

void csXORBuffer::XORSweep ()
{
  int i, x;
  uint32* buf;
  for (i = 0 ; i < numrows ; i++)
  {
    buf = &buffer[i<<w_shift];
    uint32 first = *buf++;
    for (x = 1 ; x < width ; x++)
    {
      first ^= *buf;
      *buf++ = first;
      // @@@ Optimize to stop when 'first' becomes 0 again after being non-null.
    }
  }
}

bool csXORBuffer::InsertPolygon (csVector2* verts, int num_verts, bool negative)
{
  InitializePolygonBuffer ();
  // @@@ This code is totally unoptimized and ignores the
  // bounding box of the polygon. Using the 2D bounding box of the
  // polygon a much more optimal rendering can be done.
  DrawPolygon (verts, num_verts);

  // In this routine we render the polygon to the polygon buffer
  // and then we simulate (but don't actually perform for optimization
  // purposes) a XOR sweep on that polygon buffer.

  int i, x;
  uint32* buf;
  uint32* scr_buf;
  bool mod = false;
  uint32 init;
  if (negative) init = ~0;
  else init = 0;
  for (i = 0 ; i < numrows ; i++)
  {
    buf = &buffer[i<<w_shift];
    scr_buf = &scr_buffer[i<<w_shift];
    uint32 first = init;
    for (x = 0 ; x < width ; x++)
    {
      first ^= *buf++;
      if ((~*scr_buf) & first)
      {
        mod = true;
	*scr_buf++ |= first;
      }
      else
      {
        scr_buf++;
      }
      // @@@ Optimize to stop when 'first' becomes 0 again after being non-null.
    }
  }
  return mod;
}

bool csXORBuffer::TestPolygon (csVector2* verts, int num_verts)
{
  InitializePolygonBuffer ();
  // @@@ This code is totally unoptimized and ignores the
  // bounding box of the polygon. Using the 2D bounding box of the
  // polygon a much more optimal rendering can be done.
  DrawPolygon (verts, num_verts);

  int i, x;
  uint32* buf;
  uint32* scr_buf;
  for (i = 0 ; i < numrows ; i++)
  {
    buf = &buffer[i<<w_shift];
    scr_buf = &scr_buffer[i<<w_shift];
    uint32 first = 0;
    for (x = 0 ; x < width ; x++)
    {
      first ^= *buf++;
      if ((~*scr_buf) & first) return true;
      scr_buf++;
      // @@@ Optimize to stop when 'first' becomes 0 again after being non-null.
    }
  }
  return false;
}

static void DrawZoomedPixel (iGraphics2D* g2d, int x, int y, int col, int zoom)
{
  if (zoom == 1)
    g2d->DrawPixel (x, y, col);
  else if (zoom == 2)
  {
    x <<= 1;
    y <<= 1;
    g2d->DrawPixel (x+0, y+0, col);
    g2d->DrawPixel (x+1, y+0, col);
    g2d->DrawPixel (x+0, y+1, col);
    g2d->DrawPixel (x+1, y+1, col);
  }
  else if (zoom == 3)
  {
    x *= 3;
    y *= 3;
    g2d->DrawPixel (x+0, y+0, col);
    g2d->DrawPixel (x+1, y+0, col);
    g2d->DrawPixel (x+2, y+0, col);
    g2d->DrawPixel (x+0, y+1, col);
    g2d->DrawPixel (x+1, y+1, col);
    g2d->DrawPixel (x+2, y+1, col);
    g2d->DrawPixel (x+0, y+2, col);
    g2d->DrawPixel (x+1, y+2, col);
    g2d->DrawPixel (x+2, y+2, col);
  }
}

void csXORBuffer::Debug_Dump (iGraphics2D* g2d, iGraphics3D* g3d, int zoom)
{
  iTextureManager *txtmgr = g3d->GetTextureManager ();
  int col = txtmgr->FindRGB (200, 200, 200);
  int x, y, i;
  for (i = 0 ; i < numrows ; i++)
  {
    uint32* row = &buffer[i<<w_shift];
    for (x = 0 ; x < width ; x++)
    {
      uint32 c = *row++;
      int yy = i*32;
      for (y = 0 ; y < 32 ; y++)
      {
        DrawZoomedPixel (g2d, x, yy, (c & 1) ? col : 0, zoom);
	c = c >> 1;
	yy++;
      }
    }
  }
}

void csXORBuffer::Debug_DumpScr (iGraphics2D* g2d, iGraphics3D* g3d, int zoom)
{
  iTextureManager *txtmgr = g3d->GetTextureManager ();
  int col = txtmgr->FindRGB (200, 200, 200);
  int x, y, i;
  for (i = 0 ; i < numrows ; i++)
  {
    uint32* row = &scr_buffer[i<<w_shift];
    for (x = 0 ; x < width ; x++)
    {
      uint32 c = *row++;
      int yy = i*32;
      for (y = 0 ; y < 32 ; y++)
      {
        DrawZoomedPixel (g2d, x, yy, (c & 1) ? col : 0, zoom);
	c = c >> 1;
	yy++;
      }
    }
  }
}

void csXORBuffer::Debug_DrawLine (iGraphics2D* g2d,
	float x1, float y1, float x2, float y2, int col, float l,
	int zoom)
{
  int i;
  int il = QInt (l);
  for (i = 0 ; i <= il ; i++)
  {
    float x = x1 + float (i) * (x2-x1) / l;
    float y = y1 + float (i) * (y2-y1) / l;
    DrawZoomedPixel (g2d, QInt (x), QInt (y), col, zoom);
  }
}

void csXORBuffer::Debug_DrawPolygon (iGraphics2D* g2d, iGraphics3D* g3d,
	csVector2* verts, int num_verts, int zoom)
{
  iTextureManager *txtmgr = g3d->GetTextureManager ();
  int col = txtmgr->FindRGB (255, 0, 255);
  int i;
  for (i = 0 ; i < num_verts ; i++)
  {
    int j = (i+1) % num_verts;
    float dx = verts[j].x - verts[i].x;
    float dy = verts[j].y - verts[i].y;
    float l = dx*dx + dy*dy;
    Debug_DrawLine (g2d, verts[i].x, verts[i].y,
    	verts[j].x, verts[j].y, col, qsqrt (l), zoom);
  }
}

static float rnd (int totrange, int leftpad, int rightpad)
{
  return float (((rand () >> 4) % (totrange-leftpad-rightpad)) + leftpad);
}

bool csXORBuffer::Debug_ExtensiveTest (int num_iterations, csVector2* verts,
	int& num_verts)
{
  int i;
  for (i = 0 ; i < num_iterations ; i++)
  {
    InitializePolygonBuffer ();
    num_verts = ((rand () >> 4) % 1)+3;
    switch (num_verts)
    {
      case 3:
        verts[0].Set (rnd (width, -100, 20), rnd (height, -100, -100));
        verts[1].Set (rnd (width, -100, 20), rnd (height, -100, -100));
        verts[2].Set (rnd (width, -100, 20), rnd (height, -100, -100));
        break;
      case 4:
        // @@@ TODO!
	break;
    }
    DrawPolygon (verts, num_verts);
    XORSweep ();

    int r;
    for (r = 0 ; r < numrows ; r++)
    {
      uint32* row = &buffer[(r<<w_shift)+width-2];
      if (*row) return false;
    }
  }
  return true;
}

