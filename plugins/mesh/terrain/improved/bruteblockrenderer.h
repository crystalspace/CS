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

#ifndef __CS_TERRAIN_BRUTEBLOCKRENDERER_H__
#define __CS_TERRAIN_BRUTEBLOCKRENDERER_H__

#include "csutil/scf_implementation.h"

#include "iterrain/terrainrenderer.h"
#include "iterrain/terraincellrenderproperties.h"

#include "cstool/rendermeshholder.h"

#include "csutil/dirtyaccessarray.h"

#include "iutil/comp.h"

#include "ivideo/graph3d.h"

#include "iutil/strset.h"

CS_PLUGIN_NAMESPACE_BEGIN(ImprovedTerrain)
{

struct csBruteBlockTerrainRenderData;

class csTerrainBruteBlockCellRenderProperties :
  public scfImplementation1<csTerrainBruteBlockCellRenderProperties,
                          iTerrainCellRenderProperties>
{
public:
  csTerrainBruteBlockCellRenderProperties ();

  virtual ~csTerrainBruteBlockCellRenderProperties ();

  virtual bool GetVisible () const
  {
    return visible;
  }

  virtual void SetVisible (bool value)
  {
    visible = value;
  }

  int GetBlockResolution() const 
  {
    return block_res;
  }
  void SetBlockResolution(int value)
  {
    block_res = 1 << csLog2 (value);
  }
  
  float GetLODLCoeff() const 
  {
    return lod_lcoeff;
  }
  void SetLODLCoeff(float value) 
  {
    lod_lcoeff = value;
  }

  virtual void SetParam (const char* name, const char* value);

private:
  // Per cell properties
  bool visible;

  // Block resolution in "gaps".. should be 2^n
  int block_res;

  // Lod splitting coefficient
  float lod_lcoeff;
};

class csTerrainBruteBlockRenderer :
  public scfImplementation2<csTerrainBruteBlockRenderer,
                            iTerrainRenderer,
                            iComponent>
{
public:
  csTerrainBruteBlockRenderer (iBase* parent);

  virtual ~csTerrainBruteBlockRenderer ();

  // ------------ iTerrainRenderer implementation ------------
  virtual csPtr<iTerrainCellRenderProperties> CreateProperties ();

  virtual csRenderMesh** GetRenderMeshes (int& n, iRenderView* rview,
                                   iMovable* movable, uint32 frustum_mask,
                                   iTerrainCell** cells, int cell_count);

  virtual void OnMaterialPaletteUpdate (const csRefArray<iMaterialWrapper>&
                                        material_palette);
  virtual void OnHeightUpdate (iTerrainCell* cell, const csRect& rectangle,
                               const float* data, unsigned int pitch);
  virtual void OnMaterialMaskUpdate (iTerrainCell* cell, unsigned int material,
                               const csRect& rectangle, const unsigned char*
                               data, unsigned int pitch);
  virtual void OnColorUpdate (iTerrainCell* cell, const csColor* data,
                               unsigned int res);
  
  // ------------ iComponent implementation ------------
  virtual bool Initialize (iObjectRegistry* object_reg);

  // Get index buffer with given resolution and index
  iRenderBuffer* GetIndexBuffer (int block_res_log2, int index, int& numIndices);

private:
  csRef<csBruteBlockTerrainRenderData> SetupCellRenderData (iTerrainCell* cell);

  csDirtyAccessArray<csRenderMesh*> meshes;
  csRenderMeshHolder rm_holder;

  csRefArray<iMaterialWrapper> material_palette;
  iObjectRegistry* object_reg;

  csRef<iGraphics3D> g3d;
  csRef<iStringSet> strings;

  struct IndexBufferSet : public csRefCount
  {
    csRef<iRenderBuffer> mesh_indices[16];
    int numindices[16];
  };
  csRefArray<IndexBufferSet> indexBufferList;
};

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)

#endif // __CS_TERRAIN_BRUTEBLOCKRENDERER_H__
