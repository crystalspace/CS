/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#include <math.h>
#include <stdarg.h>

#include "cssysdef.h"
#include "polybuf.h"
#include "csutil/util.h"
#include "qint.h"

csPolArrayPolygonBuffer::csPolArrayPolygonBuffer (iVertexBufferManager* mgr)
	: csPolygonBuffer (mgr)
{
}

csPolArrayPolygonBuffer::~csPolArrayPolygonBuffer ()
{
  int i;
  for (i = 0 ; i < polygons.Length () ; i++)
  {
    int* pol = polygons[i];
    delete[] pol;
  }
}

void csPolArrayPolygonBuffer::AddPolygon (int* verts, int num_verts)
{
  int* newpol = new int [num_verts];
  memcpy (newpol, verts, num_verts * sizeof (int));
  polygons.Push (newpol);
  num_vertices.Push (num_verts);
}

void csPolArrayPolygonBuffer::ClearPolygons ()
{
  int i;
  for (i = 0 ; i < polygons.Length () ; i++)
  {
    int* pol = polygons[i];
    delete[] pol;
  }
  polygons.SetLength (0);
  num_vertices.SetLength (0);
}

csPolArrayVertexBufferManager::csPolArrayVertexBufferManager
	(iObjectRegistry* object_reg) : csVertexBufferManager (object_reg)
{
}

csPolArrayVertexBufferManager::~csPolArrayVertexBufferManager()
{
}

iPolygonBuffer* csPolArrayVertexBufferManager::CreatePolygonBuffer ()
{
  csPolArrayPolygonBuffer* buf = new csPolArrayPolygonBuffer (this);
  return buf;
}

