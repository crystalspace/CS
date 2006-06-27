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
#include "csutil/refcount.h"

#include "iterrain/terrainsystem.h"
#include "iterrain/terraincell.h"
#include "iterrain/terraindatafeeder.h"
#include "iterrain/terraincellrenderproperties.h"
#include "iterrain/terraincellcollisionproperties.h"

#include "csgeom/vector2.h"
#include "csgeom/box.h"

#include "csutil/csstring.h"
#include "csutil/dirtyaccessarray.h"

struct iTerrainRenderer;
struct iTerrainCollider;

CS_PLUGIN_NAMESPACE_BEGIN(ImprovedTerrain)
{

class csTerrainCell :
  public scfImplementation1<csTerrainCell,
                            iTerrainCell>
{
public:
  enum LoadState {NotLoaded, PreLoaded, Loaded};

private:
  iTerrainSystem* parent;
  
  csString name;
  int grid_width, grid_height;
  int material_width, material_height;
  csVector2 position;
  csVector3 size;
  csRef<iTerrainDataFeeder> feeder;
  csRef<iTerrainCellRenderProperties> render_properties;
  csRef<iTerrainCellCollisionProperties> collision_properties;
  
  csDirtyAccessArray<unsigned char> materialmap;
  csDirtyAccessArray<float> heightmap;
  
  csRect mm_rect;

  LoadState state;

  csLockedHeightData locked_height_data;
  csRect locked_height_rect;

  csRef<iTerrainRenderer> renderer;
  csRef<iTerrainCollider> collider;

  csRef<csRefCount> render_data, collision_data;

  void LerpHelper (const csVector2& pos, int& x1, int& x2, float& xfrac,
                                    int& y1, int& y2, float& yfrac) const;

public:
  csTerrainCell (iTerrainSystem* parent, const char* name, int grid_width,
                 int grid_height, int material_width, int material_height,
                 const csVector2& position, const csVector3& size,
                 iTerrainDataFeeder* feeder,
                 iTerrainCellRenderProperties* render_properties,
                 iTerrainCellCollisionProperties* collision_properties,
                 iTerrainRenderer* renderer, iTerrainCollider* collider);

  virtual ~csTerrainCell ();

  iTerrainDataFeeder* GetDataFeeder () const;

  LoadState GetLoadState () const;

  void PreLoad ();
  void Load ();
  void Unload ();

  csBox3 GetBBox () const;

  // ------------ iTerrainCell implementation ------------

  virtual const char* GetName () const;
  virtual iTerrainCellRenderProperties* GetRenderProperties () const;
  virtual iTerrainCellCollisionProperties* GetCollisionProperties () const;

  virtual csRefCount* GetRenderData () const;
  virtual void SetRenderData (csRefCount* data);

  virtual csRefCount* GetCollisionData () const;
  virtual void SetCollisionData (csRefCount* data);

  virtual int GetGridWidth () const;
  virtual int GetGridHeight () const;

  virtual csLockedHeightData LockHeightData (const csRect& rectangle);
  virtual void UnlockHeightData ();

  virtual const csVector2& GetPosition () const;
  virtual const csVector3& GetSize () const;

  virtual int GetMaterialMapWidth () const;
  virtual int GetMaterialMapHeight () const;
  virtual csLockedMaterialMap LockMaterialMap (const csRect& rectangle);
  virtual void UnlockMaterialMap ();

  virtual void SetMaterialMask (unsigned int material, iImage* image);
  virtual void SetMaterialMask (unsigned int material, const unsigned char*
                             data, unsigned int width, unsigned int height);

  virtual bool CollideRay (const csVector3& start, const csVector3& end, 
                           bool oneHit, csArray<csVector3>& points) const;
  virtual bool CollideSegment (const csVector3& start, const csVector3& end,
                           bool oneHit, csArray<csVector3>& points) const;

  virtual float GetHeight (int x, int y) const;
  virtual float GetHeight (const csVector2& pos) const;
  
  virtual csVector3 GetTangent (int x, int y) const;
  virtual csVector3 GetTangent (const csVector2& pos) const;

  virtual csVector3 GetBinormal (int x, int y) const;
  virtual csVector3 GetBinormal (const csVector2& pos) const;

  virtual csVector3 GetNormal (int x, int y) const;
  virtual csVector3 GetNormal (const csVector2& pos) const;
};

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)

#endif // __CS_TERRAIN_CELL_H__
