/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef PORTAL_H
#define PORTAL_H

#include "csgeom/transfrm.h"
#include "csengine/rview.h"

class csSector;
class csPolygon2D;
class csPolygon3D;
class csStatLight;
interface ITextureHandle;

/**
 * This class represents a portal. It belongs to some polygon
 * which is then considered a portal to another sector.
 */
class csPortal
{
private:
  /// The sector that this portal points to.
  csSector* sector;

protected:
  /**
   * 0 is no alpha, 25 is 25% see through and
   * 75% texture, ... Possible values are 0, 25, 50, and 75.
   */
  int cfg_alpha;

  /**
   * If this polygon is a portal it possibly has a transformation matrix and
   * vector to warp space in some way. This way mirrors can be implemented.
   * If true the space should be warped through the portal.
   */
  bool do_warp_space;

  /**
   * A flag which indicates if this portal mirrors space (changes the order
   * of the vertices). A normal space warping transformation which just
   * relocates space does not mirror space. A reflecting wall mirrors space.
   */
  bool do_mirror;

  /**
   * If true then this portal will clip geometry in the destination sector.
   * This must be used for portals which arrive in the middle of a sector.
   * But don't use it otherwise because it decreases efficiency.
   */
  bool do_clip_portal;

  /**
   * A flag which indicates if the destination of this portal should not be
   * transformed from object to world space. For mirrors you should
   * disable this flag because you want the destination to move with the
   * source.
   */
  bool static_dest;

  /// Warp transform in object space.
  csReversibleTransform warp_obj;
  /// Warp transform in world space.
  csReversibleTransform warp_wor;

  /**
   * A portal will change the intensity/color of the light that passes
   * through it depending on the texture.
   */
  ITextureHandle* filter_texture;

  /**
   * If filter_texture is NULL then this filter is used instead.
   */
  float filter_r, filter_g, filter_b;

public:
  /**
   * Create a portal.
   */
  csPortal ();

  /// Destructor.
  virtual ~csPortal () { }

  /**
   * Return the sector that this portal points too.
   */
  csSector* GetSector () { return sector; }

  /**
   * Set the sector that this portal points too.
   */
  void SetSector (csSector* s) { sector = s; }

  /**
   * Transform the warp matrix from object space to world space.
   */
  void ObjectToWorld (const csReversibleTransform& t);

  /**
   * Set the warping transformation for this portal in object space and world space.
   */
  void SetWarp (const csTransform& t);

  /**
   * Set the warping transformation for this portal in object space and world space.
   */
  void SetWarp (const csMatrix3& m_w, const csVector3& v_w_before, const csVector3& v_w_after);

  /**
   * Check if space is warped by this portal.
   */
  bool IsSpaceWarped () { return do_warp_space; }

  /**
   * Disable space warping.
   */
  void DisableSpaceWarping () { do_warp_space = false; }

  /**
   * Disable/enable clipping portal.
   * If true then this portal will clip geometry in the destination sector.
   * This must be used for portals which arrive in the middle of a sector.
   * But don't use it otherwise because it decreases efficiency.
   */
  void SetClippingPortal (bool c) { do_clip_portal = c; }

  /**
   * Is this a clipping portal?
   */
  bool IsClippingPortal () { return do_clip_portal; }

  /**
   * Set static destination. If this field is true
   * then the portal points to a static destination. This
   * means that when the portal moves (because it is part
   * of a thing that moves for example) then the destination
   * of the portal will remain fixed. For mirrors you want
   * this to be false so that the destination moves with the
   * portal.
   */
  void SetStaticDest (bool sd) { static_dest = sd; }

  /**
   * Get static destination.
   */
  bool IsStaticDest () { return static_dest; }

  /**
   * Set the texture (used for filtering).
   */
  void SetTexture (ITextureHandle* ft) { filter_texture = ft; }

  /**
   * Set the filter (instead of the texture).
   */
  void SetFilter (float r, float g, float b) { filter_r = r; filter_g = g; filter_b = b; filter_texture = NULL; }

  ///
  int GetAlpha () { return cfg_alpha; }
  ///
  void SetAlpha (int a) { cfg_alpha = a; }

  /**
   * Warp space using the given world->camera transformation.
   * This function modifies the given camera transformation to reflect
   * the warping change.<p>
   *
   * 't' is the transformation from world to camera space.<br>
   * 'mirror' is true if the camera transformation transforms all polygons
   * so that the vertices are ordered anti-clockwise. 'mirror' will be modified
   * by warp_space if needed.
   */
  void WarpSpace (csReversibleTransform& t, bool& mirror);

  /**
   * Draw the sector that is visible through this portal.
   * This function can be overriden by a subclass of Portal
   * to support portals to other types of engines.
   * This function also takes care of space warping.<p>
   *
   * 'new_clipper' is the new 2D polygon to which all things drawn
   * should be clipped.<br>
   * 'portal_polygon' is the polygon containing this portal. This routine
   * will use the camera space plane of the portal polygon.<br>
   * 'rview' is the current csRenderView.<p>
   *
   * Return true if succesful, false otherwise.
   * Failure to draw through a portal does not need to
   * be harmful. It can just mean that some maximum number is
   * reached (like the maximum number of times a certain sector
   * can be drawn through a mirror).
   */
  virtual bool Draw (csPolygon2D* new_clipper, csPolygon3D* portal_polygon,
  	csRenderView& rview);

  /**
   * Follow a beam through this portal and return the polygon
   * that it hits with.
   * NOTE@@@! This function does not take proper care of space warping!
   */
  virtual csPolygon3D* HitBeam (csVector3& start, csVector3& end);

  /**
   * Intersects world-space sphere through this sector. Return closest
   * polygon that is hit (or NULL) and the intersection point. If 'pr' !=
   * NULL it will also return the distance where the intersection happened.
   */
  virtual csPolygon3D* IntersectSphere (csVector3& center, float radius, float* pr = NULL);

  /**
   * Follow a segment through this portal and modify the given
   * camera transformation according to the space warping.
   * This function will modify all the given parameters. These
   * should be used as the new camera transformation when you
   * decide to use it.<p>
   *
   * This function returns the destination.
   */
  virtual csSector* FollowSegment (csReversibleTransform& t,
                                  csVector3& new_position, bool& mirror);

  /**
   * Update lighting of all polygons reachable through this portal.
   */
  virtual void CalculateLighting (csLightView& lview);
};

#endif /*PORTAL_H*/

