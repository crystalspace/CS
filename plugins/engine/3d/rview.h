/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#ifndef __CS_RVIEW_H__
#define __CS_RVIEW_H__

#include "csgeom/math3d.h"
#include "csgeom/frustum.h"
#include "plugins/engine/3d/camera.h"
#include "iengine/rview.h"
#include "iengine/engine.h"

class csMatrix3;
class csVector3;
class csRenderView;
struct csFog;
struct iGraphics3D;
struct iGraphics2D;
struct iSector;
struct iClipper2D;

/**
 * This structure represents all information needed for drawing
 * a scene. It is modified while rendering according to
 * portals/warping portals and such.
 */
class csRenderView : public iRenderView
{
private:
  /**
   * The following id is used to populate the context_id in every
   * csRenderContext.
   */
  uint32 context_id;

  /// The current render context.
  csRenderContext* ctxt;

  /// Engine handle.
  csEngine* engine;
  /// The 3D graphics subsystem used for drawing.
  iGraphics3D* g3d;
  /// The 2D graphics subsystem used for drawing.
  iGraphics2D* g2d;
  /**
   * A copy to the original base camera before space warping.
   */
  iCamera* original_camera;

  /// The view frustum as defined at z=1.
  float leftx, rightx, topy, boty;

  /**
   * Update the frustum of the current context to the current clipper.
   */
  void UpdateFrustum ();

  /**
   * Given a csRenderContext (with frustum) and a bounding sphere calculate if
   * the sphere is fully inside and fully outside that frustum.
   * Works in world space.
   */
  static void TestSphereFrustumWorld (csRenderContext* frust,
    const csVector3& center, float radius, bool& inside, bool& outside);

public:
  ///
  csRenderView ();
  ///
  csRenderView (iCamera* c);
  ///
  csRenderView (iCamera* c, iClipper2D* v, iGraphics3D* ig3d,
    iGraphics2D* ig2d);

  virtual ~csRenderView ();

  /// Set the engine.
  void SetEngine (csEngine* engine);
  /// Set the camera.
  void SetCamera (iCamera* camera);
  /// Set the original camera.
  void SetOriginalCamera (iCamera* camera);
  /// Get the original camera.
  virtual iCamera* GetOriginalCamera () const { return original_camera; }

  /// Setup the clip planes for the current context and camera (in world space).
  void SetupClipPlanes ();

  /// Get the current render context.
  csRenderContext* GetCsRenderContext () const { return ctxt; }
  /// Set the current render context (only for temporary override).
  void SetCsRenderContext (csRenderContext* c) { ctxt = c; }

  /**
   * Create a new render context. This is typically used
   * when going through a portal. Note that you should remember
   * the old render context if you want to restore it later.
   * The render context will get all the values from the current context
   * (with SCF references properly incremented).
   */
  void CreateRenderContext ();
  /**
   * Restore a render context. Use this to restore a previously overwritten
   * render context. This function will take care of properly cleaning
   * up the current render context.
   */
  void RestoreRenderContext ();

  /**
   * Create a new camera in the current render context. This function
   * will create a new camera based on the current one. The new camera
   * reference is returned.
   */
  iCamera* CreateNewCamera ();

  /**
   * Set the previous sector.
   */
  void SetPreviousSector (iSector* s) { ctxt->previous_sector = s; }
  /**
   * Set the current sector.
   */
  void SetThisSector (iSector* s) { ctxt->this_sector = s; }

  /**
   * Get render recursion level.
   */
  int GetRenderRecursionLevel () const { return ctxt->draw_rec_level; }
  /**
   * Set render recursion level.
   */
  void SetRenderRecursionLevel (int rec)
  {
    ctxt->draw_rec_level = rec;
  }
  /// Set the last portal.
  void SetLastPortal (iPortal* por)
  {
    ctxt->last_portal = por;
  }
  /// Set the 2D clipper for this view.
  void SetClipper (iClipper2D* clip);
  /// Set the view frustum at z=1.
  void SetFrustum (float lx, float rx, float ty, float by);

  ///
  void UseClipPlane (bool u) { ctxt->do_clip_plane = u; }
  ///
  void UseClipFrustum (bool u) { ctxt->do_clip_frustum = u; }
  /**
   * Set the 3D clip plane that should be used to clip all geometry.
   */
  void SetClipPlane (const csPlane3& p) { ctxt->clip_plane = p; }
  /**
   * Get the 3D clip plane that should be used to clip all geometry.
   * If this function returns false then this plane is invalid and should
   * not be used. Otherwise it must be used to clip the object before
   * drawing.
   */
  bool GetClipPlane (csPlane3& pl) const
  {
    pl = ctxt->clip_plane;
    return ctxt->do_clip_plane;
  }
  /// Get the clip plane.
  const csPlane3& GetClipPlane () const
  {
    return ctxt->clip_plane;
  }
  /// Get the clip plane.
  csPlane3& GetClipPlane ()
  {
    return ctxt->clip_plane;
  }
  /**
   * If true then we have to clip all objects to the portal frustum
   * (returned with GetClipper()). Normally this is not needed but
   * some portals require this. If GetClipPlane() returns true then the
   * value of this function is also implied to be true.
   */
  bool IsClipperRequired () const { return ctxt->do_clip_frustum; }

  /**
   * Every fogged sector we encountered results in an extra structure in the
   * following list. This is only used if we are doing vertex based fog.
   * This function will return the first csFogInfo instance.
   */
  csFogInfo* GetFirstFogInfo () { return ctxt->fog_info; }
  /**
   * Set the first fog info.
   */
  void SetFirstFogInfo (csFogInfo* fi)
  {
    ctxt->fog_info = fi;
    ctxt->added_fog_info = true;
  }
  /**
   * Return true if fog info has been added.
   */
  bool AddedFogInfo () const { return ctxt->added_fog_info; }
  /**
   * Reset fog info.
   */
  void ResetFogInfo () { ctxt->added_fog_info = false; }

  /**
   * Check if the given bounding sphere (in camera and world space coordinates)
   * is visibile in this render view. If the sphere is visible this
   * function will also initialize the clip_plane, clip_z_plane, and
   * clip_portal fields which can be used for DrawTriangleMesh or
   * DrawPolygonMesh.
   */
  bool ClipBSphere (
	const csSphere &cam_sphere,
	const csSphere &world_sphere,
	int& clip_portal, int& clip_plane, int& clip_z_plane);

  SCF_DECLARE_IBASE;

  /// Get the current render context.
  virtual csRenderContext* GetRenderContext () { return ctxt; }

  /// Get the engine.
  virtual iEngine* GetEngine () { return (iEngine*)engine; }
  /// Get the 2D graphics subsystem.
  virtual iGraphics2D* GetGraphics2D () { return g2d; }
  /// Get the 3D graphics subsystem.
  virtual iGraphics3D* GetGraphics3D () { return g3d; }
  /// Get the frustum.
  virtual void GetFrustum (float& lx, float& rx, float& ty, float& by)
  {
    lx = leftx;
    rx = rightx;
    ty = topy;
    by = boty;
  }

  //-----------------------------------------------------------------
  // The following functions operate on the current render context.
  //-----------------------------------------------------------------

  /// Get the 2D clipper for this view.
  virtual iClipper2D* GetClipper () { return ctxt->iview; }

  /**
   * Get the current camera.
   */
  virtual iCamera* GetCamera () { return ctxt->icamera; }
  /**
   * Test if the given bounding sphere (in world space coordinates)
   * is visibile in this render view. The transformation will
   * transform world to camera space.
   */
  virtual bool TestBSphere (const csReversibleTransform& w2c,
    const csSphere& sphere);

  virtual void CalculateClipSettings (uint32 frustum_mask,
    int &clip_portal, int &clip_plane, int &clip_z_plane);

  virtual bool ClipBBox (csPlane3* planes, uint32& frustum_mask,
  	const csBox3& obox,
        int& clip_portal, int& clip_plane, int& clip_z_plane);
  virtual void SetupClipPlanes (const csReversibleTransform& tr_o2c,
  	csPlane3* planes, uint32& frustum_mask);

  /**
   * Get current sector.
   */
  virtual iSector* GetThisSector () { return ctxt->this_sector; }

  /**
   * Get previous sector.
   */
  virtual iSector* GetPreviousSector () { return ctxt->previous_sector; }

  /// Get the last portal.
  virtual iPortal* GetLastPortal () { return ctxt->last_portal; }

  /// Get the number of the current frame.
  virtual uint GetCurrentFrameNumber () const;
};

#endif // __CS_RVIEW_H__
