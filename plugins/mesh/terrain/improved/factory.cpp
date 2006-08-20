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

#include "cssysdef.h"

#include "factory.h"
#include "terrainsystem.h"

#include "csqsqrt.h"
#include "csgeom/math3d.h"

#include "iterrain/terrainrenderer.h"
#include "iterrain/terraincollider.h"

CS_PLUGIN_NAMESPACE_BEGIN(ImprovedTerrain)
{

csTerrainFactory::csTerrainFactory (iMeshObjectType *pParent)
  : scfImplementationType (this, pParent)
{
  terrain = new csTerrainSystem(this);
  type = pParent;

  renderer = NULL;
  collider = NULL;
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
  csPtr<iMeshObject> ptr = terrain;
  terrain = 0;
  return ptr;
}

csPtr<iMeshObjectFactory> csTerrainFactory::Clone ()
{
  return 0;
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
  logparent = lp;
}

iMeshFactoryWrapper* csTerrainFactory::GetMeshFactoryWrapper () const
{
  return logparent;
}

iMeshObjectType* csTerrainFactory::GetMeshObjectType () const
{
  return type;
}

iObjectModel* csTerrainFactory::GetObjectModel ()
{
  return NULL;
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
  terrain->SetRenderer (renderer);
}

void csTerrainFactory::SetCollider (iTerrainCollider* collider)
{
  this->collider = collider;
  terrain->SetCollider (collider);
}

iTerrainCell* csTerrainFactory::AddCell(const char* name, int grid_width,
int grid_height, int material_width, int material_height,
const csVector2& position, const csVector3& size, iTerrainDataFeeder* feeder)
{
  csRef<iTerrainCellRenderProperties> render_properties =
  renderer->CreateProperties ();
  csRef<iTerrainCellCollisionProperties> collision_properties =
  collider ? collider->CreateProperties () : 0;
  
  csRef<csTerrainCell> cell;
  cell.AttachNew (new csTerrainCell(terrain, name, grid_width, grid_height,
  material_width, material_height, position, size, feeder, render_properties,
  collision_properties, renderer, collider));
                                     
  terrain->AddCell (cell);

  return cell;
}

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)
