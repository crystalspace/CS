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

#ifndef __IMESHOBJ_H__
#define __IMESHOBJ_H__

#include "csutil/scf.h"
#include "csgeom/box.h"
#include "iplugin.h"

struct iMeshObject;
struct iMeshObjectFactory;
struct iMeshWrapper;
struct iRenderView;
struct iMovable;
struct iLight;
class csReversibleTransform;

/// A callback function for MeshObj::Draw().
typedef void (csMeshCallback) (iMeshObject* spr, iRenderView* rview,
	void* callbackData);
/// A callback function for MeshWrapper::Draw().
typedef void (csDrawCallback) (iMeshWrapper* spr, iRenderView* rview,
	void* callbackData);

SCF_VERSION (iMeshObject, 0, 0, 10);

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
   * If 'accurate' is true an effort has to be done to make the
   * bounding box as accurate as possible. Otherwise it just has
   * to be a bounding box.
   */
  virtual void GetObjectBoundingBox (csBox3& bbox, bool accurate = false) = 0;

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

SCF_VERSION (iMeshWrapper, 0, 0, 2);

/**
 * This interface corresponds to the object in the engine
 * that holds reference to the real iMeshObject.
 */
struct iMeshWrapper : public iBase
{
  /// Get the iMeshObject.
  virtual iMeshObject* GetMeshObject () = 0;

  /**
   * Update lighting as soon as the object becomes visible.
   * This will call engine->GetNearestLights with the supplied
   * parameters.
   */
  virtual void DeferUpdateLighting (int flags, int num_lights) = 0;

  /**
   * Light object according to the given array of lights (i.e.
   * fill the vertex color array).
   * No shadow calculation will be done. This is assumed to have
   * been done earlier. This is a primitive lighting process
   * based on the lights which hit one point of the sprite (usually
   * the center). More elaborate lighting systems are possible
   * but this will do for now.
   */
  virtual void UpdateLighting (iLight** lights, int num_lights) = 0;

  /**
   * Get the movable instance for this object.
   * It is very important to call GetMovable()->UpdateMove()
   * after doing any kind of modification to this movable
   * to make sure that internal data structures are
   * correctly updated.
   */
  virtual iMovable* GetMovable () = 0;

  /**
   * Check if this object is hit by this object space vector.
   * Return the collision point in object space coordinates.
   */
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr) = 0;
  /**
   * Check if this object is hit by this world space vector.
   * Return the collision point in world space coordinates.
   */
  virtual bool HitBeam (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr) = 0;

  /**
   * Set a callback which is called just before the object is drawn.
   * This is useful to do some expensive computations which only need
   * to be done on a visible object. Note that this function will be
   * called even if the object is not visible. In general it is called
   * if there is a likely probability that the object is visible (i.e.
   * it is in the same sector as the camera for example).
   */
  virtual void SetDrawCallback (csDrawCallback* cb, void* cbData) = 0;

  /// Get the draw callback.
  virtual csDrawCallback* GetDrawCallback () = 0;
};

SCF_VERSION (iMeshFactoryWrapper, 0, 0, 1);

/**
 * This interface corresponds to the object in the engine
 * that holds reference to the real iMeshObjectFactory.
 */
struct iMeshFactoryWrapper : public iBase
{
  /// Get the iMeshObjectFactory.
  virtual iMeshObjectFactory* GetMeshObjectFactory () = 0;
};

#endif

