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
#include "imesh/thing/portal.h"

class csPolygon2D;
class csPolygon3D;
class csStatLight;
class csObject;
class csMeshWrapper;
struct iRenderView;
struct iFrustumView;

/**
 * This class represents a portal. It belongs to some polygon
 * which is then considered a portal to another sector.
 */
class csPortal : public csObject
{
private:
  /// The sector that this portal points to.
  iSector* sector;

public:
  /// Set of flags
  csFlags flags;

protected:
  /// Warp transform in object space.
  csReversibleTransform warp_obj;
  /// Warp transform in world space.
  csReversibleTransform warp_wor;
  /// Callback when a sector is missing.
  iPortalCallback* sector_cb;
  /// Callback for traversing to a portal.
  iPortalCallback* portal_cb;
  /// Maximum number of time a single sector will be visited by this portal.
  int max_sector_visit;

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
  /// Create a portal.
  csPortal ();

  /// Destructor.
  virtual ~csPortal ();

  //---- misc. manipulation functions ---------------------------------------

  /// For iReference.
  iReferencedObject* GetReferencedObject () const;

  /// For iReference.
  void SetReferencedObject (iReferencedObject* b);

  /// Return the sector that this portal points too.
  iSector* GetSector () const;

  /**
   * Set the sector that this portal points too. To avoid circular
   * references, the sector is not IncRef'ed!
   */
  void SetSector (iSector* s);

  /// Set portal flags (see CS_PORTAL_XXX values)
  csFlags& GetFlags ();

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

  /// Get the portal callback.
  iPortalCallback* GetPortalCallback () const;

  /// Set the missing sector callback.
  void SetMissingSectorCallback (iPortalCallback* cb);

  /// Get the missing sector callback.
  iPortalCallback* GetMissingSectorCallback () const;

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

  /// Set warping transformation to mirror
  void SetMirror (iPolygon3D *iPoly);

  /// Transform the warp matrix from object space to world space.
  void ObjectToWorld (const csReversibleTransform& t);

  /// Hard transform the warp matrix.
  void HardTransform (const csReversibleTransform& t);

  /// Warp a position in world space.
  csVector3 Warp (const csVector3& pos) const;

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
  void WarpSpace (csReversibleTransform& t, bool& mirror) const;

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
  csMeshWrapper* HitBeam (const csVector3& start, const csVector3& end,
  	csVector3& isect, csPolygon3D** polygonPtr);

  /**
   * Check if the destination sector is NULL and if so call
   * the callback. This function returns false if the portal should
   * not be traversed.
   */
  bool CompleteSector (iBase* context);

  /**
   * Check frustum visibility of all polygons reachable through this portal.
   * Alpha is the alpha value you'd like to use to pass through this
   * portal (0 is no completely transparent, 100 is complete opaque).
   */
  void CheckFrustum (iFrustumView* lview, int alpha);

  SCF_DECLARE_IBASE_EXT (csObject);

  //------------------- iPortal implementation -----------------------
  struct Portal : public iPortal
  {
    SCF_DECLARE_EMBEDDED_IBASE (csPortal);
    virtual iReferencedObject* GetReferencedObject () const
    { return scfParent->GetReferencedObject (); }
    virtual void SetReferencedObject (iReferencedObject* b)
    { scfParent->SetReferencedObject (b); }
    virtual iObject *QueryObject () { return scfParent; }
    virtual iSector* GetSector () const { return scfParent->GetSector (); }
    virtual void SetSector (iSector* s) { scfParent->SetSector (s); }
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
    virtual iPortalCallback* GetPortalCallback () const
    {
      return scfParent->GetPortalCallback ();
    }
    virtual void SetMissingSectorCallback (iPortalCallback* cb)
    {
      scfParent->SetMissingSectorCallback (cb);
    }
    virtual iPortalCallback* GetMissingSectorCallback () const
    {
      return scfParent->GetMissingSectorCallback ();
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
    virtual void SetMirror (iPolygon3D *iPoly)
    {
      scfParent->SetMirror (iPoly);
    }
    virtual const csReversibleTransform &GetWarp () const
    {
      return scfParent->GetWarp ();
    }
    virtual void ObjectToWorld (const csReversibleTransform& t)
    {
      scfParent->ObjectToWorld (t);
    }
    virtual void HardTransform (const csReversibleTransform& t)
    {
      scfParent->HardTransform (t);
    }
    virtual csVector3 Warp (const csVector3& pos) const
    {
      return scfParent->Warp (pos);
    }
    virtual void WarpSpace (csReversibleTransform& t, bool& mirror) const
    {
      scfParent->WarpSpace (t, mirror);
    }
    virtual bool CompleteSector (iBase* context)
    {
      return scfParent->CompleteSector (context);
    }
    virtual void CheckFrustum (iFrustumView* lview, int alpha)
    {
      scfParent->CheckFrustum (lview, alpha);
    }
  } scfiPortal;
};

#endif // __CS_PORTAL_H__
