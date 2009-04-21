/*
    Copyright (C) 2005 by Jorrit Tyberghein

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

#ifndef __CS_NORMALCALC_H__
#define __CS_NORMALCALC_H__

/**\file
 * Normal Calculator
 */

#include "csextern.h"

#include "csgeom/vector3.h"
#include "csgeom/vector2.h"
#include "csutil/cscolor.h"
#include "csutil/dirtyaccessarray.h"

/**
 * A normal calculator.
 */
class CS_CRYSTALSPACE_EXPORT csNormalCalculator
{
private:
  static bool CompressVertices (
    csVector3* orig_verts, size_t orig_num_vts,
    csVector3*& new_verts, size_t& new_num_vts,
    csTriangle* orig_tris, size_t num_tris,
    csTriangle*& new_tris,
    size_t*& mapping);

public:
  /**
   * Calculate normals for the given mesh.
   * \param mesh_vertices Vertices of the mesh.
   * \param mesh_triangles Triangles of the mesh.
   * \param mesh_normals Normals of the mesh.
   * \param do_compress if true then the vertices will be
   *  compressed (equal vertices collapsed) before calculating normals.
   *  If false then this will not happen which means that you can have
   *  seams.
   */
  static void CalculateNormals (
    csDirtyAccessArray<csVector3>& mesh_vertices,
    csDirtyAccessArray<csTriangle>& mesh_triangles,
    csDirtyAccessArray<csVector3>& mesh_normals,
    bool do_compress);
};

/** @} */

#endif // __CS_NORMALCALC_H__

