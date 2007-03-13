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
#include "iutil/strset.h"
#include "ivideo/graph3d.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{

struct csBruteBlockTerrainRenderData;

class csTerrainBruteBlockCellRenderProperties :
  public scfImplementation2<csTerrainBruteBlockCellRenderProperties,
                            iTerrainCellRenderProperties,
                            scfFakeInterface<iShaderVariableContext> >
{
public:
  csTerrainBruteBlockCellRenderProperties ();
  csTerrainBruteBlockCellRenderProperties (csTerrainBruteBlockCellRenderProperties& other);

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

  virtual void SetParameter (const char* name, const char* value);
  virtual csPtr<iTerrainCellRenderProperties> Clone ();

  //-- iShaderVariableContext --
  virtual void AddVariable (csShaderVariable *variable)
  {
    svContext.AddVariable (variable);
  }
  virtual csShaderVariable* GetVariable (csStringID name) const
  {
    return svContext.GetVariable (name);
  }

  virtual const csRefArray<csShaderVariable>& GetShaderVariables () const
  {
    return svContext.GetShaderVariables ();
  }

  virtual void PushVariables (iShaderVarStack* stacks) const
  {
    svContext.PushVariables (stacks);
  }

  virtual bool IsEmpty () const
  {
    return svContext.IsEmpty ();
  }

  virtual void ReplaceVariable (csShaderVariable* variable)
  {
    svContext.ReplaceVariable (variable);
  }

  virtual void Clear()
  {
    svContext.Clear ();
  }

  virtual bool RemoveVariable (csShaderVariable* variable) 
  {
    return svContext.RemoveVariable (variable);
  }


private:
  // Per cell properties
  bool visible;

  // Block resolution in "gaps".. should be 2^n
  int block_res;

  // Lod splitting coefficient
  float lod_lcoeff;

  //@@TODO! Better handling of SVs
  csShaderVariableContext svContext;
};

class csTerrainBruteBlockRenderer :
  public scfImplementation3<csTerrainBruteBlockRenderer,
                            iTerrainRenderer,
                            iTerrainCellHeightDataCallback,
                            iComponent>
{
public:
  csTerrainBruteBlockRenderer (iBase* parent);

  virtual ~csTerrainBruteBlockRenderer ();

  // ------------ iTerrainRenderer implementation ------------
  virtual csPtr<iTerrainCellRenderProperties> CreateProperties ();

  virtual void ConnectTerrain (iTerrainSystem* system);
  virtual void DisconnectTerrain (iTerrainSystem* system);

  virtual csRenderMesh** GetRenderMeshes (int& n, iRenderView* rview,
                                   iMovable* movable, uint32 frustum_mask,
                                   const csArray<iTerrainCell*> cells);

  virtual void OnMaterialPaletteUpdate (const csRefArray<iMaterialWrapper>&
                                        material_palette);  
  virtual void OnMaterialMaskUpdate (iTerrainCell* cell, unsigned int material,
                               const csRect& rectangle, const unsigned char*
                               data, unsigned int pitch);
  
  // ------------ iTerrainCellHeightDataCallback ------------
  virtual void OnHeightUpdate (iTerrainCell* cell, const csRect& rectangle);

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

  friend struct csBruteBlockTerrainRenderData;
};

}
CS_PLUGIN_NAMESPACE_END(Terrain2)

#endif // __CS_TERRAIN_BRUTEBLOCKRENDERER_H__
