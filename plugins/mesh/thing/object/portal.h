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
#include "csutil/csobject.h"
#include "csutil/refarr.h"
#include "csutil/weakref.h"
#include "iengine/portal.h"

class csPolygon2D;
class csPolygon3DStatic;
class csStatLight;
class csObject;
struct iMeshWrapper;
struct iRenderView;
struct iFrustumView;
struct iPolygon3D;

/**
 * This class represents a portal. It belongs to some polygon
 * which is then considered a portal to another sector.
 */
class csPortalObsolete : public csObject
{
private:
  /**
   * The sector that this portal points to.
   * This is a weak reference so that we can safely remove the sector
   * in which case 'sector' here will automatically be set to 0.
   */
  csWeakRef<iSector> sector;
  /// The parent polygon.
  csPolygon3DStatic* parent;

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
  iTextureHandle* filter_texture;

  /**
   * If filter_texture is 0 then this filter is used instead.
   */
  float filter_r, filter_g, filter_b;

public:
  /// Create a portal.
  csPortalObsolete (csPolygon3DStatic* parent);

  /// Destructor.
  virtual ~csPortalObsolete ();

  /// Create a clone of this portal.
  csPortalObsolete* Clone ();

  /// Set the parent polygon.
  void SetParentPolygon (csPolygon3DStatic* parent)
  {
    csPortalObsolete::parent = parent;
  }

  //---- misc. manipulation functions ---------------------------------------

  /// Return the sector that this portal points too.
  iSector* GetSector () const;

  /**
   * Set the sector that this portal points too. To avoid circular
   * references, the sector is not IncRef'ed!
   */
  void SetSector (iSector* s);

  /// Set portal flags (see CS_PORTAL_XXX values)
  csFlags& GetFlags ();

  const csVector3* GetVertices () const;
  int* GetVertexIndices () const;
  int GetVertexIndicesCount () const;
  const csPlane3& GetObjectPlane ();
  const csPlane3& GetWorldPlane (iMovable* movable = 0);
  void ComputeCameraPlane (const csReversibleTransform& t,
  	csPlane3& camplane);
  bool PointOnPolygon (const csVector3& point, iMovable* movable = 0);

  /// Set the maximum sector visit.
  void SetMaximumSectorVisit (int msv)
  {
    max_sector_visit = msv;
  }

  /// Get the maximum sector visit.
  int GetMaximumSectorVisit () const
  {
    return max_sector_visit;
  }

  /// Set the portal callback.
  void SetPortalCallback (iPortalCallback* cb);
  void RemovePortalCallback (iPortalCallback* cb)
  {
    portal_cb_vector.Delete (cb);
  }
  int GetPortalCallbackCount () const
  {
    return portal_cb_vector.Length ();
  }
  iPortalCallback* GetPortalCallback (int idx) const;

  /// Set the missing sector callback.
  void SetMissingSectorCallback (iPortalCallback* cb);
  void RemoveMissingSectorCallback (iPortalCallback* cb)
  {
    sector_cb_vector.Delete (cb);
  }
  int GetMissingSectorCallbackCount () const
  {
    return sector_cb_vector.Length ();
  }
  iPortalCallback* GetMissingSectorCallback (int idx) const;

  /// Set the filter texture
  void SetFilter (iTextureHandle* ft);
  /// Get the filter texture
  iTextureHandle* GetTextureFilter () const;

  /// Set a color filter (instead of the texture).
  void SetFilter (float r, float g, float b);
  /// Get the current color filter
  void GetColorFilter (float &r, float &g, float &b) const;

  //---- space warping ------------------------------------------------------

  /// Get the warping transformation in object space.
  const csReversibleTransform& GetWarp () const;

  /**
   * Set the warping transformation for this portal in object space and world
   * space.
   */
  void SetWarp (const csTransform& t);

  /*
   * Set the warping transformation for this portal in object space and world
   * space.
   */
  void SetWarp (const csMatrix3 &m_w, const csVector3 &v_w_before,
    const csVector3 &v_w_after);

  /// Set warping transformation to mirror around plane.
  void SetMirror (const csPlane3& plane);

  /// Transform the warp matrix from object space to world space.
  void ObjectToWorld (const csReversibleTransform& t,
	csReversibleTransform& warp_wor) const;

  /// Hard transform the warp matrix.
  void HardTransform (const csReversibleTransform& t);

  /// Warp a position in world space.
  csVector3 Warp (const csReversibleTransform& t, const csVector3& pos) const;

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
  void WarpSpace (const csReversibleTransform& warp_wor,
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
   * 'portal_polygon' is the polygon containing this portal. This routine
   * will use the camera space plane of the portal polygon.<br>
   * 't' is the transform from object to world (this2other).
   * 'rview' is the current iRenderView.<p>
   *
   * Return true if succesful, false otherwise.
   * Failure to draw through a portal does not need to
   * be harmful. It can just mean that some maximum number is
   * reached (like the maximum number of times a certain sector
   * can be drawn through a mirror).
   */
  bool Draw (csPolygon2D* new_clipper, iPolygon3D* portal_polygon,
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
  iMeshWrapper* HitBeamPortals (const csReversibleTransform& t,
	const csVector3& start, const csVector3& end,
  	csVector3& isect, int* polygon_idx);

  /**
   * Check if the destination sector is 0 and if so call
   * the callback. This function returns false if the portal should
   * not be traversed.
   */
  bool CompleteSector (iBase* context);

  /**
   * Check frustum visibility of all polygons reachable through this portal.
   * Alpha is the alpha value you'd like to use to pass through this
   * portal (0 is no completely transparent, 100 is complete opaque).
   * 't' is the transform from object to world (this2other).
   */
  void CheckFrustum (iFrustumView* lview, const csReversibleTransform& t,
		  int alpha);

  SCF_DECLARE_IBASE_EXT (csObject);

  //------------------- iPortal implementation -----------------------
  struct Portal : public iPortal
  {
    SCF_DECLARE_EMBEDDED_IBASE (csPortalObsolete);
    virtual iObject *QueryObject () { return scfParent; }
    virtual void SetName (const char* name)
    {
      scfParent->SetName (name);
    }
    virtual const char* GetName () const
    {
      return scfParent->GetName ();
    }
    virtual iSector* GetSector () const { return scfParent->GetSector (); }
    virtual void SetSector (iSector* s) { scfParent->SetSector (s); }

    virtual const csVector3* GetVertices () const
    {
      return scfParent->GetVertices ();
    }
    virtual int* GetVertexIndices () const
    {
      return scfParent->GetVertexIndices ();
    }
    virtual int GetVertexIndicesCount () const
    {
      return scfParent->GetVertexIndicesCount ();
    }
    virtual const csPlane3& GetObjectPlane ()
    {
      return scfParent->GetObjectPlane ();
    }
    virtual const csPlane3& GetWorldPlane (iMovable* movable = 0)
    {
      return scfParent->GetWorldPlane (movable);
    }
    virtual void ComputeCameraPlane (const csReversibleTransform& t,
  	csPlane3& camplane)
    {
      scfParent->ComputeCameraPlane (t, camplane);
    }
    virtual bool PointOnPolygon (const csVector3& point,
  	iMovable* movable = 0)
    {
      return scfParent->PointOnPolygon (point, movable);
    }

    virtual csFlags& GetFlags () { return scfParent->GetFlags (); }
    virtual void SetMaximumSectorVisit (int msv)
    {
      scfParent->SetMaximumSectorVisit (msv);
    }
    virtual int GetMaximumSectorVisit () const
    {
      return scfParent->GetMaximumSectorVisit ();
    }
    virtual void SetPortalCallback (iPortalCallback* cb)
    {
      scfParent->SetPortalCallback (cb);
    }
    virtual void RemovePortalCallback (iPortalCallback* cb)
    {
      scfParent->RemovePortalCallback (cb);
    }
    virtual int GetPortalCallbackCount () const
    {
      return scfParent->GetPortalCallbackCount ();
    }
    virtual iPortalCallback* GetPortalCallback (int idx) const
    {
      return scfParent->GetPortalCallback (idx);
    }
    virtual void SetMissingSectorCallback (iPortalCallback* cb)
    {
      scfParent->SetMissingSectorCallback (cb);
    }
    virtual void RemoveMissingSectorCallback (iPortalCallback* cb)
    {
      scfParent->RemoveMissingSectorCallback (cb);
    }
    virtual int GetMissingSectorCallbackCount () const
    {
      return scfParent->GetMissingSectorCallbackCount ();
    }
    virtual iPortalCallback* GetMissingSectorCallback (int idx) const
    {
      return scfParent->GetMissingSectorCallback (idx);
    }
    virtual void SetFilter (iTextureHandle* ft)
    {
      scfParent->SetFilter (ft);
    }
    virtual iTextureHandle* GetTextureFilter () const
    {
      return scfParent->GetTextureFilter ();
    }
    virtual void SetFilter (float r, float g, float b)
    {
      scfParent->SetFilter (r, g, b);
    }
    virtual void GetColorFilter (float &r, float &g, float &b) const
    {
      scfParent->GetColorFilter (r, g, b);
    }
    virtual void SetWarp (const csMatrix3 &m_w, const csVector3 &v_w_before,
      const csVector3 &v_w_after)
    {
      scfParent->SetWarp (m_w, v_w_before, v_w_after);
    }
    virtual void SetWarp (const csTransform& t)
    {
      scfParent->SetWarp (t);
    }
    virtual void SetMirror (const csPlane3& plane)
    {
      scfParent->SetMirror (plane);
    }
    virtual const csReversibleTransform &GetWarp () const
    {
      return scfParent->GetWarp ();
    }
    virtual void HardTransform (const csReversibleTransform& t)
    {
      scfParent->HardTransform (t);
    }
    virtual void ObjectToWorld (const csReversibleTransform& t,
	csReversibleTransform& warp_wor) const
    {
      scfParent->ObjectToWorld (t, warp_wor);
    }
    virtual csVector3 Warp (const csReversibleTransform& t,
		    const csVector3& pos) const
    {
      return scfParent->Warp (t, pos);
    }
    virtual void WarpSpace (const csReversibleTransform& warp_wor,
		  csReversibleTransform& t, bool& mirror) const
    {
      scfParent->WarpSpace (warp_wor, t, mirror);
    }
    virtual bool CompleteSector (iBase* context)
    {
      return scfParent->CompleteSector (context);
    }
    virtual void CheckFrustum (iFrustumView* lview,
	const csReversibleTransform& t, int alpha)
    {
      scfParent->CheckFrustum (lview, t, alpha);
    }
    virtual iMeshWrapper* HitBeamPortals (const csReversibleTransform& t,
	const csVector3& start, const csVector3& end,
  	csVector3& isect, int* polygon_idx)
    {
      return scfParent->HitBeamPortals (t, start, end, isect, polygon_idx);
    }
  } scfiPortal;
};

#endif // __CS_PORTAL_H__
