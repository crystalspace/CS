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

#ifndef __CS_TERRAIN_FACTORY_H__
#define __CS_TERRAIN_FACTORY_H__

#include "csutil/csstring.h"
#include "csutil/flags.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"

#include "imesh/object.h"
#include "imesh/terrain2.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{
class csTerrainSystem;
class csTerrainCell;

class csTerrainFactoryCell :
  public scfImplementation1<csTerrainFactoryCell,
                            iTerrainFactoryCell>
{
public:
  csTerrainFactoryCell (const char* name, 
    int gridWidth, int gridHeight, int materialMapWidth,
    int materialMapHeight, bool materialMapPersistent,
    const csVector2& position, const csVector3& size,
    iTerrainCellRenderProperties* renderProp,
    iTerrainCellCollisionProperties* collisionProp,
    iTerrainCellFeederProperties* feederProp);

  csPtr<csTerrainCell> CreateCell (csTerrainSystem* terrain);

  // ------------ iTerrainFactoryCell implementation ------------
  virtual iTerrainCellRenderProperties* GetRenderProperties () const;
  virtual iTerrainCellCollisionProperties* GetCollisionProperties () const;
  virtual iTerrainCellFeederProperties* GetFeederProperties () const;

  virtual void SetBaseMaterial (iMaterialWrapper* material);

private:
  csString name;
  csVector2 position;
  csVector3 size;
  int gridWidth, gridHeight, materialMapWidth, materialMapHeight;
  bool materialMapPersistent;
  csRef<iMaterialWrapper> baseMaterial;

  csRef<iTerrainCellRenderProperties> rendererProperties;
  csRef<iTerrainCellCollisionProperties> colliderProperties;
  csRef<iTerrainCellFeederProperties> feederProperties;
};

class csTerrainFactory :
  public scfImplementation2<csTerrainFactory,
                            iMeshObjectFactory,
                            iTerrainFactory>
{
public:
  csTerrainFactory (iMeshObjectType *pParent);

  virtual ~csTerrainFactory ();

  // ------------ iMeshObjectFactory implementation ------------

  virtual csFlags& GetFlags ();
  virtual csPtr<iMeshObject> NewInstance ();
  virtual csPtr<iMeshObjectFactory> Clone ();
  virtual void HardTransform (const csReversibleTransform& t);
  virtual bool SupportsHardTransform () const;
  virtual void SetMeshFactoryWrapper (iMeshFactoryWrapper* lp);
  virtual iMeshFactoryWrapper* GetMeshFactoryWrapper () const;
  virtual iMeshObjectType* GetMeshObjectType () const;
  virtual iObjectModel* GetObjectModel ();
  virtual bool SetMaterialWrapper (iMaterialWrapper* material);
  virtual iMaterialWrapper* GetMaterialWrapper () const;

  virtual void SetMixMode (uint mode);
  virtual uint GetMixMode () const;

  // ------------ iTerrainFactory implementation ------------
  virtual void SetRenderer (iTerrainRenderer* renderer);
  virtual void SetCollider (iTerrainCollider* collider);
  virtual void SetFeeder (iTerrainDataFeeder* feeder);

  virtual iTerrainFactoryCell* AddCell (const char* name, 
    int gridWidth, int gridHeight, int materialMapWidth,
    int materialMapHeight, bool materialMapPersistent,
    const csVector2& position, const csVector3& size);

  virtual void SetMaxLoadedCells (size_t value);
  virtual void SetVirtualViewDistance (float distance);
  virtual void SetAutoPreLoad (bool mode);

private:
  csRef<iTerrainRenderer> renderer;
  csRef<iTerrainCollider> collider;
  csRef<iTerrainDataFeeder> dataFeeder;
  csRef<iMeshObjectType> type;

  csRefArray<csTerrainFactoryCell> cells;

  iMeshFactoryWrapper* logParent;
  csFlags flags;
  size_t maxLoadedCells;
  float virtualViewDistance;
  bool autoPreLoad;
};

}

CS_PLUGIN_NAMESPACE_END(Terrain2)

#endif // __CS_TERRAIN_FACTORY_H__
