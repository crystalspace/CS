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

#ifndef __CS_TERRAIN_CELL_H__
#define __CS_TERRAIN_CELL_H__

#include "csutil/scf_implementation.h"
#include "csutil/refcount.h"
#include "csutil/sysfunc.h"
#include "csgeom/vector2.h"
#include "csgeom/box.h"
#include "csutil/cscolor.h"

#include "csutil/csstring.h"
#include "csutil/dirtyaccessarray.h"

#include "imesh/terrain2.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{
class csTerrainSystem;

class csTerrainCell :
  public scfImplementation1<csTerrainCell,
                            iTerrainCell>
{
public:
  csTerrainCell (csTerrainSystem* terrain, const char* name, int gridWidth,
                 int gridHeight, int materialMapWidth, int materialMapHeight,
                 bool materialMapPersistent,
                 const csVector2& position, const csVector3& size,
                 iTerrainCellRenderProperties* render_properties,
                 iTerrainCellCollisionProperties* collision_properties,
                 iTerrainCellFeederProperties* feederProperties);

  virtual ~csTerrainCell ();

  csBox3 GetBBox () const;

  // ------------ iTerrainCell implementation ------------
  virtual LoadState GetLoadState () const;
  virtual void SetLoadState (LoadState state);

  virtual iTerrainSystem* GetTerrain ();

  virtual const char* GetName () const;
  virtual void SetName (const char* name);

  virtual iTerrainCellRenderProperties* GetRenderProperties () const;
  virtual iTerrainCellCollisionProperties* GetCollisionProperties () const;
  virtual iTerrainCellFeederProperties* GetFeederProperties () const;

  virtual int GetGridWidth () const;
  virtual int GetGridHeight () const;

  virtual csLockedHeightData GetHeightData ();
  virtual csLockedHeightData LockHeightData (const csRect& rectangle);
  virtual void UnlockHeightData ();

  virtual csLockedNormalData GetNormalData ();
  virtual csLockedNormalData LockNormalData (const csRect& rectangle);
  virtual void UnlockNormalData ();
  virtual void RecalculateNormalData ();
  virtual csLockedNormalData GetTangentData ();
  virtual csLockedNormalData GetBitangentData ();
  void RecalculateTangentData ();

  virtual const csVector2& GetPosition () const;
  virtual const csVector3& GetSize () const;

  virtual int GetMaterialMapWidth () const;
  virtual int GetMaterialMapHeight () const;
  virtual bool GetMaterialPersistent() const;

  virtual csLockedMaterialMap LockMaterialMap (const csRect& rectangle);
  virtual void UnlockMaterialMap();

  virtual void SetMaterialMask (unsigned int material, iImage* image);
  virtual void SetMaterialMask (unsigned int material, const unsigned char*
    data, unsigned int width, unsigned int height);

  virtual void SetAlphaMask (iMaterialWrapper* material, iImage* alphaMap);

  virtual void SetBaseMaterial (iMaterialWrapper* material);
  virtual iMaterialWrapper* GetBaseMaterial () const;

  virtual void SetAlphaSplatMaterial (iMaterialWrapper* material);
  virtual iMaterialWrapper* GetAlphaSplatMaterial () const;

  virtual void SetSplatBaseMaterial (iMaterialWrapper* material);
  virtual iMaterialWrapper* GetSplatBaseMaterial () const;

  virtual bool CollideSegment (const csVector3& start, const csVector3& end,
    bool oneHit, iTerrainVector3Array* points);
  virtual csTerrainColliderCollideSegmentResult CollideSegment (
      const csVector3& start, const csVector3& end);
  virtual bool CollideSegment (const csVector3& start, const csVector3& end,
			       csVector3& hitPoint);

  virtual bool CollideTriangles (const csVector3* vertices,
    size_t tri_count,
    const unsigned int* indices, float radius,
    const csReversibleTransform& trans,
    bool oneHit, iTerrainCollisionPairArray* pairs);

  virtual bool Collide (iCollider* collider, float radius,
    const csReversibleTransform& trans, bool oneHit,
    iTerrainCollisionPairArray* pairs);

  virtual float GetHeight (int x, int y) const;
  virtual float GetHeight (const csVector2& pos) const;

  inline csVector3 GetTangentDN (int x, int y) const;
  virtual csVector3 GetTangent (int x, int y) const;
  virtual csVector3 GetTangent (const csVector2& pos) const;

  inline csVector3 GetBinormalDN (int x, int y) const;
  virtual csVector3 GetBinormal (int x, int y) const;
  virtual csVector3 GetBinormal (const csVector2& pos) const;

  inline csVector3 GetNormalDN (int x, int y) const;
  virtual csVector3 GetNormal (int x, int y) const;
  virtual csVector3 GetNormal (const csVector2& pos) const;

  virtual csRefCount* GetRenderData () const;
  virtual void SetRenderData (csRefCount* data);

  virtual csRefCount* GetCollisionData () const;
  virtual void SetCollisionData (csRefCount* data);

  virtual csRefCount* GetFeederData () const;
  virtual void SetFeederData (csRefCount* data);

  // unloading
  csTicks GetLRU () const
  {
    return lruTicks;
  }
  void Touch () 
  {
    lruTicks = csGetTicks ();
  }

private:
  csTerrainSystem* terrain;

  csString name;

  int gridWidth, gridHeight;
  int materialMapWidth, materialMapHeight;
  bool materialMapPersistent;
  csVector2 position;
  csVector3 size;
  float step_x, step_z;
  float minHeight, maxHeight;
  csBox3 boundingBox;
  
  csRef<iTerrainCellRenderProperties> renderProperties;
  csRef<iTerrainCellCollisionProperties> collisionProperties;
  csRef<iTerrainCellFeederProperties> feederProperties;
  csRef<iMaterialWrapper> baseMaterial;
  csRef<iMaterialWrapper> splatBaseMaterial;
  csRef<iMaterialWrapper> alphaSplatMaterial;

  // Stored data (if any)
  csDirtyAccessArray<unsigned char> materialmap;
  csDirtyAccessArray<float> heightmap;
  csDirtyAccessArray<csVector3> normalmap;
  bool needTangentsUpdate;
  csDirtyAccessArray<csVector3> tangentmap;
  csDirtyAccessArray<csVector3> bitangentmap;

  LoadState loadState;

  csRect lockedHeightRect;
  csRect lockedMaterialMapRect;

  csRef<csRefCount> renderData, collisionData, feederData;

  void LerpHelper (const csVector2& pos, int& x1, int& x2, float& xfrac,
    int& y1, int& y2, float& yfrac) const;

  csTicks lruTicks;
};

}
CS_PLUGIN_NAMESPACE_END(Terrain2)

#endif // __CS_TERRAIN_CELL_H__
