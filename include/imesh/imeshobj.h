/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein
  
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

#ifndef __IMESH_IMESHOBJ_H__
#define __IMESH_IMESHOBJ_H__

#include "csutil/scf.h"
#include "csgeom/box.h"
#include "isys/iplugin.h"

struct iMeshObject;
struct iMeshObjectFactory;
struct iRenderView;
struct iMovable;
struct iLight;
class csReversibleTransform;

/**
 * For iMeshObject::GetObjectBoundingBox() get a normal bounding box which
 * may or may not be recalculated depending on the changing geometry of
 * the object.
 */
#define CS_BBOX_NORMAL 0
/**
 * For iMeshObject::GetObjectBoundingBox() get a totally accurate bounding
 * box. Not all plugins support this. Some will just return a normal
 * bounding box.
 */
#define CS_BBOX_ACCURATE 1
/**
 * For iMeshObject::GetObjectBoundingBox() get the maximum bounding box
 * that this object will ever use. For objects that don't have a preset
 * maximum bounding box this just has to be a reasonable estimate of
 * a realistic maximum bounding box.
 */
#define CS_BBOX_MAX 2

/// A callback function for MeshObj::Draw().
typedef void (csMeshCallback) (iMeshObject* spr, iRenderView* rview,
	void* callbackData);

SCF_VERSION (iMeshObject, 0, 0, 13);

/**
 * This is a general mesh object that the engine can interact with.
 */
struct iMeshObject : public iBase
{
  /**
   * Get the reference to the factory that created this mesh object.
   */
  virtual iMeshObjectFactory* GetFactory () = 0;

  /**
   * First part of Draw. The engine will call this DrawTest() before
   * calling Draw() so DrawTest() can (if needed) remember computationally
   * expensive data. If DrawTest() returns false the engine will not
   * call Draw(). Possibly UpdateLighting() will be called in between
   * DrawTest() and Draw().
   */
  virtual bool DrawTest (iRenderView* rview, iMovable* movable) = 0;

  /**
   * Update lighting for the object on the given position.
   */
  virtual void UpdateLighting (iLight** lights, int num_lights,
      	iMovable* movable) = 0;

  /**
   * Draw this mesh object. Returns false if not visible.
   * If this function returns true it does not mean that the object
   * is invisible. It just means that this MeshObject thinks that the
   * object was probably visible. DrawTest() will be called before
   * this function (possibly with an UpdateLighting() in between.
   */
  virtual bool Draw (iRenderView* rview, iMovable* movable) = 0;

  /**
   * Register a callback to the mesh object which will be called
   * from within Draw() if the mesh object thinks that the object is
   * really visible. Depending on the type of mesh object this can be
   * very accurate or not accurate at all. But in all cases it will
   * certainly be called if the object is visible.
   */
  virtual void SetVisibleCallback (csMeshCallback* cb, void* cbData) = 0;

  /**
   * Get the current visible callback.
   */
  virtual csMeshCallback* GetVisibleCallback () = 0;

  /**
   * Get the bounding box in object space for this mesh object.
   * Type has three possibilities:
   * <ul>
   * <li> CS_BBOX_NORMAL: get a normal bounding box which
   *      may or may not be recalculated depending on the changing
   *      geometry of the object.
   * <li> CS_BBOX_ACCURATE: get a totally accurate bounding
   *      box. Not all plugins support this. Some will just return a normal
   *      bounding box.
   * <li> CS_BBOX_MAX: get the maximum bounding box
   *      that this object will ever use. For objects that don't have a
   *      preset maximum bounding box this just has to be a reasonable
   *      estimate of a realistic maximum bounding box.
   * </ul>
   */
  virtual void GetObjectBoundingBox(csBox3& bbox, int type = CS_BBOX_NORMAL)=0;

  /**
   * Get the radius of this object in object space.
   */
  virtual csVector3 GetRadius () = 0;

  /**
   * Control animation of this object.
   */
  virtual void NextFrame (cs_time current_time) = 0;

  /**
   * If this method returns true this object wants to die. The
   * user of this object should take care to make it die at the
   * soonest possible time. This is usally used for things like
   * particle systems that only have a limited time to live.
   */
  virtual bool WantToDie () = 0;

  /**
   * Do a hard transform of this object.
   * This transformation and the original coordinates are not
   * remembered but the object space coordinates are directly
   * computed (world space coordinates are set to the object space
   * coordinates by this routine). Note that some implementations
   * of mesh objects will not change the orientation of the object but
   * only the position.
   */
  virtual void HardTransform (const csReversibleTransform& t) = 0;

  /**
   * Return true if HardTransform is supported for this mesh object type.
   */
  virtual bool SupportsHardTransform () = 0;

  /**
   * Check if this mesh is hit by this object space vector.
   * Return the collision point in object space coordinates.
   */
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr) = 0;

  /**
   * Return a number which will change as soon as the object undergoes
   * a fundamental change that affects the maximum bounding box. Using
   * this number you can test if you need to get the bounding box again
   * and perform calculations on that.
   */
  virtual long GetShapeNumber () = 0;
};

SCF_VERSION (iMeshObjectFactory, 0, 0, 4);

/**
 * This object is a factory which can generate
 * mesh objects of a certain type. For example, if you want to have
 * multiple sets of sprites from the same sprite template then
 * you should have an instance of iMeshObjectFactory for evey sprite
 * template and an instance of iMeshObject for every sprite.
 */
struct iMeshObjectFactory : public iBase
{
  /// Create an instance of iMeshObject.
  virtual iMeshObject* NewInstance () = 0;

  /**
   * Do a hard transform of this factory.
   * This transformation and the original coordinates are not
   * remembered but the object space coordinates are directly
   * computed (world space coordinates are set to the object space
   * coordinates by this routine). Note that some implementations
   * of mesh objects will not change the orientation of the object but
   * only the position.
   */
  virtual void HardTransform (const csReversibleTransform& t) = 0;

  /**
   * Return true if HardTransform is supported for this factory.
   */
  virtual bool SupportsHardTransform () = 0;
};

SCF_VERSION (iMeshObjectType, 0, 0, 1);

/**
 * This plugin describes a specific type of mesh objects. Through
 * this plugin the user can create instances of mesh object factories
 * which can then be used to create instances of mesh objects.
 */
struct iMeshObjectType : public iPlugIn
{
  /// Create an instance of iMeshObjectFactory.
  virtual iMeshObjectFactory* NewFactory () = 0;
};

#endif // __IMESH_IMESHOBJ_H__
