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

#ifndef __POLYBUF_H__
#define __POLYBUF_H__

#include "csutil/garray.h"
#include "plugins/video/renderer/common/vbufmgr.h"

/**
 * This implementation of csPolygonBuffer just keeps the polygons
 * as an array of polygons. It is the most efficient representation for
 * the software renderer.
 */
class csPolArrayPolygonBuffer : public csPolygonBuffer
{
protected:
  CS_DECLARE_GROWING_ARRAY (num_vertices, int);
  typedef int* intp;
  CS_DECLARE_GROWING_ARRAY (polygons, intp);

public:
  ///
  csPolArrayPolygonBuffer (iVertexBufferManager* mgr);
  ///
  virtual ~csPolArrayPolygonBuffer ();

  virtual void AddPolygon (int* verts, int num_verts);
  virtual void ClearPolygons ();
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

#endif // __POLYBUF_H__

