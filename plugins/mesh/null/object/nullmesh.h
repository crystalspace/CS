/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef _NULLMESH_H_
#define _NULLMESH_H_

#include "csgeom/vector3.h"
#include "csgeom/transfrm.h"
#include "csutil/cscolor.h"
#include "csutil/refarr.h"
#include "imesh/object.h"
#include "imesh/nullmesh.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivideo/graph3d.h"
#include "ivideo/vbufmgr.h"
#include "igeom/objmodel.h"
#include "igeom/polymesh.h"

struct iMaterialWrapper;
struct iObjectRegistry;

/**
 * Nullmesh version of mesh object.
 */
class csNullmeshMeshObject : public iMeshObject
{
private:
  iMeshObjectFactory* factory;
  iBase* logparent;
  iMeshObjectDrawCallback* vis_cb;
  float radius;
  long shapenr;
  csRefArray<iObjectModelListener> listeners;

public:
  /// Constructor.
  csNullmeshMeshObject (iMeshObjectFactory* factory);

  /// Destructor.
  virtual ~csNullmeshMeshObject ();

  void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL);
  void GetRadius (csVector3& rad, csVector3& cent);

  void SetRadius (float radius)
  {
    csNullmeshMeshObject::radius = radius;
    shapenr++;
    FireListeners ();
  }
  float GetRadius () const { return radius; }
  void FireListeners ();
  void AddListener (iObjectModelListener* listener);
  void RemoveListener (iObjectModelListener* listener);

  //----------------------- iMeshObject implementation ------------------------
  SCF_DECLARE_IBASE;

  virtual iMeshObjectFactory* GetFactory () const
  {
    return (iMeshObjectFactory*)factory;
  }
  virtual bool DrawTest (iRenderView* rview, iMovable* movable);
  virtual void UpdateLighting (iLight** lights, int num_lights,
      	iMovable* movable);
  virtual bool Draw (iRenderView* rview, iMovable* movable, csZBufMode mode);
  virtual void SetVisibleCallback (iMeshObjectDrawCallback* cb)
  {
    if (cb) cb->IncRef ();
    if (vis_cb) vis_cb->DecRef ();
    vis_cb = cb;
  }
  virtual iMeshObjectDrawCallback* GetVisibleCallback () const
  {
    return vis_cb;
  }
  virtual void NextFrame (csTicks /*current_time*/) { }
  virtual bool WantToDie () const { return false; }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual bool HitBeamOutline (const csVector3& start, const csVector3& end,
    csVector3& isect, float *pr);
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr);
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }

  //------------------------- iObjectModel implementation ----------------
  class ObjectModel : public iObjectModel
  {
    SCF_DECLARE_EMBEDDED_IBASE (csNullmeshMeshObject);
    virtual long GetShapeNumber () const { return scfParent->shapenr; }
    virtual iPolygonMesh* GetPolygonMesh () { return NULL; }
    virtual iPolygonMesh* GetSmallerPolygonMesh () { return NULL; }
    virtual csPtr<iPolygonMesh> CreateLowerDetailPolygonMesh (float)
    { return NULL; }
    virtual void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL)
    {
      scfParent->GetObjectBoundingBox (bbox, type);
    }
    virtual void GetRadius (csVector3& rad, csVector3& cent)
    {
      scfParent->GetRadius (rad, cent);
    }
    virtual void AddListener (iObjectModelListener* listener)
    {
      scfParent->AddListener (listener);
    }
    virtual void RemoveListener (iObjectModelListener* listener)
    {
      scfParent->RemoveListener (listener);
    }
  } scfiObjectModel;
  friend class ObjectModel;

  virtual iObjectModel* GetObjectModel () { return &scfiObjectModel; }
  virtual bool SetColor (const csColor&) { return false; }
  virtual bool GetColor (csColor&) const { return false; }
  virtual bool SetMaterialWrapper (iMaterialWrapper*) { return false; }
  virtual iMaterialWrapper* GetMaterialWrapper () const { return NULL; }

  //------------------------- iGeneralMeshState implementation ----------------
  class NullMeshState : public iNullMeshState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csNullmeshMeshObject);
    virtual void SetRadius (float radius)
    {
      scfParent->SetRadius (radius);
    }
    virtual float GetRadius () const
    {
      return scfParent->GetRadius ();
    }
  } scfiNullMeshState;
  friend class NullMeshState;

  //------------------ iMeshObjectFactory interface implementation ----------------//
  struct MeshObjectFactory : public iMeshObjectFactory
  {
    SCF_DECLARE_EMBEDDED_IBASE (csNullmeshMeshObject);

    virtual csPtr<iMeshObject> NewInstance ();
    virtual void HardTransform (const csReversibleTransform&) {}
    virtual bool SupportsHardTransform () const { return false; }
    virtual void SetLogicalParent (iBase* lp) { scfParent->logparent = lp; }
    virtual iBase* GetLogicalParent () const { return scfParent->logparent; }
  } scfiMeshObjectFactory;
  friend struct MeshObjectFactory;
};

/**
 * Genmesh type. This is the plugin you have to use to create instances
 * of csNullmeshMeshObjectFactory.
 */
class csNullmeshMeshObjectType : public iMeshObjectType
{
public:
  iObjectRegistry* object_reg;

  SCF_DECLARE_IBASE;

  /// Constructor.
  csNullmeshMeshObjectType (iBase*);
  /// Destructor.
  virtual ~csNullmeshMeshObjectType ();
  /// Draw.
  virtual csPtr<iMeshObjectFactory> NewFactory ();
  /// Initialize.
  bool Initialize (iObjectRegistry* object_reg)
  {
    csNullmeshMeshObjectType::object_reg = object_reg;
    return true;
  }

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csNullmeshMeshObjectType);
    virtual bool Initialize (iObjectRegistry* object_reg)
    {
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;
};

#endif // _NULLMESH_H_

