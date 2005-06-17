/*
    Crystal Space 3D engine
    Copyright (C) 1998-2001 by Jorrit Tyberghein
                  2004 by Marten Svanfeldt

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

#ifndef __CS_IENGINE_SECTOR_H__
#define __CS_IENGINE_SECTOR_H__

/**\file
 */
/**
 * \addtogroup engine3d
 * @{ */

#include "cstypes.h"
#include "csutil/scf.h"
#include "csutil/hash.h"
#include "csutil/set.h"
#include "iutil/object.h"

class csVector3;
class csColor;
class csBox3;
class csReversibleTransform;
struct iMeshWrapper;
struct iMeshList;
struct iLightList;
struct iLight;
struct iThing;
struct iVisibilityCuller;
struct iVisibilityObject;
struct iObject;
struct csFog;
struct iGraphics3D;
struct iRenderView;
struct iRenderLoop;
struct iFrustumView;
struct iSector;
struct iPortal;
struct iDocumentNode;
class csRenderMeshList;

SCF_VERSION (iSectorCallback, 0, 0, 1);

/**
 * Set a callback which is called when this sector is traversed.
 * The given context will be either an instance of iRenderView, iFrustumView,
 * or else 0.
 */
struct iSectorCallback : public iBase
{
  /**
   * Sector will be traversed. It is safe to delete this callback
   * in this function.
   */
  virtual void Traverse (iSector* sector, iBase* context) = 0;
};

SCF_VERSION (iSectorMeshCallback, 0, 0, 1);

/**
 * Set a callback which is called when a mesh is added or removed
 * from this sector.
 */
struct iSectorMeshCallback : public iBase
{
  /**
   * New mesh. Note that this is also called if the mesh is added as child
   * of another mesh that is in the sector.
   */
  virtual void NewMesh (iSector* sector, iMeshWrapper* mesh) = 0;

  /**
   * Remove mesh.
   */
  virtual void RemoveMesh (iSector* sector, iMeshWrapper* mesh) = 0;
};

SCF_VERSION (iSector, 0, 5, 5);

/**
 * The iSector interface is used to work with "sectors". A "sector"
 * is an empty region of space that can contain other objects (mesh
 * objects). A sector itself does not represent geometry but only
 * contains other geometry. A sector does contain lights though.
 * The sector is the basic building block for any Crystal Space level.
 * A level can be made from one or more sectors. Using the thing mesh
 * object one can use portals to connect multiple sectors.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iEngine::CreateSector()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iEngine::FindSector()
 *   <li>iSectorList::Get()
 *   <li>iSectorList::FindByName()
 *   <li>iLoaderContext::FindSector()
 *   <li>iPortal::GetSector()
 *   <li>iCamera::GetSector()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEngine
 *   </ul>
 */
struct iSector : public iBase
{
  /// Get the iObject for this sector.
  virtual iObject *QueryObject () = 0;

  /**
   * Set the renderloop to use for this sector. If this is not set then
   * the default engine renderloop will be used.
   */
  virtual void SetRenderLoop (iRenderLoop* rl) = 0;

  /**
   * Get the renderloop for this sector. If this returns 0 then it
   * means there is no specific renderloop for this sector. In that case
   * the default renderloop in the engine will be used.
   */
  virtual iRenderLoop* GetRenderLoop () = 0;

  /// Has this sector fog?
  virtual bool HasFog () const = 0;
  /// Return the fog structure (even if fog is disabled)
  virtual csFog *GetFog () const = 0;
  /// Fill the fog structure with the given values
  virtual void SetFog (float density, const csColor& color) = 0;
  /// Disable fog in this sector
  virtual void DisableFog () = 0;

  /// Get the list of meshes in this sector.
  virtual iMeshList* GetMeshes () = 0;
  /**
   * Get the list of static and pseudo-dynamic lights in this sector.
   */
  virtual iLightList* GetLights () = 0;

  /// Calculate lighting for all objects in this sector
  virtual void ShineLights () = 0;
  /// Version of ShineLights() which only affects one mesh object.
  virtual void ShineLights (iMeshWrapper*) = 0;

  /**
   * Sets dynamic ambient light this sector. This works in addition
   * to the dynamic light you can specify for every object.
   */
  virtual void SetDynamicAmbientLight (const csColor& color) = 0;

  /// Get the last set dynamic ambient light for this sector.
  virtual csColor GetDynamicAmbientLight () const = 0;

  /**
   * Get the version number of the dynamic ambient color. This
   * number is increased whenever dynamic ambient changes.
   */
  virtual uint GetDynamicAmbientVersion () const = 0;

  /**
   * Calculate the bounding box of all objects in this sector.
   * This function is not very efficient as it will traverse all objects
   * in the sector one by one and compute a bounding box from that.
   */
  virtual void CalculateSectorBBox (csBox3& bbox,
    bool do_meshes) const = 0;

  /**
   * Use the specified plugin as the visibility culler for
   * this sector. Returns false if the culler could not be
   * loaded for some reason.
   * The optional culler parameters will be given to the new
   * visibility culler.
   */
  virtual bool SetVisibilityCullerPlugin (const char* name,
  	iDocumentNode* culler_params = 0) = 0;
  /**
   * Get the visibility culler that is used for this sector.
   * If there is no culler yet a culler of type 'crystalspace.culling.frustvis'
   * will be created and used for this sector.
   */
  virtual iVisibilityCuller* GetVisibilityCuller () = 0;

  /**
   * Get the current draw recursion level.
   */
  virtual int GetRecLevel () const = 0;

  /**
   * Add one draw recursion level.
   */
  virtual void IncRecLevel () = 0;

  /**
   * Remove one draw recursion level.
   */
  virtual void DecRecLevel () = 0;

  /**
   * Follow a beam from start to end and return the first polygon that
   * is hit. This function correctly traverse portals and space warping
   * portals. Normally the sector you call this on should be the sector
   * containing the 'start' point. 'isect' will be the intersection point
   * if a polygon is returned. This function returns -1 if no polygon
   * was hit or the polygon index otherwise.
   */
  virtual iMeshWrapper* HitBeamPortals (const csVector3& start,
  	const csVector3& end, csVector3& isect, int* polygon_idx) = 0;

  /**
   * Follow a beam from start to end and return the first object
   * that is hit. In case it is a thing the polygon_idx field will be
   * filled with the indices of the polygon that was hit.
   * If polygon_idx is null then the polygon will not be filled in.
   * This function doesn't support portals.
   */
  virtual iMeshWrapper* HitBeam (const csVector3& start, const csVector3& end,
    csVector3& intersect, int* polygon_idx, bool accurate = false) = 0;

  /**
   * Follow a segment starting at this sector. If the segment intersects
   * with a polygon it will stop there unless the polygon is a portal in which
   * case it will recursively go to that sector (possibly applying warping
   * transformations) and continue there.<p>
   *
   * This routine will modify all the given parameters to reflect space warping.
   * These should be used as the new camera transformation when you decide to
   * really go to the new position.<p>
   *
   * This function returns the resulting sector and new_position will be set
   * to the last position that you can go to before hitting a wall.<p>
   *
   * If only_portals is true then only portals will be checked. This
   * means that intersection with normal polygons is not checked. This
   * is a lot faster but it does mean that you need to use another
   * collision detection system to test with walls.
   */
  virtual iSector* FollowSegment (csReversibleTransform& t,
    csVector3& new_position, bool& mirror, bool only_portals = false) = 0;

  /// Draw the sector with the given render view
  virtual void Draw (iRenderView* rview) = 0;

  /**
   * Prepare the sector to draw.
   * Must be called before any rendermesh is requested.
   */
  virtual void PrepareDraw (iRenderView* rview) = 0;

  /**
   * Get a set of visible meshes for given camera. These will be cached for
   * a given frame and camera, but if the cached result isn't enough it will
   * be reculled. The returned pointer is valid as long as the sector exsist
   * (the sector will delete it)
   */
  virtual csRenderMeshList* GetVisibleMeshes (iRenderView *) = 0;

  /**
   * Set the sector callback. This will call IncRef() on the callback
   * So make sure you call DecRef() to release your own reference.
   */
  virtual void SetSectorCallback (iSectorCallback* cb) = 0;

  /**
   * Remove a sector callback.
   */
  virtual void RemoveSectorCallback (iSectorCallback* cb) = 0;

  /// Get the number of sector callbacks.
  virtual int GetSectorCallbackCount () const = 0;

  /// Get the specified sector callback.
  virtual iSectorCallback* GetSectorCallback (int idx) const = 0;

  /**
   * Add a mesh callback. This will call IncRef() on the callback
   * So make sure you call DecRef() to release your own reference.
   */
  virtual void AddSectorMeshCallback (iSectorMeshCallback* cb) = 0;

  /**
   * Remove a mesh callback.
   */
  virtual void RemoveSectorMeshCallback (iSectorMeshCallback* cb) = 0;

  /// Used for portal traversal.
  virtual void CheckFrustum (iFrustumView* lview) = 0;

  /**
   * Get the set of meshes containing portals that leave from this sector.
   * Note that portals are uni-directional. The portals represented
   * by this list are portals that are on some mesh object that is
   * actually located in this sector.
   */
  virtual const csSet<csPtrKey<iMeshWrapper> >& GetPortalMeshes () const = 0;
  /**
   * Register a mesh with a portal. @@@ TO BE REMOVED...
   */
  virtual void RegisterPortalMesh (iMeshWrapper* mesh) = 0;
  /**
   * Unregister a mesh with a portal. @@@ TO BE REMOVED...
   */
  virtual void UnregisterPortalMesh (iMeshWrapper* mesh) = 0;

  /**
   * Unlink all mesh objects from this sector. This will not remove
   * the mesh objects but simply unlink them so that they are no longer
   * part of this sector. This happens automatically when the sector
   * is removed but you can force it by calling this function here.
   */
  virtual void UnlinkObjects () = 0;
};


SCF_VERSION (iSectorList, 0, 0, 2);

/**
 * A list of sectors.
 * <p>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iEngine::GetSectors()
 *   <li>iMovable::GetSectors()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEngine
 *   </ul>
 */
struct iSectorList : public iBase
{
  /// Return the number of sectors in this list.
  virtual int GetCount () const = 0;

  /// Return a sector by index.
  virtual iSector *Get (int n) const = 0;

  /// Add a sector.
  virtual int Add (iSector *obj) = 0;

  /// Remove a sector.
  virtual bool Remove (iSector *obj) = 0;

  /// Remove the nth sector.
  virtual bool Remove (int n) = 0;

  /// Remove all sectors.
  virtual void RemoveAll () = 0;

  /// Find a sector and return its index.
  virtual int Find (iSector *obj) const = 0;

  /// Find a sector by name.
  virtual iSector *FindByName (const char *Name) const = 0;
};

SCF_VERSION (iSectorIterator, 0, 1, 0);

/**
 * An iterator to iterate over sectors. Some functions in CS
 * return this.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iEngine::GetNearbySectors()
 *   </ul>
 */
struct iSectorIterator : public iBase
{
  /// Return true if there are more elements.
  virtual bool HasNext () = 0;

  /// Get sector from iterator. Return 0 at end.
  virtual iSector* Next () = 0;

  /**
   * Get last position that was used from Fetch. This can be
   * different from 'pos' because of space warping.
   */
  virtual const csVector3& GetLastPosition () = 0;

  /// Restart iterator.
  virtual void Reset () = 0;
};

/** @} */

#endif // __CS_IENGINE_SECTOR_H__
