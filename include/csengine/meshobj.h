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

#include "csgeom/transfrm.h"
#include "csutil/csobject.h"
#include "csutil/nobjvec.h"
#include "csutil/flags.h"
#include "csutil/garray.h"
#include "csengine/movable.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/viscull.h"
#include "ivideo/graph3d.h"

struct iMeshWrapper;
struct iRenderView;
struct iMovable;
class csMeshWrapper;
class csMeshFactoryWrapper;
class csLight;

CS_DECLARE_OBJECT_VECTOR (csMeshListHelper, iMeshWrapper);

/**
 * General list of meshes. This class implements iMeshList.
 * Subclasses of this class can override FreeItem(), AddMesh(),
 * and RemoveMesh() for more specific functionality.
 */
class csMeshList : public csMeshListHelper
{
public:
  SCF_DECLARE_IBASE;

  /// constructor
  csMeshList ();
  /// destructor
  ~csMeshList ();

  /// Override FreeItem
  virtual bool FreeItem (csSome Item);
  /// Add a mesh.
  virtual void AddMesh (iMeshWrapper* mesh);
  /// Remove a mesh.
  virtual void RemoveMesh (iMeshWrapper* mesh);

  class MeshList : public iMeshList
  {
    SCF_DECLARE_EMBEDDED_IBASE (csMeshList);
    virtual int GetCount () const;
    virtual iMeshWrapper *Get (int n) const;
    virtual int Add (iMeshWrapper *obj);
    virtual bool Remove (iMeshWrapper *obj);
    virtual bool Remove (int n);
    virtual void RemoveAll ();
    virtual int Find (iMeshWrapper *obj) const;
    virtual iMeshWrapper *FindByName (const char *Name) const;
  } scfiMeshList;
};

/**
 * Subclass of csMeshList to hold the children of another mesh object.
 */
class csMeshMeshList : public csMeshList
{
private:
  csMeshWrapper* mesh;

public:
  csMeshMeshList () : mesh (NULL) { }
  void SetMesh (csMeshWrapper* m) { mesh = m; }
  virtual void AddMesh (iMeshWrapper* child);
  virtual void RemoveMesh (iMeshWrapper* child);
};

CS_DECLARE_OBJECT_VECTOR (csMeshFactoryListHelper, iMeshFactoryWrapper);

/**
 * A list of mesh factories.
 */
class csMeshFactoryList : public csMeshFactoryListHelper
{
public:
  SCF_DECLARE_IBASE;

  /// constructor
  csMeshFactoryList ();
  /// destructor
  ~csMeshFactoryList ();

  /// Add a mesh factory.
  virtual void AddMeshFactory (iMeshFactoryWrapper* mesh);
  /// Remove a mesh factory.
  virtual void RemoveMeshFactory (iMeshFactoryWrapper* mesh);

  class MeshFactoryList : public iMeshFactoryList
  {
    SCF_DECLARE_EMBEDDED_IBASE (csMeshFactoryList);
    virtual int GetCount () const;
    virtual iMeshFactoryWrapper *Get (int n) const;
    virtual int Add (iMeshFactoryWrapper *obj);
    virtual bool Remove (iMeshFactoryWrapper *obj);
    virtual bool Remove (int n);
    virtual void RemoveAll ();
    virtual int Find (iMeshFactoryWrapper *obj) const;
    virtual iMeshFactoryWrapper *FindByName (const char *Name) const;
  } scfiMeshFactoryList;
};

/**
 * Subclass of csMeshFactoryList to hold the children of another mesh factory.
 */
class csMeshFactoryFactoryList : public csMeshFactoryList
{
private:
  csMeshFactoryWrapper* meshfact;

public:
  csMeshFactoryFactoryList () : meshfact (NULL) { }
  void SetMeshFactory (csMeshFactoryWrapper* m) { meshfact = m; }
  virtual void AddMeshFactory (iMeshFactoryWrapper* child);
  virtual void RemoveMeshFactory (iMeshFactoryWrapper* child);
};

SCF_VERSION (csMeshWrapper, 0, 0, 1);

/**
 * The holder class for all implementations of iMeshObject.
 */
class csMeshWrapper : public csObject
{
  friend class csMovable;
  friend class csMovableSectorList;

protected:

  /// The parent mesh object, or NULL
  iMeshWrapper *Parent;

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
  csTicks last_anim_time;

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

  /// Children of this object (other instances of iMeshWrapper).
  csMeshMeshList children;

  /// The callback which is called just before drawing.
  iMeshDrawCallback* draw_cb;

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

  /**
   * Destructor.  This is private in order to force clients to use DecRef()
   * for object destruction.
   */
  virtual ~csMeshWrapper ();

public:
  /// Constructor.
  csMeshWrapper (iMeshWrapper* theParent, iMeshObject* mesh);
  /// Constructor.
  csMeshWrapper (iMeshWrapper* theParent);

  /// Set parent container for this object.
  void SetParentContainer (iMeshWrapper* newParent) { Parent = newParent; }
  /// Get parent container for this object.
  iMeshWrapper* GetParentContainer () const { return Parent; }

  /// Set the mesh factory.
  void SetFactory (csMeshFactoryWrapper* factory)
  {
    csMeshWrapper::factory = factory;
  }
  /// Get the mesh factory.
  csMeshFactoryWrapper* GetFactory () const
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
  csZBufMode GetZBufMode () const { return zbufMode; }

  /**
   * Set a callback which is called just before the object is drawn.
   * This is useful to do some expensive computations which only need
   * to be done on a visible object. Note that this function will be
   * called even if the object is not visible. In general it is called
   * if there is a likely probability that the object is visible (i.e.
   * it is in the same sector as the camera for example).
   */
  void SetDrawCallback (iMeshDrawCallback* cb)
  {
    if (cb) cb->IncRef ();
    if (draw_cb) draw_cb->DecRef ();
    draw_cb = cb;
  }

  /// Get the draw callback.
  iMeshDrawCallback* GetDrawCallback () const
  {
    return draw_cb;
  }

  /// Mark this object as visible.
  void MarkVisible () { is_visible = true; }

  /// Mark this object as invisible.
  void MarkInvisible () { is_visible = false; }

  /// Return if this object is visible.
  bool IsVisible () const { return is_visible; }

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
  void PlaceMesh ();

  /**
   * Check if this object is hit by this object space vector.
   * BBox version.
   */
  int HitBeamBBox (const csVector3& start, const csVector3& end, 
         csVector3& isect, float* pr);
  /**
   * Check if this object is hit by this object space vector.
   * Outline version.
   */
  bool HitBeamOutline (const csVector3& start, const csVector3& end,
         csVector3& isect, float* pr);
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
  const csMeshMeshList& GetChildren () const { return children; }

  /// Get the radius of this mesh (ignoring children).
  void GetRadius (csVector3& rad, csVector3& cent) const 
	{ mesh->GetRadius (rad,cent); }

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
  float GetScreenBoundingBox (const iCamera *camera, csBox2& sbox,
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
  long GetRenderPriority () const
  {
    return render_priority;
  }

  /// Add a child to this mesh.
  void AddChild (iMeshWrapper* child);
  /// Remove a child from this mesh.
  void RemoveChild (iMeshWrapper* child);

  SCF_DECLARE_IBASE_EXT (csObject);

  //--------------------- iMeshWrapper implementation --------------------//
  struct MeshWrapper : public iMeshWrapper
  {
    SCF_DECLARE_EMBEDDED_IBASE (csMeshWrapper);
    virtual csMeshWrapper* GetPrivateObject ()
    {
      return (csMeshWrapper*)scfParent;
    }
    virtual iMeshObject* GetMeshObject () const
    {
      return scfParent->GetMeshObject ();
    }
    virtual void SetMeshObject (iMeshObject* m)
    {
      scfParent->SetMeshObject (m);
    }
    virtual iObject *QueryObject ()
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
    virtual iMovable* GetMovable () const
    {
      return &(scfParent->movable.scfiMovable);
    }
    virtual void PlaceMesh ()
    {
      scfParent->PlaceMesh ();
    }
    virtual int HitBeamBBox (const csVector3& start, const csVector3& end,
          csVector3& isect, float* pr)
    {
      return scfParent->HitBeamBBox (start, end, isect, pr);
    }
    virtual bool HitBeamOutline (const csVector3& start, const csVector3& end,
          csVector3& isect, float* pr)
    {
      return scfParent->HitBeamOutline (start, end, isect, pr);
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
    virtual void SetDrawCallback (iMeshDrawCallback* cb)
    {
      scfParent->SetDrawCallback (cb);
    }
    virtual iMeshDrawCallback* GetDrawCallback () const
    {
      return scfParent->GetDrawCallback ();
    }
    virtual void SetFactory (iMeshFactoryWrapper* factory);
    virtual void SetRenderPriority (long rp)
    {
      scfParent->SetRenderPriority (rp);
    }
    virtual long GetRenderPriority () const
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
    virtual csZBufMode GetZBufMode () const
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
    virtual float GetScreenBoundingBox (iCamera* camera, csBox2& sbox,
  	csBox3& cbox);
    virtual iMeshList* GetChildren ()
    {
      return &(scfParent->children.scfiMeshList);
    }
    virtual iMeshWrapper* GetParentContainer ()
    {
      return scfParent->GetParentContainer ();
    }
    virtual void SetParentContainer (iMeshWrapper* p)
    {
      scfParent->SetParentContainer (p);
    }
    virtual void GetRadius (csVector3& rad, csVector3 &cent) const 
	  { scfParent->GetRadius (rad,cent); }
    virtual void Draw (iRenderView* rview)
    {
      scfParent->Draw (rview);
    }
    virtual bool WantToDie ()
    {
      return scfParent->WantToDie ();
    }
  } scfiMeshWrapper;
  friend struct MeshWrapper;

  //-------------------- iVisibilityObject interface implementation ----------
  struct VisObject : public iVisibilityObject
  {
    SCF_DECLARE_EMBEDDED_IBASE (csMeshWrapper);
    virtual iMovable* GetMovable () const
    {
      return &(scfParent->movable.scfiMovable);
    }
    virtual long GetShapeNumber () const
    {
      return scfParent->mesh->GetShapeNumber ();
    }
    virtual void GetBoundingBox (csBox3& bbox)
    {
      scfParent->mesh->GetObjectBoundingBox (bbox, CS_BBOX_MAX);
    }
    virtual void MarkVisible () { scfParent->MarkVisible (); }
    virtual void MarkInvisible () { scfParent->MarkInvisible (); }
    virtual bool IsVisible () const { return scfParent->IsVisible (); }
  } scfiVisibilityObject;
  friend struct VisObject;
};

SCF_VERSION (csMeshFactoryWrapper, 0, 0, 3);

/**
 * The holder class for all implementations of iMeshObjectFactory.
 */
class csMeshFactoryWrapper : public csObject
{
private:
  /// Mesh object factory corresponding with this csMeshFactoryWrapper.
  iMeshObjectFactory* meshFact;

  /// Optional parent of this object (can be NULL).
  csMeshFactoryWrapper* parent;
  /// Optional relative transform to parent.
  csReversibleTransform transform;

  /// Children of this object (other instances of iMeshFactoryWrapper).
  csMeshFactoryFactoryList children;

private:
  /// Destructor.
  virtual ~csMeshFactoryWrapper ();

public:
  /// Constructor.
  csMeshFactoryWrapper (iMeshObjectFactory* meshFact);
  /// Constructor.
  csMeshFactoryWrapper ();

  /// Set the mesh object factory.
  void SetMeshObjectFactory (iMeshObjectFactory* meshFact);

  /// Get the mesh object factory.
  iMeshObjectFactory* GetMeshObjectFactory () const
  {
    return meshFact;
  }

  /**
   * Create a new mesh object for this template.
   */
  csMeshWrapper* NewMeshObject ();

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

  /// Add a child to this mesh factory.
  void AddChild (iMeshFactoryWrapper* child);
  /// Remove a child from this mesh factory.
  void RemoveChild (iMeshFactoryWrapper* child);

  /**
   * Get optional relative transform (relative to parent).
   */
  csReversibleTransform& GetTransform () { return transform; }

  /**
   * Set optional relative transform (relative to parent).
   */
  void SetTransform (const csReversibleTransform& tr) { transform = tr; }

  SCF_DECLARE_IBASE_EXT (csObject);

  //----------------- iMeshFactoryWrapper implementation --------------------//
  struct MeshFactoryWrapper : public iMeshFactoryWrapper
  {
    SCF_DECLARE_EMBEDDED_IBASE (csMeshFactoryWrapper);
    virtual csMeshFactoryWrapper* GetPrivateObject ()
    {
      return (csMeshFactoryWrapper*)scfParent;
    }
    virtual iMeshObjectFactory* GetMeshObjectFactory () const
    {
      return scfParent->GetMeshObjectFactory ();
    }
    virtual void SetMeshObjectFactory (iMeshObjectFactory* fact)
    {
      scfParent->SetMeshObjectFactory (fact);
    }
    virtual iObject *QueryObject ()
    {
      return scfParent;
    }
    virtual void HardTransform (const csReversibleTransform& t)
    {
      scfParent->HardTransform (t);
    }
    virtual iMeshWrapper* CreateMeshWrapper ();
    virtual iMeshFactoryWrapper* GetParentContainer () const;
    virtual iMeshFactoryList* GetChildren ()
    {
      return &(scfParent->children.scfiMeshFactoryList);
    }
    virtual csReversibleTransform& GetTransform ()
    {
      return scfParent->GetTransform ();
    }
    virtual void SetTransform (const csReversibleTransform& tr)
    {
      scfParent->SetTransform (tr);
    }
  } scfiMeshFactoryWrapper;
  friend struct MeshFactoryWrapper;
};

#endif // __CS_MESHOBJ_H__
