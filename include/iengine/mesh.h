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

#ifndef __IENGINE_MESH_H__
#define __IENGINE_MESH_H__

#include "csutil/scf.h"
#include "csgeom/box.h"
#include "isys/plugin.h"

struct iMeshObject;
struct iMeshObjectFactory;
struct iMeshWrapper;
struct iRenderView;
struct iMovable;
struct iLight;

/// A callback function for MeshWrapper::Draw().
typedef void (csDrawCallback) (iMeshWrapper* spr, iRenderView* rview,
	void* callbackData);

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

#endif // __IENGINE_MESH_H__
