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
  vertices = 0;
}

csPolArrayPolygonBuffer::~csPolArrayPolygonBuffer ()
{
  Clear ();
}

void csPolArrayPolygonBuffer::SetVertexArray (csVector3* verts, int num_verts)
{
  delete[] vertices;
  num_vertices = num_verts;
  vertices = new csVector3 [num_verts];
  memcpy (vertices, verts, num_verts * sizeof (csVector3));
  bbox.StartBoundingBox (vertices[0]);
  int i;
  for (i = 1 ; i < num_verts ; i++)
    bbox.AddBoundingVertexSmart (vertices[i]);
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
    //delete[] pol.lmcoords;
    //delete[] pol.texcoords;
    //if (pol.poly_texture)
      //pol.poly_texture->DecRef ();
  }
  polygons.SetLength (0);

  materials.SetLength (0);

  delete[] vertices; vertices = 0;
  num_vertices = 0;
}

void csPolArrayPolygonBuffer::MarkLightmapsDirty()
{};

void csPolArrayPolygonBuffer::AddPolygon (int num_verts,
				  	  int* verts,
					  csPolyTextureMapping* texmap,
					  const csPlane3& poly_normal,
					  int mat_index,
					  iRendererLightmap* lm)
{
  csPolArrayPolygon pol;
  pol.num_vertices = num_verts;
  pol.vertices = new int [num_verts];
  memcpy (pol.vertices, verts, num_verts * sizeof (int));
  /*if (lmcoords)
  {
    pol.lmcoords = new csVector2 [num_verts];
    memcpy (pol.lmcoords, lmcoords, num_verts * sizeof (csVector2));
  }
  else
  {
    pol.lmcoords = 0;
  }
  pol.texcoords = new csVector2 [num_verts];
  memcpy (pol.texcoords, texcoords, num_verts * sizeof (csVector2));*/
  //pol.t_obj2tex = t_obj2tex;
  //pol.t_obj2lm = t_obj2lm;
  pol.texmap = texmap;
  pol.normal = poly_normal;
  pol.mat_index = mat_index;
  pol.rlm = lm;
  polygons.Push (pol);
/*  pol.m_obj2tex = m_obj2tex;
  pol.v_obj2tex = v_obj2tex;
  pol.mat_index = mat_index;
  pol.poly_texture = poly_texture;
  if (poly_texture)
    poly_texture->IncRef ();
  polygons.Push (pol);*/
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

