/*
    Crystal Space 3D engine
    Copyright (C) 2000 by Jorrit Tyberghein
  
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

#ifndef __IENGINE_POLYMESH_H__
#define __IENGINE_POLYMESH_H__

#include "csutil/scf.h"

/**
 * A polygon. Note that this structure is only valid if used
 * in combination with a vertex table. The vertex array then
 * contains indices in that table.
 */
struct csMeshedPolygon
{
  int num_vertices;
  int* vertices;
};

class csVector3;

SCF_VERSION (iPolygonMesh, 0, 1, 0);

/**
 * This interface reprents a mesh of polygons. It is useful to communicate
 * geometry information outside of the engine. One place where this will
 * be useful is for communicating geometry information to the collision
 * detection plugin.<br>
 * All Crystal Space entities classes (csSector, csThing, csSprite, ...)
 * should implement and/or embed an implementation of this interface.<p>
 *
 * A polygon mesh has the concept of a vertex buffer and an array of polygons.
 */
struct iPolygonMesh : public iBase
{
  /// Get the number of vertices for this mesh.
  virtual int GetVertexCount () = 0;
  /// Get the pointer to the array of vertices.
  virtual csVector3* GetVertices () = 0;
  /// Get the number of polygons for this mesh.
  virtual int GetPolygonCount () = 0;
  /// Get the pointer to the array of polygons.
  virtual csMeshedPolygon* GetPolygons () = 0;
};

#endif // __IENGINE_POLYMESH_H__

