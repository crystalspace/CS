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

#include "builder.h"
#include "terrainsystem.h"

#include "iterrain/terrainrenderer.h"
#include "iterrain/terraincollider.h"

CS_PLUGIN_NAMESPACE_BEGIN(ImprovedTerrain)
{

SCF_IMPLEMENT_FACTORY (csTerrainBuilder)

csTerrainBuilder::csTerrainBuilder (iBase* parent)
  : scfImplementationType (this, parent)
{
  terrain = new csTerrainSystem(NULL);
}

csTerrainBuilder::~csTerrainBuilder ()
{
}

void csTerrainBuilder::SetRenderer(iTerrainRenderer* renderer)
{
  terrain->SetRenderer(renderer);
}

void csTerrainBuilder::SetCollider(iTerrainCollider* collider)
{
  terrain->SetCollider(collider);
}

void csTerrainBuilder::AddCell(const char* name, int grid_width, int grid_height, int material_width, int material_height, const csVector2& position, const csVector2& size, iTerrainDataFeeder* feeder)
{
  csRef<iTerrainCellRenderProperties> render_properties = terrain->GetRenderer()->CreateProperties();
  csRef<iTerrainCellCollisionProperties> collision_properties = terrain->GetCollider()->CreateProperties();
  
  csRef<csTerrainCell> cell;
  cell.AttachNew(new csTerrainCell(NULL, name, grid_width, grid_height, material_width, material_height, position, size, feeder, render_properties, collision_properties));
                                     
  terrain->AddCell(cell);
}

csPtr<iTerrainSystem> csTerrainBuilder::BuildTerrain()
{
  csPtr<iTerrainSystem> ptr = terrain;
  terrain = 0;
  return ptr;
}

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)
