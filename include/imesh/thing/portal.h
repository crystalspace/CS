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

#ifndef __IENGINE_PORTAL_H__
#define __IENGINE_PORTAL_H__

#include "csutil/scf.h"
#include "iutil/objref.h"

class csReversibleTransform;

/**
 * If this flag is set then this portal will clip all geometry in
 * the destination sector. This must be used for portals which arrive
 * in the middle of a sector.
 */
#define CS_PORTAL_CLIPDEST 0x00000001

/**
 * If this flag is set then this portal will do a Z-fill after
 * rendering the contents. This is mainly useful for floating portals
 * where it is possible that there is geometry in the same sector
 * that will be rendered behind the portal (and does could accidently
 * get written in the portal sector because the Z-buffer cannot
 * be trusted).
 */
#define CS_PORTAL_ZFILL 0x00000002

/**
 * If this flag is set then this portal will do space warping.
 * You can use this to implement mirrors or other weird portal effects.
 * Don't set this flag directly. Use SetWarp() instead. It is safe
 * to disable and query this flag though.
 */
#define CS_PORTAL_WARP 0x00000004

/**
 * If this flag is set then this portal mirrors space (changes order
 * of the vertices of polygons). Don't set this flag directly. It will
 * be automatically set if a mirroring space warp is used with SetWarp().
 */
#define CS_PORTAL_MIRROR 0x00000008

/**
 * A flag which indicates if the destination of this portal should not be
 * transformed from object to world space. For mirrors you should
 * disable this flag because you want the destination to move with the
 * source.
 */
#define CS_PORTAL_STATICDEST 0x00000010

class csTransform;
class csMatrix3;
class csVector3;
class csFlags;
struct iTextureHandle;
struct iSector;
struct iPolygon3D;
struct iPortal;
struct iFrustumView;

SCF_VERSION (iPortalCallback, 0, 0, 1);

/**
 * When a sector is missing this callback will be called. If this callback
 * returns false then this portal will not be traversed. Otherwise this
 * callback has to set up the destination sector and return true.
 * The given context will be either an instance of iRenderView, iFrustumView,
 * or else NULL.
 */
struct iPortalCallback : public iBase
{
  /// Traverse to the portal.
  virtual bool Traverse (iPortal* portal, iBase* context) = 0;
};

SCF_VERSION (iPortal, 0, 0, 7);

/**
 * This is the interface to the Portal objects. Polygons that are
 * really `openings' to different areas have a portal associated
 * with them. The portal object defines where the opening leads,
 * how exactly the geometry behind portal is to be handled and
 * so on.
 */
struct iPortal : public iReference
{
  /// Get the iObject for this portal.
  virtual iObject *QueryObject () = 0;

  /// Return the sector that this portal points too.
  virtual iSector* GetSector () const = 0;

  /**
   * Set the sector that this portal points too. To avoid circular
   * references, the sector is not IncRef'ed!
   */
  virtual void SetSector (iSector* s) = 0;

  /// Set portal flags (see CS_PORTAL_XXX values)
  virtual csFlags& GetFlags () = 0;

  /**
   * Set the portal callback. This will call IncRef() on the callback
   * (and possible DecRef() on the old callback). So make sure you
   * call DecRef() to release your own reference.
   */
  virtual void SetPortalCallback (iPortalCallback* cb) = 0;

  /// Get the portal callback.
  virtual iPortalCallback* GetPortalCallback () const = 0;

  /**
   * Set the missing sector callback. This will call IncRef() on the callback
   * (and possible DecRef() on the old callback). So make sure you
   * call DecRef() to release your own reference.
   */
  virtual void SetMissingSectorCallback (iPortalCallback* cb) = 0;

  /// Get the missing sector callback.
  virtual iPortalCallback* GetMissingSectorCallback () const = 0;

  /// Set the filter texture
  virtual void SetFilter (iTextureHandle* ft) = 0;
  /// Get the filter texture
  virtual iTextureHandle* GetTextureFilter () const = 0;

  /// Set a color filter (instead of the texture).
  virtual void SetFilter (float r, float g, float b) = 0;
  /// Get the current color filter
  virtual void GetColorFilter (float &r, float &g, float &b) const = 0;

  //---- space warping ------------------------------------------------------

  /**
   * Set the warping transformation for this portal in object space and world
   * space.
   */
  virtual void SetWarp (const csMatrix3 &m_w, const csVector3 &v_w_before,
    const csVector3 &v_w_after) = 0;
  /**
   * Set the warping transformation for this portal in object space and world
   * space.
   */
  virtual void SetWarp (const csTransform& t) = 0;

  /// Set warping transformation to mirror around given polygon
  virtual void SetMirror (iPolygon3D *iPoly) = 0;

  /// Get the warping transformation
  virtual const csReversibleTransform &GetWarp () const = 0;

  /// Transform the warp matrix from object space to world space.
  virtual void ObjectToWorld (const csReversibleTransform& t) = 0;

  /// Hard transform the warp matrix.
  virtual void HardTransform (const csReversibleTransform& t) = 0;

  /// Warp a position in world space.
  virtual csVector3 Warp (const csVector3& pos) const = 0;

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
  virtual void WarpSpace (csReversibleTransform& t, bool& mirror) const = 0;

  //-------------------------------------------------------------------------

  /**
   * Check if the destination sector is NULL and if so call
   * the callback. This function returns false if the portal should
   * not be traversed.
   */
  virtual bool CompleteSector (iBase* context) = 0;

  /**
   * Check frustum visibility of all polygons reachable through this portal.
   * Alpha is the alpha value you'd like to use to pass through this
   * portal (0 is no completely transparent, 100 is complete opaque).
   */
  virtual void CheckFrustum (iFrustumView* lview, int alpha) = 0;
};

#endif // __IENGINE_PORTAL_H__
