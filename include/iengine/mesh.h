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
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivideo/graph3d.h"

struct iMeshObject;
struct iCamera;
struct iMeshObjectFactory;
struct iMeshWrapper;
struct iMeshList;
struct iMeshFactoryList;
class csMeshWrapper;
class csMeshFactoryWrapper;
struct iMeshFactoryWrapper;
struct iRenderView;
struct iMovable;
struct iLight;
struct iObject;
class csFlags;

/**
 * If CS_ENTITY_CONVEX is set then this entity is convex (what did
 * you expect :-)
 * This means the 3D engine can do various optimizations.
 * If you set 'convex' to true the center vertex will also be calculated.
 * It is unset by default (@@@ should be calculated).
 */
#define CS_ENTITY_CONVEX 1

/**
 * If CS_ENTITY_DETAIL is set then this entity is a detail
 * object. A detail object is treated as a single object by
 * the engine. The engine can do several optimizations on this.
 * In general you should use this flag for small and detailed
 * objects. Detail objects are not included in BSP or octrees.
 */
#define CS_ENTITY_DETAIL 2

/**
 * If CS_ENTITY_CAMERA is set then this entity will be always
 * be centerer around the same spot relative to the camera. This
 * is useful for skyboxes or skydomes.
 */
#define CS_ENTITY_CAMERA 4

/**
 * If CS_ENTITY_INVISIBLE is set then this thing will not be rendered.
 * It will still cast shadows and be present otherwise. Use the
 * CS_ENTITY_NOSHADOWS flag to disable shadows.
 */
#define CS_ENTITY_INVISIBLE 8

/**
 * If CS_ENTITY_NOSHADOWS is set then this thing will not cast
 * shadows. Lighting will still be calculated for it though. Use the
 * CS_ENTITY_NOLIGHTING flag to disable that.
 */
#define CS_ENTITY_NOSHADOWS 16

/**
 * If CS_ENTITY_NOLIGHTING is set then this thing will not be lit.
 * It may still cast shadows though. Use the CS_ENTITY_NOSHADOWS flag
 * to disable that.
 */
#define CS_ENTITY_NOLIGHTING 32

/**
 * If CS_ENTITY_BACK2FRONT is set then all objects with the same
 * render order as this one and which also have this flag set will
 * be rendered in roughly back to front order. All objects with
 * the same render order but which do not have this flag set will
 * be rendered later. This flag is important if you want to have
 * alpha transparency rendered correctly.
 */
#define CS_ENTITY_BACK2FRONT 64

SCF_VERSION (iMeshDrawCallback, 0, 0, 1);

/**
 * Set a callback which is called just before the object is drawn.
 * This is useful to do some expensive computations which only need
 * to be done on a visible object. Note that this function will be
 * called even if the object is not visible. In general it is called
 * if there is a likely probability that the object is visible (i.e.
 * it is in the same sector as the camera for example).
 */
struct iMeshDrawCallback : public iBase
{
  /// Before drawing.
  virtual bool BeforeDrawing (iMeshWrapper* spr, iRenderView* rview) = 0;
};


SCF_VERSION (iMeshWrapper, 0, 1, 17);

/**
 * This interface corresponds to the object in the engine
 * that holds reference to the real iMeshObject.
 */
struct iMeshWrapper : public iBase
{
  /// UGLY!!!@@@
  virtual csMeshWrapper* GetPrivateObject () = 0;
  /// Get the iMeshObject.
  virtual iMeshObject* GetMeshObject () const = 0;
  /// Set the iMeshObject.
  virtual void SetMeshObject (iMeshObject*) = 0;
  /// Get the iObject for this mesh object.
  virtual iObject *QueryObject () = 0;

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
  virtual iMovable* GetMovable () const = 0;

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
  virtual void PlaceMesh () = 0;

  /**
   * Check if this mesh is hit by this object space vector.
   * This will do a rough but fast test based on bounding box only.
   * So this means that it might return a hit even though the object
   * isn't really hit at all. Depends on how much the bounding box
   * overestimates the object. This also returns the face number
   * as defined in csBox3 on which face the hit occured. Usefull for
   * grid structures. 
   */
  virtual int HitBeamBBox (const csVector3& start, const csVector3& end,
                csVector3& isect, float* pr) = 0;

  /**
   * Check if this object is hit by this object space vector.
   * Outline check.
   */
  virtual bool HitBeamOutline (const csVector3& start,
  	const csVector3& end, csVector3& isect, float* pr) = 0;

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
  virtual void SetDrawCallback (iMeshDrawCallback* cb) = 0;

  /// Get the draw callback.
  virtual iMeshDrawCallback* GetDrawCallback () const = 0;

  /// Set the parent factory.
  virtual void SetFactory (iMeshFactoryWrapper* factory) = 0;

  /**
   * The renderer will render all objects in a sector based on this
   * number. Low numbers get rendered first. High numbers get rendered
   * later. There are a few often used slots:
   * <ul>
   * <li>1. Sky objects are rendered before
   *     everything else. Usually they are rendered using ZFILL (or ZNONE).
   * <li>2. Walls are rendered after that. They
   *     usually use ZFILL.
   * <li>3. After that normal objects are
   *     rendered using the Z-buffer (ZUSE).
   * <li>4. Alpha transparent objects or objects
   *     using some other transparency system are rendered after that. They
   *     are usually rendered using ZTEST.
   * </ul>
   */
  virtual void SetRenderPriority (long rp) = 0;
  /**
   * Get the render priority.
   */
  virtual long GetRenderPriority () const = 0;

  /**
   * Get flags for this meshwrapper.
   */
  virtual csFlags& GetFlags () = 0;

  /**
   * Set the Z-buf drawing mode to use for this object.
   */
  virtual void SetZBufMode (csZBufMode mode) = 0;
  /**
   * Get the Z-buf drawing mode.
   */
  virtual csZBufMode GetZBufMode () const = 0;

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
   * Get the bounding box of this object in world space.
   * This routine will cache the bounding box and only recalculate it
   * if the movable changes.
   */
  virtual void GetWorldBoundingBox (csBox3& cbox) = 0;

  /**
   * Get the bounding box of this object after applying a transformation to it.
   * This is really a very inaccurate function as it will take the bounding
   * box of the object in object space and then transform this bounding box.
   */
  virtual void GetTransformedBoundingBox (const csReversibleTransform& trans,
  	csBox3& cbox) = 0;

  /**
   * Get a very inaccurate bounding box of the object in screen space.
   * Returns -1 if object behind the camera or else the distance between
   * the camera and the furthest point of the 3D box.
   */
  virtual float GetScreenBoundingBox (iCamera* camera, csBox2& sbox,
  	csBox3& cbox) = 0;

  /**
   * Get all the children of this mesh object.
   */
  virtual iMeshList* GetChildren () = 0;
  /**
   * Get the parent of this mesh. This will be either a pointer to the
   * engine or another meshwrapper (or NULL if the mesh is not linked
   * to anything). Use SCF_QUERY_INTERFACE/SCF_QUERY_INTERFACE_FAST to see the
   * type of the parent.
   */
  virtual iBase* GetParentContainer () = 0;
  /// Get the radius of this mesh (ignoring children).
  virtual void GetRadius (csVector3& rad, csVector3& cent) const = 0;

  /**
   * Draw this mesh. This is a function that you normally don't have
   * to call on your own as the engine does this for you. However it can
   * be useful to have this function in some special cases where you
   * want to render mesh objects in a CSWS window or something like that.
   */
  virtual void Draw (iRenderView* rview) = 0;
};

SCF_VERSION (iMeshFactoryWrapper, 0, 1, 6);

/**
 * This interface corresponds to the object in the engine
 * that holds reference to the real iMeshObjectFactory.
 */
struct iMeshFactoryWrapper : public iBase
{
  /// UGLY!!!@@@
  virtual csMeshFactoryWrapper* GetPrivateObject () = 0;
  /// Get the iObject for this mesh factory.
  virtual iObject *QueryObject () = 0;
  /// Get the iMeshObjectFactory.
  virtual iMeshObjectFactory* GetMeshObjectFactory () const = 0;
  /// Set the mesh object factory.
  virtual void SetMeshObjectFactory (iMeshObjectFactory* fact) = 0;
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
   * Create mesh objects from this factory. If the factory has a hierarchy
   * then a hierarchical mesh object will be created.
   */
  virtual iMeshWrapper* CreateMeshWrapper () = 0;

  /**
   * Get the parent of this factory. This will be NULL if this factory
   * has no parent.
   */
  virtual iMeshFactoryWrapper* GetParentContainer () const = 0;

  /**
   * Get all the children of this mesh factory.
   */
  virtual iMeshFactoryList* GetChildren () = 0;

  /**
   * Get optional relative transform (relative to parent).
   */
  virtual csReversibleTransform& GetTransform () = 0;

  /**
   * Set optional relative transform (relative to parent).
   */
  virtual void SetTransform (const csReversibleTransform& tr) = 0;
};

SCF_VERSION (iMeshList, 0, 0, 1);

struct iMeshList : public iBase
{
  /// Return the number of meshes in this list.
  virtual int GetMeshCount () const = 0;
  /// Return a single mesh.
  virtual iMeshWrapper *GetMesh (int idx) const = 0;
  /// Add a mesh
  virtual void AddMesh (iMeshWrapper *mesh) = 0;
  /// Remove a mesh
  virtual void RemoveMesh (iMeshWrapper *mesh) = 0;
  /// Find a mesh by name
  virtual iMeshWrapper *FindByName (const char *name) const = 0;
  /// Find a mesh and return its index
  virtual int Find (iMeshWrapper *mesh) const = 0;
};

SCF_VERSION (iMeshFactoryList, 0, 0, 1);

struct iMeshFactoryList : public iBase
{
  /// Return the number of mesh factories in this list.
  virtual int GetMeshFactoryCount () const = 0;
  /// Return a single mesh factory.
  virtual iMeshFactoryWrapper *GetMeshFactory (int idx) const = 0;
  /// Add a mesh factory
  virtual void AddMeshFactory (iMeshFactoryWrapper *mesh) = 0;
  /// Remove a mesh factory
  virtual void RemoveMeshFactory (iMeshFactoryWrapper *mesh) = 0;
  /// Find a mesh factory by name
  virtual iMeshFactoryWrapper *FindByName (const char *name) const = 0;
  /// Find a mesh factory and return its index
  virtual int Find (iMeshFactoryWrapper *mesh) const = 0;
};

#endif // __IENGINE_MESH_H__

