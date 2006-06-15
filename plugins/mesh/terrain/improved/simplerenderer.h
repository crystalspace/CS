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

#ifndef __CS_TERRAIN_SIMPLERENDERER_H__
#define __CS_TERRAIN_SIMPLERENDERER_H__

#include "csutil/scf_implementation.h"

#include "iterrain/terrainrenderer.h"
#include "iterrain/terraincellrenderproperties.h"

#include "cstool/rendermeshholder.h"

#include "csutil/dirtyaccessarray.h"

#include "iutil/comp.h"

CS_PLUGIN_NAMESPACE_BEGIN(ImprovedTerrain)
{

class csTerrainSimpleCellRenderProperties :
  public scfImplementation1<csTerrainSimpleCellRenderProperties,
                          iTerrainCellRenderProperties>
{
private:
  bool visible;

public:
  csTerrainSimpleCellRenderProperties (iBase* parent);

  virtual ~csTerrainSimpleCellRenderProperties ();

  virtual bool GetVisible() const;
  virtual void SetVisible(bool value);
};

class csTerrainSimpleRenderer :
  public scfImplementation2<csTerrainSimpleRenderer,
                            iTerrainRenderer,
                            iComponent>
{
  csDirtyAccessArray<csRenderMesh*> meshes;
  
  csRenderMeshHolder rm_holder;

  csRefArray<iMaterialWrapper> material_palette;

  iObjectRegistry* object_reg;
public:
  csTerrainSimpleRenderer (iBase* parent);

  virtual ~csTerrainSimpleRenderer ();

  // ------------ iTerrainRenderer implementation ------------

  virtual csPtr<iTerrainCellRenderProperties> CreateProperties();

  virtual csRenderMesh** GetRenderMeshes(int& n, iRenderView* rview, iMovable* movable, uint32 frustum_mask, iTerrainCell** cells, int cell_count);
  
  virtual void OnMaterialPaletteUpdate(const csRefArray<iMaterialWrapper>& material_palette);
  virtual void OnHeightUpdate(iTerrainCell* cell, const csRect& rectangle, const float* data, unsigned int pitch);
  virtual void OnMaterialMaskUpdate(iTerrainCell* cell, unsigned int material, const csRect& rectangle, const unsigned char* data, unsigned int pitch);
  
  // ------------ iComponent implementation ------------
  virtual bool Initialize (iObjectRegistry* object_reg);
};

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)

#endif // __CS_TERRAIN_SIMPLERENDERER_H__
