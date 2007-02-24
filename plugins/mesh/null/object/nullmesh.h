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

#ifndef __CS_NULLMESH_H__
#define __CS_NULLMESH_H__

#include "cstool/objmodel.h"
#include "csgeom/transfrm.h"
#include "csgeom/vector3.h"
#include "csutil/cscolor.h"
#include "csutil/flags.h"
#include "csutil/refarr.h"
#include "imesh/nullmesh.h"
#include "imesh/object.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "ivideo/graph3d.h"

struct iMaterialWrapper;
struct iObjectRegistry;

class csNullmeshMeshObjectType;

class csNullmeshMeshFactory : public scfImplementation2<csNullmeshMeshFactory,
                                                        iMeshObjectFactory,
                                                        iNullFactoryState>
{
public:
  csNullmeshMeshFactory (csNullmeshMeshObjectType* type);
  
  void SetRadius (float radius);
  float GetRadius () const { return radius; }
  void SetBoundingBox (const csBox3& box);
  void GetBoundingBox (csBox3& box)
  {
    box = csNullmeshMeshFactory::box;
  }

  virtual csFlags& GetFlags () { return factory_flags; }
  virtual csPtr<iMeshObject> NewInstance ();
  virtual csPtr<iMeshObjectFactory> Clone () { return 0; }
  virtual void HardTransform (const csReversibleTransform&) {}
  virtual bool SupportsHardTransform () const { return false; }
  virtual void SetMeshFactoryWrapper (iMeshFactoryWrapper* lp)
  { logparent_factory = lp; }
  virtual iMeshFactoryWrapper* GetMeshFactoryWrapper () const
  { return logparent_factory; }
  virtual iMeshObjectType* GetMeshObjectType () const
  { return nullmesh_type; }
  virtual iObjectModel* GetObjectModel () { return 0; }
  virtual bool SetMaterialWrapper (iMaterialWrapper*) { return false; }
  virtual iMaterialWrapper* GetMaterialWrapper () const { return 0; }
  virtual void SetMixMode (uint) { }
  virtual uint GetMixMode () const { return 0; }
  
private:
  iMeshFactoryWrapper* logparent_factory;
  iMeshObjectType* nullmesh_type;
  float radius;
  csBox3 box;
  csFlags factory_flags;
};

/**
 * Nullmesh version of mesh object.
 */
class csNullmeshMeshObject : public scfImplementationExt2<csNullmeshMeshObject,
                                                          csObjectModel,
                                                          iMeshObject,
                                                          iNullMeshState>
{
public:
  /// Constructor.
  csNullmeshMeshObject (csNullmeshMeshFactory* factory, iMeshObjectType* parent);

  /// Destructor.
  virtual ~csNullmeshMeshObject ();

  void GetObjectBoundingBox (csBox3& bbox);
  const csBox3& GetObjectBoundingBox ();
  void SetObjectBoundingBox (const csBox3& bbox);
  void GetRadius (float& rad, csVector3& cent);

  void SetRadius (float radius);
  float GetRadius () const { return radius; }
  void SetBoundingBox (const csBox3& box);
  void GetBoundingBox (csBox3& box)
  {
    box = csNullmeshMeshObject::box;
  }

  //----------------------- iMeshObject implementation ------------------------
  virtual iMeshObjectFactory* GetFactory () const
  {
    return (iMeshObjectFactory*)factory;
  }
  virtual csFlags& GetFlags () { return object_flags; }
  virtual csPtr<iMeshObject> Clone () { return 0; }
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
  virtual void NextFrame (csTicks/*current_time*/, const csVector3&/*pos*/,
    uint /*currentFrame*/) { }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual bool HitBeamOutline (const csVector3& start, const csVector3& end,
    csVector3& isect, float *pr);
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
    csVector3& isect, float* pr, int* polygon_idx = 0,
    iMaterialWrapper** material = 0);
  virtual void SetMeshWrapper (iMeshWrapper* lp) { logparent = lp; }
  virtual iMeshWrapper* GetMeshWrapper () const { return logparent; }

  virtual csRenderMesh **GetRenderMeshes (int &num, iRenderView*, 
    iMovable*, uint32)
  {
    num = 0;
    return 0;
  }
  virtual iObjectModel* GetObjectModel () { return this; }
  virtual bool SetColor (const csColor&) { return false; }
  virtual bool GetColor (csColor&) const { return false; }
  virtual bool SetMaterialWrapper (iMaterialWrapper*) { return false; }
  virtual iMaterialWrapper* GetMaterialWrapper () const { return 0; }
  virtual void SetMixMode (uint) { }
  virtual uint GetMixMode () const { return CS_FX_COPY; }
  virtual void InvalidateMaterialHandles () { }
  /**
  * see imesh/object.h for specification. The default implementation
  * does nothing.
  */
  virtual void PositionChild (iMeshObject* /*child*/, csTicks /*current_time*/) { }


private:
  iMeshObjectFactory* factory;
  iMeshObjectType* nullmesh_type;
  iMeshWrapper* logparent;
  iMeshObjectDrawCallback* vis_cb;
  float radius;
  csBox3 box;
  csFlags object_flags;
};


/**
 * Genmesh type. This is the plugin you have to use to create instances
 * of csNullmeshMeshObjectFactory.
 */
class csNullmeshMeshObjectType : 
  public scfImplementation2<csNullmeshMeshObjectType,
                            iMeshObjectType,
                            iComponent>
{
public:
  iObjectRegistry* object_reg;

  /// Constructor.
  csNullmeshMeshObjectType (iBase*);
  /// Destructor.
  virtual ~csNullmeshMeshObjectType ();
  /// Draw.
  virtual csPtr<iMeshObjectFactory> NewFactory ();
  /// Initialize.
  virtual bool Initialize (iObjectRegistry* object_reg)
  {
    csNullmeshMeshObjectType::object_reg = object_reg;
    return true;
  }
};

#endif // __CS_NULLMESH_H__
