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

#include "cssysdef.h"

#include "factory.h"
#include "cell.h"
#include "terrainsystem.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{

csTerrainFactory::csTerrainFactory (iMeshObjectType *pParent)
  : scfImplementationType (this), type (pParent),
  defaultCell (0, 128, 128, 128, 128, false, csVector2 (0, 0),
    csVector3 (128, 32, 128), 0, 0, 0), logParent (0),
  maxLoadedCells (~0), virtualViewDistance (2.0f),
  autoPreLoad (false)
{
}

csTerrainFactory::~csTerrainFactory ()
{
}

csFlags& csTerrainFactory::GetFlags ()
{
  return flags;
}

csPtr<iMeshObject> csTerrainFactory::NewInstance ()
{
  csTerrainSystem* terrain = new csTerrainSystem (this, renderer, collider,
      dataFeeder);

  terrain->SetMaxLoadedCells (maxLoadedCells);
  terrain->SetVirtualViewDistance (virtualViewDistance);
  terrain->SetAutoPreLoad (autoPreLoad);

  for (size_t i = 0; i < cells.GetSize (); ++i)
  {
    csRef<csTerrainCell> c = cells[i]->CreateCell (terrain);
    terrain->AddCell (c);
  }

  return csPtr<iMeshObject> (terrain);
}

csPtr<iMeshObjectFactory> csTerrainFactory::Clone ()
{
  return csPtr<iMeshObjectFactory> (new csTerrainFactory (*this));
}

void csTerrainFactory::HardTransform (const csReversibleTransform& t)
{
}

bool csTerrainFactory::SupportsHardTransform () const
{
  return false;
}

void csTerrainFactory::SetMeshFactoryWrapper (iMeshFactoryWrapper* lp)
{
  logParent = lp;
}

iMeshFactoryWrapper* csTerrainFactory::GetMeshFactoryWrapper () const
{
  return logParent;
}

iMeshObjectType* csTerrainFactory::GetMeshObjectType () const
{
  return type;
}

iObjectModel* csTerrainFactory::GetObjectModel ()
{
  return 0;
}

bool csTerrainFactory::SetMaterialWrapper (iMaterialWrapper* material)
{
  return false;
}

iMaterialWrapper* csTerrainFactory::GetMaterialWrapper () const
{
  return 0;
}

void csTerrainFactory::SetMixMode (uint mode)
{
}

uint csTerrainFactory::GetMixMode () const
{
  return 0;
}

void csTerrainFactory::SetRenderer (iTerrainRenderer* renderer)
{
  this->renderer = renderer;

  csRef<iTerrainCellRenderProperties> renderProp =
    renderer ? renderer->CreateProperties () : 0;
  defaultCell.SetRenderProperties (renderProp);
}

void csTerrainFactory::SetCollider (iTerrainCollider* collider)
{
  this->collider = collider;

  csRef<iTerrainCellCollisionProperties> collProp =
    collider ? collider->CreateProperties () : 0;
  defaultCell.SetCollisionProperties (collProp);
}

void csTerrainFactory::SetFeeder (iTerrainDataFeeder* feeder)
{
  this->dataFeeder = feeder;

  csRef<iTerrainCellFeederProperties> feederProp =
    dataFeeder ? dataFeeder->CreateProperties () : 0;
  defaultCell.SetFeederProperties (feederProp);
}

iTerrainFactoryCell* csTerrainFactory::GetCell (const char* name)
{
  for (size_t i = 0; i < cells.GetSize(); i++)
  {
    const char* cellName = cells[i]->GetName();
    if (cellName == 0) continue;
    if (strcmp (cellName, name) == 0)
      return cells[i];
  }
  return 0;
}

iTerrainFactoryCell* csTerrainFactory::AddCell(const char* name, 
  int gridWidth, int gridHeight, int materialMapWidth,
  int materialMapHeight, bool materialMapPersistent,
  const csVector2& position, const csVector3& size)
{
  csRef<csTerrainFactoryCell> cell;

  csRef<iTerrainCellRenderProperties> renderProp =
    renderer ? renderer->CreateProperties () : 0;

  csRef<iTerrainCellCollisionProperties> collProp =
    collider ? collider->CreateProperties () : 0;

  csRef<iTerrainCellFeederProperties> feederProp =
    dataFeeder ? dataFeeder->CreateProperties () : 0;

  cell.AttachNew (new csTerrainFactoryCell (name, gridWidth, gridHeight,
    materialMapWidth, materialMapHeight, materialMapPersistent, position, size,
    renderProp, collProp, feederProp));

  cells.Push (cell);

  return cell;
}

iTerrainFactoryCell* csTerrainFactory::AddCell()
{
  csRef<csTerrainFactoryCell> cell;

  cell.AttachNew (new csTerrainFactoryCell (defaultCell));

  cells.Push (cell);

  return cell;
}

void csTerrainFactory::RemoveCell (iTerrainFactoryCell* cell)
{
  cells.Delete (static_cast<csTerrainFactoryCell*>(cell));
}

void csTerrainFactory::SetMaxLoadedCells (size_t value)
{
  maxLoadedCells = value;
}

void csTerrainFactory::SetVirtualViewDistance (float distance)
{
  virtualViewDistance = distance;
}

void csTerrainFactory::SetAutoPreLoad (bool mode)
{
  autoPreLoad = mode; 
}


csTerrainFactoryCell::csTerrainFactoryCell (const char* name, 
  int gridWidth, int gridHeight, int materialMapWidth, int materialMapHeight, 
  bool materialMapPersistent, const csVector2& position, const csVector3& size, 
  iTerrainCellRenderProperties* renderProp, 
  iTerrainCellCollisionProperties* collisionProp, 
  iTerrainCellFeederProperties* feederProp)
  : scfImplementationType (this), name (name), position (position), size (size),
  gridWidth (gridWidth), gridHeight (gridHeight),
  materialMapWidth (materialMapWidth), materialMapHeight (materialMapHeight),
  materialMapPersistent (materialMapPersistent),
  rendererProperties (renderProp), colliderProperties (collisionProp),
  feederProperties (feederProp)
{
}

csTerrainFactoryCell::csTerrainFactoryCell (const csTerrainFactoryCell& other)
  : scfImplementationType (this), name (other.name), position (other.position),
  size (other.size),
  gridWidth (other.gridWidth), gridHeight (other.gridHeight),
  materialMapWidth (other.materialMapWidth),
  materialMapHeight (other.materialMapHeight),
  materialMapPersistent (other.materialMapPersistent),
  baseMaterial (other.baseMaterial),
  splatBaseMaterial (other.splatBaseMaterial),
  alphaSplatMaterial (other.alphaSplatMaterial),
  rendererProperties (other.rendererProperties->Clone()),
  colliderProperties (other.colliderProperties->Clone()),
  feederProperties (other.feederProperties->Clone())
{
}

csPtr<csTerrainCell> csTerrainFactoryCell::CreateCell (csTerrainSystem* terrain)
{
  csTerrainCell* cell;

  csRef<iTerrainCellRenderProperties> renderProp =
    rendererProperties ? rendererProperties->Clone () : 0;

  csRef<iTerrainCellCollisionProperties> collProp =
    colliderProperties ? colliderProperties->Clone () : 0;

  csRef<iTerrainCellFeederProperties> feederProp =
    feederProperties ? feederProperties->Clone () : 0;

  cell = new csTerrainCell (terrain, name.GetData (), gridWidth, gridHeight,
    materialMapWidth, materialMapHeight, materialMapPersistent, position, size,
    renderProp, collProp, feederProp);

  cell->SetBaseMaterial (baseMaterial);
  cell->SetAlphaSplatMaterial (alphaSplatMaterial);
  cell->SetSplatBaseMaterial (splatBaseMaterial);

  return csPtr<csTerrainCell> (cell);
}

iTerrainCellRenderProperties* csTerrainFactoryCell::GetRenderProperties () const
{
  return rendererProperties;
}

iTerrainCellCollisionProperties* csTerrainFactoryCell::GetCollisionProperties () const
{
  return colliderProperties;
}

iTerrainCellFeederProperties* csTerrainFactoryCell::GetFeederProperties () const
{
  return feederProperties;
}

void csTerrainFactoryCell::SetBaseMaterial (iMaterialWrapper* material)
{
  baseMaterial = material;
}

void csTerrainFactoryCell::SetAlphaSplatMaterial (iMaterialWrapper* material)
{
  alphaSplatMaterial = material;
}

void csTerrainFactoryCell::SetSplatBaseMaterial (iMaterialWrapper* material)
{
  splatBaseMaterial = material;
}

}
CS_PLUGIN_NAMESPACE_END(Terrain2)
