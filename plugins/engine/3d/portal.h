/*
    Copyright (C) 1998-2003 by Jorrit Tyberghein

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

#ifndef __CS_ENGINE_PORTAL_H__
#define __CS_ENGINE_PORTAL_H__

#include "csutil/scf.h"
#include "csutil/util.h"
#include "csutil/flags.h"
#include "csutil/refarr.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/weakref.h"
#include "csgeom/transfrm.h"
#include "csutil/scf_implementation.h"
#include "iengine/sector.h"
#include "iengine/portal.h"
#include "ivideo/texture.h"

class csPortalContainer;

/**
 * This class represents a portal to another sector.
 */
class csPortal : public scfImplementation1<csPortal,
                                           iPortal>
{
  friend class csPortalContainer;

private:
  /**
   * The sector that this portal points to.
   * This is a weak reference so that we can safely remove the sector
   * in which case 'sector' here will automatically be set to 0.
   */
  csWeakRef<iSector> sector;
  /// The parent portal container.
  csPortalContainer* parent;
  /// Vertex indices.
  csDirtyAccessArray<int> vertex_indices;
  /// Object plane.
  csPlane3 object_plane;
  /// World plane.
  csPlane3 world_plane;
  /// Name.
  char* name;

public:
  /// Set of flags
  csFlags flags;

protected:
  /// Warp transform in object space.
  csReversibleTransform warp_obj;
  /// List of callbacks called when a sector is missing (iPortalCallback).
  csRefArray<iPortalCallback> sector_cb_vector;
  /// List of callbacks called for traversing to a portal (iPortalCallback).
  csRefArray<iPortalCallback> portal_cb_vector;
  /// Maximum number of time a single sector will be visited by this portal.
  int max_sector_visit;

  /**
   * A portal will change the intensity/color of the light that passes
   * through it depending on the texture.
   */
  csRef<iTextureHandle> filter_texture;

  /**
   * If filter_texture is 0 then this filter is used instead.
   */
  float filter_r, filter_g, filter_b;

public:
  /// Create a portal.
  csPortal (csPortalContainer* parent);

  /// Destructor.
  virtual ~csPortal ();

  virtual void SetName (const char* n)
  {
    delete[] name;
    name = csStrNew (n);
  }
  virtual const char* GetName () const
  {
    return name;
  }

  //---- misc. manipulation functions ---------------------------------------

  void AddVertexIndex (int idx)
  {
    vertex_indices.Push (idx);
  }

  csDirtyAccessArray<int>& GetVertexIndices ()
  {
    return vertex_indices;
  }

  const csPlane3& GetIntObjectPlane () { return object_plane; }
  const csPlane3& GetIntWorldPlane () { return world_plane; }
  void SetObjectPlane (const csPlane3& pl) { object_plane = pl; }
  void SetWorldPlane (const csPlane3& pl) { world_plane = pl; }

  /// Return the sector that this portal points too.
  iSector* GetCsSector () const { return sector; }
  virtual iSector* GetSector () const { return GetCsSector (); }

  /**
   * Set the sector that this portal points too. To avoid circular
   * references, the sector is not IncRef'ed!
   */
  virtual void SetSector (iSector* s);

  virtual const csPlane3& GetObjectPlane () { return object_plane; }
  virtual const csPlane3& GetWorldPlane ();
  virtual void ComputeCameraPlane (const csReversibleTransform& t,
  	csPlane3& camplane);
  virtual bool PointOnPolygon (const csVector3& point);

  void CastShadows (iMovable* movable, iFrustumView* fview);

  bool IntersectRay (const csVector3 &start, const csVector3 &end) const;
  bool IntersectSegmentPlane (const csVector3 &start, const csVector3 &end,
    csVector3 &isect, float *pr) const;
  bool IntersectSegment (const csVector3 &start, const csVector3 &end,
    csVector3 &isect, float *pr) const;

  /// Set portal flags (see CS_PORTAL_XXX values)
  virtual csFlags& GetFlags () { return flags; }

  virtual const csVector3* GetVertices () const;
  virtual const csVector3* GetWorldVertices ();
  virtual int* GetVertexIndices () const;
  virtual int GetVertexIndicesCount () const;
  virtual size_t GetVerticesCount () const;

  /// Set the maximum sector visit.
  virtual void SetMaximumSectorVisit (int msv)
  {
    max_sector_visit = msv;
  }

  /// Get the maximum sector visit.
  virtual int GetMaximumSectorVisit () const
  {
    return max_sector_visit;
  }

  /// Set the portal callback.
  virtual void SetPortalCallback (iPortalCallback* cb);
  virtual void RemovePortalCallback (iPortalCallback* cb)
  {
    portal_cb_vector.Delete (cb);
  }
  virtual int GetPortalCallbackCount () const
  {
    return (int)portal_cb_vector.Length ();
  }
  virtual iPortalCallback* GetPortalCallback (int idx) const;

  /// Set the missing sector callback.
  virtual void SetMissingSectorCallback (iPortalCallback* cb);
  virtual void RemoveMissingSectorCallback (iPortalCallback* cb)
  {
    sector_cb_vector.Delete (cb);
  }
  virtual int GetMissingSectorCallbackCount () const
  {
    return (int)sector_cb_vector.Length ();
  }
  virtual iPortalCallback* GetMissingSectorCallback (int idx) const;

  /// Set the filter texture
  virtual void SetFilter (iTextureHandle* ft);
  /// Get the filter texture
  virtual iTextureHandle* GetTextureFilter () const;

  /// Set a color filter (instead of the texture).
  virtual void SetFilter (float r, float g, float b);
  /// Get the current color filter
  virtual void GetColorFilter (float &r, float &g, float &b) const;

  //---- space warping ------------------------------------------------------

  /// Get the warping transformation in object space.
  virtual const csReversibleTransform& GetWarp () const;

  /**
   * Set the warping transformation for this portal in object space and world
   * space.
   */
  virtual void SetWarp (const csTransform& t);

  /*
   * Set the warping transformation for this portal in object space and world
   * space.
   */
  virtual void SetWarp (const csMatrix3 &m_w, const csVector3 &v_w_before,
    const csVector3 &v_w_after);

  /// Set warping transformation to mirror around plane.
  virtual void SetMirror (const csPlane3& plane);

  /// Transform the warp matrix from object space to world space. @@@ WHY IS THIS IN iPortal???
  virtual void ObjectToWorld (const csReversibleTransform& t,
	csReversibleTransform& warp_wor) const;

  /// Hard transform the warp matrix. @@@ WHY IS THIS IN iPortal???
  virtual void HardTransform (const csReversibleTransform& t);

  /// Warp a position in world space.
  virtual csVector3 Warp (const csReversibleTransform& t,
  	const csVector3& pos) const;

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
		  csReversibleTransform& t, bool& mirror) const;

  //-------------------------------------------------------------------------

  /**
   * Draw the sector that is visible through this portal.
   * This function can be overriden by a subclass of Portal
   * to support portals to other types of engines.
   * This function also takes care of space warping.<p>
   *
   * 'new_clipper' is the new 2D polygon to which all things drawn
   * should be clipped.<br>
   * 't' is the transform from object to world (this2other).
   * 'rview' is the current iRenderView.<p>
   *
   * Return true if succesful, false otherwise.
   * Failure to draw through a portal does not need to
   * be harmful. It can just mean that some maximum number is
   * reached (like the maximum number of times a certain sector
   * can be drawn through a mirror).
   */
  bool Draw (const csPoly2D& new_clipper,
	const csReversibleTransform& t,
  	iRenderView* rview, const csPlane3& camera_plane);

  /**
   * Follow a beam through this portal and return the polygon
   * that it hits with. This function properly acounts for space
   * warping portals and also checks for infinite recursion (does
   * not allow traversing the same sector more than five times).
   * Returns the intersection point with the polygon in 'isect'.
   * The given transform 't' is used to transform the warping matrix
   * in the portal from object to world space (this==object, other==world).
   */
  virtual iMeshWrapper* HitBeamPortals (const csReversibleTransform& t,
	const csVector3& start, const csVector3& end,
  	csVector3& isect, int* polygon_idx);

  /**
   * Check if the destination sector is 0 and if so call
   * the callback. This function returns false if the portal should
   * not be traversed.
   */
  virtual bool CompleteSector (iBase* context);

  /**
   * Check frustum visibility of all polygons reachable through this portal.
   * Alpha is the alpha value you'd like to use to pass through this
   * portal (0 is no completely transparent, 100 is complete opaque).
   * 't' is the transform from object to world (this2other).
   */
  virtual void CheckFrustum (iFrustumView* lview,
  	const csReversibleTransform& t, int alpha);

  //------------------- iPortal implementation -----------------------
  virtual iObject *QueryObject () { return 0; } /*@@@ REMOVE */
};

#endif // __CS_ENGINE_PORTAL_H__

