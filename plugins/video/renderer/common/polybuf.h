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

#ifndef __CS_POLYBUF_H__
#define __CS_POLYBUF_H__

#include "csgeom/vector3.h"
#include "csgeom/plane3.h"
#include "csgeom/matrix3.h"
#include "csgeom/transfrm.h"
#include "csutil/garray.h"
#include "plugins/video/renderer/common/vbufmgr.h"

class csPolArrayPolygon
{
public:
  int num_vertices;
  int* vertices;
  csPlane3 normal;
  //csTransform t_obj2tex;
  //csTransform t_obj2lm;
  int mat_index;
  csPolyTextureMapping* texmap;
  iRendererLightmap* rlm;
};

/**
 * This implementation of csPolygonBuffer just keeps the polygons
 * as an array of polygons. It is the most efficient representation for
 * the software renderer.
 */
class csPolArrayPolygonBuffer : public csPolygonBuffer
{
protected:
  csDirtyAccessArray<csPolArrayPolygon> polygons;
  typedef iMaterialHandle* iMaterialHandleP;
  csDirtyAccessArray<iMaterialHandleP> materials;

  csVector3* vertices;
  int num_vertices;
  csBox3 bbox;

public:
  ///
  csPolArrayPolygonBuffer (iVertexBufferManager* mgr);
  ///
  virtual ~csPolArrayPolygonBuffer ();

  /// Get the number of polygons.
  int GetPolygonCount () const { return polygons.Length (); }
  /// Get the polygon info.
  const csPolArrayPolygon& GetPolygon (int i) const { return polygons[i]; }
  /// Get the number of vertices.
  virtual int GetVertexCount () const { return num_vertices; }
  /// Get the vertices array.
  virtual csVector3* GetVertices () const { return vertices; }

  virtual void AddMaterial (iMaterialHandle* mat_handle);
  virtual int GetMaterialCount () const { return materials.Length (); }
  virtual iMaterialHandle* GetMaterial (int idx) const
  {
    return materials[idx];
  }
  virtual void SetMaterial (int idx, iMaterialHandle* mat_handle);
  virtual void SetVertexArray (csVector3* verts, int num_verts);
  virtual void Clear ();
  //Does nothing as default
  virtual void MarkLightmapsDirty();
  virtual const csBox3& GetBoundingBox () const { return bbox; }

  virtual void AddPolygon (int num_verts,
	int* verts,
	//csVector2* texcoords,
	//csVector2* lmcoords,
	csPolyTextureMapping* texmap,
	const csPlane3& poly_normal,
	int mat_index,
	iRendererLightmap* lm);
};

/**
 * Version of the vertex buffer manager that understands
 * csPolArrayPolygonBuffer.
 */
class csPolArrayVertexBufferManager : public csVertexBufferManager
{
public:
  /// Initialize the vertex buffer manager
  csPolArrayVertexBufferManager (iObjectRegistry* object_reg);
  /// Destroy the vertex buffer manager
  virtual ~csPolArrayVertexBufferManager ();

  virtual iPolygonBuffer* CreatePolygonBuffer ();
};

#endif // __CS_POLYBUF_H__

