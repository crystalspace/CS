/*
  Copyright (C) 2004 by Frank Richter

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

/**\file
 * Some helpers for stuff related to normal maps.
 */
 
/**\addtogroup gfx
 * @{ 
 */

#ifndef __CS_CSGFX_NORMALMAPTOOLS_H__
#define __CS_CSGFX_NORMALMAPTOOLS_H__

#include "csextern.h"

struct csTriangle;
class csVector3;
class csVector2;

/**
 * Some helpers for stuff related to normal maps.
 */
class CS_CRYSTALSPACE_EXPORT csNormalMappingTools
{
public:
  /**
   * Calculate tangents & bitangents for a triangle mesh.
   */
  static void CalculateTangents (size_t numTriangles, 
    const csTriangle* triangles, size_t numVertices, const csVector3* vertices,
    const csVector3* normals, const csVector2* texcoords, 
    csVector3* outTangents, csVector3* outBitangents);
};

/** @} */

#endif // __CS_CSGFX_NORMALMAPTOOLS_H__
