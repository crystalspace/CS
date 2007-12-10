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

#include "cssysdef.h"
#include "csgeom/sphere.h"
#include "cstool/rviewclipper.h"
#include "ivideo/graph3d.h"
#include "iengine/rview.h"
#include "iengine/camera.h"
#include <ctype.h>

namespace CS
{
  void RenderViewClipper::TestSphereFrustumWorld (
    csRenderContext *ctxt,
    const csVector3 &center,
    float radius,
    bool &inside,
    bool &outside)
  {
    float dist;
    csPlane3 *frust = ctxt->clip_planes;
    outside = true;
    inside = true;

    dist = frust[0].Classify (center);
    if (dist < radius) inside = false;
    if ((-dist) <= radius)
    {
      dist = frust[1].Classify (center);
      if (dist < radius) inside = false;
      if ((-dist) <= radius)
      {
        dist = frust[2].Classify (center);
        if (dist < radius) inside = false;
        if ((-dist) <= radius)
        {
          dist = frust[3].Classify (center);
          if (dist < radius) inside = false;
          if ((-dist) <= radius) outside = false;
        }
      }
    }
  }

  void RenderViewClipper::CalculateClipSettings (
      csRenderContext* ctxt,
      uint32 frustum_mask,
      int &clip_portal, int &clip_plane, int &clip_z_plane)
  {
    if (frustum_mask & 0xf)
      clip_portal = CS_CLIP_NEEDED;
    else
      clip_portal = CS_CLIP_NOT;

    if (frustum_mask & 0x10)
      clip_z_plane = CS_CLIP_NEEDED;
    else
      clip_z_plane = CS_CLIP_NOT;

    // Only if we really need an exact clip plane will we set clip_plane
    // to needed (depending on frustum mask).
    if ((frustum_mask & 0x20) && ctxt->do_clip_plane)
      clip_plane = CS_CLIP_NEEDED;
    else
      clip_plane = CS_CLIP_NOT;
  }

  bool RenderViewClipper::TestBSphere (
    csRenderContext* ctxt,
    const csReversibleTransform &w2c,
    const csSphere &sphere)
  {
    //------
    // First transform bounding sphere from object space to camera space
    // by using the given transform (if needed).
    //------
    csSphere tr_sphere = w2c.Other2This (sphere);
    const csVector3 &tr_center = tr_sphere.GetCenter ();
    float radius = tr_sphere.GetRadius ();

    //------
    // Test if object is behind the camera plane.
    //------
    if (tr_center.z + radius <= 0) return false;

    //------
    // Test against far plane if needed.
    //------
    csPlane3 *far_plane = ctxt->icamera->GetFarPlane ();
    if (far_plane)
    {
      // Ok, so this is not really far plane clipping - we just dont draw this
      // object if the bounding sphere is further away than the D
      // part of the farplane.
      if (tr_center.z - radius > far_plane->D ()) return false;
    }

    //------
    // Check if we're fully inside the bounding sphere.
    //------
    bool fully_inside = csSquaredDist::PointPoint (
        csVector3 (0),
        tr_center) <= radius *
      radius;

    //------
    // Test if there is a chance we must clip to current portal.
    //------
    bool outside, inside;
    if (!fully_inside)
    {
      TestSphereFrustumWorld (
        ctxt,
        sphere.GetCenter (),
        radius,
        inside,
        outside);
      if (outside) return false;
    }

    //------
    // Test if there is a chance we must clip to current plane.
    //------
    if (ctxt->do_clip_plane)
    {
      //bool mirror = GetCamera ()->IsMirrored ();
      float dist = ctxt->clip_plane.Classify (tr_center);
      dist = -dist;
      if ((-dist) > radius) return false;
    }

    return true;
  }

  bool RenderViewClipper::CullBSphere (
    csRenderContext* ctxt,
    const csSphere &cam_sphere,
    const csSphere &world_sphere,
    int &clip_portal,
    int &clip_plane,
    int &clip_z_plane)
  {
    float radius = cam_sphere.GetRadius ();
    float cam_z = cam_sphere.GetCenter ().z;

    //------
    // Test if object is behind the camera plane.
    //------
    if (cam_z + radius <= 0) return false;

    //------
    // Test against far plane if needed.
    //------
    csPlane3 *far_plane = ctxt->icamera->GetFarPlane ();
    if (far_plane)
    {
      // Ok, so this is not really far plane clipping - we just dont draw this
      // object if the bounding sphere is further away than the D
      // part of the farplane.
      if (cam_z - radius > far_plane->D ()) return false;
    }

    //------
    // Check if we're fully inside the bounding sphere.
    //------
    bool fully_inside = csSquaredDist::PointPoint (
          csVector3 (0), cam_sphere.GetCenter ()) <= radius * radius;

    //------
    // Test if there is a chance we must clip to current portal.
    //------
    bool outside, inside;
    if (fully_inside)
    {
      clip_portal = CS_CLIP_NEEDED;
    }
    else
    {
      TestSphereFrustumWorld (
        ctxt,
        world_sphere.GetCenter (),
        radius,
        inside,
        outside);
      if (outside)
        return false;
      if (!inside)
        clip_portal = CS_CLIP_NEEDED;
      else
        clip_portal = CS_CLIP_NOT;
    }

    //------
    // Test if there is a chance we must clip to the z-plane.
    //------
    if (cam_z - radius > 0)
      clip_z_plane = CS_CLIP_NOT;
    else
      clip_z_plane = CS_CLIP_NEEDED;

    //------
    // Test if there is a chance we must clip to current plane.
    //------
    clip_plane = CS_CLIP_NOT;
    if (ctxt->do_clip_plane)
    {
      float dist = ctxt->clip_plane.Classify (cam_sphere.GetCenter ());
      dist = -dist;
      if ((-dist) > radius)
        return false;
      else if (dist <= radius)
        clip_plane = CS_CLIP_NEEDED;
    }

    return true;
  }

  bool RenderViewClipper::CullBBox (
    const csRenderContext* ctxt,
    const csPlane3* planes,
    uint32& frustum_mask,
    const csBox3 &obox,
    int &clip_portal,
    int &clip_plane,
    int &clip_z_plane)
  {
    uint32 outClipMask;
    if (!csIntersect3::BoxFrustum (obox, planes, frustum_mask, outClipMask))
      return false;	// Not visible.
    frustum_mask = outClipMask;

    if (outClipMask & 0xf)
      clip_portal = CS_CLIP_NEEDED;
    else
      clip_portal = CS_CLIP_NOT;

    if (outClipMask & 0x10)
      clip_z_plane = CS_CLIP_NEEDED;
    else
      clip_z_plane = CS_CLIP_NOT;

    // Only if we really need an exact clip plane will we set clip_plane
    // to needed (depending on frustum mask).
    if (ctxt->do_clip_plane && (outClipMask & 0x20))
      clip_plane = CS_CLIP_NEEDED;
    else
      clip_plane = CS_CLIP_NOT;

    return true;
  }

  void RenderViewClipper::SetupClipPlanes (
      csRenderContext* ctxt,
      const csReversibleTransform& tr_o2c,
      csPlane3* planes, uint32& frustum_mask)
  {
    csPlane3* frust = ctxt->frustum;
    csVector3 o2tmult = tr_o2c.GetO2T () * tr_o2c.GetO2TTranslation ();
    planes[0].Set (tr_o2c.GetT2O() * frust[0].norm, -frust[0].norm*o2tmult);
    planes[1].Set (tr_o2c.GetT2O() * frust[1].norm, -frust[1].norm*o2tmult);
    planes[2].Set (tr_o2c.GetT2O() * frust[2].norm, -frust[2].norm*o2tmult);
    planes[3].Set (tr_o2c.GetT2O() * frust[3].norm, -frust[3].norm*o2tmult);
    csPlane3 pz0 (0, 0, 1, 0);	// Inverted!!!.
    planes[4] = tr_o2c.This2Other (pz0);

    // Even if we don't need the clip plane we will still add it to the
    // frustum_mask so that the cullers will use it to trivially reject
    // objects and nodes.
    csPlane3 pznear = ctxt->clip_plane;
    pznear.Invert ();
    planes[5] = tr_o2c.This2Other (pznear);
    frustum_mask = 0x3f;

    csPlane3 *far_plane = ctxt->icamera->GetFarPlane ();
    if (far_plane)
    {
      csPlane3 fp = *far_plane;
      planes[6] = tr_o2c.This2Other (fp);
      frustum_mask |= 0x40;
    }
  }

  void RenderViewClipper::SetupClipPlanes (csRenderContext* ctxt)
  {
    csPlane3* frust = ctxt->frustum;
    const csReversibleTransform& tr = ctxt->icamera->GetTransform ();
    csVector3 o2tmult = tr.GetO2T () * tr.GetO2TTranslation ();
    csPlane3* clip_planes = ctxt->clip_planes;
    clip_planes[0].Set (tr.GetT2O() * frust[0].norm, -frust[0].norm*o2tmult);
    clip_planes[1].Set (tr.GetT2O() * frust[1].norm, -frust[1].norm*o2tmult);
    clip_planes[2].Set (tr.GetT2O() * frust[2].norm, -frust[2].norm*o2tmult);
    clip_planes[3].Set (tr.GetT2O() * frust[3].norm, -frust[3].norm*o2tmult);
    csPlane3 pz0 (0, 0, 1, 0);	// Inverted!!!.
    clip_planes[4] = tr.This2Other (pz0);

    // Even if we don't need the clip plane we will still add it to the
    // frustum_mask so that the cullers will use it to trivially reject
    // objects and nodes.
    csPlane3 pznear = ctxt->clip_plane;
    pznear.Invert ();
    clip_planes[5] = tr.This2Other (pznear);
    ctxt->clip_planes_mask = 0x3f;

    csPlane3 *far_plane = ctxt->icamera->GetFarPlane ();
    if (far_plane)
    {
      csPlane3 fp = *far_plane;
      clip_planes[6] = tr.This2Other (fp);
      ctxt->clip_planes_mask |= 0x40;
    }
  }

}

