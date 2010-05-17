/*
    Copyright (C) 2006 by Kapoulkine Arseny

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

#ifndef __CS_TERRAIN_TERRAINSYSTEM_H__
#define __CS_TERRAIN_TERRAINSYSTEM_H__

#include "csgeom/box.h"
#include "cstool/meshobjtmpl.h"
#include "cstool/objmodel.h"
#include "csutil/flags.h"
#include "csutil/list.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"
#include "csutil/scfarray.h"
#include "csutil/set.h"

#include "iengine/engine.h"
#include "iengine/lightmgr.h"
#include "iengine/material.h"
#include "imesh/object.h"
#include "imesh/terrain2.h"
#include "iutil/comp.h"
#include "ivaria/collider.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{
class csTerrainCell;

class csTerrainSystem :
  public scfImplementationExt2<csTerrainSystem,
                               csMeshObject,
                               iTerrainSystem,
                               iCollider>
{
public:
  csTerrainSystem (iMeshObjectFactory* factory,
    iTerrainRenderer* renderer, iTerrainCollider* collider, 
    iTerrainDataFeeder* feeder);

  virtual ~csTerrainSystem ();

  iTerrainCell* AddCell (iTerrainFactoryCell*);

  virtual void RemoveCell (iTerrainCell*);

  void AddCell (csTerrainCell* cell);

  void FireLoadCallbacks (csTerrainCell* cell);
  void FirePreLoadCallbacks (csTerrainCell* cell);
  void FireUnloadCallbacks (csTerrainCell* cell);
  void FireHeightUpdateCallbacks (csTerrainCell* cell, const csRect& rectangle);

  iTerrainRenderer* GetRenderer () const
  {
    return renderer;
  }

  iTerrainCollider* GetCollider () const
  {
    return collider;
  }

  iTerrainDataFeeder* GetFeeder () const
  {
    return dataFeeder;
  }

  void CellSizeUpdate (csTerrainCell* cell);

  // ------------ iTerrainSystem implementation ------------
  virtual iTerrainCell* GetCell (const char* name, bool loadData = false);
  virtual iTerrainCell* GetCell (const csVector2& pos, bool loadData = false);
  virtual iTerrainCell* GetCell (size_t index, bool loadData = false);

  virtual size_t GetCellCount () const;

  virtual const csRefArray<iMaterialWrapper>& GetMaterialPalette () const;
  virtual void SetMaterialPalette (const csRefArray<iMaterialWrapper>& array);

  virtual bool CollideSegment (const csVector3& start, const csVector3& end,
    bool oneHit, iTerrainVector3Array* points, iMaterialArray* materials);
  virtual csTerrainColliderCollideSegmentResult CollideSegment (
      const csVector3& start, const csVector3& end, bool use_ray);

  virtual bool CollideTriangles (const csVector3* vertices,
    size_t tri_count,
    const unsigned int* indices, float radius,
    const csReversibleTransform& trans,
    bool oneHit, iTerrainCollisionPairArray* pairs);

  virtual bool Collide (iCollider* collider, float radius,
    const csReversibleTransform& trans, bool oneHit,
    iTerrainCollisionPairArray* pairs);

  virtual float GetVirtualViewDistance () const;
  virtual void SetVirtualViewDistance (float distance);

  virtual bool GetAutoPreLoad () const;
  virtual void SetAutoPreLoad (bool mode);

  virtual void PreLoadCells (iRenderView* rview, iMovable* movable);

  virtual float GetHeight (const csVector2& pos);
  virtual csVector3 GetTangent (const csVector2& pos);
  virtual csVector3 GetBinormal (const csVector2& pos);
  virtual csVector3 GetNormal (const csVector2& pos);

  virtual size_t GetMaxLoadedCells () const;
  virtual void SetMaxLoadedCells (size_t value);

  virtual void UnloadOldCells ();

  virtual void AddCellLoadListener (iTerrainCellLoadCallback* cb);
  virtual void RemoveCellLoadListener (iTerrainCellLoadCallback* cb);

  virtual void AddCellHeightUpdateListener (iTerrainCellHeightDataCallback* cb);
  virtual void RemoveCellHeightUpdateListener (
      iTerrainCellHeightDataCallback* cb);  

  // ------------ iMeshObject implementation ------------

  virtual iMeshObjectFactory* GetFactory () const;
  virtual CS::Graphics::RenderMesh** GetRenderMeshes (
      int& num, iRenderView* rview, iMovable* movable,
      uint32 frustum_mask);

  virtual bool HitBeamOutline (const csVector3& start,
        const csVector3& end, csVector3& isect, float* pr);

  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
        csVector3& isect, float* pr, int* polygon_idx,
        iMaterialWrapper** material, iMaterialArray* materials);

  // ------------ iObjectModel implementation ------------
  virtual iTerrainSystem* GetTerrainColldet () { return this; }
  
  // ------------ iCollider implementation ------------
  virtual csColliderType GetColliderType ();
  
private:
  csRef<iMeshObjectFactory> factory;

  csRef<iTerrainRenderer> renderer;
  csRef<iTerrainCollider> collider;
  csRef<iTerrainDataFeeder> dataFeeder;

  csRefArray<csTerrainCell> cells;

  csRefArray<iMaterialWrapper> materialPalette;

  csRefArray<iTerrainCellLoadCallback> loadCallbacks;
  csRefArray<iTerrainCellHeightDataCallback> heightDataCallbacks;

  float virtualViewDistance;
  size_t maxLoadedCells;
  bool autoPreload, bbStarted;

  void ComputeBBox();

  bool HitBeamOutline (const csVector3& start,
    const csVector3& end, csVector3& isect, float* pr,
    iMaterialArray* materials);
};

}
CS_PLUGIN_NAMESPACE_END(Terrain2)

#endif // __CS_TERRAIN_TERRAINSYSTEM_H__
