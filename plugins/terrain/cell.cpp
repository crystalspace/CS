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

#include "cell.h"

#include "csgeom/vector3.h"
#include "csgeom/csrect.h"

CS_PLUGIN_NAMESPACE_BEGIN(ImprovedTerrain)
{

SCF_IMPLEMENT_FACTORY (csTerrainCell)

csTerrainCell::csTerrainCell (iBase* parent)
  : scfImplementationType (this, parent)
{
}

csTerrainCell::csTerrainCell (iBase* parent, const char* name, int grid_width, int grid_height, int material_width, int material_height,
                              const csVector2& position, const csVector2& size, iTerrainDataFeeder* feeder,
                              iTerrainCellRenderProperties* render_properties, iTerrainCellCollisionProperties* collision_properties)
  : scfImplementationType (this, parent)
{
  this->name = name;
  
  this->grid_width = grid_width;
  this->grid_height = grid_height;
  
  this->material_width = material_width;
  this->material_height = material_height;
  
  this->position = position;
  this->size = size;
  
  this->feeder = feeder;
  
  this->render_properties = render_properties;
  this->collision_properties = collision_properties;

  state = NotLoaded;
}

csTerrainCell::~csTerrainCell ()
{
}

csTerrainCell::LoadState csTerrainCell::GetLoadState() const
{
  return state;
}

void csTerrainCell::PreLoad()
{
  heightmap.SetSize(grid_width * grid_height, 0);

  feeder->PreLoad(this);

  state = PreLoaded;
}

void csTerrainCell::Load()
{
  heightmap.SetSize(grid_width * grid_height, 0);

  feeder->Load(this);

  state = Loaded;
}

void csTerrainCell::Unload()
{
  heightmap.SetSize(0);

  state = NotLoaded;
}

csBox3 csTerrainCell::GetBBox()
{
  csBox3 box;
  box.Set(position.x, position.y, -100, position.x + size.x, position.y + size.y, 100);

  return box;
}

iTerrainDataFeeder* csTerrainCell::GetDataFeeder()
{
  return feeder;
}

const char* csTerrainCell::GetName()
{
  return name.GetData();
}

iTerrainCellRenderProperties* csTerrainCell::GetRenderProperties()
{
  return render_properties;
}

iTerrainCellCollisionProperties* csTerrainCell::GetCollisionProperties()
{
  return collision_properties;
}

int csTerrainCell::GetGridWidth()
{
  return grid_width;
}

int csTerrainCell::GetGridHeight()
{
  return grid_height;
}

csLockedHeightData csTerrainCell::LockHeightData(const csRect& rectangle)
{
  csLockedHeightData data;

  data.data = heightmap.GetArray() + rectangle.ymin * grid_width + rectangle.xmin;
  data.pitch = grid_width;

  return data;
}

void csTerrainCell::UnlockHeightData()
{
}

const csVector2& csTerrainCell::GetPosition()
{
  return position;
}

const csVector2& csTerrainCell::GetSize()
{
  return size;
}

const csArray<iMaterialWrapper*>& csTerrainCell::GetMaterialPalette()
{
  throw 1;
}

void csTerrainCell::SetMaterialPalette(const csArray<iMaterialWrapper*>& array)
{
}

int csTerrainCell::GetMaterialMapWidth()
{
  return material_width;
}

int csTerrainCell::GetMaterialMapHeight()
{
  return material_height;
}

csLockedMaterialMap csTerrainCell::LockMaterialMap(const csRect& rectangle)
{
  csLockedMaterialMap data;

  data.data = NULL;
  data.pitch = material_width;

  return data;
}

void csTerrainCell::UnlockMaterialMap()
{
}

void csTerrainCell::SetMaterialMask(int material, iImage* image)
{
}

void csTerrainCell::SetMaterialMask(int material, const csArray<char>& data, int width, int height)
{
}

bool csTerrainCell::CollideRay(const csVector3& start, const csVector3& end, bool oneHit, csArray<csVector3>& points)
{
  return false;
}

bool csTerrainCell::CollideSegment(const csVector3& start, const csVector3& end, bool oneHit, csArray<csVector3>& points)
{
  return false;
}

float csTerrainCell::GetHeight(const csVector2& pos)
{
  return 0;
}

csVector3 csTerrainCell::GetTangent(const csVector2& pos)
{
  return csVector3(1, 0, 0);
}

csVector3 csTerrainCell::GetBinormal(const csVector2& pos)
{
  return csVector3(1, 0, 0);
}

csVector3 csTerrainCell::GetNormal(const csVector2& pos)
{
  return csVector3(1, 0, 0);
}

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)
