/*
    Crystal Space 3D engine
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

#ifndef __IENGINE_SECTOR_H__
#define __IENGINE_SECTOR_H__

#include "cstypes.h"
#include "csutil/scf.h"

class csVector3;
class csSector;
class csColor;
class csBox3;
class csReversibleTransform;
struct iCollection;
struct iMeshWrapper;
struct iThing;
struct iStatLight;
struct iVisibilityCuller;
struct iObject;
struct csFog;
struct iGraphics3D;
struct iPolygon3D;

SCF_VERSION (iSector, 0, 2, 12);

/**
 * The iSector interface is used to work with "sectors". A "sector"
 * is a convex polyhedron, that possibly contains portals, things,
 * sprites, lights and so on. Simply speaking, a "sector" is analogous
 * to an real-world room, rooms are interconnected with doors and windows
 * (e.g. portals), and rooms contain miscelaneous things, sprites
 * and lights.
 */
struct iSector : public iBase
{
  /// Used by the engine to retrieve internal sector object (ugly)
  virtual csSector *GetPrivateObject () = 0;
  /// Get the iObject for this sector.
  virtual iObject *QueryObject() = 0;

  /// Has this sector fog?
  virtual bool HasFog () const = 0;
  /// Return the fog structure (even if fog is disabled)
  virtual csFog *GetFog () const = 0;
  /// Fill the fog structure with the given values
  virtual void SetFog (float density, const csColor& color) = 0;
  /// Disable fog in this sector
  virtual void DisableFog () = 0;

  /// Return the number of mesh objects in this sector.
  virtual int GetMeshCount () const = 0;
  /// Return a mesh wrapper by index.
  virtual iMeshWrapper *GetMesh (int n) const = 0;
  /// Add a mesh object to this sector.
  virtual void AddMesh (iMeshWrapper *pMesh) = 0;
  /// Find a mesh object by name.
  virtual iMeshWrapper *GetMesh (const char *name) const = 0;
  /// Unlink a mesh object.
  virtual void UnlinkMesh (iMeshWrapper *pMesh) = 0;

  /// Return the number of collection objects in this sector.
  virtual int GetCollectionCount () const = 0;
  /// Return a collection by index.
  virtual iCollection *GetCollection (int n) const = 0;
  /// Add a collection to this sector.
  virtual void AddCollection (iCollection* col) = 0;
  /// Find a collection by name.
  virtual iCollection *GetCollection (const char *name) const = 0;
  /// Unlink a collection.
  virtual void UnlinkCollection (iCollection* col) = 0;

  /// Return the number of lights in this sector.
  virtual int GetLightCount () const = 0;
  /// Return a light by index.
  virtual iStatLight *GetLight (int n) const = 0;
  /// Return a light by name.
  virtual iStatLight *GetLight (const char* name) const = 0;
  /// Add a static or pseudo-dynamic light to this sector.
  virtual void AddLight (iStatLight *light) = 0;
  /// Find a light with the given position and radius.
  virtual iStatLight *FindLight (float x, float y, float z, float dist)
    const = 0;

  /// Calculate lighting for all objects in this sector
  virtual void ShineLights () = 0;
  /// Version of ShineLights() which only affects one mesh object.
  virtual void ShineLights (iMeshWrapper*) = 0;

  /**
   * Calculate the bounding box of all objects in this sector.
   * This function is not very efficient as it will traverse all objects
   * in the sector one by one and compute a bounding box from that.
   */
  virtual void CalculateSectorBBox (csBox3& bbox,
  	bool do_meshes) const = 0;

  /**
   * Use the specified mesh object as the visibility culler for
   * this sector.
   */
  virtual void SetVisibilityCuller (const char *Name) = 0;
  /**
   * Get the visibility culler that is used for this sector.
   * NULL if none.
   */
  virtual iVisibilityCuller* GetVisibilityCuller () const = 0;

  /**
   * Get the current draw recursion level.
   */
  virtual int GetRecLevel () const = 0;

  /**
   * Follow a beam from start to end and return the first polygon that
   * is hit. This function correctly traverse portals and space warping
   * portals. Normally the sector you call this on should be the sector
   * containing the 'start' point. 'isect' will be the intersection point
   * if a polygon is returned.
   */
  virtual iPolygon3D* HitBeam (const csVector3& start, const csVector3& end,
  	csVector3& isect) = 0;

  /**
   * Follow a beam from start to end and return the first object
   * that is hit. Objects can be meshes, things, or sectors.
   * In case it is a thing or sector the iPolygon3D field will be
   * filled with the polygon that was hit.
   * If polygonPtr is null then the polygon will not be filled in.
   */
  virtual iObject* HitBeam (const csVector3& start, const csVector3& end,
  	csVector3& intersect, iPolygon3D** polygonPtr) = 0;

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
   * to the last position that you can go to before hitting a wall.
   */
  virtual iSector* FollowSegment (csReversibleTransform& t,
  	csVector3& new_position, bool& mirror) = 0;
};

#endif // __IENGINE_SECTOR_H__

