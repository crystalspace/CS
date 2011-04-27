/*
    Copyright (C) 2000-2007 by Jorrit Tyberghein

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

#ifndef __CS_MESHOBJ_H__
#define __CS_MESHOBJ_H__

#include "csgeom/transfrm.h"
#include "csutil/scf_implementation.h"
#include "csutil/csobject.h"
#include "csutil/nobjvec.h"
#include "csutil/refarr.h"
#include "csutil/flags.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/weakref.h"
#include "csutil/leakguard.h"
#include "csutil/hash.h"
#include "csutil/threading/rwmutex.h"
#include "iutil/selfdestruct.h"
#include "csgfx/shadervarcontext.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/imposter.h"
#include "iengine/viscull.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"

#include "movable.h"
#include "impmesh.h"
#include "meshlod.h"
#include "scenenode.h"
#include "light.h"

struct iMeshLoaderIterator;
struct iMeshWrapper;
struct iMovable;
struct iRenderView;
struct iSharedVariable;
class csEngine;
class csImposterManager;

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{
  class csLight;
  class csMeshWrapper;
  class csMovable;
  class csMovableSectorList;

/**
 * General list of meshes.
 */
class csMeshList : public scfImplementation1<csMeshList, iMeshList>
{
private:
  csRefArrayObject<iMeshWrapper, CS::Container::ArrayAllocDefault,
    csArrayCapacityVariableGrow> list;
  csHash<iMeshWrapper*, csString> meshes_hash;
  mutable CS::Threading::ReadWriteMutex meshLock;

  class NameChangeListener : public scfImplementation1<NameChangeListener,
  	iObjectNameChangeListener>
  {
  private:
    csWeakRef<csMeshList> list;

  public:
    NameChangeListener (csMeshList* list) : scfImplementationType (this),
  	  list (list)
    {
    }
    virtual ~NameChangeListener () { }

    virtual void NameChanged (iObject* obj, const char* oldname,
  	  const char* newname)
    {
      if (list)
        list->NameChanged (obj, oldname, newname);
    }
  };
  csRef<NameChangeListener> listener;

public:

  void NameChanged (iObject* object, const char* oldname,
  	const char* newname);

  /// constructor
  csMeshList (int cap, int thresshold);
  virtual ~csMeshList ();

  /// Override PrepareMesh
  virtual void PrepareMesh (iMeshWrapper*) { }
  /// Override FreeMesh
  virtual void FreeMesh (iMeshWrapper*) { }

  virtual int GetCount () const;
  virtual iMeshWrapper *Get (int n) const;
  virtual int Add (iMeshWrapper *obj);
  void AddBatch (csRef<iMeshLoaderIterator> itr);
  virtual bool Remove (iMeshWrapper *obj);
  virtual bool Remove (int n);
  virtual void RemoveAll ();
  virtual int Find (iMeshWrapper *obj) const;
  virtual iMeshWrapper *FindByName (const char *Name) const;
};


#include "csutil/deprecated_warn_off.h"

/**
 * The holder class for all implementations of iMeshObject.
 */
class csMeshWrapper : 
  public scfImplementationExt5<csMeshWrapper,
                               csObject,
                               iMeshWrapper,
                               scfFakeInterface<iShaderVariableContext>,
                               iVisibilityObject,
    		                       iSceneNode,
                               iSelfDestruct>,
  public CS::Graphics::OverlayShaderVariableContextImpl
{
  friend class csMovable;
  friend class csMovableSectorList;
  friend class ::csImposterManager;

protected:
  /**
   * Bounding box in world space.
   * This is a cache for GetWorldBoundingBox() which will recalculate this
   * if the movable changes (by using movablenr).
   */
  csBox3 wor_bbox;
  /// Last used movable number for wor_bbox.
  long wor_bbox_movablenr;

  /**
   * Position in the world.
   */
  csMovable movable; //@@MS: BAAAD

  /**
   * The renderer will render all objects in a sector based on this
   * number. Low numbers get rendered first. High numbers get rendered
   * later. There are a few predefined slots which the application is
   * free to use or not.
   */
  CS::Graphics::RenderPriority render_priority;

  // Used to store extra rendermeshes that something might attach to this
  // mesh (ie, for decals or lines)
  csDirtyAccessArray<csRenderMesh*> extraRenderMeshes;

  /**
   * This value indicates the last time that was used to do animation.
   * If 0 then we haven't done animation yet. We compare this value
   * with the value returned by engine->GetLastAnimationTime() to see
   * if we need to call meshobj->NextFrame() again.
   */
  csTicks last_anim_time;

  /**
   * Optional LOD control that will turn a hierarchical mesh in a
   * mesh that supports static LOD.
   */
  csRef<csStaticLODMesh> static_lod;

  /**
   * Minimum/maximum range for rendering this object.
   */
  bool do_minmax_range;
  float min_render_dist, max_render_dist;
  csRef<iSharedVariable> var_min_render_dist, var_max_render_dist;
  csRef<csLODListener> var_min_render_dist_listener;
  csRef<csLODListener> var_max_render_dist_listener;

protected:
  virtual void InternalRemove() { SelfDestruct(); }

private:
  /// Mesh object corresponding with this csMeshWrapper.
  csRef<iMeshObject> meshobj;

  /**
   * For optimization purposes we keep the portal container interface here
   * (only if this object is a portal container of course).
   */
  csRef<iPortalContainer> portal_container;

  /**
   * The callbacks which are called just before drawing.
   * Type: iMeshDrawCallback.
   */
  csRefArray<iMeshDrawCallback> draw_cb_vector;

  /// Optional reference to the parent csMeshFactoryWrapper.
  iMeshFactoryWrapper* factory;

  /// Z-buf mode to use for drawing this object.
  csZBufMode zbufMode;

  /// Whether or not an imposter is currently being used for this mesh.
  bool using_imposter;

  /// Whether we're drawing to an imposter texture.
  /// TODO: Work out a better way to detect this.
  csWeakRef<iBase> drawing_imposter;

  // In case the mesh has CS_ENTITY_NOCLIP set then this will
  // contain the value of the last frame number and camera pointer.
  // This is used to detect if we can skip rendering the mesh.
  iCamera* last_camera;
  uint last_frame_number;

  // An infinite bounding box.
  static csBox3 infBBox;

  // Data used when instancing is used on this mesh
  struct InstancingData
  {
    // Shadervars for instancing.
    csRef<csShaderVariable> fadeFactors;
    csRef<csShaderVariable> transformVars;
    bool instancingTransformsDirty;
    struct InstancingBbox
    {
      csBox3 oldBox;
      csBox3 newBox;
    };
    csArray<InstancingBbox> instancingBoxes;
    struct RenderMeshesSet : public CS::NonCopyable
    {
      int n;
      csRenderMesh** meshArray;
      csRenderMesh* meshes;
      
      RenderMeshesSet ();
      ~RenderMeshesSet ();
      void CopyOriginalMeshes (int n, csRenderMesh** meshes);
    };
    csFrameDataHolder<RenderMeshesSet> instancingRMs;
    
    InstancingData() : instancingTransformsDirty (false) {}
  };
  typedef csBlockAllocator<InstancingData> InstancingAlloc;
  CS_DECLARE_STATIC_CLASSVAR_REF(instancingAlloc, GetInstancingAlloc,
    InstancingAlloc)
  InstancingData* instancing;
  InstancingData* GetInstancingData();
  
  bool DoInstancing() const { return instancing != 0; }
  /* Given a box in object space return a box that contains all boxes
     transformed by instance transforms. */
  csBox3 AdjustBboxForInstances (const csBox3& origBox) const;
  csRenderMesh** FixupRendermeshesForInstancing (int n, csRenderMesh** meshes);
public:
  CS_LEAKGUARD_DECLARE (csMeshWrapper);

  /// Set of flags
  csFlags flags;
  /// Culler flags.
  csFlags culler_flags;

  csEngine* engine;

  /**
   * Clear this object from all sector portal lists.
   * If a sector is given then it will only clear for that sector.
   */
  void ClearFromSectorPortalLists (iSector* sector = 0);
  /// Add this object to all sector portal lists.
  void AddToSectorPortalLists ();

protected:
  /// Get the bounding box in world space and correct in hierarchy.
  void GetFullBBox (csBox3& box);

  /// Move this object to the specified sector. Can be called multiple times.
  void MoveToSector (iSector* s);

  /**
   * Remove this object from all sectors it is in (but not from the engine).
   * If a sector is given then it will only be removed from that sector.
   */
  void RemoveFromSectors (iSector* sector = 0);

  /**
   * Update transformations after the object has moved
   * (through updating the movable instance).
   * This MUST be done after you change the movable otherwise
   * some of the internal data structures will not be updated
   * correctly. This function is called by movable.UpdateMove();
   */
  void UpdateMove ();

  /**
   * Destructor.  This is private in order to force clients to use DecRef()
   * for object destruction.
   */
  virtual ~csMeshWrapper ();

public:
  /// Constructor.
  csMeshWrapper (csEngine* engine, iMeshObject* meshobj = 0);

  /// Set the mesh factory.
  virtual void SetFactory (iMeshFactoryWrapper* factory);

  /// Get the mesh factory.
  virtual iMeshFactoryWrapper* GetFactory () const
  {
    return factory;
  }

  /// Set the mesh object.
  virtual void SetMeshObject (iMeshObject* meshobj);
  /// Get the mesh object.
  virtual iMeshObject* GetMeshObject () const { return meshobj; }

  virtual iPortalContainer* GetPortalContainer () const
  { return portal_container; }

  /// For iVisibilityObject: Get the object model.
  virtual iObjectModel* GetObjectModel ()
  {
    return meshobj->GetObjectModel ();
  }

  // For iVisibilityObject:
  virtual iMeshWrapper* GetMeshWrapper () const
  {
    return (iMeshWrapper*)this;
  }

  // For iVisibilityObject:
  virtual csFlags& GetCullerFlags () { return culler_flags; }

  // For iVisibilityObject:
  virtual const csBox3& GetBBox () const
  {
    // 'Always visible' mesh objects have an infinite bounding box.
    if (flags.Check (CS_ENTITY_ALWAYSVISIBLE))
      return infBBox;

    return wor_bbox;
  }

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
    return (iMovable*)&movable;
  }

  /// Set the render priority for this object.
  virtual void SetRenderPriority (CS::Graphics::RenderPriority rp);
  /// Set the render priority for this object and children.
  virtual void SetRenderPriorityRecursive (CS::Graphics::RenderPriority rp);
  /// Get the render priority for this object.
  virtual CS::Graphics::RenderPriority GetRenderPriority () const
  {
    return render_priority;
  }

  /// Set the Z-buf drawing mode to use for this object.
  virtual void SetZBufMode (csZBufMode mode) { zbufMode = mode; }
  void SetZBufModeRecursive (csZBufMode mode);
  /// Get the Z-buf drawing mode.
  virtual csZBufMode GetZBufMode () const { return zbufMode; }

  /// Set flags for mesh and children.
  virtual void SetFlagsRecursive (uint32 mask, uint32 value);

  /**
   * Set a callback which is called just before the object is drawn.
   * This is useful to do some expensive computations which only need
   * to be done on a visible object. Note that this function will be
   * called even if the object is not visible. In general it is called
   * if there is a likely probability that the object is visible (i.e.
   * it is in the same sector as the camera for example).
   */
  virtual void SetDrawCallback (iMeshDrawCallback* cb)
  {
    draw_cb_vector.Push (cb);
  }

  virtual void RemoveDrawCallback (iMeshDrawCallback* cb)
  {
    draw_cb_vector.Delete (cb);
  }

  virtual int GetDrawCallbackCount () const
  {
    return (int)draw_cb_vector.GetSize ();
  }

  virtual iMeshDrawCallback* GetDrawCallback (int idx) const
  {
    return draw_cb_vector.Get (idx);
  }

  /**
   * Draw this mesh object given a camera transformation.
   * If needed the skeleton state will first be updated.
   */
  void Draw (iRenderView* rview, uint32 frustum_mask);

  // Static LOD methods.
  virtual iLODControl* CreateStaticLOD ();
  virtual void DestroyStaticLOD ();
  virtual iLODControl* GetStaticLOD ();
  virtual void RemoveMeshFromStaticLOD (iMeshWrapper* mesh);
  virtual void AddMeshToStaticLOD (int lod, iMeshWrapper* mesh);
  csStaticLODMesh* GetStaticLODMesh () const { return static_lod; }
  /// Return true if there is a parent mesh that has static lod.
  bool SomeParentHasStaticLOD () const;

  virtual CS::Graphics::RenderMesh** GetRenderMeshes (int& num, iRenderView* rview,
  	uint32 frustum_mask);
  /**
   * Adds a render mesh to the list of extra render meshes.
   * This list is used for special cases (like decals) where additional
   * things need to be renderered for the mesh in an abstract way.
   */
  virtual size_t AddExtraRenderMesh(CS::Graphics::RenderMesh* renderMesh);
  virtual size_t AddExtraRenderMesh(CS::Graphics::RenderMesh* renderMesh, 
          csZBufMode zBufMode);

  /**
   * Grabs any additional render meshes this mesh might have on top
   * of the normal rendermeshes through GetRenderMeshes.
   */
  virtual CS::Graphics::RenderMesh** GetExtraRenderMeshes (size_t& num, iRenderView* rview,
    uint32 frustum_mask);

  /// Get a specific extra render mesh.
  virtual CS::Graphics::RenderMesh* GetExtraRenderMesh (size_t idx) const;
  
  /// Get number of extra render meshes.
  virtual size_t GetExtraRenderMeshCount () const
  { return extraRenderMeshes.GetSize(); }

  /**
   * Gets the z-buffer mode of a specific extra rendermesh
   */
  virtual csZBufMode GetExtraRenderMeshZBufMode(size_t idx) const;

  //@{
  /**
   * Deletes a specific extra rendermesh
   */
  virtual void RemoveExtraRenderMesh(CS::Graphics::RenderMesh* renderMesh);
  virtual void RemoveExtraRenderMesh(size_t idx);
  //@}

  /**
   * Do a hard transform of this object.
   * This transformation and the original coordinates are not
   * remembered but the object space coordinates are directly
   * computed (world space coordinates are set to the object space
   * coordinates by this routine). Note that some implementations
   * of mesh objects will not change the orientation of the object but
   * only the position.
   */
  virtual void HardTransform (const csReversibleTransform& t);

  /// Check if we should use an imposter (distance based).
  bool UseImposter (iRenderView *rview);

  void SetLODFade (float fade);
  void UnsetLODFade ();

  void SetDefaultEnvironmentTexture ();

  virtual csShaderVariable* AddInstance(csVector3& position, csMatrix3& rotation);
  virtual void RemoveInstance(csShaderVariable* instance);

  //---------- Bounding volume and beam functions -----------------//

  virtual csSphere GetRadius () const;
  
  virtual csHitBeamResult HitBeamBBox (const csVector3& start,
  	const csVector3& end);
  virtual csHitBeamResult HitBeamOutline (const csVector3& start,
  	const csVector3& end);
  virtual csHitBeamResult HitBeamObject (const csVector3& start,
  	const csVector3& end, bool do_material = false);
  virtual csHitBeamResult HitBeam (const csVector3& start,
  	const csVector3& end, bool do_material = false);

  /**
   * Calculate the squared distance between the camera and the object.
   */
  float GetSquaredDistance (iRenderView *rview);
  /**
   * Calculate the squared distance between a position and the object.
   */
  float GetSquaredDistance (const csVector3& pos);

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
  virtual void PlaceMesh ();

  virtual const csBox3& GetWorldBoundingBox ();
  virtual csBox3 GetTransformedBoundingBox (const csReversibleTransform& trans);
  virtual csScreenBoxResult GetScreenBoundingBox (iCamera *camera);

  //--------------------- SCF stuff follows ------------------------------//

  //--------------------- iSelfDestruct implementation -------------------//
  virtual void SelfDestruct ();

  /**\name iSceneNode implementation
   * @{ */
  virtual void SetParent (iSceneNode* parent);
  virtual iSceneNode* GetParent () const
  {
    if (movable.GetParent ())
      return movable.GetParent ()->GetSceneNode ();
    else
      return 0;
  }
  virtual const csRefArray<iSceneNode>& GetChildren () const
  {
    return movable.GetChildren ();
  }
  virtual iMeshWrapper* QueryMesh () { return this; }
  virtual iLight* QueryLight () { return 0; }
  virtual iCamera* QueryCamera () { return 0; }
  virtual csPtr<iSceneNodeArray> GetChildrenArray () const
  {
    return csPtr<iSceneNodeArray> (
      new scfArrayWrapConst<iSceneNodeArray, csRefArray<iSceneNode> > (
      movable.GetChildren ()));
  }
  /** @} */

  //--------------------- iMeshWrapper implementation --------------------//

  virtual iObject *QueryObject ()
  {
    return this;
  }
  virtual iSceneNode* QuerySceneNode ()
  {
    return this;
  }
  virtual iMeshWrapper* FindChildByName (const char* name);
  virtual csFlags& GetFlags ()
  {
    return flags;
  }
  virtual iShaderVariableContext* GetSVContext()
  {
    return (iShaderVariableContext*)this;
  }

  void ClearMinVariable ();
  void ClearMaxVariable ();
  bool DoMinMaxRange () const { return do_minmax_range; }
  virtual void ResetMinMaxRenderDistance ();
  virtual void SetMinimumRenderDistance (float min);
  virtual float GetMinimumRenderDistance () const { return min_render_dist; }
  virtual void SetMaximumRenderDistance (float max);
  virtual float GetMaximumRenderDistance () const { return max_render_dist; }
  virtual void SetMinimumRenderDistanceVar (iSharedVariable* min);
  virtual iSharedVariable* GetMinimumRenderDistanceVar () const
  {
    return var_min_render_dist;
  }
  virtual void SetMaximumRenderDistanceVar (iSharedVariable* max);
  virtual iSharedVariable* GetMaximumRenderDistanceVar () const
  {
    return var_max_render_dist;
  }
};

#include "csutil/deprecated_warn_on.h"

}
CS_PLUGIN_NAMESPACE_END(Engine)

#endif // __CS_MESHOBJ_H__
