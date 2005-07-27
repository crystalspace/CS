/*
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

#ifndef __CS_IENGINE_RVIEW_H__
#define __CS_IENGINE_RVIEW_H__

/**\file
 */
/**
 * \addtogroup engine3d_views
 * @{ */
 
#include "csutil/scf.h"

#include "csgeom/plane3.h"

struct iCamera;
struct iClipper2D;
struct iEngine;
struct iGraphics2D;
struct iGraphics3D;
struct iPortal;
struct iSector;

struct csFog;

class csBox3;
class csRenderView;
class csReversibleTransform;
class csSphere;
class csVector3;

/**
 * Information for vertex based fog. There is an instance of this
 * structure in iRenderView for every fogged sector that
 * we encounter. It contains information which allows us to calculate
 * the thickness of the fog for any given ray through the incoming
 * and outgoing portals of the sector.
 */
class csFogInfo
{
public:
  /// Next in list (back in recursion time).
  csFogInfo* next;

  /// The incoming plane (plane of the portal).
  csPlane3 incoming_plane;
  /// The outgoing plane (also of a portal).
  csPlane3 outgoing_plane;
  /**
   * If this is false then there is no incoming plane (the current sector has
   * fog and is not being drawn through a portal).
   */
  bool has_incoming_plane;

  /**
   * If this is false there is no outgoing plane.  The 'outgoing plane
   * distance' is then calculated by straight distance to a vertex instead of
   * projecting throught the outgoing plane
   */
  bool has_outgoing_plane;

  /// The structure describing the fog.
  csFog* fog;
};

/**
 * This structure keeps track of the current render context.
 * It is used by iRenderView. When recursing through a portal
 * a new render context will be created and set in place of the
 * old one.
 */
class csRenderContext
{
  friend class csRenderView;

public:
  /// A pointer back to the previous render context.
  csRenderContext* previous;

  /// The current camera.
  csRef<iCamera> icamera;
  /// The 2D polygon describing how everything drawn inside should be clipped.
  csRef<iClipper2D> iview;
  ///// The frustum corresponding with iview.
  csPlane3 frustum[5];

  /// A set of clip planes for this context in world space.
  csPlane3 clip_planes[7];
  /// A frustum masks which indicates which planes of clip_planes are used.
  uint32 clip_planes_mask;

  /// The last portal we traversed through (or 0 if first sector).
  iPortal* last_portal;
  /// The previous sector (or 0 if the first sector).
  iSector* previous_sector;
  /// This sector.
  iSector* this_sector;

  /**
   * This variable holds the plane of the portal through which the camera
   * is looking.
   */
  csPlane3 clip_plane;

  /**
   * If true then we clip all objects to 'clip_plane'. In principle
   * one should always clip to 'clip_plane'. However, in many cases
   * this is not required because portals mostly arrive in at the
   * boundaries of a sector so there can actually be no objects
   * after the portal plane. But it is possible that portals arive
   * somewhere in the middle of a sector (for example with BSP sectors
   * or with Things containing portals). In that case this variable
   * will be set to true and clipping to 'clip_plane' is required.
   */
  bool do_clip_plane;

  /**
   * If true then we have to clip all objects to the portal frustum
   * (either in 2D or 3D). Normally this is not needed but some portals
   * require this. If do_clip_plane is true then the value of this
   * field is also implied to be true. The top-level portal should
   * set do_clip_frustum to true in order for all geometry to be
   * correctly clipped to screen boundaries.
   */
  bool do_clip_frustum;

  /**
   * Every fogged sector we encountered results in an extra structure in the
   * following list. This is only used if we are doing vertex based fog.
   */
  csFogInfo* fog_info;

  /**
   * If the following variable is true then a fog_info was added in this
   * recursion level.
   */
  bool added_fog_info;

  /**
   * A number indicating the recursion level we are in. Starts with 0.
   * Whenever the engine goes through a portal this number increases.
   * Returning from a portal decreases the number again.
   */
  int draw_rec_level;

  /**
   * This unique id can be used to check if you are still in the same
   * render context. Checking on pointers is not safe since render contexts
   * are reused so a different render context can result in the same pointer.
   */
  uint32 context_id;
};

SCF_VERSION (iRenderView, 0, 5, 1);

/**
 * This interface represents all information needed to render
 * some object in a current draw context.
 */
struct iRenderView : public iBase
{
  /// Get the current render context.
  virtual csRenderContext* GetRenderContext () = 0;

  /// Get the engine.
  virtual iEngine* GetEngine () = 0;
  /// Get the 2D graphics subsystem.
  virtual iGraphics2D* GetGraphics2D () = 0;
  /// Get the 3D graphics subsystem.
  virtual iGraphics3D* GetGraphics3D () = 0;
  /// Get the frustum.
  virtual void GetFrustum (float& lx, float& rx, float& ty, float& by) = 0;

  //-----------------------------------------------------------------
  // The following functions operate on the current render context.
  //-----------------------------------------------------------------

  /// Get the 2D clipper for this view.
  virtual iClipper2D* GetClipper () = 0;

  /**
   * Get the current camera.
   */
  virtual iCamera* GetCamera () = 0;

  /**
   * Given a frustum_mask, calculate the clip settings.
   */
  virtual void CalculateClipSettings (uint32 frustum_mask,
    int &clip_portal, int &clip_plane, int &clip_z_plane) = 0;

  /**
   * Test if the given bounding sphere (in world space coordinates)
   * is visible in this render view. The optional will
   * transform world to camera space.
   */
  virtual bool TestBSphere (const csReversibleTransform& w2c,
  	const csSphere& sphere) = 0;

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
  virtual bool ClipBBox (csPlane3* planes, uint32& frustum_mask,
  	const csBox3& obox,
      	int& clip_portal, int& clip_plane, int& clip_z_plane) = 0;

  /**
   * Setup clipping planes in object space. The input arrays for planes
   * should each be able to hold 10 planes. Returns a mask that you can
   * use for the csIntersect3::BoxFrustum() function.
   */
  virtual void SetupClipPlanes (const csReversibleTransform& tr_o2c,
  	csPlane3* planes, uint32& frustum_mask) = 0;

  /**
   * Get current sector.
   */
  virtual iSector* GetThisSector () = 0;

  /**
   * Get previous sector.
   */
  virtual iSector* GetPreviousSector () = 0;

  /**
   * Get the portal we last traversed through.
   */
  virtual iPortal* GetLastPortal () = 0;

  /**
   * Get the original camera for this render view. This is
   * the camera before any space warping portals.
   */
  virtual iCamera* GetOriginalCamera () const = 0;

  /**
   * Get the number of the current frame.
   */
  virtual uint GetCurrentFrameNumber () const = 0;

  /**
   * Check if the given bounding sphere (in camera and world space coordinates)
   * is visibile in this render view. If the sphere is visible this
   * function will also initialize the clip_plane, clip_z_plane, and
   * clip_portal fields which can be used for DrawTriangleMesh or
   * DrawPolygonMesh.
   */
  virtual bool ClipBSphere (
	const csSphere &cam_sphere,
	const csSphere &world_sphere,
	int& clip_portal, int& clip_plane, int& clip_z_plane) = 0;
  


  // @@@ ADDED B/C OF FATLOOP PORTAL HACKING
  // @@@ REMOVE AGAIN ASAP
  virtual void CreateRenderContext () = 0;
  virtual int GetRenderRecursionLevel () const = 0;
  virtual void SetRenderRecursionLevel (int rec) = 0;
  virtual void SetClipper (iClipper2D* clip) = 0;
  virtual void ResetFogInfo () = 0;
  virtual void SetPreviousSector (iSector* s) = 0;
  virtual void SetClipPlane (const csPlane3& p) = 0;
  virtual bool GetClipPlane (csPlane3& pl) const = 0;
  virtual const csPlane3& GetClipPlane () const = 0;
  virtual csPlane3& GetClipPlane () = 0;
  virtual void UseClipPlane (bool u) = 0;
  virtual void UseClipFrustum (bool u) = 0;
  virtual void SetLastPortal (iPortal* por) = 0;
  virtual bool IsClipperRequired () const = 0;
  virtual iCamera* CreateNewCamera () = 0;
  virtual void RestoreRenderContext () = 0;
  virtual void SetThisSector (iSector* s) = 0;
  virtual void SetupClipPlanes () = 0;
};

/** @} */

#endif // __CS_IENGINE_RVIEW_H__

