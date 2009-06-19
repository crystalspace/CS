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
#include "iutil/selfdestruct.h"
#include "csgfx/shadervarcontext.h"
#include "imesh/object.h"
#include "imesh/lighting.h"
#include "iengine/mesh.h"
#include "iengine/imposter.h"
#include "iengine/viscull.h"
#include "iengine/shadcast.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"

#include "movable.h"
#include "impmesh.h"
#include "meshlod.h"
#include "scenenode.h"
#include "light.h"

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
  mutable CS::Threading::RecursiveMutex removeLock;

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

  virtual int GetCount () const { return (int)list.GetSize (); }
  virtual iMeshFactoryWrapper *Get (int n) const { return list.Get (n); }
  virtual int Add (iMeshFactoryWrapper *obj);
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
			       iImposter,
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
  long render_priority;
  /// Suggestion for new children created from factory.
  csZBufMode zbufMode;

  csFlags flags;

  csEngine* engine;

  /// Class for keeping track of imposter information.
  csImposterFactory* imposter_factory;

protected:
  virtual void InternalRemove() { SelfDestruct(); }

public:
  /// Flag indicating whether this factory should try to imposter or not.
  bool imposter_active;
  /// Imposter Threshold Range.
  csRef<iSharedVariable> min_imposter_distance;
  /// Imposter Redo Threshold angle change.
  csRef<iSharedVariable> imposter_rotation_tolerance;
  csRef<iSharedVariable> imposter_camera_rotation_tolerance;

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
  void SetRenderPriority (long rp);
  long GetRenderPriority () const
  {
    return render_priority;
  }
  void SetRenderPriorityRecursive (long rp);

  //---------- iImposter Functions -----------------//

  /**
   * Set true if this meshes created from this
   * factory should use Impostering. Note that this only
   * affects meshes created from this factory after this
   * has been called.
   */
  virtual void SetImposterActive (bool flag);

  /**
   * Determine if impostering is enabled for this factory
   * (not if Imposter is being drawn, but simply considered).
   */
  virtual bool GetImposterActive () const { return imposter_active; }

  /**
   * Minimum Imposter Distance is the distance from camera 
   * beyond which imposter is used. Imposter gets a 
   * ptr here because value is a shared variable 
   * which can be changed at runtime for many objects.
   *
   * All meshes created from this factory after this function
   * is called will get this variable. Meshes created before
   * calling this function will get the previous value.
   */
  virtual void SetMinDistance (iSharedVariable* dist)
  { min_imposter_distance = dist; }

  /** 
   * Rotation Tolerance is the maximum allowable 
   * angle difference between when the imposter was 
   * created and the current position of the camera.
   * Angle greater than this triggers a re-render of
   * the imposter.
   *
   * All meshes created from this factory after this function
   * is called will get this variable. Meshes created before
   * calling this function will get the previous value.
   */
  virtual void SetRotationTolerance (iSharedVariable* angle)
  { imposter_rotation_tolerance = angle; }

  /** 
   * Camera Rotation Tolerance is the tolerance angle
   * between z->1 vector and object on screen. Exceeding this
   * value triggers updating of the imposter whenever the
   * object slides too much away from the center of screen.
   *
   * All meshes created from this factory after this function
   * is called will get this variable. Meshes created before
   * calling this function will get the previous value.
   */
  virtual void SetCameraRotationTolerance (iSharedVariable* angle)
  { imposter_camera_rotation_tolerance = angle; }

  /// Determine if imposter or true rendering will be used.
  virtual bool WouldUseImposter (csReversibleTransform& /*pov*/) const
  { /* implement later */ return false; }

  /// Get the imposter factory.
  csImposterFactory* GetImposterFactory () { return imposter_factory; }

  //--------------------- iSelfDestruct implementation -------------------//

  virtual void SelfDestruct ();
};

#endif // __CS_MESHFACT_H__
