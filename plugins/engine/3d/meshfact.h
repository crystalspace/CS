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

#ifndef __CS_MESHFACT_H__
#define __CS_MESHFACT_H__

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

struct iMeshFactLoaderIterator;
struct iMeshWrapper;
struct iMovable;
struct iRenderView;
struct iSharedVariable;
class csEngine;
class csMeshFactoryWrapper;

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{
  class csLight;
}
CS_PLUGIN_NAMESPACE_END(Engine)

/**
 * A list of mesh factories.
 */
class csMeshFactoryList : public scfImplementation1<csMeshFactoryList,
	iMeshFactoryList>
{
private:
  csRefArrayObject<iMeshFactoryWrapper, CS::Container::ArrayAllocDefault,
    csArrayCapacityFixedGrow<64> > list;
  csHash<iMeshFactoryWrapper*, csString>
  	factories_hash;
  mutable CS::Threading::ReadWriteMutex meshFactLock;

  class NameChangeListener : public scfImplementation1<NameChangeListener,
  	iObjectNameChangeListener>
  {
  private:
    csWeakRef<csMeshFactoryList> list;

  public:
    NameChangeListener (csMeshFactoryList* list) : scfImplementationType (this),
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
  csMeshFactoryList ();
  virtual ~csMeshFactoryList ();
  virtual void PrepareFactory (iMeshFactoryWrapper*) { }
  virtual void FreeFactory (iMeshFactoryWrapper*) { }

  virtual int GetCount () const;
  virtual iMeshFactoryWrapper *Get (int n) const;
  virtual int Add (iMeshFactoryWrapper *obj);
  void AddBatch (csRef<iMeshFactLoaderIterator> itr);
  virtual bool Remove (iMeshFactoryWrapper *obj);
  virtual bool Remove (int n);
  virtual void RemoveAll ();
  virtual int Find (iMeshFactoryWrapper *obj) const;
  virtual iMeshFactoryWrapper *FindByName (const char *Name) const;
};

/**
 * Subclass of csMeshFactoryList to hold the children of another mesh factory.
 */
class csMeshFactoryFactoryList : public csMeshFactoryList
{
private:
  csMeshFactoryWrapper* meshfact;

public:
  csMeshFactoryFactoryList () : meshfact (0) {}
  virtual ~csMeshFactoryFactoryList () { RemoveAll (); }
  void SetMeshFactory (csMeshFactoryWrapper* m) { meshfact = m; }
  virtual void PrepareFactory (iMeshFactoryWrapper* item);
  virtual void FreeFactory (iMeshFactoryWrapper* item);
};

/**
 * The holder class for all implementations of iMeshObjectFactory.
 */
class csMeshFactoryWrapper : 
  public scfImplementationExt4<csMeshFactoryWrapper,
                               csObject, 
                               iMeshFactoryWrapper,
                               iImposterFactory,
                               scfFakeInterface<iShaderVariableContext>,
			                         iSelfDestruct>,
  public CS::ShaderVariableContextImpl
{
private:
  /// Mesh object factory corresponding with this csMeshFactoryWrapper.
  csRef<iMeshObjectFactory> meshFact;

  /// Optional parent of this object (can be 0).
  iMeshFactoryWrapper* parent;

  /// Optional relative transform to parent.
  csReversibleTransform transform;

  /// Children of this object (other instances of iMeshFactoryWrapper).
  csMeshFactoryFactoryList children;

  /**
   * Optional LOD control that will turn a hierarchical mesh in a
   * mesh that supports static LOD.
   */
  csRef<csStaticLODFactoryMesh> static_lod;

  /// Suggestion for new children created from factory.
  CS::Graphics::RenderPriority render_priority;
  /// Suggestion for new children created from factory.
  csZBufMode zbufMode;

  csFlags flags;

  csEngine* engine;

  /// Hash of active imposters.
  csHash<csRef<iImposterMesh>, csPtrKey<iMeshWrapper> > imposters;

  /// Imposter Threshold Range.
  float min_imposter_distance;

  /// Imposter Redo Threshold angle change.
  float imposter_rotation_tolerance;
  float imposter_camera_rotation_tolerance;

  // Array of shaders that are applied to the imposter material.
  csArray<ImposterShader> imposter_shaders;

  // Whether to render the 'real' mesh while waiting for the imposter to init.
  bool imposter_renderReal;

  // Factory to be instanced.
  iMeshFactoryWrapper* instanceFactory;

  // Array of transform vars for the imposter instances.
  csRef<csShaderVariable> transformVars;

  // Instance transform shadervar.
  CS::ShaderVarStringID varTransform;

  // Used to store extra rendermeshes that something might attach to this
  // mesh (ie, for decals or lines)
  csDirtyAccessArray<csRenderMesh*> extraRenderMeshes;

protected:
  virtual void InternalRemove() { SelfDestruct(); }

public:
  /// Constructor.
  csMeshFactoryWrapper (csEngine* engine, iMeshObjectFactory* meshFact);
  /// Constructor.
  csMeshFactoryWrapper (csEngine* engine);
  /// Destructor.
  virtual ~csMeshFactoryWrapper ();

  CS_LEAKGUARD_DECLARE (csMeshFactoryWrapper);

  /// Set the mesh object factory.
  void SetMeshObjectFactory (iMeshObjectFactory* meshFact);

  csEngine* GetEngine () const { return engine; }

  /// Get the mesh object factory.
  iMeshObjectFactory* GetMeshObjectFactory () const
  {
    return meshFact;
  }

  virtual csFlags& GetFlags ()
  {
    return flags;
  }

  /**
   * Create a new mesh object for this template.
   */
  csPtr<iMeshWrapper> CreateMeshWrapper ();

  /**
   * Do a hard transform of this factory.
   * This transformation and the original coordinates are not
   * remembered but the object space coordinates are directly
   * computed (world space coordinates are set to the object space
   * coordinates by this routine). Note that some implementations
   * of mesh objects will not change the orientation of the object but
   * only the position.
   */
  void HardTransform (const csReversibleTransform& t);

  /// This function is called for every child that is added
  void PrepareChild (iMeshFactoryWrapper* child);
  /// This function is called for every child that is removed
  void UnprepareChild (iMeshFactoryWrapper* child);

  /**
   * Get optional relative transform (relative to parent).
   */
  csReversibleTransform& GetTransform () { return transform; }

  /**
   * Set optional relative transform (relative to parent).
   */
  void SetTransform (const csReversibleTransform& tr) { transform = tr; }

  virtual iMeshFactoryWrapper* GetParentContainer () const { return parent; }
  virtual void SetParentContainer (iMeshFactoryWrapper *p) { parent = p; }
  virtual iMeshFactoryList* GetChildren (){ return &children; }

  virtual iObject* QueryObject () { return this; }

  virtual iShaderVariableContext* GetSVContext()
  { return static_cast<iShaderVariableContext*> (this); }

  // Static LOD methods.
  iLODControl* CreateStaticLOD ();
  void DestroyStaticLOD ();
  iLODControl* GetStaticLOD ();
  void SetStaticLOD (float m, float a);
  void GetStaticLOD (float& m, float& a) const;
  void RemoveFactoryFromStaticLOD (iMeshFactoryWrapper* mesh);
  void AddFactoryToStaticLOD (int lod, iMeshFactoryWrapper* mesh);

  // Flags that are used for children.
  void SetZBufMode (csZBufMode mode) { zbufMode = mode; }
  csZBufMode GetZBufMode () const { return zbufMode; }
  void SetZBufModeRecursive (csZBufMode mode);
  void SetRenderPriority (CS::Graphics::RenderPriority rp);
  CS::Graphics::RenderPriority GetRenderPriority () const
  {
    return render_priority;
  }
  void SetRenderPriorityRecursive (CS::Graphics::RenderPriority rp);

  /**
   * Sets the instance factory.
   */
  virtual void SetInstanceFactory(iMeshFactoryWrapper* meshfact)
  {
    instanceFactory = meshfact;
    meshFact = instanceFactory->GetMeshObjectFactory();
  }

  /**
   * Returns the instance factory.
   */
  virtual iMeshFactoryWrapper* GetInstanceFactory() const
  {
    return instanceFactory;
  }

  /**
   * Adds a (pseudo-)instance of the instance factory at the given position.
   */
  virtual void AddInstance(csVector3& position, csMatrix3& rotation);

  /**
   * Returns the instancing transforms array shadervar/
   */
  virtual csShaderVariable* GetInstances() const
  {
    return transformVars;
  }

  //---------- iImposterFactory Functions -----------------//

  /**
   * Given a mesh, activate its imposter.
   */
  virtual void AddImposter (iMeshWrapper* mesh, iRenderView* rview);

 /**
  * Whether we are currently rendering the imposter
  */
  virtual bool RenderingImposter(iMeshWrapper* mesh);

  /**
   * Given a mesh, deactivate and remove its imposter.
   */
  virtual void RemoveImposter (iMeshWrapper* mesh);

  /**
   * Sets the minimum imposter distance.
   * This is the distance from camera beyond which an imposter is used.
   * The imposter gets a ptr here because the value is a shared variable 
   * which can be changed at runtime for many objects.
   *
   * All meshes created from this factory after this function
   * is called will get this variable. Meshes created before
   * calling this function will get the previous value.
   */
  virtual void SetMinDistance (float dist)
  { min_imposter_distance = dist; }

  /**
   * Gets the minimum imposter distance.
   */
  virtual float GetMinDistance()
  {
    return min_imposter_distance;
  }

  /** 
   * Sets the rotation tolerance.
   * This is the maximum allowable angle difference between when the
   * imposter was created and the current position of the camera.
   * Angles greater than this trigger a re-render of the imposter.
   *
   * All meshes created from this factory after this function
   * is called will get this variable. Meshes created before
   * calling this function will get the previous value.
   */
  virtual void SetRotationTolerance (float angle)
  { imposter_rotation_tolerance = angle; }

  /**
   * Gets the rotation tolerance.
   */
  virtual float GetRotationTolerance()
  { return imposter_rotation_tolerance; }

  /** 
   * Sets the camera rotation tolerance.
   * This is the tolerance angle between the z->1 vector and the object
   * on screen. Exceeding this value triggers the updating of the imposter
   * whenever the object slides too much away from the center of screen.
   *
   * All meshes created from this factory after this function
   * is called will get this variable. Meshes created before
   * calling this function will get the previous value.
   */
  virtual void SetCameraRotationTolerance (float angle)
  { imposter_camera_rotation_tolerance = angle; }

  /**
   * Gets the camera rotation tolerance.
   */
  virtual float GetCameraRotationTolerance()
  { return imposter_camera_rotation_tolerance; }

 /**
  * Sets the shader to be used by the imposters.
  */
  virtual void SetShader(const char* type, const char* name)
  {
    imposter_shaders.Push (ImposterShader (type, name));
  }

  /**
   * Sets whether to render the real mesh while waiting for the imposter to init.
   */
  virtual void SetRenderReal(bool renderReal)
  {
    imposter_renderReal = renderReal;
  }

  //--------------------- iSelfDestruct implementation -------------------//

  virtual void SelfDestruct ();

  //---------- Extra Render Meshes Functions -----------------//

  /**
   * Adds a render mesh to the list of extra render meshes.
   * This list is used for special cases (like decals) where additional
   * things need to be renderered for the mesh in an abstract way.
   */
  virtual size_t AddExtraRenderMesh(CS::Graphics::RenderMesh* renderMesh);

  /// Get a specific extra render mesh.
  virtual CS::Graphics::RenderMesh* GetExtraRenderMesh (size_t idx) const;
  
  /// Get number of extra render meshes.
  virtual size_t GetExtraRenderMeshCount () const
  { return extraRenderMeshes.GetSize(); }

  /**
   * Deletes a specific extra rendermesh
   */
  virtual void RemoveExtraRenderMesh(CS::Graphics::RenderMesh* renderMesh);
  virtual void RemoveExtraRenderMesh(size_t idx);
};

#endif // __CS_MESHFACT_H__
