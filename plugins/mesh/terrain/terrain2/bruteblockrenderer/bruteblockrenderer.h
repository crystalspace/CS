/*
  Copyright (C) 2006 by Kapoulkine Arseny
                2007 by Marten Svanfeldt

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

#include "cstool/rendermeshholder.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/scf_implementation.h"
#include "imesh/terrain2.h"
#include "iutil/comp.h"
/*#include "iutil/strset.h"
#include "ivideo/graph3d.h"*/
#include "ivideo/rendermesh.h"
/*#include "ivideo/shader/shader.h"*/

struct iEngine;

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{

struct TerrainCellRData;

class csTerrainBruteBlockRenderer :
  public scfImplementation4<csTerrainBruteBlockRenderer,
                            iTerrainRenderer,
                            iTerrainCellHeightDataCallback,
                            iTerrainCellLoadCallback,
                            iComponent>
{
public:
  // @@@ FIXME: static IDs aren't good...
  static CS::ShaderVarStringID textureLodDistanceID;
  
  csTerrainBruteBlockRenderer (iBase* parent);

  virtual ~csTerrainBruteBlockRenderer ();

  // ------------ iTerrainRenderer implementation ------------
  virtual csPtr<iTerrainCellRenderProperties> CreateProperties ();

  virtual void ConnectTerrain (iTerrainSystem* system);
  virtual void DisconnectTerrain (iTerrainSystem* system);

  virtual csRenderMesh** GetRenderMeshes (int& n, iRenderView* rview,
                                   iMovable* movable, uint32 frustum_mask,
                                   const csArray<iTerrainCell*>& cells);

  virtual void OnMaterialPaletteUpdate (const csTerrainMaterialPalette&
                                        material_palette);  

  virtual void OnMaterialMaskUpdate (iTerrainCell* cell, 
    const csRect& rectangle, const unsigned char* materialMap, size_t pitch);

  virtual void OnMaterialMaskUpdate (iTerrainCell* cell, size_t matIdx,
    const csRect& rectangle, const unsigned char* materialMap, size_t pitch); 

  virtual void OnAlphaMapUpdate (iTerrainCell* cell,
    iMaterialWrapper* material, iImage* alphaMap);

  // ------------ iTerrainCellHeightDataCallback ------------
  virtual void OnHeightUpdate (iTerrainCell* cell, const csRect& rectangle);

  // ------------ iTerrainCellLoadCallback ------------
  virtual void OnCellLoad (iTerrainCell* cell);
  virtual void OnCellPreLoad (iTerrainCell* cell);
  virtual void OnCellUnload (iTerrainCell* cell);

  // ------------ iComponent implementation ------------
  virtual bool Initialize (iObjectRegistry* object_reg); 

  // ------------ Internal helpers ------------
  void SetupCellData (iTerrainCell* cell);

  inline csRenderMeshHolder& GetMeshHolder ()
  {
    return meshHolder;
  }

  inline const csTerrainMaterialPalette& GetMaterialPalette () const
  {
    if (materialPalette)
      return *materialPalette;

    return emptyPalette;
  }

  inline iShaderVarStringSet* GetStringSet ()
  {
    return stringSet;
  }

  // Get index buffer with given resolution and index
  iRenderBuffer* GetIndexBuffer (size_t blockResolution, size_t indexType, uint& numIndices);

  // Allocate the material palette related data in cell
  void SetupCellMMArrays (iTerrainCell* cell);
  
private:
  // Holder for render meshes while rendering
  csDirtyAccessArray<csRenderMesh*> renderMeshCache;
  csRenderMeshHolder meshHolder;

  iObjectRegistry* objectRegistry;
  csRef<iGraphics3D> graph3d;
  csRef<iShaderVarStringSet> stringSet;
  iEngine* engine;

  const csTerrainMaterialPalette* materialPalette;
  csTerrainMaterialPalette emptyPalette;  

  struct IndexBufferSet : public csRefCount
  {
    csRef<iRenderBuffer> meshIndices[16];
    int numIndices[16];
  };
  csRefArray<IndexBufferSet> indexBufferList;

  csRefArray<TerrainCellRData> activeCellList;
};

}
CS_PLUGIN_NAMESPACE_END(Terrain2)

#endif // __CS_TERRAIN_BRUTEBLOCKRENDERER_H__
