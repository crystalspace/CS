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

#ifndef __CS_PORTAL_H__
#define __CS_PORTAL_H__

#include "csgeom/transfrm.h"
#include "csutil/flags.h"
#include "iengine/portal.h"

class csObject;
class csSector;
class csPolygon2D;
class csPolygon3D;
class csStatLight;
class csFrustumView;
struct iRenderView;
struct iTextureHandle;

/**
 * This class represents a portal. It belongs to some polygon
 * which is then considered a portal to another sector.
 */
class csPortal : public iPortal
{
private:
  /// The sector that this portal points to.
  csSector* sector;

public:
  /// Set of flags
  csFlags flags;

protected:
  /// Warp transform in object space.
  csReversibleTransform warp_obj;
  /// Warp transform in world space.
  csReversibleTransform warp_wor;

  /**
   * A portal will change the intensity/color of the light that passes
   * through it depending on the texture.
   */
  iTextureHandle* filter_texture;

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
   * Complete a sector destination. This function is called
   * whenever a destination sector is NULL. A subclass of csPortal
   * can implement this to dynamically create a new sector at this
   * point.
   */
  virtual void CompleteSector () { }

  /**
   * Transform the warp matrix from object space to world space.
   */
  void ObjectToWorld (const csReversibleTransform& t);

  /**
   * Hard transform the warp matrix.
   */
  void HardTransform (const csReversibleTransform& t);

  /**
   * Set the warping transformation for this portal in object space and world
   * space.
   */
  void SetWarp (const csTransform& t);

  /**
   * Get the warping transformation in object space.
   */
  const csReversibleTransform& GetWarp () { return warp_obj; }

  /**
   * Set the texture (used for filtering).
   */
  void SetTexture (iTextureHandle* ft) { filter_texture = ft; }

  /**
   * Set the filter (instead of the texture).
   */
  void SetFilter (float r, float g, float b)
  { filter_r = r; filter_g = g; filter_b = b; filter_texture = NULL; }

  /**
   * Warp a position in world space.
   */
  csVector3 Warp (const csVector3& pos)
  {
    return warp_wor.Other2This (pos);
  }

  /**
   * Warp space using the given world->camera transformation.
   * This function modifies the given camera transformation to reflect
   * the warping change.<p>
   *
   * 't' is the transformation from world to camera space.<br>
   * 'mirror' is true if the camera transformation transforms all polygons so
   * that the vertices are ordered anti-clockwise.  'mirror' will be modified
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
   * 'rview' is the current iRenderView.<p>
   *
   * Return true if succesful, false otherwise.
   * Failure to draw through a portal does not need to
   * be harmful. It can just mean that some maximum number is
   * reached (like the maximum number of times a certain sector
   * can be drawn through a mirror).
   */
  bool Draw (csPolygon2D* new_clipper, csPolygon3D* portal_polygon,
  	iRenderView* rview);

  /**
   * Follow a beam through this portal and return the polygon
   * that it hits with. This function properly acounts for space
   * warping portals and also checks for infinite recursion (does
   * not allow traversing the same sector more than five times).
   * Returns the intersection point with the polygon in 'isect'.
   */
  csPolygon3D* HitBeam (const csVector3& start, const csVector3& end,
  	csVector3& isect);

  /**
   * Follow a beam through this portal and return the object
   * that it hits with. This function properly acounts for space
   * warping portals and also checks for infinite recursion (does
   * not allow traversing the same sector more than five times).
   * Optionally returns the polygon in 'polygonPtr'.
   */
  csObject* HitBeam (const csVector3& start, const csVector3& end,
  	csPolygon3D** polygonPtr);

  /**
   * Check frustum visibility of all polygons reachable through this portal.
   * Alpha is the alpha value you'd like to use to pass through this
   * portal (0 is no completely transparent, 100 is complete opaque).
   */
  virtual void CheckFrustum (csFrustumView& lview, int alpha);

  //---------------------------- iPortal interface -----------------------------
  DECLARE_IBASE;

  /// Set portal flags (see CS_PORTAL_XXX values)
  virtual csFlags& GetFlags () { return flags; }

  /// Get the sector that the portal points to
  virtual iSector *GetPortal ();
  /// Set portal to point to specified sector
  virtual void SetPortal (iSector *iDest);

  /*
   * Set the warping transformation for this portal in object space and world
   * space.
   */
  virtual void SetWarp (const csMatrix3 &m_w, const csVector3 &v_w_before,
    const csVector3 &v_w_after);
  /// Set warping transformation to mirror
  virtual void SetMirror (iPolygon3D *iPoly);
};

#endif // __CS_PORTAL_H__
