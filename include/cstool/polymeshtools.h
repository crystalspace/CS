/*
    iPolygonMesh tool functions
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#ifndef __CS_CSTOOL_POLYMESHTOOLS_H__
#define __CS_CSTOOL_POLYMESHTOOLS_H__

/**\file
 * iPolygonMesh tool functions
 */

#include "igeom/polymesh.h"
#include "csutil/array.h" 
 
/**
 * \addtogroup geom_utils
 * @{ */

struct iPolygonMesh; 
 
class csPolyMeshTools
{
public:
  /**
   * Test whether a polygon mesh is closed.
   * \remark This function works best if vertices are shared.
   */
  static bool IsMeshClosed (iPolygonMesh* polyMesh);

  /**
   * Close a polygon mesh.
   * The current implementation is rather naive; it just returns all faces,
   * but flipped.
   * \remark Don't forget to delete[] the 'vertices' fields of the polygons
   *   returned in \p newPolys when you're done.
   */
  static void CloseMesh (iPolygonMesh* polyMesh, 
    csArray<csMeshedPolygon>& newPolys);
};
 
/** @} */


#endif // __CS_CSTOOL_POLYMESHTOOLS_H__
