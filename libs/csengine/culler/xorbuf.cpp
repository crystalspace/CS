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
}

csXORBuffer::~csXORBuffer ()
{
  delete[] buffer;
}

void csXORBuffer::Initialize ()
{
  memset (buffer, 0, bufsize << 2);
}

void csXORBuffer::DrawLeftLine (int x1, int y1, int x2, int y2)
{
  int dy = y2-y1;
  int x = x1<<16;
  int y = y1;
  int dx = ((x2-x1)<<16) / dy;
  int dx2 = dx>>1;
  if (dx2 < 0) dx2 = -dx2;
  x -= dx2;
  while (dy > 0)
  {
    uint32* buf = &buffer[(y>>5) << w_shift];
    buf[x>>16] |= 1 << (y & 0x1f);
    x += dx;
    y++;
    dy--;
  }
}

void csXORBuffer::DrawRightLine (int x1, int y1, int x2, int y2)
{
  int dy = y2-y1;
  int x = x1<<16;
  int y = y1;
  int dx = ((x2-x1)<<16) / dy;
  int dx2 = dx>>1;
  if (dx2 < 0) dx2 = -dx2;
  x += dx2;
  while (dy > 0)
  {
    uint32* buf = &buffer[(y>>5) << w_shift];
    buf[x>>16] |= 1 << (y & 0x1f);
    x += dx;
    y++;
    dy--;
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
  //@@@ TODO
  int dir_right = 1;

  //---------
  // Draw all right lines.
  //---------
  int dir = dir_right;
  i = top_vt;
  j = (i+num_verts+dir)%num_verts;

  while (i != bot_vt)
  {
    if (ya[i] != ya[j])
      DrawRightLine (xa[i], ya[i], xa[j], ya[j]);
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
      DrawLeftLine (xa[i], ya[i], xa[j], ya[j]);
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

void csXORBuffer::GfxDump (iGraphics2D *ig2d, iGraphics3D *ig3d)
{
  iTextureManager *txtmgr = ig3d->GetTextureManager ();
  int col = txtmgr->FindRGB (255, 255, 0);
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
        ig2d->DrawPixel (x, yy, (c & 1) ? col : 0);
	c = c >> 1;
	yy++;
      }
    }
  }
}

