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

#ifndef __CS_SECTOROBJ_H__
#define __CS_SECTOROBJ_H__

#include "csutil/csobject.h"
#include "csutil/flags.h"
#include "csgeom/box.h"
#include "csengine/movable.h"
#include "iengine/viscull.h"

struct iObjectModel;
struct iSector;
struct iCamera;
struct iMeshWrapper;

/**
 * This is a base class for all objects that can be put in a sector.
 * Currently these are csMeshWrapper and csPortalContainer.
 */
class csSectorObject : public csObject, public iVisibilityObject
{
  friend class csMovable;
  friend class csMovableSectorList;

protected:
  /// The parent sector object, or 0
  iMeshWrapper *Parent;

  /**
   * Bounding box in world space.
   * This is a cache for GetWorldBoundingBox() which will recalculate this
   * if the movable changes (by using movablenr).
   */
  csBox3 wor_bbox;
  /// Last used movable number for wor_bbox.
  long wor_bbox_movablenr;

  /**
   * Current visibility number used by the visibility culler.
   */
  uint32 visnr;

  /**
   * Position in the world.
   */
  csMovable movable;

  /**
   * The renderer will render all objects in a sector based on this
   * number. Low numbers get rendered first. High numbers get rendered
   * later. There are a few predefined slots which the application is
   * free to use or not.
   */
  long render_priority;

  /// Get the bounding box in world space and correct in hierarchy.
  void GetFullBBox (csBox3& box);

  /// Move this object to the specified sector. Can be called multiple times.
  virtual void MoveToSector (iSector* s) = 0;

  /// Remove this object from all sectors it is in (but not from the engine).
  virtual void RemoveFromSectors () = 0;

  /**
   * Update transformations after the object has moved
   * (through updating the movable instance).
   * This MUST be done after you change the movable otherwise
   * some of the internal data structures will not be updated
   * correctly. This function is called by movable.UpdateMove();
   */
  virtual void UpdateMove () = 0;

public:
  /// Set of flags
  csFlags flags;
  /// Culler flags.
  csFlags culler_flags;

protected:
  /**
   * Destructor.  This is private in order to force clients to use DecRef()
   * for object destruction.
   */
  virtual ~csSectorObject ();

public:
  /// Constructor.
  csSectorObject (iMeshWrapper* theParent);

  /// Set parent container for this object.
  void SetParentContainer (iMeshWrapper* newParent) { Parent = newParent; }
  /// Get parent container for this object.
  iMeshWrapper* GetParentContainer () const { return Parent; }

  /// Mark this object as visible (for iVisibilityObject).
  virtual void SetVisibilityNumber (uint32 vis)
  {
    visnr = vis;
  }

  /// Return if this object is visible (for iVisibilityObject). 
  virtual uint32 GetVisibilityNumber () const { return visnr; }

  /**
   * Pure abstract function to return the object model associated with
   * this object (for iVisibilityObject).
   */
  virtual iObjectModel* GetObjectModel () = 0;

  // For iVisibilityObject:
  virtual iMeshWrapper* GetMeshWrapper () const = 0;

  // For iVisibilityObject:
  virtual csFlags& GetCullerFlags () { return culler_flags; }

  /**
   * Calculate the squared distance between the camera and the object.
   */
  float GetSquaredDistance (iRenderView *rview);

  /**
   * Get the movable instance for this object.
   * It is very important to call GetMovable().UpdateMove()
   * after doing any kind of modification to this movable
   * to make sure that internal data structures are
   * correctly updated.
   */
  csMovable& GetCsMovable () { return movable; }

  // For iVisibilityObject.
  virtual iMovable* GetMovable () const
  {
    return (iMovable*)&(movable.scfiMovable);
  }

  /**
   * This routine will find out in which sectors a mesh object
   * is positioned. To use it the mesh has to be placed in one starting
   * sector. This routine will then start from that sector, find all
   * portals that touch the sprite and add all additional sectors from
   * those portals. Note that this routine using a bounding sphere for
   * this test so it is possible that the mesh will be added to sectors
   * where it really isn't located (but the sphere is).
   * <p>
   * If the mesh is already in several sectors those additional sectors
   * will be ignored and only the first one will be used for this routine.
   */
  void PlaceMesh ();

  /**
   * Check if this object is hit by this object space vector.
   * BBox version.
   */
  int HitBeamBBox (const csVector3& start, const csVector3& end,
         csVector3& isect, float* pr);

  /**
   * Get the bounding box of this object in world space.
   * This routine will cache the bounding box and only recalculate it
   * if the movable changes.
   */
  void GetWorldBoundingBox (csBox3& cbox);

  /**
   * Get the bounding box of this object after applying a transformation to it.
   * This is really a very inaccurate function as it will take the bounding
   * box of the object in object space and then transform this bounding box.
   */
  void GetTransformedBoundingBox (const csReversibleTransform& trans,
  	csBox3& cbox);

  /**
   * Get a very inaccurate bounding box of the object in screen space.
   * Returns -1 if object behind the camera or else the distance between
   * the camera and the furthest point of the 3D box.
   */
  float GetScreenBoundingBox (const iCamera *camera, csBox2& sbox,
  	csBox3& cbox);

  /// Set the render priority for this object.
  virtual void SetRenderPriority (long rp)
  {
    render_priority = rp;
  }
  /// Get the render priority for this object.
  long GetRenderPriority () const
  {
    return render_priority;
  }

  //--------------------- SCF stuff follows ------------------------------//
  SCF_DECLARE_IBASE_EXT (csObject);
};

#endif // __CS_SECTOROBJ_H__

