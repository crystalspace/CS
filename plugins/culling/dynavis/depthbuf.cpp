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
#include "cssys/sysfunc.h"
#include "csutil/scfstr.h"
#include "iutil/string.h"
#include "qint.h"
#include "qsqrt.h"
#include "csgeom/math3d.h"
#include "csgeom/box.h"
#include "depthbuf.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csDepthBuffer)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iDebugHelper)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csDepthBuffer::DebugHelper)
  SCF_IMPLEMENTS_INTERFACE(iDebugHelper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csDepthBuffer::csDepthBuffer ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiDebugHelper);
  children = NULL;
}

void csDepthBuffer::Setup (int sx, int sy, int w, int h)
{
  CS_ASSERT (w == h);
  CS_ASSERT (w >= 8);
  startx = sx;
  starty = sy;
  width = w;
  height = h;

  delete[] children;
  if (w == 8)
  {
    children = NULL;
  }
  else
  {
    children = new csDepthBuffer[4];
    int w2 = w >> 1;
    int h2 = h >> 1;
    children[CS_DEPTH_CHILD_00].Setup (sx, sy, w2, h2);
    children[CS_DEPTH_CHILD_10].Setup (sx+w2, sy, w2, h2);
    children[CS_DEPTH_CHILD_01].Setup (sx, sy+h2, w2, h2);
    children[CS_DEPTH_CHILD_11].Setup (sx+w2, sy+w2, w2, h2);
  }
}

csDepthBuffer::~csDepthBuffer ()
{
  delete[] children;
}

void csDepthBuffer::Initialize ()
{
  min_depth = 0;
  max_depth = 0;
}

bool csDepthBuffer::InsertRectangle (const csBox2Int& bbox,
	float rect_mindepth, float rect_maxdepth)
{
  if (bbox.maxx < startx || bbox.maxy < starty ||
  	bbox.minx >= startx+width || bbox.miny >= starty+height)
  {
    // Bounding box of rectangle is out of range.
    return false;
  }

  // If rect_maxdepth is smaller than max_depth we will return true.
  // In that case the rectangle may be visible but the maximum depth
  // doesn't have to be modified.
  if (rect_maxdepth <= max_depth)
    return true;

//@@@


  // Update max_depth and min_depth if needed.
  if (rect_maxdepth > max_depth) max_depth = rect_maxdepth;
  if (rect_mindepth < min_depth) min_depth = rect_mindepth;

  return false;
}

bool csDepthBuffer::InsertRectangle (const csBox2& bbox,
	float rect_mindepth, float rect_maxdepth)
{
  csBox2Int bboxi;
  bboxi.minx = QRound (bbox.MinX ());
  bboxi.miny = QRound (bbox.MinY ());
  bboxi.maxx = QRound (bbox.MaxX ());
  bboxi.maxy = QRound (bbox.MaxY ());

  return InsertRectangle (bboxi, rect_mindepth, rect_maxdepth);
}

bool csDepthBuffer::InsertPolygon (csVector2Int* verts, int num_verts,
  	const csBox2Int& bbox, float poly_mindepth, float poly_maxdepth)
{
  if (bbox.maxx < startx || bbox.maxy < starty ||
  	bbox.minx >= startx+width || bbox.miny >= starty+height)
  {
    // Bounding box of polygon is out of range.
    return false;
  }

//@@@
  return true;
}

bool csDepthBuffer::InsertPolygon (csVector2* verts, int num_verts,
	const csBox2& bbox, float poly_mindepth, float poly_maxdepth)
{
  // If poly_maxdepth is smaller than max_depth we will return true.
  // In that case the polygon may be visible but the maximum depth
  // doesn't have to be modified.
  if (poly_maxdepth <= max_depth)
    return true;

  // If poly_mindepth <= max_depth we also want to return true but in
  // that case we possibly have to update the max_depth value of this
  // node. So we have to proceed further here...

  csBox2Int bboxi;
  csVector2Int vertsi[256];
  CS_ASSERT (num_verts >= 3);
  CS_ASSERT (num_verts < 256);
  int i;
  for (i = 0 ; i < num_verts ; i++)
  {
    vertsi[i].x = QRound (verts[i].x);
    vertsi[i].y = QRound (verts[i].y);
  }
  bboxi.minx = QRound (bbox.MinX ());
  bboxi.miny = QRound (bbox.MinY ());
  bboxi.maxx = QRound (bbox.MaxX ());
  bboxi.maxy = QRound (bbox.MaxY ());

  return InsertPolygon (vertsi, num_verts, bboxi, poly_mindepth, poly_maxdepth);
}

iString* csDepthBuffer::Debug_UnitTest ()
{
  scfString* rc = new scfString ();



  rc->DecRef ();
  return NULL;
}

