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

#ifndef __IENGINE_RVIEW_H__
#define __IENGINE_RVIEW_H__

#include "csutil/scf.h"
#include "csgeom/plane3.h"
#include "csgeom/transfrm.h"
#include "csgeom/box.h"
#include "ivideo/graph3d.h"

struct iEngine;
struct iClipper2D;
struct iGraphics2D;
struct iGraphics3D;
struct iCamera;
struct iSector;
struct iPolygon3D;
struct csFog;

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
public:
  /// The current camera.
  iCamera* icamera;
  /// The 2D polygon describing how everything drawn inside should be clipped.
  iClipper2D* iview;

  /// The portal polygon (or NULL if the first sector).
  iPolygon3D* portal_polygon;
  /// The previous sector (or NULL if the first sector).
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
};

SCF_VERSION (iRenderView, 0, 1, 0);

/**
 * This interface represents all information needed to render
 * some object in a current draw context.
 */
struct iRenderView : public iBase
{
  /// Get the current render context.
  virtual csRenderContext* GetRenderContext () = 0;
  /**
   * Create a new render context. This is typically used
   * when going through a portal. Note that you should remember
   * the old render context if you want to restore it later.
   * The render context will get all the values from the current context
   * (with SCF references properly incremented).
   */
  virtual void CreateRenderContext () = 0;
  /**
   * Restore a render context. Use this to restore a previously overwritten
   * render context. This function will take care of properly cleaning
   * up the current render context.
   */
  virtual void RestoreRenderContext (csRenderContext* original) = 0;
  /**
   * Create a new camera in the current render context. This function
   * will create a new camera based on the current one. The new camera
   * reference is returned.
   */
  virtual iCamera* CreateNewCamera () = 0;

  /// Get the engine.
  virtual iEngine* GetEngine () = 0;
  /// Get the 2D graphics subsystem.
  virtual iGraphics2D* GetGraphics2D () = 0;
  /// Get the 3D graphics subsystem.
  virtual iGraphics3D* GetGraphics3D () = 0;
  /// Set the view frustum at z=1.
  virtual void SetFrustum (float lx, float rx, float ty, float by) = 0;
  /// Get the frustum.
  virtual void GetFrustum (float& lx, float& rx, float& ty, float& by) = 0;

  //-----------------------------------------------------------------
  // The following functions operate on the current render context.
  //-----------------------------------------------------------------

  /// Get the 2D clipper for this view.
  virtual iClipper2D* GetClipper () = 0;
  /// Set the 2D clipper for this view.
  virtual void SetClipper (iClipper2D* clip) = 0;
  /**
   * If true then we have to clip all objects to the portal frustum
   * (returned with GetClipper()). Normally this is not needed but
   * some portals require this. If GetClipPlane() returns true then the
   * value of this function is also implied to be true.
   */
  virtual bool IsClipperRequired () = 0;
  /**
   * Get the 3D clip plane that should be used to clip all geometry.
   * If this function returns false then this plane is invalid and should
   * not be used. Otherwise it must be used to clip the object before
   * drawing.
   */
  virtual bool GetClipPlane (csPlane3& pl) = 0;
  /// Get the clip plane.
  virtual csPlane3& GetClipPlane () = 0;
  /**
   * Set the 3D clip plane that should be used to clip all geometry.
   */
  virtual void SetClipPlane (const csPlane3& pl) = 0;
  /// Enable the use of a clip plane.
  virtual void UseClipPlane (bool u) = 0;
  /// Enable the use of a clip frustum.
  virtual void UseClipFrustum (bool u) = 0;

  /**
   * Every fogged sector we encountered results in an extra structure in the
   * following list. This is only used if we are doing vertex based fog.
   * This function will return the first csFogInfo instance.
   */
  virtual csFogInfo* GetFirstFogInfo () = 0;
  /**
   * Set the first fog info.
   */
  virtual void SetFirstFogInfo (csFogInfo* fi) = 0;
  /**
   * Return true if fog info has been added.
   */
  virtual bool AddedFogInfo () = 0;
  /**
   * Reset fog info.
   */
  virtual void ResetFogInfo () = 0;
  /**
   * Get the current camera.
   */
  virtual iCamera* GetCamera () = 0;
  /**
   * Calculate the fog information in the given G3DPolygonDP structure.
   */
  virtual void CalculateFogPolygon (G3DPolygonDP& poly) = 0;
  /**
   * Calculate the fog information in the given G3DPolygonDPFX structure.
   */
  virtual void CalculateFogPolygon (G3DPolygonDPFX& poly) = 0;
  /**
   * Calculate the fog information in the given G3DTriangleMesh
   * structure. This function assumes the fog array is already preallocated
   * and the rest of the structure should be filled in.
   * This function will take care of correctly enabling/disabling fog.
   */
  virtual void CalculateFogMesh (const csTransform& tr_o2c, G3DTriangleMesh& mesh) = 0;
  /**
   * Check if the screen bounding box of an object is visible in
   * this render view. If true is returned (visible) then do_clip
   * will be set to true or false depending on wether or not clipping
   * is wanted. This function also does far plane clipping.
   */
  virtual bool ClipBBox (const csBox2& sbox, const csBox3& cbox,
      	bool& do_clip) = 0;

  /**
   * Get current sector.
   */
  virtual iSector* GetThisSector () = 0;

  /**
   * Set the current sector.
   */
  virtual void SetThisSector (iSector* s) = 0;

  /**
   * Get previous sector.
   */
  virtual iSector* GetPreviousSector () = 0;

  /**
   * Set the previous sector.
   */
  virtual void SetPreviousSector (iSector* s) = 0;

  /**
   * Get the portal polygon.
   */
  virtual iPolygon3D* GetPortalPolygon () = 0;

  /**
   * Set the portal polygon.
   */
  virtual void SetPortalPolygon (iPolygon3D* poly) = 0;
};

#endif

