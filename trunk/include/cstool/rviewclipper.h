/*
  Copyright (C) 2007 by Jorrit Tyberghein

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

#ifndef __CSTOOL_RVIEW_CLIPPER_H__
#define __CSTOOL_RVIEW_CLIPPER_H__

#include "csextern.h"
#include "csutil/csstring.h"
#include "csgeom/transfrm.h"

class csRenderContext;
class csBox3;

namespace CS
{
  /**
   * Helper to clip in the context of a render view.
   * This class is mostly useful for mesh object implementors.
   */
  class CS_CRYSTALSPACE_EXPORT RenderViewClipper
  {
  private:
    /**
     * Given a csRenderContext (with frustum) and a bounding sphere calculate if
     * the sphere is fully inside and fully outside that frustum.
     * Works in world space.
     */
    static void TestSphereFrustumWorld (csRenderContext* frust,
      const csVector3& center, float radius, bool& inside, bool& outside);

  public:
    /**
     * Given a frustum_mask, calculate the clip settings.
     */
    static void CalculateClipSettings (csRenderContext* ctxt,
	uint32 frustum_mask, int &clip_portal, int &clip_plane,
	int &clip_z_plane);

    /**
     * Test if the given bounding sphere (in world space coordinates)
     * is visible for the render context. The optional will
     * transform world to camera space.
     */
    static bool TestBSphere (
      csRenderContext* ctxt,
      const csReversibleTransform &w2c,
      const csSphere &sphere);

    /**
     * Check if the given bounding sphere (in camera and world space coordinates)
     * is visibile for the render context. If the sphere is visible this
     * function will also initialize the clip_plane, clip_z_plane, and
     * clip_portal fields which can be used for the renderer.
     */
    static bool CullBSphere (
      csRenderContext* ctxt,
      const csSphere &cam_sphere,
      const csSphere &world_sphere,
      int& clip_portal, int& clip_plane, int& clip_z_plane);

    /**
     * Check if the object space bounding box of an object is visible in this
     * render view. If true is returned (visible) then clip_plane,
     * clip_z_plane, and clip_portal will be set to the right value depending
     * on wether or not clipping is wanted. This function also does far
     * plane clipping. Use SetupClipPlanes() to get the clipping planes
     * for this function.
     * The frustum_mask will be modified according to all clip planes
     * that were relevant for the given box. That can be used to hierarchically
     * cull smaller objects.
     */
    static bool CullBBox (
      const csRenderContext* ctxt,
      const csPlane3* planes, uint32& frustum_mask,
      const csBox3& obox,
      int& clip_portal, int& clip_plane, int& clip_z_plane);

    /**
     * Setup clipping planes in object space. The input arrays for planes
     * should each be able to hold 10 planes. Returns a mask that you can
     * use for the csIntersect3::BoxFrustum() function.
     * \deprecated Use CS::RenderViewClipper::SetupClipPlanes() instead.
     */
    static void SetupClipPlanes (
      csRenderContext* ctxt,
      const csReversibleTransform& tr_o2c,
      csPlane3* planes, uint32& frustum_mask);

    /**
     * Setup the clip planes for the current context and camera (in world space).
     */
    static void SetupClipPlanes (csRenderContext* ctxt);
  };
}

#endif // __CSTOOL_RVIEW_CLIPPER_H__

