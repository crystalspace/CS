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
#include "imesh/thing/polygon.h"
#include "qint.h"

csPolArrayPolygonBuffer::csPolArrayPolygonBuffer (iVertexBufferManager* mgr)
	: csPolygonBuffer (mgr)
{
  vertices = NULL;
}

csPolArrayPolygonBuffer::~csPolArrayPolygonBuffer ()
{
  Clear ();
}

void csPolArrayPolygonBuffer::AddPolygon (int* verts, int num_verts,
	const csPlane3& poly_normal,
  	int mat_index,
	const csMatrix3& m_obj2tex, const csVector3& v_obj2tex,
	iPolygonTexture* poly_texture)
{
  csPolArrayPolygon pol;
  pol.num_vertices = num_verts;
  pol.vertices = new int [num_verts];
  memcpy (pol.vertices, verts, num_verts * sizeof (int));
  pol.normal = poly_normal;
  pol.m_obj2tex = m_obj2tex;
  pol.v_obj2tex = v_obj2tex;
  pol.mat_index = mat_index;
  pol.poly_texture = poly_texture;
  poly_texture->IncRef ();
  polygons.Push (pol);
}

void csPolArrayPolygonBuffer::SetVertexArray (csVector3* verts, int num_verts)
{
  delete[] vertices;
  num_vertices = num_verts;
  vertices = new csVector3 [num_verts];
  memcpy (vertices, verts, num_verts * sizeof (csVector3));
}

void csPolArrayPolygonBuffer::AddMaterial (iMaterialHandle* mat_handle)
{
  materials.Push (mat_handle);
}

void csPolArrayPolygonBuffer::SetMaterial (int idx,
	iMaterialHandle* mat_handle)
{
  materials[idx] = mat_handle;
}

void csPolArrayPolygonBuffer::Clear ()
{
  int i;
  for (i = 0 ; i < polygons.Length () ; i++)
  {
    csPolArrayPolygon& pol = polygons[i];
    delete[] pol.vertices;
    pol.poly_texture->DecRef ();
  }
  polygons.SetLength (0);

  materials.SetLength (0);

  delete[] vertices; vertices = NULL;
  num_vertices = 0;
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

