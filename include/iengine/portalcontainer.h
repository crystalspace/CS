/*
    Copyright (C) 2003 by Jorrit Tyberghein
              (C) 2004 by Marten Svanfeldt

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

#ifndef __CS_IENGINE_PORTALCONTAINER_H__
#define __CS_IENGINE_PORTALCONTAINER_H__

/**\file
 * Portal container
 */

#include "csutil/scf.h"

/**
 * \addtogroup engine3d
 * @{ */

struct iPortal;
struct iRenderView;

class csVector2;

/**
 * A container for portals.
 * 
 * Main creators of instances implementing this interface:
 * - iEngine::CreatePortalContainer()
 * - iEngine::CreatePortal()
 * 
 * Main ways to get pointers to this interface:
 * - scfQueryInterface<iMeshObject>() from a portal container mesh.
 * 
 * Main users of this interface:
 * - iEngine
 */
struct iPortalContainer : public virtual iBase
{
  SCF_INTERFACE(iPortalContainer, 3,0,0);
  /// Get the number of portals in this contain.
  virtual int GetPortalCount () const = 0;

  /// Get a specific portal.
  virtual iPortal* GetPortal (int idx) const = 0;

  /// Create a new portal.
  virtual iPortal* CreatePortal (csVector3* vertices, int num) = 0;

  /// Remove a portal.
  virtual void RemovePortal (iPortal* portal) = 0;

  /// Render the portal container
  virtual void Draw (iRenderView* rview) = 0;
  
  /**
   * Compute the normalized screen-space and camera-space polygons for all 
   * portals.
   * \param rview Render view for which to compute the screen space polys.
   * \param verts2D Output buffer receiving the normalized screen space 
   *   coordinates.
   * \param verts3D Output buffer receiving the camera space coordinates
   *   corresponding to the screen space coordinates.
   * \param vertsSize Size of the \a verts buffer.
   * \param numVerts Output buffer receiving the number of vertices in each
   *   polygon.
   * \param viewWidth Width of the view in which the polys are computed.
   * \param viewHeight Height of the view in which the polys are computed.
   * \remarks The polygon vertices are stored in a flat fashion. To obtain
   *  the vertices for a certain polygon, sum up the vertex numbers for all
   *  previous polygons and use that as an index into the vertices array.
   * \remarks Portals that face away from the camera, are culled etc. will
   *  result in polygons with 0 vertices.
   */
  virtual void ComputeScreenPolygons (iRenderView* rview,
    csVector2* verts2D, csVector3* verts3D, size_t vertsSize,
    size_t* numVerts, int viewWidth, int viewHeight) = 0;
  
  /**
   * Get the total amount of vertices used by all portals.
   */
  virtual size_t GetTotalVertexCount () const = 0;
};

/** @} */

#endif // __CS_IENGINE_PORTALCONTAINER_H__

