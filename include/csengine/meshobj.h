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

#ifndef __CS_MESHOBJ_H__
#define __CS_MESHOBJ_H__

#include "csobject/pobject.h"
#include "csobject/nobjvec.h"
#include "csengine/movable.h"
#include "csutil/flags.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/viscull.h"
#include "ivideo/graph3d.h"

struct iMeshWrapper;
struct iRenderView;
struct iMovable;
class Dumper;
class csMeshWrapper;
class csCamera;
class csMeshFactoryWrapper;
class csLight;

SCF_VERSION (csMeshWrapper, 0, 0, 1);

/**
 * The holder class for all implementations of iMeshObject.
 */
class csMeshWrapper : public csPObject
{
  friend class Dumper;
  friend class csMovable;

protected:
  /// Points to Actor class which "owns" this object.
  csObject* myOwner;

  /**
   * Points to the parent container object of this object.
   * This is usually csEngine or csParticleSystem.
   */
  csObject* parent;

  /**
   * Bounding box in world space.
   * This is a cache for GetWorldBoundingBox() which will recalculate this
   * if the movable changes (by using movablenr).
   */
  csBox3 wor_bbox;
  /// Last used movable number for wor_bbox.
  long wor_bbox_movablenr;

  /// Defered lighting. If > 0 then we have defered lighting.
  int defered_num_lights;

  /// Flags to use for defered lighting.
  int defered_lighting_flags;

  /**
   * This value indicates the last time that was used to do animation.
   * If 0 then we haven't done animation yet. We compare this value
   * with the value returned by engine->GetLastAnimationTime() to see
   * if we need to call meshobj->NextFrame() again.
  */
  cs_time last_anim_time;

  /**
   * Flag which is set to true when the object is visible.
   * This is used by the c-buffer/bsp routines. The object itself
   * will not use this flag in any way at all. It is simply intended
   * for external visibility culling routines.
   */
  bool is_visible;

  /**
   * Position in the world.
   */
  csMovable movable;

  /**
   * The renderer will render all objects in a sector based on this
   * number. Low numbers get rendered first. High numbers get rendered
   * later. There are a few predefined slots which the application is
   * free to use or not.
   */
  long render_priority;

  /// Update defered lighting.
  void UpdateDeferedLighting (const csVector3& pos);

private:
  /// Mesh object corresponding with this csMeshWrapper.
  iMeshObject* mesh;

  /// Children of this object (other instances of csMeshWrapper).
  csNamedObjVector children;

  /// The callback which is called just before drawing.
  csDrawCallback* draw_cb;
  /// Userdata for the draw_callback.
  void* draw_cbData;

  /// Optional reference to the parent csMeshFactoryWrapper.
  csMeshFactoryWrapper* factory;

  /// Z-buf mode to use for drawing this object.
  csZBufMode zbufMode;

public:
  /// Set of flags
  csFlags flags;

protected:
  /// Move this object to the specified sector. Can be called multiple times.
  void MoveToSector (csSector* s);

  /// Remove this object from all sectors it is in (but not from the engine).
  void RemoveFromSectors ();

  /**
   * Update transformations after the object has moved
   * (through updating the movable instance).
   * This MUST be done after you change the movable otherwise
   * some of the internal data structures will not be updated
   * correctly. This function is called by movable.UpdateMove();
   */
  void UpdateMove ();

  /**
   * Draw this mesh object given a camera transformation.
   * If needed the skeleton state will first be updated.
   * Optionally update lighting if needed (DeferUpdateLighting()).
   */
  void DrawInt (iRenderView* rview);

public:
  /// Constructor.
  csMeshWrapper (csObject* theParent, iMeshObject* mesh);
  /// Constructor.
  csMeshWrapper (csObject* theParent);
  /// Destructor.
  virtual ~csMeshWrapper ();

  /// Set owner (actor) for this object.
  void SetMyOwner (csObject* newOwner) { myOwner = newOwner; }
  /// Get owner (actor) for this object.
  csObject* GetMyOwner () { return myOwner; }

  /// Set parent container for this object.
  void SetParentContainer (csObject* newParent) { parent = newParent; }
  /// Get parent container for this object.
  csObject* GetParentContainer () { return parent; }

  /// Set the mesh factory.
  void SetFactory (csMeshFactoryWrapper* factory)
  {
    csMeshWrapper::factory = factory;
  }
  /// Get the mesh factory.
  csMeshFactoryWrapper* GetFactory ()
  {
    return factory;
  }

  /// Set the mesh object.
  void SetMeshObject (iMeshObject* mesh);
  /// Get the mesh object.
  iMeshObject* GetMeshObject () const {return mesh;}

  /// Set the Z-buf drawing mode to use for this object.
  void SetZBufMode (csZBufMode mode) { zbufMode = mode; }
  /// Get the Z-buf drawing mode.
  csZBufMode GetZBufMode () { return zbufMode; }

  /**
   * Set a callback which is called just before the object is drawn.
   * This is useful to do some expensive computations which only need
   * to be done on a visible object. Note that this function will be
   * called even if the object is not visible. In general it is called
   * if there is a likely probability that the object is visible (i.e.
   * it is in the same sector as the camera for example).
   */
  void SetDrawCallback (csDrawCallback* cb, void* cbData)
  {
    draw_cb = cb;
    draw_cbData = cbData;
  }

  /// Get the draw callback.
  csDrawCallback* GetDrawCallback ()
  {
    return draw_cb;
  }

  /// Mark this object as visible.
  void MarkVisible () { is_visible = true; }

  /// Mark this object as invisible.
  void MarkInvisible () { is_visible = false; }

  /// Return if this object is visible.
  bool IsVisible () { return is_visible; }

  /**
   * Light object according to the given array of lights (i.e.
   * fill the vertex color array).
   */
  void UpdateLighting (iLight** lights, int num_lights);

  /**
   * Update lighting as soon as the object becomes visible.
   * This will call engine->GetNearestLights with the supplied
   * parameters.
   */
  void DeferUpdateLighting (int flags, int num_lights);

  /**
   * Draw this mesh object given a camera transformation.
   * If needed the skeleton state will first be updated.
   * Optionally update lighting if needed (DeferUpdateLighting()).
   */
  void Draw (iRenderView* rview);

  /// Returns true if this object wants to die.
  bool WantToDie () { return mesh->WantToDie (); }

  /**
   * Get the movable instance for this object.
   * It is very important to call GetMovable().UpdateMove()
   * after doing any kind of modification to this movable
   * to make sure that internal data structures are
   * correctly updated.
   */
  csMovable& GetMovable () { return movable; }

  /**
   * Check if this object is hit by this object space vector.
   * Return the collision point in object space coordinates.
   */
  bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr);
  /**
   * Check if this object is hit by this world space vector.
   * Return the collision point in world space coordinates.
   */
  bool HitBeam (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr);

  /// Get the children of this mesh object.
  csNamedObjVector& GetChildren () { return children; }

  /// Get the radius of this mesh (ignoring children).
  csVector3 GetRadius () { return mesh->GetRadius (); }

  /**
   * Do a hard transform of this object.
   * This transformation and the original coordinates are not
   * remembered but the object space coordinates are directly
   * computed (world space coordinates are set to the object space
   * coordinates by this routine). Note that some implementations
   * of mesh objects will not change the orientation of the object but
   * only the position.
   */
  void HardTransform (const csReversibleTransform& t);

  /**
   * Get the bounding box of this object in world space.
   * This routine will cache the bounding box and only recalculate it
   * if the movable changes.
   */
  void GetWorldBoundingBox (csBox3& cbox);

  /**
   * Get the bounding box of this object after applying a transformation to it.
   * This is really a very inaccurate function as it will take the bounding
   * box of the object in object space and then transform this bounding box.
   */
  void GetTransformedBoundingBox (const csReversibleTransform& trans,
  	csBox3& cbox);

  /**
   * Get a very inaccurate bounding box of the object in screen space.
   * Returns -1 if object behind the camera or else the distance between
   * the camera and the furthest point of the 3D box.
   */
  float GetScreenBoundingBox (const csCamera& camera, csBox2& sbox,
  	csBox3& cbox);

  /**
   * Rotate object in some manner in radians.
   * This function operates by rotating the movable transform.
   */
  void Rotate (float angle);

  /**
   * Scale object by this factor.
   * This function operates by scaling the movable transform.
   */
  void ScaleBy (float factor);

  /// Set the render priority for this object.
  void SetRenderPriority (long rp);
  /// Get the render priority for this object.
  long GetRenderPriority ()
  {
    return render_priority;
  }

  CSOBJTYPE;
  DECLARE_IBASE_EXT (csPObject);

  //--------------------- iMeshWrapper implementation --------------------//
  struct MeshWrapper : public iMeshWrapper
  {
    DECLARE_EMBEDDED_IBASE (csMeshWrapper);
    virtual csMeshWrapper* GetPrivateObject ()
    {
      return (csMeshWrapper*)scfParent;
    }
    virtual iMeshObject* GetMeshObject ()
    {
      return scfParent->GetMeshObject ();
    }
    virtual void SetMeshObject (iMeshObject* m)
    {
      scfParent->SetMeshObject (m);
    }
    virtual iObject *QueryObject()
    {
      return scfParent;
    }
    virtual void DeferUpdateLighting (int flags, int num_lights)
    {
      scfParent->DeferUpdateLighting (flags, num_lights);
    }
    virtual void UpdateLighting (iLight** lights, int num_lights)
    {
      scfParent->UpdateLighting (lights, num_lights);
    }
    virtual iMovable* GetMovable ()
    {
      return &(scfParent->movable.scfiMovable);
    }
    virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr)
    {
      return scfParent->HitBeamObject (start, end, isect, pr);
    }
    virtual bool HitBeam (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr)
    {
      return scfParent->HitBeam (start, end, isect, pr);
    }
    virtual void SetDrawCallback (csDrawCallback* cb, void* cbData)
    {
      scfParent->SetDrawCallback (cb, cbData);
    }
    virtual csDrawCallback* GetDrawCallback ()
    {
      return scfParent->GetDrawCallback ();
    }
    virtual void SetFactory (iMeshFactoryWrapper* factory);
    virtual void SetRenderPriority (long rp)
    {
      scfParent->SetRenderPriority (rp);
    }
    virtual long GetRenderPriority ()
    {
      return scfParent->GetRenderPriority ();
    }
    virtual csFlags& GetFlags ()
    {
      return scfParent->flags;
    }
    virtual void SetZBufMode (csZBufMode mode)
    {
      scfParent->SetZBufMode (mode);
    }
    virtual csZBufMode GetZBufMode ()
    {
      return scfParent->GetZBufMode ();
    }
    virtual void HardTransform (const csReversibleTransform& t)
    {
      scfParent->HardTransform (t);
    }
    virtual void GetWorldBoundingBox (csBox3& cbox)
    {
      scfParent->GetWorldBoundingBox (cbox);
    }
    virtual void GetTransformedBoundingBox (const csReversibleTransform& trans,
  	csBox3& cbox)
    {
      scfParent->GetTransformedBoundingBox (trans, cbox);
    }
    virtual void AddChild (iMeshWrapper* child);
    virtual int GetChildCount () { return scfParent->GetChildren ().Length (); }
    virtual iMeshWrapper* GetChild (int idx);
    virtual csVector3 GetRadius () { return scfParent->GetRadius (); }
  } scfiMeshWrapper;
  friend struct MeshWrapper;

  //-------------------- iVisibilityObject interface implementation ----------
  struct VisObject : public iVisibilityObject
  {
    DECLARE_EMBEDDED_IBASE (csMeshWrapper);
    virtual iMovable* GetMovable ()
    {
      return &(scfParent->movable.scfiMovable);
    }
    virtual long GetShapeNumber ()
    {
      return scfParent->mesh->GetShapeNumber ();
    }
    virtual void GetBoundingBox (csBox3& bbox)
    {
      scfParent->mesh->GetObjectBoundingBox (bbox, CS_BBOX_MAX);
    }
    virtual void MarkVisible () { scfParent->MarkVisible (); }
    virtual void MarkInvisible () { scfParent->MarkInvisible (); }
    virtual bool IsVisible () { return scfParent->IsVisible (); }
  } scfiVisibilityObject;
  friend struct VisObject;
};

SCF_VERSION (csMeshFactoryWrapper, 0, 0, 1);

/**
 * The holder class for all implementations of iMeshObjectFactory.
 */
class csMeshFactoryWrapper : public csObject
{
  friend class Dumper;

private:
  /// Mesh object factory corresponding with this csMeshFactoryWrapper.
  iMeshObjectFactory* meshFact;

public:
  /// Constructor.
  csMeshFactoryWrapper (iMeshObjectFactory* meshFact);
  /// Constructor.
  csMeshFactoryWrapper ();
  /// Destructor.
  virtual ~csMeshFactoryWrapper ();

  /// Set the mesh object factory.
  void SetMeshObjectFactory (iMeshObjectFactory* meshFact);

  /// Get the mesh object factory.
  iMeshObjectFactory* GetMeshObjectFactory ()
  {
    return meshFact;
  }

  /**
   * Create a new mesh object for this template.
   */
  csMeshWrapper* NewMeshObject (csObject* parent);

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

  CSOBJTYPE;
  DECLARE_IBASE_EXT (csObject);

  //--------------------- iMeshFactoryWrapper implementation --------------------//
  struct MeshFactoryWrapper : public iMeshFactoryWrapper
  {
    DECLARE_EMBEDDED_IBASE (csMeshFactoryWrapper);
    virtual csMeshFactoryWrapper* GetPrivateObject ()
    {
      return (csMeshFactoryWrapper*)scfParent;
    }
    virtual iMeshObjectFactory* GetMeshObjectFactory ()
    {
      return scfParent->GetMeshObjectFactory ();
    }
    virtual void SetMeshObjectFactory (iMeshObjectFactory* fact)
    {
      scfParent->SetMeshObjectFactory (fact);
    }
    virtual iObject *QueryObject()
    {
      return scfParent;
    }
    virtual void HardTransform (const csReversibleTransform& t)
    {
      scfParent->HardTransform (t);
    }
  } scfiMeshFactoryWrapper;
};

#endif // __CS_MESHOBJ_H__
