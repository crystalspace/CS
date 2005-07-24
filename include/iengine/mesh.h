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

#ifndef __CS_IENGINE_MESH_H__
#define __CS_IENGINE_MESH_H__

/**\file
 */
/**
 * \addtogroup engine3d_meshes
 * @{ */

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
struct iMeshFactoryWrapper;
struct iRenderView;
struct iMovable;
struct iLODControl;
struct iLight;
struct iLightingInfo;
struct iShadowReceiver;
struct iShadowCaster;
struct iObject;
struct iPortalContainer;
struct csRenderMesh;
class csFlags;

/** \name Meshwrapper flags
 * @{ */
/**
 * If CS_ENTITY_DETAIL is set then this entity is a detail
 * object. A detail object is treated as a single object by
 * the engine. The engine can do several optimizations on this.
 * In general you should use this flag for small and detailed
 * objects.
 * This flag is currently not used.
 */
#define CS_ENTITY_DETAIL 2

/**
 * If CS_ENTITY_CAMERA is set then this entity will be always
 * be centerer around the same spot relative to the camera. This
 * is useful for skyboxes or skydomes. Important note! When you
 * use an object with this flag you should also add this object to
 * a render priority that also has the camera flag set (see
 * iEngine->SetRenderPriorityCamera()).
 */
#define CS_ENTITY_CAMERA 4

/**
 * If CS_ENTITY_INVISIBLEMESH is set then this thing will not be
 * rendered. It will still cast shadows and be present otherwise.
 * Use the CS_ENTITY_NOSHADOWS flag to disable shadows. Using this
 * flag does NOT automatically imply that HitBeam() will ignore this
 * mesh. For that you need to set CS_ENTITY_NOHITBEAM.
 */
#define CS_ENTITY_INVISIBLEMESH 8

/**
 * If CS_ENTITY_INVISIBLE is set then this thing will not be rendered.
 * It will still cast shadows and be present otherwise. Use the
 * CS_ENTITY_NOSHADOWS flag to disable shadows. Making a mesh invisible
 * will also imply that HitBeam() will ignore it.
 */
#define CS_ENTITY_INVISIBLE (CS_ENTITY_INVISIBLEMESH+CS_ENTITY_NOHITBEAM)

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
 * If CS_ENTITY_NOHITBEAM is set then this thing will not react to
 * HitBeam calls.
 */
#define CS_ENTITY_NOHITBEAM 64

/**
 * If CS_ENTITY_NOCLIP is set then this entity will be drawn fully
 * (unclipped to portal frustum) and only once for every frame/camera
 * combination. This is useful in a scenario where you have an indoor
 * sector with lots of portals to an outdoor sector. In the outdoor sector
 * there is a complex terrain mesh object and you really only want to render
 * that once with full screen and not many times clipped to individual
 * portals.
 */
#define CS_ENTITY_NOCLIP 128

/** @} */

/** \name SetLightingUpdate flags
 * @{ */

/**
 * This is a flag for iMeshWrapper->SetLightingUpdate(). If this
 * flag is set then only the 'N' most relevant lights will be returned
 * to the object. If not set then 'N' random lights will be returned.
 */
#define CS_LIGHTINGUPDATE_SORTRELEVANCE 1

/**
 * If this flag for iMeshWrapper->SetLightingUpdate() is set then
 * the set of relevant lights will be recalculated every time.
 * Otherwise the lights are only recalculated when the object moves or
 * when one of the affected lights changes (default).
 */
#define CS_LIGHTINGUPDATE_ALWAYSUPDATE 2

/** @} */

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
  /**
   * Before drawing. It is safe to delete this callback
   * in this function.
   */
  virtual bool BeforeDrawing (iMeshWrapper* spr, iRenderView* rview) = 0;
};

/**
 * Return structure for the iMeshWrapper->HitBeam() routines.
 */
struct csHitBeamResult
{
  /// Intersection point in object space.
  csVector3 isect;
  /**
   * Value between 0 and 1 indicating where on the segment the intersection
   * occured.
   */
  float r;
  /// Only for HitBeamObject: the polygon/triangle index that was hit.
  int polygon_idx;
  /**
   * Only for HitBeamBBox: Face number that was hit.
   * \sa csIntersect3::BoxSegment
   */
  int facehit;
  /**
   * For all except HitBeamBBox: true if hit, false otherwise.
   */
  bool hit;
};

/**
 * Return structure for iMeshWrapper->GetScreenBoundingBox().
 */
struct csScreenBoxResult
{
  /// 2D box in screen space.
  csBox2 sbox;
  /// 3D box in camera space.
  csBox3 cbox;
  /**
   * -1 if object behind the camera or else the distance between
   * the camera and the furthest point of the 3D box.
   */
  float distance;
};

SCF_VERSION (iMeshWrapper, 0, 8, 1);

/**
 * A mesh wrapper is an engine-level object that wraps around an actual
 * mesh object (iMeshObject). Every mesh object in the engine is represented
 * by a mesh wrapper, which keeps the pointer to the mesh object, its position,
 * its name, etc.
 * <p>
 * Think of the mesh wrapper as the hook that holds the mesh object in the
 * engine. An effect of this is that the i???State interfaces (e.g.
 * iSprite3DState) must be queried from the mesh *objects*, not the wrappers!
 * <p>
 * Note that a mesh object should never be contained in more than one wrapper.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iEngine::CreateSectorWallsMesh()
 *   <li>iEngine::CreateThingMesh()
 *   <li>iEngine::CreateMeshWrapper()
 *   <li>iEngine::LoadMeshWrapper()
 *   <li>iEngine::CreatePortalContainer()
 *   <li>iEngine::CreatePortal()
 *   <li>iLoader::LoadMeshObject()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iEngine::FindMeshObject()
 *   <li>iMeshList::Get()
 *   <li>iMeshList::FindByName()
 *   <li>iMeshWrapperIterator::Next()
 *   <li>iLoaderContext::FindMeshObject()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEngine
 *   </ul>
 */
struct iMeshWrapper : public iBase
{
  /**
   * Get the iObject for this mesh object. This can be used to get the
   * name of the mesh wrapper and also to attach other user objects
   * to this mesh (like for collision detection or game data).
   */
  virtual iObject *QueryObject () = 0;

  /// Get the iMeshObject.
  virtual iMeshObject* GetMeshObject () const = 0;
  /// Set the iMeshObject.
  virtual void SetMeshObject (iMeshObject*) = 0;
  /**
   * If this mesh is a portal container you can use GetPortalContainer() to
   * get the portal container interface.
   */
  virtual iPortalContainer* GetPortalContainer () const = 0;

  /**
   * Get the optional lighting information that is implemented
   * by this mesh object. If the mesh object doesn't implement it
   * then this will return 0. This is similar (but more efficient)
   * to calling SCF_QUERY_INTERFACE on the mesh object for iLightingInfo.
   */
  virtual iLightingInfo* GetLightingInfo () const = 0;

  /**
   * Get the optional shadow receiver that is implemented
   * by this mesh object. If the mesh object doesn't implement it
   * then this will return 0. This is similar (but more efficient)
   * to calling SCF_QUERY_INTERFACE on the mesh object for iShadowReceiver.
   * <p>
   * Note! If the mesh is a static lod mesh (i.e. a parent of a mesh
   * hierarchy that is used for static lod) then this will return
   * a shadow receiver that automatically multiplexes the receiving shadows
   * to all child meshes.
   */
  virtual iShadowReceiver* GetShadowReceiver () = 0;

  /**
   * Get the optional shadow caster that is implemented
   * by this mesh object. If the mesh object doesn't implement it
   * then this will return 0. This is similar (but more efficient)
   * to calling SCF_QUERY_INTERFACE on the mesh object for iShadowCaster.
   * <p>
   * Note! If the mesh is a static lod mesh (i.e. a parent of a mesh
   * hierarchy that is used for static lod) then this will return a
   * shadow caster that gets shadows from the highest detail objects.
   */
  virtual iShadowCaster* GetShadowCaster () = 0;

  /// Get the parent factory.
  virtual iMeshFactoryWrapper *GetFactory () const = 0;
  /// Set the parent factory (this only sets a pointer).
  virtual void SetFactory (iMeshFactoryWrapper* factory) = 0;

  /**
   * Control how lighting updates should take place.
   * 'num_lights' is the number of lights that will be given to the
   * mesh object at maximum (default is 8). 'flags' can be a combination
   * of one of the following:
   * <ul>
   * <li>#CS_LIGHTINGUPDATE_SORTRELEVANCE (default on).
   * <li>#CS_LIGHTINGUPDATE_ALWAYSUPDATE (default off).
   * </ul>
   * Note that this function has no effect on thing
   * mesh objects as they use another lighting system (lightmaps).
   * Also some genmesh objects can optionally also use the other lighting
   * system in which nothing will happen either.
   */
  virtual void SetLightingUpdate (int flags, int num_lights) = 0;

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
   * <p>
   * Placing a mesh in different sectors is important when the mesh crosses
   * a portal boundary. If you don't do this then it is possible that the
   * mesh will be clipped wrong. For small mesh objects you can get away
   * by not doing this in most cases.
   */
  virtual void PlaceMesh () = 0;

  /**
   * Check if this mesh is hit by this object space vector.
   * This will do a rough but fast test based on bounding box only.
   * So this means that it might return a hit even though the object
   * isn't really hit at all. Depends on how much the bounding box
   * overestimates the object. This also returns the face number
   * as defined in csBox3 on which face the hit occured. Useful for
   * grid structures.
   * \deprecated Use HitBeamBBox() with csHitBeamResult instead.
   */
  CS_DEPRECATED_METHOD virtual int HitBeamBBox (const csVector3& start,
  	const csVector3& end, csVector3& isect, float* pr) = 0;

  /**
   * Check if this object is hit by this object space vector.
   * Outline check.
   * \deprecated Use HitBeamOutline() with csHitBeamResult instead.
   */
  CS_DEPRECATED_METHOD virtual bool HitBeamOutline (const csVector3& start,
  	const csVector3& end, csVector3& isect, float* pr) = 0;

  /**
   * Check if this object is hit by this object space vector.
   * Return the collision point in object space coordinates. This version
   * is more accurate than HitBeamOutline.
   * \deprecated Use HitBeamObject() with csHitBeamResult instead.
   */
  CS_DEPRECATED_METHOD virtual bool HitBeamObject (const csVector3& start,
  	const csVector3& end, csVector3& isect, float* pr,
	int* polygon_idx = 0) = 0;

  /**
   * Check if this object is hit by this world space vector.
   * Return the collision point in world space coordinates.
   * \deprecated Use HitBeamObject() with csHitBeamResult instead.
   */
  CS_DEPRECATED_METHOD virtual bool HitBeam (const csVector3& start,
  	const csVector3& end, csVector3& isect, float* pr) = 0;

  /**
   * Check if this mesh is hit by this object space vector.
   * This will do a rough but fast test based on bounding box only.
   * So this means that it might return a hit even though the object
   * isn't really hit at all. Depends on how much the bounding box
   * overestimates the object. This also returns the face number
   * as defined in csBox3 on which face the hit occured. Useful for
   * grid structures.
   * \sa csHitBeamResult
   */
  virtual csHitBeamResult HitBeamBBox (const csVector3& start,
  	const csVector3& end) = 0;

  /**
   * Check if this object is hit by this object space vector.
   * Outline check.
   * \sa csHitBeamResult
   */
  virtual csHitBeamResult HitBeamOutline (const csVector3& start,
  	const csVector3& end) = 0;

  /**
   * Check if this object is hit by this object space vector.
   * Return the collision point in object space coordinates. This version
   * is more accurate than HitBeamOutline.
   * \sa csHitBeamResult
   */
  virtual csHitBeamResult HitBeamObject (const csVector3& start,
  	const csVector3& end) = 0;

  /**
   * Check if this object is hit by this world space vector.
   * Return the collision point in world space coordinates.
   * \sa csHitBeamResult
   */
  virtual csHitBeamResult HitBeam (const csVector3& start,
  	const csVector3& end) = 0;

  /**
   * Set a callback which is called just before the object is drawn.
   * This is useful to do some expensive computations which only need
   * to be done on a visible object. Note that this function will be
   * called even if the object is not visible. In general it is called
   * if there is a likely probability that the object is visible (i.e.
   * it is in the same sector as the camera for example).
   */
  virtual void SetDrawCallback (iMeshDrawCallback* cb) = 0;

  /**
   * Remove a draw callback.
   */
  virtual void RemoveDrawCallback (iMeshDrawCallback* cb) = 0;

  /// Get the number of draw callbacks.
  virtual int GetDrawCallbackCount () const = 0;

  /// Get the specified draw callback.
  virtual iMeshDrawCallback* GetDrawCallback (int idx) const = 0;

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
   * Same as SetRenderPriority() but this version will recursively set
   * render priority for the children too.
   */
  virtual void SetRenderPriorityRecursive (long rp) = 0;

  /**
   * Get flags for this meshwrapper. The following flags are supported:
   * <ul>
   * <li>#CS_ENTITY_CONVEX: entity is convex. This can help the engine with
   *     optimizing rendering. Currently not used.
   * <li>#CS_ENTITY_DETAIL: this is a detail object. Again this is a hint
   *     for the engine to render this object differently. Currently not used.
   * <li>#CS_ENTITY_CAMERA: entity will always be centered around the camera.
   * <li>#CS_ENTITY_INVISIBLEMESH: entity is not rendered. 
   * <li>#CS_ENTITY_NOHITBEAM: this entity will not be considered by HitBeam() 
   *     calls.
   * <li>#CS_ENTITY_INVISIBLE: means that either CS_ENTITY_INVISIBLEMESH and 
   *     CS_ENTITY_NOHITBEAM are set.
   * <li>#CS_ENTITY_NOSHADOWS: cast no shadows.
   * <li>#CS_ENTITY_NOLIGHTING: do not light this object.
   * <li>#CS_ENTITY_NOCLIP: do not clip this object.
   * </ul>
   * \remarks Despite the name, this method does not only provide read access
   *   to the mesh flags, as the returned reference to a csFlags object also 
   *   provides write access.
   */
  virtual csFlags& GetFlags () = 0;

  /**
   * Set some flags with the given mask for this mesh and all children.
   * \param mask The bits to modify; only those bits are affected.
   * \param flags The values the bits specified in \a mask are set to.
   * <p>
   * Enabling flags:
   * \code
   * csRef<iMeshWrapper> someWrapper = ...;
   * someWrapper->SetFlags (CS_ENTITY_INVISIBLE | CS_ENTITY_NOCLIP);
   * \endcode
   * <p>
   * Disabling flags:
   * \code
   * csRef<iMeshWrapper> someWrapper = ...;
   * someWrapper->SetFlags (CS_ENTITY_INVISIBLE | CS_ENTITY_NOCLIP, 0);
   * \endcode
   * \remarks To set flags non-recursive, use GetFlags().Set().
   */
  virtual void SetFlagsRecursive (uint32 mask, uint32 flags = ~0) = 0;

  /**
   * Set the Z-buf drawing mode to use for this object.
   * Possible values are:
   * <ul>
   * <li>#CS_ZBUF_NONE: do not read nor write the Z-buffer.
   * <li>#CS_ZBUF_FILL: only write the Z-buffer but do not read.
   * <li>#CS_ZBUF_USE: write and read the Z-buffer.
   * <li>#CS_ZBUF_TEST: only read the Z-buffer but do not write.
   * </ul>
   */
  virtual void SetZBufMode (csZBufMode mode) = 0;
  /**
   * Get the Z-buf drawing mode.
   */
  virtual csZBufMode GetZBufMode () const = 0;
  /**
   * Same as SetZBufMode() but this will also set the z-buf
   * mode for the children too.
   */
  virtual void SetZBufModeRecursive (csZBufMode mode) = 0;

  /**
   * Do a hard transform of this object.
   * This transformation and the original coordinates are not
   * remembered but the object space coordinates are directly
   * computed (world space coordinates are set to the object space
   * coordinates by this routine). Note that some implementations
   * of mesh objects will not change the orientation of the object but
   * only the position.
   * <p>
   * Note also that some mesh objects don't support HardTransform. You
   * can find out by calling iMeshObject->SupportsHardTransform().
   * In that case you can sometimes still call HardTransform() on the
   * factory.
   */
  virtual void HardTransform (const csReversibleTransform& t) = 0;

  /**
   * Get the bounding box of this object in world space.
   * This routine will cache the bounding box and only recalculate it
   * if the movable changes.
   * \deprecated Use GetWorldBoundingBox that returns a csBox3 instead.
   */
  CS_DEPRECATED_METHOD virtual void GetWorldBoundingBox (csBox3& cbox) = 0;

  /**
   * Get the bounding box of this object after applying a transformation to it.
   * This is really a very inaccurate function as it will take the bounding
   * box of the object in object space and then transform this bounding box.
   * \deprecated Use GetTransformedBoundingBox that returns a csBox3 instead.
   */
  CS_DEPRECATED_METHOD virtual void GetTransformedBoundingBox (
  	const csReversibleTransform& trans, csBox3& cbox) = 0;

  /**
   * Get a very inaccurate bounding box of the object in screen space.
   * Returns -1 if object behind the camera or else the distance between
   * the camera and the furthest point of the 3D box.
   * \deprecated Use GetScreenBoundingBox() that returns a csScreenBoxResult
   * instead.
   */
  CS_DEPRECATED_METHOD virtual float GetScreenBoundingBox (iCamera* camera,
  	csBox2& sbox, csBox3& cbox) = 0;

  /**
   * Get the bounding box of this object in world space.
   * This routine will cache the bounding box and only recalculate it
   * if the movable changes.
   */
  virtual const csBox3& GetWorldBoundingBox () = 0;

  /**
   * Get the bounding box of this object after applying a transformation to it.
   * This is really a very inaccurate function as it will take the bounding
   * box of the object in object space and then transform this bounding box.
   */
  virtual csBox3 GetTransformedBoundingBox (
  	const csReversibleTransform& trans) = 0;

  /**
   * Get a very inaccurate bounding box of the object in screen space.
   * Returns -1 if object behind the camera or else the distance between
   * the camera and the furthest point of the 3D box.
   */
  virtual csScreenBoxResult GetScreenBoundingBox (iCamera* camera) = 0;

  /**
   * Get all the children of this mesh object. This is used for hierarchical
   * meshes. If you want to make a hierarchical mesh you can call
   * GetChildren ()->Add (mesh).
   */
  virtual iMeshList* GetChildren () = 0;
  /**
   * Get the parent of this mesh. Returns 0 if the mesh has no parent (i.e.
   * it is contained in the engine directly). If not 0 then this mesh
   * is part of a hierarchical mesh.
   */
  virtual iMeshWrapper* GetParentContainer () = 0;
  /**
   * Set the parent of this mesh. This only changes the 'parent' pointer but
   * does not add the mesh as a child mesh. Internal use only.
   */
  virtual void SetParentContainer (iMeshWrapper *) = 0;

  /**
   * Get the radius of this mesh and all its children.
   * \deprecated Use GetRadius() that returns csEllipsoid.
   */
  CS_DEPRECATED_METHOD virtual void GetRadius (csVector3& rad,
  	csVector3& cent) const = 0;
  /// Get the radius of this mesh and all its children.
  virtual csEllipsoid GetRadius () const = 0;

  /**
   * Create a LOD control for this mesh wrapper. This is relevant
   * only if the mesh is a hierarchical mesh. The LOD control will be
   * used to select which children are visible and which are not.
   * Use this to create static lod.
   */
  virtual iLODControl* CreateStaticLOD () = 0;

  /**
   * Destroy the LOD control for this mesh. After this call the hierarchical
   * mesh will act as usual.
   */
  virtual void DestroyStaticLOD () = 0;

  /**
   * Get the LOD control for this mesh. This will return 0 if this is a normal
   * (hierarchical) mesh. Otherwise it will return an object with which you
   * can control the static LOD of this object.
   */
  virtual iLODControl* GetStaticLOD () = 0;

  /**
   * Set a given child mesh at a specific lod level. Note that a mesh
   * can be at several lod levels at once.
   */
  virtual void AddMeshToStaticLOD (int lod, iMeshWrapper* mesh) = 0;

  /**
   * Remove a child mesh from all lod levels. The mesh is not removed
   * from the list of child meshes however.
   */
  virtual void RemoveMeshFromStaticLOD (iMeshWrapper* mesh) = 0;

  /**
   * Draws the shadow buffer pass.  This sets of the stencil for the lights
   */
  virtual void DrawShadow (iRenderView* rview, iLight *light) = 0;
  /**
   * Draws the diffuse light mesh object
   */
  virtual void DrawLight (iRenderView* rview, iLight *light) = 0;

  /**
   * Enable/disable hardware based shadows alltogheter
   */ 
  virtual void CastHardwareShadow (bool castShadow) = 0;
  /**
   * Sets so that the meshobject is rendered after all fancy HW-shadow-stuff
   */
  virtual void SetDrawAfterShadow (bool drawAfter) = 0;
  /** 
   * Get if the meshobject is rendered after all fancy HW-shadow-stuff
   */
  virtual bool GetDrawAfterShadow () = 0;
  
  /**
   * Get the shader variable context of the mesh object.
   */
  virtual iShaderVariableContext* GetSVContext() = 0;
};

SCF_VERSION (iMeshFactoryWrapper, 0, 1, 7);

/**
 * A mesh factory wrapper is an engine-level object that wraps around a
 * mesh object factory (iMeshObjectFactory). Every mesh object factory in
 * the engine is represented by a mesh factory wrapper, which keeps the
 * pointer to the mesh factory, its name, etc.
 * <p>
 * Think of the mesh factory wrapper as the hook that holds the mesh
 * factory in the engine. An effect of this is that the i???FactoryState
 * interfaces (e.g. iSprite3DFactoryState) must be queried from the mesh
 * *factories*, not the wrappers!
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iEngine::CreateMeshFactory()
 *   <li>iEngine::LoadMeshFactory()
 *   <li>iLoader::LoadMeshObjectFactory()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iEngine::FindMeshFactory()
 *   <li>iMeshFactoryList::Get()
 *   <li>iMeshFactoryList::FindByName()
 *   <li>iLoaderContext::FindMeshFactory()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEngine
 *   </ul>
 */
struct iMeshFactoryWrapper : public iBase
{
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
   * Get the parent of this factory. This will be 0 if this factory
   * has no parent.
   */
  virtual iMeshFactoryWrapper* GetParentContainer () const = 0;
  /**
   * Set the parent of this factory. This will only change the 'parent'
   * pointer but not add the factory as a child! Internal use only.
   */
  virtual void SetParentContainer (iMeshFactoryWrapper *p) = 0;

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

  /**
   * Create a LOD control template for this factory. This is relevant
   * only if the factory is hierarchical. The LOD control will be
   * used to select which children are visible and which are not.
   * Use this to create static lod.
   */
  virtual iLODControl* CreateStaticLOD () = 0;

  /**
   * Destroy the LOD control for this factory.
   */
  virtual void DestroyStaticLOD () = 0;

  /**
   * Get the LOD control for this factory. This will return 0 if this is a
   * normal (hierarchical) factory. Otherwise it will return an object with
   * which you can control the static LOD of this factory.
   */
  virtual iLODControl* GetStaticLOD () = 0;

  /**
   * Set the LOD function parameters for this factory. These control the
   * function:
   * <pre>
   *    float lod = m * distance + a;
   * </pre>
   */
  virtual void SetStaticLOD (float m, float a) = 0;

  /**
   * Get the LOD function parameters for this factory.
   */
  virtual void GetStaticLOD (float& m, float& a) const = 0;

  /**
   * Set a given child factory at a specific lod level. Note that a factory
   * can be at several lod levels at once.
   */
  virtual void AddFactoryToStaticLOD (int lod, iMeshFactoryWrapper* fact) = 0;

  /**
   * Remove a child factory from all lod levels. The factory is not removed
   * from the list of factories however.
   */
  virtual void RemoveFactoryFromStaticLOD (iMeshFactoryWrapper* fact) = 0;

  /**
   * Set the Z-buf drawing mode to use for this factory. All objects created
   * from this factory will have this mode as default.
   * Possible values are:
   * <ul>
   * <li>#CS_ZBUF_NONE: do not read nor write the Z-buffer.
   * <li>#CS_ZBUF_FILL: only write the Z-buffer but do not read.
   * <li>#CS_ZBUF_USE: write and read the Z-buffer.
   * <li>#CS_ZBUF_TEST: only read the Z-buffer but do not write.
   * </ul>
   */
  virtual void SetZBufMode (csZBufMode mode) = 0;
  /**
   * Get the Z-buf drawing mode.
   */
  virtual csZBufMode GetZBufMode () const = 0;
  /**
   * Same as SetZBufMode() but this will also set the z-buf
   * mode for the children too.
   */
  virtual void SetZBufModeRecursive (csZBufMode mode) = 0;

  /**
   * The renderer will render all objects in a sector based on this
   * number. Low numbers get rendered first. High numbers get rendered
   * later. The value for the factory is used as a default for objects
   * created from that factory. There are a few often used slots:
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
   * Same as SetRenderPriority() but this version will recursively set
   * render priority for the children too.
   */
  virtual void SetRenderPriorityRecursive (long rp) = 0;

  /**
   * Get the shader variable context of the mesh factory.
   */
  virtual iShaderVariableContext* GetSVContext() = 0;
};

SCF_VERSION (iMeshList, 0, 0, 1);

/**
 * A list of meshes.
 * <p>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iEngine::GetMeshes()
 *   <li>iSector::GetMeshes()
 *   <li>iMeshWrapper::GetChildren()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEngine
 *   </ul>
 */
struct iMeshList : public iBase
{
  /// Return the number of meshes in this list
  virtual int GetCount () const = 0;

  /// Return a mesh by index
  virtual iMeshWrapper *Get (int n) const = 0;

  /// Add a mesh
  virtual int Add (iMeshWrapper *obj) = 0;

  /// Remove a mesh
  virtual bool Remove (iMeshWrapper *obj) = 0;

  /// Remove the nth mesh
  virtual bool Remove (int n) = 0;

  /// Remove all meshes
  virtual void RemoveAll () = 0;

  /// Find a mesh and return its index
  virtual int Find (iMeshWrapper *obj) const = 0;

  /**
   * Find a mesh by name. If there is a colon in the name
   * then this function is able to search for children too.
   * i.e. like mesh:childmesh:childmesh.
   */
  virtual iMeshWrapper *FindByName (const char *Name) const = 0;
};

SCF_VERSION (iMeshFactoryList, 0, 0, 1);

/**
 * A list of mesh factories.
 * <p>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iEngine::GetMeshFactories()
 *   <li>iMeshFactoryWrapper::GetChildren()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEngine
 *   </ul>
 */
struct iMeshFactoryList : public iBase
{
  /// Return the number of mesh factory wrappers in this list.
  virtual int GetCount () const = 0;

  /// Return a mesh factory wrapper by index.
  virtual iMeshFactoryWrapper *Get (int n) const = 0;

  /// Add a mesh factory wrapper.
  virtual int Add (iMeshFactoryWrapper *obj) = 0;

  /// Remove a mesh factory wrapper.
  virtual bool Remove (iMeshFactoryWrapper *obj) = 0;

  /// Remove the nth mesh factory wrapper.
  virtual bool Remove (int n) = 0;

  /// Remove all mesh factory wrappers.
  virtual void RemoveAll () = 0;

  /// Find a mesh factory wrapper and return its index.
  virtual int Find (iMeshFactoryWrapper *obj) const = 0;

  /// Find a mesh factory wrapper by name.
  virtual iMeshFactoryWrapper *FindByName (const char *Name) const = 0;
};

SCF_VERSION (iMeshWrapperIterator, 0, 1, 0);

/**
 * This is an iterator mesh wrappers.
 * <p>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iEngine::GetNearbyMeshes()
 *   <li>iEngine::GetVisibleMeshes()
 *   </ul>
 */
struct iMeshWrapperIterator : public iBase
{
  /// Move forward.
  virtual iMeshWrapper* Next () = 0;

  /// Reset the iterator to the beginning.
  virtual void Reset () = 0;

  /// Check if we have any more meshes.
  virtual bool HasNext () const = 0;
};


/** @} */

#endif // __CS_IENGINE_MESH_H__

