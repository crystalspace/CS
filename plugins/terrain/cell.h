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

#ifndef __CS_TERRAIN_CELL_H__
#define __CS_TERRAIN_CELL_H__

#include "csutil/scf_implementation.h"

#include "iterrain/terraincell.h"
#include "iterrain/terraindatafeeder.h"
#include "iterrain/terraincellrenderproperties.h"
#include "iterrain/terraincellcollisionproperties.h"

#include "csgeom/vector2.h"
#include "csgeom/box.h"

#include "csutil/csstring.h"
#include "csutil/dirtyaccessarray.h"

CS_PLUGIN_NAMESPACE_BEGIN(ImprovedTerrain)
{

class csTerrainCell :
  public scfImplementation1<csTerrainCell,
                            iTerrainCell>
{
public:
  enum LoadState {NotLoaded, PreLoaded, Loaded};

private:
  csString name;
  int grid_width, grid_height;
  int material_width, material_height;
  csVector2 position, size;
  csRef<iTerrainDataFeeder> feeder;
  csRef<iTerrainCellRenderProperties> render_properties;
  csRef<iTerrainCellCollisionProperties> collision_properties;

  csDirtyAccessArray<float> heightmap;

  LoadState state;

public:

  csTerrainCell (iBase* parent);

  csTerrainCell (iBase* parent, const char* name, int grid_width, int grid_height, int material_width, int material_height, const csVector2& position, const csVector2& size, iTerrainDataFeeder* feeder, iTerrainCellRenderProperties* render_properties, iTerrainCellCollisionProperties* collision_properties);

  virtual ~csTerrainCell ();

  iTerrainDataFeeder* GetDataFeeder();

  LoadState GetLoadState() const;

  void PreLoad();
  void Load();
  void Unload();

  csBox3 GetBBox();

  // ------------ iTerrainCell implementation ------------

  virtual const char* GetName();
  virtual iTerrainCellRenderProperties* GetRenderProperties();
  virtual iTerrainCellCollisionProperties* GetCollisionProperties();

  virtual int GetGridWidth();
  virtual int GetGridHeight();

  virtual csLockedHeightData LockHeightData(const csRect& rectangle);
  virtual void UnlockHeightData();

  virtual const csVector2& GetPosition();
  virtual const csVector2& GetSize();

  virtual const csArray<iMaterialWrapper*>& GetMaterialPalette();
  virtual void SetMaterialPalette(const csArray<iMaterialWrapper*>& array);

  virtual int GetMaterialMapWidth();
  virtual int GetMaterialMapHeight();
  virtual csLockedMaterialMap LockMaterialMap(const csRect& rectangle);
  virtual void UnlockMaterialMap();

  virtual void SetMaterialMask(int material, iImage* image);
  virtual void SetMaterialMask(int material, const csArray<char>& data, int width, int height);

  virtual bool CollideRay(const csVector3& start, const csVector3& end, bool oneHit, csArray<csVector3>& points);
  virtual bool CollideSegment(const csVector3& start, const csVector3& end, bool oneHit, csArray<csVector3>& points);

  virtual float GetHeight(const csVector2& pos);
  virtual csVector3 GetTangent(const csVector2& pos);
  virtual csVector3 GetBinormal(const csVector2& pos);
  virtual csVector3 GetNormal(const csVector2& pos);
};

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)

#endif // __CS_TERRAIN_CELL_H__
