/*
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

#ifndef __CS_IVIDEO_POLYRENDER_H__
#define __CS_IVIDEO_POLYRENDER_H__

/**\file
 */

/**
 * \addtogroup gfx3d
 * @{ */
 
#include "csutil/scf.h"
#include "csgeom/plane3.h"
#include "csgeom/matrix3.h"
#include "csgeom/vector3.h"

struct iRenderBufferSource;

/**
 * This structure is used for communicating polygon information to the
 * polygon renderer.
 */
struct csPolygonRenderData
{
  /// Object space plane of the polygon.
  csPlane3 plane_obj;
  /// Texture mapping information.
  csPolyTextureMapping* tmapping;
  /// Number of vertices in this polygon.
  int num_vertices;
  /// Pointer to vertex indices.
  int* vertices;
  /**
   * Double pointer to the array of vertices in object space.
   */
  csVector3** p_obj_verts;
  /// Poly uses lightmap
  bool useLightmap;
};



SCF_VERSION (iPolygonRenderer, 0, 1, 0);

// @@@ Document me.
struct iPolygonRenderer : public iBase
{
  /*virtual iRenderBufferSource* GetBufferSource (uint& indexStart, 
    uint& indexEnd) = 0;*/
  virtual void PrepareRenderMesh (csRenderMesh& mesh) = 0;
  
  virtual void Clear () = 0;
  virtual void AddPolygon (csPolygonRenderData* poly) = 0;
};

/** @} */

#endif // __CS_IVIDEO_POLYRENDER_H__

