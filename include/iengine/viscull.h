/*
    Crystal Space 3D engine
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __IENGINE_VISCULL_H__
#define __IENGINE_VISCULL_H__

/**\file
 */
/**
 * \addtogroup engine3d_vis
 * @{ */
 
#include "csutil/scf.h"

struct iRenderView;
struct iFrustumView;
struct iVisibilityObject;
struct iMovable;
struct iShadowReceiver;
struct iPolygon3D;
struct iMeshWrapper;
struct iPolygonMesh;
struct iObjectModel;
class csVector3;
class csBox3;
class csSphere;

SCF_VERSION (iVisibilityObjectIterator, 0, 0, 1);

/**
 * Iterator to iterate over some visibility objects.
 */
struct iVisibilityObjectIterator : public iBase
{
  /// Move forward.
  virtual bool Next () = 0;

  /// Reset the iterator to the beginning.
  virtual void Reset () = 0;

  /// Get the object we are pointing at.
  virtual iVisibilityObject* GetObject () const = 0;

  /// Check if we have any more children.
  virtual bool IsFinished () const = 0;
};

SCF_VERSION (iVisibilityCuller, 0, 2, 0);

/**
 * This interface represents a visibility culling system.
 * To use it you first register visibility objects (which are all the
 * objects for which you want to test visibility) to this culler.
 * A visibility culler can usually also support shadow calculation.
 */
struct iVisibilityCuller : public iBase
{
  /**
   * Setup all data for this visibility culler. This needs
   * to be called before the culler is used for the first time.
   * The given name will be used to cache the data.
   */
  virtual void Setup (const char* name) = 0;
  /**
   * Register a visibility object with this culler.
   * If this visibility object also supports iShadowCaster and
   * this visibility culler supports shadow casting then it will
   * automatically get registered as a shadow caster as well.
   * Same for iShadowReceiver.
   */
  virtual void RegisterVisObject (iVisibilityObject* visobj) = 0;
  /// Unregister a visibility object with this culler.
  virtual void UnregisterVisObject (iVisibilityObject* visobj) = 0;
  /**
   * Do the visibility test from a given viewpoint. This will first
   * clear the visible flag on all registered objects and then it will
   * mark all visible objects. If this function returns false then
   * all objects are visible.
   */
  virtual bool VisTest (iRenderView* irview) = 0;

  /**
   * Mark all objects as visible that intersect with the given bounding
   * box.
   */
  virtual csPtr<iVisibilityObjectIterator> VisTest (const csBox3& box) = 0;

  /**
   * Mark all objects as visible that intersect with the given bounding
   * sphere.
   */
  virtual csPtr<iVisibilityObjectIterator> VisTest (const csSphere& sphere) = 0;

  /**
   * Intersect a beam using this culler and return the intersection
   * point, the mesh and optional polygon. If the returned mesh is NULL
   * then this means that the object belonging to the culler itself was
   * hit. This function will also detect hits with non-thing objects.
   * In that case the returned polygon will always be NULL.
   */
  virtual bool IntersectSegment (const csVector3& start,
    const csVector3& end, csVector3& isect, float* pr = NULL,
    iMeshWrapper** p_mesh = NULL, iPolygon3D** poly = NULL) = 0;

  /**
   * Start casting shadows from a given point in space.
   */
  virtual void CastShadows (iFrustumView* fview) = 0;

  /**
   * Get the current visibility number. You can compare this number
   * to the visibility number as returned by
   * iVisibilityObject->GetVisibilityNumber(). If equal then the object
   * was visible.
   */
  virtual uint32 GetCurrentVisibilityNumber () const = 0;
};

SCF_VERSION (iVisibilityObject, 0, 2, 0);

/**
 * An object that wants to know if it is visible or not
 * for some visibility culler needs to implement this interface.
 */
struct iVisibilityObject : public iBase
{
  /// Get the reference to the movable from this object.
  virtual iMovable* GetMovable () const = 0;
  /// Get the reference to the mesh wrapper from this object.
  virtual iMeshWrapper* GetMeshWrapper () const = 0;

  /**
   * Set the visibility number for this object. A visibility culler
   * will set the visibility number of an object equal to the current
   * visibility culler number if the object is visible.
   */
  virtual void SetVisibilityNumber (uint32 visnr) = 0;
  /**
   * Get the visibility number. You can compare this with
   * iVisibilityCuller->GetCurrentVisibilityNumber(). If equal then
   * this object is visible.
   */
  virtual uint32 GetVisibilityNumber () const = 0;

  /**
   * Get the object model corresponding with this object.
   */
  virtual iObjectModel* GetObjectModel () = 0;
};

/** @} */

#endif // __IENGINE_VISCULL_H__

