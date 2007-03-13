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

#ifndef __CS_TERRAIN_SIMPLERENDERER_H__
#define __CS_TERRAIN_SIMPLERENDERER_H__

#include "csgfx/shadervarcontext.h"
#include "cstool/rendermeshholder.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/scf_implementation.h"
#include "imesh/terrain2.h"
#include "iutil/comp.h"
#include "ivideo/graph3d.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{

class csTerrainSimpleCellRenderProperties :
  public scfImplementation2<csTerrainSimpleCellRenderProperties,
                            iTerrainCellRenderProperties,
                            scfFakeInterface<iShaderVariableContext> >
{
public:
  csTerrainSimpleCellRenderProperties ();
  csTerrainSimpleCellRenderProperties (csTerrainSimpleCellRenderProperties& other);

  virtual ~csTerrainSimpleCellRenderProperties ();

  //-- iTerrainCellRenderProperties --
  virtual bool GetVisible () const;
  virtual void SetVisible (bool value);

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
  bool visible;
  csShaderVariableContext svContext;
};

class csTerrainSimpleRenderer :
  public scfImplementation3<csTerrainSimpleRenderer,
                            iTerrainRenderer,
                            iTerrainCellHeightDataCallback,
                            iComponent>
{
  csDirtyAccessArray<csRenderMesh*> meshes;
  csRenderMeshHolder rm_holder;
  csRefArray<iMaterialWrapper> material_palette;

  iObjectRegistry* object_reg;

  csRef<iGraphics3D> g3d;
  csRef<iStringSet> strings;
public:
  csTerrainSimpleRenderer (iBase* parent);

  virtual ~csTerrainSimpleRenderer ();

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

  // -- iTerrainCellHeightDataCallback --
  virtual void OnHeightUpdate (iTerrainCell* cell, const csRect& rectangle);

  // ------------ iComponent implementation ------------
  virtual bool Initialize (iObjectRegistry* object_reg);
};

}
CS_PLUGIN_NAMESPACE_END(Terrain2)

#endif // __CS_TERRAIN_SIMPLERENDERER_H__
