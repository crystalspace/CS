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

#ifndef __CS_IENGINE_VISCULL_H__
#define __CS_IENGINE_VISCULL_H__

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
struct iMeshWrapper;
struct iPolygonMesh;
struct iObjectModel;
struct iDocumentNode;
class csVector3;
class csBox3;
class csSphere;
class csFlags;

SCF_VERSION (iVisibilityObjectIterator, 0, 1, 0);

/**
 * Iterator to iterate over some visibility objects.
 */
struct iVisibilityObjectIterator : public iBase
{
  /// Are there more elements?
  virtual bool HasNext () const = 0;

  /// Get next element.
  virtual iVisibilityObject* Next () = 0;

  /// Reset the iterator to the beginning.
  virtual void Reset () = 0;
};


SCF_VERSION (iVisibilityCullerListener, 0, 0, 1);

/**
 * Implement this interface when you want to get notified about visible
 * objects detected by the visibility cullers.
 */
struct iVisibilityCullerListener : public iBase
{
  /**
   * This function is called whenever the visibilty culler discovers a new
   * visible mesh.  The frustum_mask is a mask that is compatible with
   * the current set of clip planes in the iRenderView. The mask will be
   * true for every relevant plane. This can be used to further optimize
   * clipping testing.
   */
  virtual void ObjectVisible (iVisibilityObject *visobject, 
    iMeshWrapper *mesh, uint32 frustum_mask) = 0;
};

SCF_VERSION (iVisibilityCuller, 0, 7, 0);

/**
 * This interface represents a visibility culling system.
 * To use it you first register visibility objects (which are all the
 * objects for which you want to test visibility) to this culler.
 * A visibility culler can usually also support shadow calculation.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>Dynavis culler plugin (crystalspace.culling.dynavis)
 *   <li>Frustvis culler plugin (crystalspace.culling.frustvis)
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>CS_QUERY_REGISTRY()
 *   <li>CS_LOAD_PLUGIN()
 *   <li>iSector::GetVisibilityCuller()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEngine
 *   </ul>
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
  virtual bool VisTest (iRenderView* irview, 
    iVisibilityCullerListener* viscallback) = 0;
  /**
   * Precache visibility culling. This can be useful in case you want
   * to ensure that render speed doesn't get any hickups as soon as a portal
   * to this sector becomes visible. iEngine->PrecacheDraw() will call this
   * function.
   */
  virtual void PrecacheCulling () = 0;

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
   * Notify the visibility callback of all objects that intersect with the 
   * given bounding sphere.
   */
  virtual void VisTest (const csSphere& sphere, 
    iVisibilityCullerListener* viscallback) = 0;

  /**
   * Mark all objects as visible that are in the volume formed by the set
   * of planes. Can be used for frustum intersection, box intersection, ....
   * Warning! This function can only use up to 32 planes.
   */
  virtual csPtr<iVisibilityObjectIterator> VisTest (csPlane3* plane,
  	int num_planes) = 0;

  /**
   * Notify the visibility callback of all objects that are in the volume 
   * formed by the set of planes. Can be used for frustum intersection, 
   * box intersection, ....
   * \remarks Warning! This function can only use up to 32 planes.
   */
  virtual void VisTest (csPlane3* plane, int num_planes, 
    iVisibilityCullerListener* viscallback) = 0;

  /**
   * Intersect a segment with all objects in the visibility culler and
   * return them all in an iterator. This function is less accurate
   * then IntersectSegment() because it might also return objects that
   * are not even hit by the beam but just close to it.
   */
  virtual csPtr<iVisibilityObjectIterator> IntersectSegmentSloppy (
    const csVector3& start, const csVector3& end) = 0;

  /**
   * Intersect a segment with all objects in the visibility culler and
   * return them all in an iterator.
   * If accurate is true then a more accurate (and slower) method is used.
   */
  virtual csPtr<iVisibilityObjectIterator> IntersectSegment (
    const csVector3& start, const csVector3& end, bool accurate = false) = 0;

  /**
   * Intersect a beam using this culler and return the intersection
   * point, the mesh and optional polygon index. If the returned mesh is 0
   * then this means that the object belonging to the culler itself was
   * hit. Some meshes don't support returning polygon indices in which case
   * that field will always be -1.
   * If accurate is false then a less accurate (and faster) method is used.
   * In that case the polygon index will never be filled.
   */
  virtual bool IntersectSegment (const csVector3& start,
    const csVector3& end, csVector3& isect, float* pr = 0,
    iMeshWrapper** p_mesh = 0, int* poly_idx = 0,
    bool accurate = true) = 0;

  /**
   * Start casting shadows from a given point in space. What this will
   * do is traverse all objects registered to the visibility culler.
   * If some object implements iShadowCaster then this function will
   * use the shadows casted by that object and put them in the frustum
   * view. This function will then also call the object function which
   * is assigned to iFrustumView. That object function will (for example)
   * call iShadowReceiver->CastShadows() to cast the collected shadows
   * on the shadow receiver.
   */
  virtual void CastShadows (iFrustumView* fview) = 0;

  /**
   * Parse a document node with additional parameters for this culler.
   * Returns error message on error or 0 otherwise.
   */
  virtual const char* ParseCullerParameters (iDocumentNode* node) = 0;
};

/** \name GetCullerFlags() flags
 * @{ */
/**
 * This is a good occluder. With this hint you say that
 * this object is a good occluder. The culler can still ignore
 * this hint of course.
 */
#define CS_CULLER_HINT_GOODOCCLUDER 4

/**
 * This is a bad occluder. With this hint you say that this
 * object is almost certainly a bad occluder.
 */
#define CS_CULLER_HINT_BADOCCLUDER 8

/** @} */

SCF_VERSION (iVisibilityObject, 0, 2, 1);

/**
 * An object that wants to know if it is visible or not
 * for some visibility culler needs to implement this interface.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>Mesh wrapper in the engine implements this.
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>SCF_QUERY_INTERFACE() from iMeshWrapper.
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>Visibility culler plugins (iVisibilityCuller).
 *   </ul>
 */
struct iVisibilityObject : public iBase
{
  /// Get the reference to the movable from this object.
  virtual iMovable* GetMovable () const = 0;
  /// Get the reference to the mesh wrapper from this object.
  virtual iMeshWrapper* GetMeshWrapper () const = 0;

  /**
   * Get the object model corresponding with this object.
   */
  virtual iObjectModel* GetObjectModel () = 0;

  /**
   * Get flags for this object. This is a combination of zero or more of the
   * following flags. See the documentation with these flags for more info:
   * <ul>
   * <li>#CS_CULLER_HINT_GOODOCCLUDER
   * <li>#CS_CULLER_HINT_BADOCCLUDER
   * </ul>
   */
  virtual csFlags& GetCullerFlags () = 0;
};

/** @} */

#endif // __CS_IENGINE_VISCULL_H__

