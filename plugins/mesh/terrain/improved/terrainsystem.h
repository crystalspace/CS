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

#include "csutil/scf_implementation.h"

#include "cstool/objmodel.h"

#include "iterrain/terrainsystem.h"
#include "iterrain/terrainrenderer.h"
#include "iterrain/terraincollider.h"

#include "imesh/object.h"

#include "iengine/material.h"

#include "cell.h"

#include "csutil/refarr.h"
#include "csutil/flags.h"

#include "csgeom/box.h"

#include "csutil/refarr.h"

CS_PLUGIN_NAMESPACE_BEGIN(ImprovedTerrain)
{

class csTerrainSystem :
  public scfImplementationExt2<csTerrainSystem,
                            csObjectModel,
                            iTerrainSystem,
                            iMeshObject>
{
  csRef<iMeshObjectFactory> factory;

  csRef<iTerrainRenderer> renderer;
  csRef<iTerrainCollider> collider;

  csRefArray<csTerrainCell> cells;

  float vview_distance;
  bool auto_preload;

  csFlags imo_flags;
  csRef<iMeshObjectDrawCallback> imo_viscb;
  iMeshWrapper* logparent;

  csDirtyAccessArray<iTerrainCell*> needed_cells;

  csRefArray<iMaterialWrapper> material_palette;

  csBox3 bbox;
  bool bbox_valid;

  csArray<csVector3> collision_result;

  void ComputeBBox();

public:
  csTerrainSystem (iMeshObjectFactory* factory = 0);

  virtual ~csTerrainSystem ();

  void AddCell (csTerrainCell* cell);

  void SetRenderer (iTerrainRenderer* renderer);
  void SetCollider (iTerrainCollider* collider);

  // ------------ iTerrainSystem implementation ------------

  virtual iTerrainCell* GetCell (const char* name);
  virtual iTerrainCell* GetCell (const csVector2& pos);

  virtual const csRefArray<iMaterialWrapper>& GetMaterialPalette () const;
  virtual void SetMaterialPalette (const csRefArray<iMaterialWrapper>& array);

  virtual bool CollideSegment (const csVector3& start, const csVector3& end,
                           bool oneHit, csArray<csVector3>& points);

  virtual float GetVirtualViewDistance () const;
  virtual void SetVirtualViewDistance (float distance);

  virtual bool GetAutoPreLoad () const;
  virtual void SetAutoPreLoad (bool mode);
  virtual void PreLoadCells (iRenderView* rview, iMovable* movable);
  
  virtual float GetHeight (const csVector2& pos);
  virtual csVector3 GetTangent (const csVector2& pos);
  virtual csVector3 GetBinormal (const csVector2& pos);
  virtual csVector3 GetNormal (const csVector2& pos);
  
  // ------------ iMeshObject implementation ------------

  virtual iMeshObjectFactory* GetFactory () const;

  virtual csFlags& GetFlags ();

  virtual csPtr<iMeshObject> Clone ();

  virtual csRenderMesh** GetRenderMeshes (int& num, iRenderView* rview, 
    iMovable* movable, uint32 frustum_mask);

  virtual void SetVisibleCallback (iMeshObjectDrawCallback* cb);

  virtual iMeshObjectDrawCallback* GetVisibleCallback () const;

  virtual void NextFrame (csTicks current_time,const csVector3& pos,
    uint currentFrame);

  virtual void HardTransform (const csReversibleTransform& t);

  virtual bool SupportsHardTransform () const;

  virtual bool HitBeamOutline (const csVector3& start,
        const csVector3& end, csVector3& isect, float* pr);

  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
        csVector3& isect, float* pr, int* polygon_idx = 0,
        iMaterialWrapper** material = 0);

  virtual void SetMeshWrapper (iMeshWrapper* lp);

  virtual iMeshWrapper* GetMeshWrapper () const;

  virtual iObjectModel* GetObjectModel ();

  virtual bool SetColor (const csColor& color);

  virtual bool GetColor (csColor& color) const;

  virtual bool SetMaterialWrapper (iMaterialWrapper* material);

  virtual iMaterialWrapper* GetMaterialWrapper () const;

  virtual void SetMixMode (uint mode);
  virtual uint GetMixMode () const;

  virtual void InvalidateMaterialHandles ();

  virtual void PositionChild (iMeshObject* child,csTicks current_time);

  // ------------ iObjectModel implementation ------------
  virtual void GetObjectBoundingBox (csBox3& box);
  virtual void SetObjectBoundingBox (const csBox3& box);

  virtual void GetRadius (float& radius, csVector3& center);
};

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)

#endif // __CS_TERRAIN_TERRAINSYSTEM_H__
