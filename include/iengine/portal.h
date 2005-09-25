/*
    Copyright (C) 2003 by Jorrit Tyberghein

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

#ifndef __CS_IENGINE_PORTAL_H__
#define __CS_IENGINE_PORTAL_H__

/**\file
 * Portal interfaces
 */
/**
 * \addtogroup engine3d
 * @{ */

#include "csutil/scf.h"

class csFlags;
class csMatrix3;
class csPlane3;
class csReversibleTransform;
class csTransform;
class csVector3;

struct iFrustumView;
struct iMeshWrapper;
struct iMovable;
struct iObject;
struct iPortal;
struct iSector;
struct iTextureHandle;



/**
 * If this flag is set then this portal will clip all geometry in
 * the destination sector. This must be used for portals which arrive
 * in the middle of a sector.
 */
#define CS_PORTAL_CLIPDEST 0x00000001

/**
 * If this flag is set then this portal will clip geometry of an object
 * that is straddling this portal (i.e. the object is both in the source
 * and destination sector and the portal 'cuts' the object). This is only
 * needed if the portal is on a surface that is transparent. A space warping
 * portal will do this automatically.
 */
#define CS_PORTAL_CLIPSTRADDLING 0x00000002

/**
 * If this flag is set then this portal will do a Z-fill after
 * rendering the contents. This is mainly useful for floating portals
 * where it is possible that there is geometry in the same sector
 * that will be rendered behind the portal (and does could accidently
 * get written in the portal sector because the Z-buffer cannot
 * be trusted).
 */
#define CS_PORTAL_ZFILL 0x00000004

/**
 * If this flag is set then this portal will do space warping.
 * You can use this to implement mirrors or other weird portal effects.
 * Don't set this flag directly. Use SetWarp() instead. It is safe
 * to disable and query this flag though.
 */
#define CS_PORTAL_WARP 0x00000008

/**
 * If this flag is set then this portal mirrors space (changes order
 * of the vertices of polygons). Don't set this flag directly. It will
 * be automatically set if a mirroring space warp is used with SetWarp().
 */
#define CS_PORTAL_MIRROR 0x00000010

/**
 * A flag which indicates if the destination of this portal should not be
 * transformed from object to world space. For mirrors you should
 * disable this flag because you want the destination to move with the
 * source.
 */
#define CS_PORTAL_STATICDEST 0x00000020

/**
 * If this flag is used then the portal will use possible available
 * stencil buffer on the hardware to do good clipping. This flag should
 * be used if you have a portal that is not at the boundary of the sector
 * and that can be covered (or itself covers) other objects. It is usually
 * used in combination with CS_PORTAL_ZFILL and sometimes with
 * CS_PORTAL_CLIPDEST if the destination of the portal enters in the middle
 * of a sector.
 */
#define CS_PORTAL_FLOAT 0x00000040

/**
 * If this flag is set then this portal is used for collision detection.
 */
#define CS_PORTAL_COLLDET 0x00000080

/**
 * If this flag is set then this portal is used for visibility culling.
 */
#define CS_PORTAL_VISCULL 0x00000100


SCF_VERSION (iPortalCallback, 0, 0, 1);

/**
 * When a sector is missing this callback will be called. If this callback
 * returns false then this portal will not be traversed. Otherwise this
 * callback has to set up the destination sector and return true.
 * The given context will be either an instance of iRenderView, iFrustumView,
 * or else 0.
 */
struct iPortalCallback : public iBase
{
  /**
   * Traverse to the portal. It is safe to delete this callback
   * in this function.
   */
  virtual bool Traverse (iPortal* portal, iBase* context) = 0;
};

/**
 * This is the interface to the Portal objects. Polygons that are
 * really `openings' to different areas have a portal associated
 * with them. The portal object defines where the opening leads,
 * how exactly the geometry behind portal is to be handled and
 * so on.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iPortalContainer::CreatePortal()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iPortalContainer::GetPortal()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEngine
 *   </ul>
 */
struct iPortal : public virtual iBase
{
  SCF_INTERFACE(iPortal, 2,0,0);
  /// Get the iObject for this portal. @@@ OBSOLETE!!!
  virtual iObject *QueryObject () = 0;

  /// Set the name of this portal.
  virtual void SetName (const char* name) = 0;
  /// Get the name of this portal.
  virtual const char* GetName () const = 0;

  /// Return the sector that this portal points too.
  virtual iSector* GetSector () const = 0;

  /**
   * Get an array of object space vertices. Use this in combination
   * with GetVertexIndices() to find out where the portal is.
   */
  virtual const csVector3* GetVertices () const = 0;

  /**
   * Get an array of world space vertices. Use this in combination
   * with GetVertexIndices() to find out where the portal is.
   */
  virtual const csVector3* GetWorldVertices () = 0;

  /**
   * Get an array of vertex indices (indices in the array returned
   * by GetVertices()).
   */
  virtual int* GetVertexIndices () const = 0;

  /**
   * Get the number of vertex indices.
   */
  virtual int GetVertexIndicesCount () const = 0;

  /**
   * Get the object space plane of this portal.
   */
  virtual const csPlane3& GetObjectPlane () = 0;

  /**
   * Get the world space plane of this portal.
   */
  virtual const csPlane3& GetWorldPlane () = 0;

  /**
   * Calculate the camera space plane for this portal.
   */
  virtual void ComputeCameraPlane (const csReversibleTransform& t,
  	csPlane3& camplane) = 0;

  /**
   * Test if a point is on the polygon represented by this portal. This
   * test happens in world space.
   */
  virtual bool PointOnPolygon (const csVector3& point) = 0;

  /**
   * Set the sector that this portal points too. To avoid circular
   * references, the sector is not IncRef'ed!
   */
  virtual void SetSector (iSector* s) = 0;

  /// Set portal flags (see CS_PORTAL_XXX values)
  virtual csFlags& GetFlags () = 0;

  /**
   * Set the number of times that this portal will allow for watching
   * the same portal. By default this is 5 which means that in one
   * recursion level this portal will visit every sector at maximum
   * 5 times.
   */
  virtual void SetMaximumSectorVisit (int msv) = 0;
  /// Get the maximum sector visit.
  virtual int GetMaximumSectorVisit () const = 0;

  /**
   * Set the portal callback. This will call IncRef() on the callback
   * So make sure you call DecRef() to release your own reference.
   * Note that ALL portal callbacks have to return true before
   * the portal is traversed.
   */
  virtual void SetPortalCallback (iPortalCallback* cb) = 0;

  /**
   * Remove a portal callback.
   */
  virtual void RemovePortalCallback (iPortalCallback* cb) = 0;

  /// Get the number of portal callbacks.
  virtual int GetPortalCallbackCount () const = 0;
  
  /// Get the specified portal callback.
  virtual iPortalCallback* GetPortalCallback (int idx) const = 0;

  /**
   * Set the missing sector callback. This will call IncRef() on the callback
   * So make sure you call DecRef() to release your own reference.
   * Note that as soon as one of these callbacks creates the missing
   * sector, the loop to call these callbacks will stop.
   */
  virtual void SetMissingSectorCallback (iPortalCallback* cb) = 0;

  /**
   * Remove a missing sector callback.
   */
  virtual void RemoveMissingSectorCallback (iPortalCallback* cb) = 0;

  /// Get the number of missing sector callbacks.
  virtual int GetMissingSectorCallbackCount () const = 0;
  
  /// Get the specified missing sector callback.
  virtual iPortalCallback* GetMissingSectorCallback (int idx) const = 0;

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

  /// Set warping transformation to mirror around the given plane.
  virtual void SetMirror (const csPlane3& plane) = 0;

  /// Get the warping transformation
  virtual const csReversibleTransform &GetWarp () const = 0;

  /// Hard transform the warp matrix.
  virtual void HardTransform (const csReversibleTransform& t) = 0;

  /**
   * Transform the warp matrix from object space to world space.
   * The transform 't' is object to world (this==object, other==world).
   */
  virtual void ObjectToWorld (const csReversibleTransform& t,
	csReversibleTransform& warp_wor) const = 0;

  /**
   * Warp a position in world space given a object space to world space
   * transform (this==object, other==world).
   */
  virtual csVector3 Warp (const csReversibleTransform& t,
		  const csVector3& pos) const = 0;

  /**
   * Warp space using the given world->camera transformation.
   * This function modifies the given camera transformation to reflect
   * the warping change.<p>
   *
   * 'warp_wor' is the warp transformation in world space.
   * 't' is the transformation from world to camera space.<br>
   * 'mirror' is true if the camera transformation transforms all polygons so
   * that the vertices are ordered anti-clockwise.  'mirror' will be modified
   * by warp_space if needed.
   */
  virtual void WarpSpace (const csReversibleTransform& warp_wor,
		  csReversibleTransform& t, bool& mirror) const = 0;

  //-------------------------------------------------------------------------

  /**
   * Check if the destination sector is 0 and if so call
   * the callback. This function returns false if the portal should
   * not be traversed.
   */
  virtual bool CompleteSector (iBase* context) = 0;

  /**
   * Check frustum visibility of all polygons reachable through this portal.
   * Alpha is the alpha value you'd like to use to pass through this
   * portal (0 is no completely transparent, 100 is complete opaque).
   * 't' is the transform from object to world (this2other).
   */
  virtual void CheckFrustum (iFrustumView* lview,
	  const csReversibleTransform& t, int alpha) = 0;

  /**
   * Follow a beam through this portal and return the mesh and polygon index
   * that it hits with (0 incase no hit). This function properly acounts for
   * space warping portals and also checks for infinite recursion (does
   * not allow traversing the same sector more than five times).
   * Returns the intersection point with the polygon in 'isect'.
   * The given transform 't' is used to transform the warping matrix
   * in the portal from object to world space (this==object, other==world).
   */
  virtual iMeshWrapper* HitBeamPortals (const csReversibleTransform& t,
	const csVector3& start, const csVector3& end,
  	csVector3& isect, int* polygon_idx) = 0;

  /**
   * Get number of vertices in the array returned by GetVertices().
   */
  virtual size_t GetVerticesCount () const = 0;

};

/** @} */

#endif // __CS_IENGINE_PORTAL_H__

