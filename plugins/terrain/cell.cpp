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

#include "iterrain/terrainrenderer.h"

#include "csgeom/vector3.h"
#include "csgeom/csrect.h"

CS_PLUGIN_NAMESPACE_BEGIN(ImprovedTerrain)
{

csTerrainCell::csTerrainCell (iBase* parent, const char* name, int grid_width, int grid_height, int material_width, int material_height,
                              const csVector2& position, const csVector2& size, iTerrainDataFeeder* feeder,
                              iTerrainCellRenderProperties* render_properties, iTerrainCellCollisionProperties* collision_properties,
                              iTerrainRenderer* renderer)
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

  this->renderer = renderer;

  state = NotLoaded;

  render_data = NULL;

  min_height = 0;
  max_height = 33;
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
  heightmap.SetSize(grid_width * grid_height);

  feeder->Load(this);

  state = Loaded;
}

void csTerrainCell::Unload()
{
  heightmap.DeleteAll();

  state = NotLoaded;
}

csBox3 csTerrainCell::GetBBox()
{
  csBox3 box;
  box.Set(position.x, min_height - EPSILON, position.y, position.x + size.x, max_height + EPSILON, position.y + size.y);

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

csRefCount* csTerrainCell::GetRenderData()
{
  return render_data;
}

void csTerrainCell::SetRenderData(csRefCount* data)
{
  render_data = data;
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
  locked_height_rect = rectangle;

  locked_height_data.data = heightmap.GetArray() + rectangle.ymin * grid_width + rectangle.xmin;
  locked_height_data.pitch = grid_width;

  return locked_height_data;
}

void csTerrainCell::UnlockHeightData()
{
  renderer->OnHeightUpdate(this, locked_height_rect, heightmap.GetArray(), grid_width);
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

static inline float Lerp(const float x, const float y, const float t)
{
  return x + (y - x) * t;
}

void csTerrainCell::LerpHelper(const csVector2& pos, int& x1, int& x2, float& xfrac, int& y1, int& y2, float& yfrac)
{
  float x = (pos.x / size.x) * (grid_width - 1);
  float y = (pos.y / size.y) * (grid_height - 1);

  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x > grid_width - 1) x = grid_width - 1;
  if (y > grid_height - 1) y = grid_height - 1;

  x1 = floor(x);
  x2 = ceil(x);
  xfrac = x - x1;

  y1 = floor(y);
  y2 = ceil(y);
  yfrac = y - y1;
}

float csTerrainCell::GetHeight(int x, int y)
{
  return heightmap[y * grid_width + x];
}

float csTerrainCell::GetHeight(const csVector2& pos)
{
  int x1, y1, x2, y2;
  float xfrac, yfrac;

  LerpHelper(pos, x1, x2, xfrac, y1, y2, yfrac);

  float h1 = Lerp(heightmap[y1 * grid_width + x1], heightmap[y1 * grid_width + x2], xfrac);
  float h2 = Lerp(heightmap[y2 * grid_width + x1], heightmap[y2 * grid_width + x2], xfrac);

  return Lerp(h1, h2, yfrac);
}

static inline csVector3 Lerp(const csVector3& x, const csVector3& y, const float t)
{
  return x + (y - x) * t;
}

csVector3 csTerrainCell::GetTangent(int x, int y)
{
  float center = heightmap[y * grid_width + x];
  float left = x == 0 ? center : heightmap[y * grid_width + x - 1];
  float right = x + 1 == grid_width ? center : heightmap[y * grid_width + x + 1];

  return csVector3(1.0f / grid_width, right - left, 0);
}

csVector3 csTerrainCell::GetTangent(const csVector2& pos)
{
  int x1, y1, x2, y2;
  float xfrac, yfrac;

  LerpHelper(pos, x1, x2, xfrac, y1, y2, yfrac);

  csVector3 n1 = Lerp(GetTangent(x1, y1), GetTangent(x2, y1), xfrac);
  csVector3 n2 = Lerp(GetTangent(x1, y2), GetTangent(x2, y2), xfrac);

  return Lerp(n1, n2, yfrac).Unit();
}

csVector3 csTerrainCell::GetBinormal(int x, int y)
{
  float center = heightmap[y * grid_width + x];
  float up = y == 0 ? center : heightmap[(y - 1) * grid_width + x];
  float down = y + 1 == grid_height ? center : heightmap[(y + 1) * grid_width + x];

  return csVector3(0, down - up, 1.0f / grid_height);
}

csVector3 csTerrainCell::GetBinormal(const csVector2& pos)
{
  int x1, y1, x2, y2;
  float xfrac, yfrac;

  LerpHelper(pos, x1, x2, xfrac, y1, y2, yfrac);

  csVector3 n1 = Lerp(GetBinormal(x1, y1), GetBinormal(x2, y1), xfrac);
  csVector3 n2 = Lerp(GetBinormal(x1, y2), GetBinormal(x2, y2), xfrac);

  return Lerp(n1, n2, yfrac).Unit();
}

csVector3 csTerrainCell::GetNormal(int x, int y)
{
  float center = heightmap[y * grid_width + x];
  float up = y == 0 ? center : heightmap[(y - 1) * grid_width + x];
  float down = y + 1 == grid_height ? center : heightmap[(y + 1) * grid_width + x];
  float left = x == 0 ? center : heightmap[y * grid_width + x - 1];
  float right = x + 1 == grid_width ? center : heightmap[y * grid_width + x + 1];

  return csVector3((center - left) + (center - right), 1, (center - up) + (center - down));
}

csVector3 csTerrainCell::GetNormal(const csVector2& pos)
{
  int x1, y1, x2, y2;
  float xfrac, yfrac;

  LerpHelper(pos, x1, x2, xfrac, y1, y2, yfrac);

  csVector3 n1 = Lerp(GetNormal(x1, y1), GetNormal(x2, y1), xfrac);
  csVector3 n2 = Lerp(GetNormal(x1, y2), GetNormal(x2, y2), xfrac);

  return Lerp(n1, n2, yfrac).Unit();
}

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)
