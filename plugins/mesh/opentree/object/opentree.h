/*
    Copyright (C) 2006 by Christoph "Fossi" Mewes

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

#ifndef __CS_OPENTREE_H__
#define __CS_OPENTREE_H__

#include "csgeom/vector3.h"
#include "cstool/objmodel.h"
#include "csutil/leakguard.h"
#include "csutil/weakref.h"
#include "cstool/rendermeshholder.h"
#include "imesh/objmodel.h"
#include "iutil/comp.h"
#include "imesh/protomesh.h"
#include "imesh/object.h"
#include "ivideo/graph3d.h"
#include "csutil/flags.h"
#include "csutil/cscolor.h"
#include "imesh/genmesh.h"

#include "imesh/object.h"
#include "imesh/opentree.h"

#include <opentree/opentree.h>

CS_PLUGIN_NAMESPACE_BEGIN(OpenTree)
{

class csOpenTreeObjectFactory;

class csOpenTreeObject :
  public scfImplementation2<csOpenTreeObject, 
                            iMeshObject,
                            iOpenTreeState>
{
private:

  //iMeshObject variables
  uint MixMode;
  csFlags flags;
  csRef<iMeshObjectDrawCallback> vis_cb;
  iMeshWrapper* logparent;
  csRef<iMaterialWrapper> material;
  csRef<csOpenTreeObjectFactory> factory;

  //use genmesh internally for now
  csRef<iMeshWrapper> treemesh;
  csRef<iGeneralMeshState> treemeshstate;
  csRef<iMeshWrapper> leafmesh;
  csRef<iGeneralMeshState> leafmeshstate;

public:
  /// Constructor.
  csOpenTreeObject (csOpenTreeObjectFactory* factory);

  /// Destructor.
  virtual ~csOpenTreeObject ();

  /**\name iOpenTreeState implementation
   * @{ */
  virtual bool SetMaterialWrapper (char level, iMaterialWrapper* mat);
  virtual iMaterialWrapper* GetMaterialWrapper (char level);
  /** @} */

  /**\name iMeshObject implementation
   * @{ */
  void SetMixMode (uint mode) { MixMode = mode; }
  uint GetMixMode () const { return MixMode; }


  virtual iMeshObjectFactory* GetFactory () const;
  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> Clone ()
  {
    // We don't support making clones.
    return 0;
  }
  virtual csRenderMesh** GetRenderMeshes (int &n, iRenderView* rview,
    iMovable* movable, uint32 frustum_mask);
  virtual void SetVisibleCallback (iMeshObjectDrawCallback* cb)
  {
    vis_cb = cb;
  }
  virtual iMeshObjectDrawCallback* GetVisibleCallback () const
  {
    return vis_cb;
  }
  virtual void NextFrame (csTicks, const csVector3&, uint)
  {
    // We don't support animation.
  }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const
  {
    // We don't support hard transform.
    return false;
  }
  virtual bool HitBeamOutline (const csVector3& start, const csVector3& end,
    csVector3& isect, float *pr);
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
    csVector3& isect, float* pr, int* polygon_idx = 0,
    iMaterialWrapper** material = 0);
  virtual void SetMeshWrapper (iMeshWrapper* lp)
  {
    logparent = lp;
    CS_ASSERT (logparent != 0);
  }
  virtual iMeshWrapper* GetMeshWrapper () const { return logparent; }

  virtual iObjectModel* GetObjectModel ();
  virtual bool SetColor (const csColor& col)
  {
    //we don't support colors
    return true;
  }
  virtual bool GetColor (csColor& col) const { return false; }
  virtual bool SetMaterialWrapper (iMaterialWrapper* mat);
  virtual iMaterialWrapper* GetMaterialWrapper () const
  { return material; }
  virtual void InvalidateMaterialHandles ()
  {
    // We visit our material all the time so this is not needed here.
  }
  virtual void PositionChild (iMeshObject* /*child*/, csTicks /*current_time*/)
  {  // We don't support sockets.
  }
  /** @} */

};

/**
 * Factory for OpenTree meshes.
 */
class csOpenTreeObjectFactory : 
  public scfImplementationExt2<csOpenTreeObjectFactory, 
                               csObjectModel,
                              iMeshObjectFactory,
                              iOpenTreeFactoryState>
{
private:
  iObjectRegistry* object_reg;
  iMeshFactoryWrapper* logparent;
  csFlags flags;
  csRef<iMeshObjectType> proto_type;

  //flag if mesh was generated
  bool generated;

  //data for this tree
  opentree::TreeData treedata;

  //use genmesh internally for now
  csRef<iMeshFactoryWrapper> treefact;
  csRef<iGeneralFactoryState> treefactstate;
  csRef<iMeshFactoryWrapper> leaffact;
  csRef<iGeneralFactoryState> leaffactstate;

#if 0
#define CS_TOKEN_ITEM_FILE \
  "plugins/mesh/opentree/opentree_factory.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE
#endif

  //cached csStringIDs
  csStringID string_ratiopower;
  csStringID string_lobedepth;
  csStringID string_basesize;
  csStringID string_attractionup;
  csStringID string_leafscalex;
  csStringID string_scale;
  csStringID string_scalev;
  csStringID string_ratio;
  csStringID string_leafquality;
  csStringID string_flare;
  csStringID string_leafscale;
  csStringID string_leafbend;
  csStringID string_leafdistrib;
  csStringID string_prunewidth;
  csStringID string_prunewidthpeak;
  csStringID string_pruneratio;
  csStringID string_prunepowerhigh;
  csStringID string_prunepowerlow;
  csStringID string_leaves;
  csStringID string_shape;
  csStringID string_lobes;
  csStringID string_levels;
  csStringID string_levelnumber;
  csStringID string_basesplits;
  csStringID string_branchdist;
  csStringID string_downangle;
  csStringID string_downanglev;
  csStringID string_rotate;
  csStringID string_rotatev;
  csStringID string_length;
  csStringID string_lengthv;
  csStringID string_taper;
  csStringID string_segsplits;
  csStringID string_splitangle;
  csStringID string_splitanglev;
  csStringID string_curve;
  csStringID string_curveback;
  csStringID string_curvev;
  csStringID string_branches;
  csStringID string_curveres;

  friend class csOpenTreeObject;

public:
  /// Constructor.
  csOpenTreeObjectFactory (iMeshObjectType *pParent,
        iObjectRegistry* object_reg);

  /// Destructor.
  virtual ~csOpenTreeObjectFactory ();

  /**\name iOpenTreeFactoryState implementation
   * @{ */
  bool SetParam (char, csStringID, float);
  bool SetParam (char, csStringID, int);
  bool SetParam (char, csStringID, const char*);
  void GenerateTree ();
  /** @} */

  /**\name iMeshObjectFactory implementation
   * @{ */
  virtual csFlags& GetFlags () { return flags; }
  virtual csPtr<iMeshObject> NewInstance ();
  virtual csPtr<iMeshObjectFactory> Clone () { return 0; }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual void SetMeshFactoryWrapper (iMeshFactoryWrapper* lp)
  {
    logparent = lp;
    CS_ASSERT (logparent != 0);
  }
  virtual iMeshFactoryWrapper* GetMeshFactoryWrapper () const
  { return logparent; }
  virtual iMeshObjectType* GetMeshObjectType () const { return proto_type; }
  virtual bool SetMaterialWrapper (iMaterialWrapper*) { return false; }
  virtual iMaterialWrapper* GetMaterialWrapper () const { return 0; }
  virtual void SetMixMode (uint) { }
  virtual uint GetMixMode () const { return 0; }  
  /** @} */

  //iObjectModel
  void GetObjectBoundingBox (csBox3& bbox);
  void SetObjectBoundingBox (const csBox3& b);
  void GetRadius (float& radius, csVector3& center);
  virtual iPolygonMesh* GetPolygonMeshColldet () { return 0; }
  virtual iTerraFormer* GetTerraFormerColldet () { return 0; }
  virtual iObjectModel* GetObjectModel () { return this; }
};

class csOpenTreeObjectType : 
  public scfImplementation2<csOpenTreeObjectType, 
                            iMeshObjectType,
                            iComponent>
{
public:
  iObjectRegistry* object_reg;

  /// Constructor.
  csOpenTreeObjectType (iBase*);
  /// Destructor.
  virtual ~csOpenTreeObjectType ();
  /// Draw.
  virtual csPtr<iMeshObjectFactory> NewFactory ();
  /// Initialize.
  bool Initialize (iObjectRegistry* object_reg);
};

}
CS_PLUGIN_NAMESPACE_END(OpenTree)

#endif // __CS_OPENTREE_H__

