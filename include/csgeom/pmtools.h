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

#ifndef __CS_PMTOOLS_H__
#define __CS_PMTOOLS_H__

class csVector3;
struct iPolygonMesh;

/**
 * A set of tools to work with iPolygonMesh instances.
 */
class csPolygonMeshTools
{
public:
  /**
   * This function will calculate normals for all polygons in the mesh.
   * The given array of 'normals' should be big enough to have normals
   * for the number of polygons as defined in the mesh itself.
   */
  static void CalculateNormals (iPolygonMesh* mesh, csVector3* normals);
};

#endif // __CS_PMTOOLS_H__

